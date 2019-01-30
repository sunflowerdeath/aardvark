#include "Catch2/catch.hpp"
#include "../base_types.hpp"
#include "../inline_layout/text_span.hpp"

using namespace aardvark;

TEST_CASE("inline_layout::TextSpan", "[inline_layout] [text_span]" ) {
    auto text = UnicodeString((UChar*)u"Hello, World!");
    SkPaint paint;
    paint.setTextEncoding(SkPaint::kUTF16_TextEncoding);
    auto span = std::make_shared<inline_layout::TextSpan>(text, paint);
    auto text_width = inline_layout::measure_text_width(text, paint);
    auto hello = UnicodeString((UChar*)u"Hello, ");
    auto world = UnicodeString((UChar*)u"World!");
    auto hello_width = inline_layout::measure_text_width(hello, paint);
    auto world_width = inline_layout::measure_text_width(world, paint);

    SECTION("linebreak normal: fit") {
        auto constraints = inline_layout::InlineConstraints{
            1000,  // remaining_width
            1000,  // total_width
            0,     // padding_before
            0      // padding_after
        };
        auto result = inline_layout::Span::layout(span, constraints);
        REQUIRE(result.fit_span.value() == span);
        REQUIRE(result.width == text_width);
        REQUIRE(result.remainder_span == std::nullopt);
    }

    SECTION("linebreak normal: wrap") {
        auto constraints = inline_layout::InlineConstraints{
            0,     // remaining_width
            1000,  // total_width
            0,     // padding_before
            0      // padding_after
        };
        auto result = inline_layout::Span::layout(span, constraints);
        REQUIRE(result.fit_span == std::nullopt);
        REQUIRE(result.remainder_span.value() == span);
    }

    SECTION("linebreak normal: split") {
        auto constraints = inline_layout::InlineConstraints{
            hello_width,     // remaining_width
            1000,            // total_width
            0,               // padding_before
            0                // padding_after
        };
        auto result = inline_layout::Span::layout(span, constraints);
        auto fit_span = (inline_layout::TextSpan*)result.fit_span.value().get();
        auto remainder_span =
            (inline_layout::TextSpan*)result.remainder_span.value().get();
        REQUIRE(fit_span->text == hello);
        REQUIRE(result.width == hello_width);
        REQUIRE(remainder_span->text == world);
    }

    SECTION("linebreak normal: fit when cannot split") {
        auto hello_span =
            std::make_shared<inline_layout::TextSpan>(hello, paint);
        auto constraints = inline_layout::InlineConstraints{
            hello_width - 10,  // remaining_width
            hello_width - 10,  // total_width
            0,                 // padding_before
            0                  // padding_after
        };
        auto result = inline_layout::Span::layout(hello_span, constraints);
        REQUIRE(result.fit_span.value() == hello_span);
        REQUIRE(result.remainder_span == std::nullopt);
    }

    /*
    SECTION("padding") {
        auto alpha_beta_gamma = UnicodeString((UChar*)u"alpha beta gamma");
        auto alpha = UnicodeString((UChar*)u"alpha ");
        auto beta = UnicodeString((UChar*)u"beta ");
        auto gamma = UnicodeString((UChar*)u"gamma");

        // alpha + beta < size < padding_before + alpha + beta
    }
    */

    SECTION("linebreak never") {
        auto span = std::make_shared<inline_layout::TextSpan>(
            text, paint, inline_layout::LineBreak::never);

        // Wrap if not at the start of the line
        auto wrap_constraints = inline_layout::InlineConstraints{
            hello_width - 1,  // remaining_width
            1000,             // total_width
            0,                // padding_before
            0                 // padding_after
        };
        auto result = inline_layout::Span::layout(span, wrap_constraints);
        REQUIRE(result.type == inline_layout::InlineLayoutResult::Type::wrap);

        auto fit_constraints = inline_layout::InlineConstraints{
            hello_width - 1,  // remaining_width
            hello_width - 1,  // total_width
            0,                // padding_before
            0                 // padding_after
        };
        result = inline_layout::Span::layout(span, fit_constraints);
        REQUIRE(result.type == inline_layout::InlineLayoutResult::Type::fit);
    }

    SECTION("linebreak anywhere") {
        auto hell = UnicodeString((UChar*)u"Hell");
        auto hell_width = inline_layout::measure_text_width(hell, paint);
        auto span = std::make_shared<inline_layout::TextSpan>(
            hello, paint, inline_layout::LineBreak::anywhere);
        auto constraints = inline_layout::InlineConstraints{
            hell_width,  // remaining_width
            hell_width,  // total_width
            0,           // padding_before
            0            // padding_after
        };
        auto result = inline_layout::Span::layout(span, constraints);
        REQUIRE(result.type == inline_layout::InlineLayoutResult::Type::split);
        auto fit_span = dynamic_cast<inline_layout::TextSpan*>(
            result.fit_span.value().get());
        REQUIRE(fit_span->text == hell);
        auto remainder_span = dynamic_cast<inline_layout::TextSpan*>(
            result.remainder_span.value().get());
        auto remaining_text = UnicodeString((UChar*)u"o, ");
        REQUIRE(remainder_span->text == remaining_text);
    }
}
