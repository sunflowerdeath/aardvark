#include "Catch2/catch.hpp"
#include "../base_types.hpp"
#include "../inline_layout/text_span.hpp"
#include <iostream>

using namespace aardvark;

TEST_CASE("inline_layout::TextSpan", "[inline_layout] [text_span]" ) {
    auto text = UnicodeString((UChar*)u"Hello, World!");
    SkPaint paint;
    auto span = inline_layout::TextSpan(text, paint);
    auto text_width = inline_layout::measure_text(text, paint);
    auto hello = UnicodeString((UChar*)u"Hello, ");
    auto world = UnicodeString((UChar*)u"World!");
    auto hello_width = inline_layout::measure_text(hello, paint);
    auto world_width = inline_layout::measure_text(world, paint);

    SECTION("fits") {
        auto constraints = inline_layout::InlineConstraints{
            1000,  // remaining_width
            1000,  // total_width
            0,     // padding_before
            0      // padding_after
        };
        auto result = span.layout(constraints);
        REQUIRE(result.fit_span == &span);
        REQUIRE(result.size.width == text_width);
        REQUIRE(result.remainder_span == std::nullopt);
    }

    SECTION("does not fit") {
        auto constraints = inline_layout::InlineConstraints{
            0,     // remaining_width
            1000,  // total_width
            0,     // padding_before
            0      // padding_after
        };
        auto result = span.layout(constraints);
        REQUIRE(result.fit_span == std::nullopt);
        REQUIRE(result.remainder_span == &span);
    }

    SECTION("splits") {
        auto constraints = inline_layout::InlineConstraints{
            hello_width,     // remaining_width
            1000,            // total_width
            0,               // padding_before
            0                // padding_after
        };
        auto result = span.layout(constraints);
        auto fit_span = (inline_layout::TextSpan*)result.fit_span.value();
        auto remainder_span =
            (inline_layout::TextSpan*)result.remainder_span.value();
        REQUIRE(fit_span->text == hello);
        REQUIRE(result.size.width == hello_width);
        REQUIRE(remainder_span->text == world);
    }
}
