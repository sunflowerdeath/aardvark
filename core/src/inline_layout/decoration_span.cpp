#include "decoration_span.hpp"

namespace aardvark::inline_layout {

Decoration Decoration::left(); {
    return Decoration{
        background,
        BoxBorders{borders.left, borders.top, BorderSide::none(),
                   borders.bottom},
        EdgeInsets{insets.left, insets.top, Value::none(), insets.bottom}};
};

Decoration Decoration::right() {
    return Decoration{
        background,
        BoxBorders{BorderSide::none(), borders.top, borders.right,
                   borders.bottom},
        EdgeInsets{Value::none(), insets.top, insets.right, insets.bottom}};
};

DecorationSpan::DecorationSpan(std::vector<std::shared_ptr<Span>> content,
                               Decoration decoration,
                               std::optional<SpanBase> base_span)
    : content(content), decoration(decoration), Span(base_span){};

InlineLayoutResult DecorationSpan::layout(InlineConstraints constraints) {
    std::vector<Span*> fit_spans;
    std::vector<Span*> remaining_spans;
    auto reached_end = false;
    for (auto& span : content) {
        if (reached_end) {
            remaining_spans.push_back(span);
            continue;
        }

        auto result = span->layout(constraints);
        if (result.fit_span != std::nullopt) {
            fit_spans.push_back(span);
            fit_span->metrics = result.metrics;
            fit_span->width = result.width;
            constraints.remaining_width -= result.width;
        }
        if (result.remainder_span != std::nullopt) {
            reached_end = false;
            remaining_spans.push_back(span);
        }
    }

    if (remaining_spans.size() == 0) {
        // all fit
        fit_span.reset();
        remainder_span.reset();
        return InlineLayoutResult{
            this,           // fit_span
            0,              // width
            LineMetrics(),  // metrics
            std::nullopt    // remainder_span
        };
    } else if (fit_spans.size() == 0) {
        // nothing fit
        fit_span.reset();
        remainder_span.reset();
        return InlineLayoutResult{
            std::nullopt,   // fit_span
            0,              // width
            LineMetrics(),  // metrics
            this            // remainder_span
        };
    } else {
        // split
        fit_span = std::make_unique<DecorationSpan>(
            fit_spans, decoration.left(), SpanBase{this, 0});
        remainder_span = std::make_unique<DecorationSpan>(
            remaining_spans, decoration.right(),
            SpanBase{this, fit_spans.size()});
        return InlineLayoutResult{
            fit_span.get(),       // fit_span
            0,                    // width
            LineMetrics(),        // metrics
            remainder_span.get()  // remainder_span
        };
    }
};

std::shared_ptr<Element> DecorationSpan::render(
    std::optional<SpanSelection> selection) {
    auto container = std::make_shared<elements::Stack>();
    if (decoration.background != std::nullopt) {
        auto background = std::make_shared<elements::Background>(
            decoration.background.value());
        container->children.push_back(background);
    }
    auto line = Line(content);
    auto height = line.render_to(container);
    auto align = 0;
    if (decoration.insets != std::nullopt) {
        auto insets = decoration.insets.value();
        container = std::make_shared<elements::Align>(container, insets);
        align += insets.top.calc(height);
    }
    if (decoration.borders != std::nullopt) {
        auto borders = decoration.borders.value();
        container = std::make_shared<elements::Border>(container, borders);
        align += borders.top.height;
    }
    if (align > 0) {
        container =
            std::make_shared<elements::Align>(container, EdgeInsets{0, -align});
    }
    return container;
};

}  // namespace aardvark::inline_layout
