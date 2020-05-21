#include "inline_layout/decoration_span.hpp"

#include "elements/translated.hpp"
#include "elements/overflow.hpp"

namespace aardvark::inline_layout {

Decoration Decoration::left() {
    auto res = *this;
    if (borders != std::nullopt) res.borders.value().right = BorderSide::none();
    if (padding != std::nullopt) res.padding.value().right = 0;
    return res;
}

Decoration Decoration::right() {
    auto res = *this;
    if (borders != std::nullopt) res.borders.value().left = BorderSide::none();
    if (padding != std::nullopt) res.padding.value().left = 0;
    return res;
}

std::pair<float, float> Decoration::get_paddings(float total_line_width) {
    auto before = 0;
    auto after = 0;
    if (padding != std::nullopt) {
        before += padding.value().left;
        after += padding.value().right;
    }
    if (borders != std::nullopt) {
        before += borders.value().left.width;
        after += borders.value().right.width;
    }
    return std::make_pair(before, after);
}

DecorationSpan::DecorationSpan(
    std::vector<std::shared_ptr<Span>> children,
    Decoration decoration,
    std::optional<SpanBase> base_span)
    : MultipleChildrenSpan(std::move(children), base_span),
      decoration(decoration){};

InlineLayoutResult DecorationSpan::layout(InlineConstraints constraints) {
    std::vector<std::shared_ptr<Span>> fit_spans;
    std::vector<std::shared_ptr<Span>> remaining_spans;
    auto fit_spans_width = 0;
    auto reached_end = false;
    auto paddings = decoration.get_paddings(constraints.total_line_width);
    for (auto& span : children) {
        if (reached_end) {
            remaining_spans.push_back(span);
            continue;
        }

        auto padding_before =
            span == children.front() ? std::get<0>(paddings) : 0;
        auto padding_after =
            span == children.back() ? std::get<1>(paddings) : 0;
        auto span_constraints =
            InlineConstraints{constraints.remaining_line_width -
                                  fit_spans_width,  // remaining_line_width
                              constraints.total_line_width,  // total_line_width
                              padding_before,
                              padding_after};
        auto result = span->layout(span_constraints);
        if (result.fit_span != std::nullopt) {
            auto fit_span = result.fit_span.value();
            fit_span->metrics = result.metrics;
            fit_span->width = result.width;
            fit_spans.push_back(fit_span);
            fit_spans_width +=
                padding_before + result.width +
                (result.type == InlineLayoutResult::Type::fit ? padding_after
                                                              : 0);
        }
        if (result.remainder_span != std::nullopt) {
            reached_end = true;
            remaining_spans.push_back(result.remainder_span.value());
        }
    }

    if (remaining_spans.size() == 0) {
        // TODO use default_metrics?
        auto fit_span_metrics =
            calc_combined_metrics(fit_spans, LineMetrics{0, 0, 0});
        return InlineLayoutResult::fit(
            fit_spans_width, fit_span_metrics, shared_from_this());
    } else if (fit_spans.size() == 0) {
        return InlineLayoutResult::wrap(shared_from_this());
    } else {
        auto fit_span = std::make_shared<DecorationSpan>(
            fit_spans,          // children
            decoration.left(),  // decoration
            SpanBase{this, 0}   // base_span
        );
        // TODO use default_metrics?
        auto fit_span_metrics =
            calc_combined_metrics(fit_spans, LineMetrics{0, 0, 0});
        auto remainder_span = std::make_shared<DecorationSpan>(
            remaining_spans,                                    // children
            decoration.right(),                                 // decoration
            SpanBase{this, static_cast<int>(fit_spans.size())}  // base_span
        );
        return InlineLayoutResult::split(
            constraints.remaining_line_width,  // width
            fit_span_metrics,                  // metrics
            fit_span,                          // fit_span
            remainder_span                     // remainder_span
        );
    }
}

std::shared_ptr<Element> DecorationSpan::render() {
    auto stack = std::make_shared<StackElement>();
    render_spans(
        children, metrics, Position{0, 0}, &stack->children, stack.get());

    std::shared_ptr<Element> container = stack;
    auto top_offset = 0;
    if (decoration.padding != std::nullopt) {
        auto padding = decoration.padding.value();
        container = std::make_shared<PaddedElement>(container, padding);
        top_offset += padding.top;
    }
    if (decoration.background != std::nullopt) {
        auto background =
            std::make_shared<BackgroundElement>(decoration.background.value());
        container = std::make_shared<StackElement>(
            std::vector<std::shared_ptr<Element>>{background, container});
    }
    if (decoration.borders != std::nullopt) {
        auto borders = decoration.borders.value();
        container = std::make_shared<OverflowElement>(
            std::make_shared<BorderElement>(
                container, borders, BoxRadiuses::all(Radius{0, 0})),
            OverflowConstraint::original,  // max_width
            OverflowConstraint::sized(
                metrics.height + borders.height())  // max_height
        );
        top_offset += borders.top.width;
    }
    if (top_offset > 0) {
        container = std::make_shared<TranslatedElement>(
            container, Translation{Value::abs(0), Value::abs(-top_offset)});
    }
    return container;
}

UnicodeString DecorationSpan::get_text() {
    auto text = UnicodeString();
    for (auto& span : children) text.append(span->get_text());
    return text;
}

int DecorationSpan::get_text_length() {
    auto length = 0;
    for (auto& span : children) length += span->get_text_length();
    return length;
}

std::shared_ptr<Span> DecorationSpan::slice(int start, int end) {
    auto slice_children = std::vector<std::shared_ptr<Span>>();
    auto span_start = 0;
    auto span_end = -1;
    for (auto& span : children) {
        auto span_len = span->get_text_length();
        span_start = span_end + 1;
        span_end = span_start + span_len - 1;
        if (start <= span_start && span_end <= end) {
            slice_children.push_back(span);
        } else if (span_start <= start || end <= span_end) {
            auto split_start = std::max(start - span_start, 0);
            auto split_end = std::min(end - span_start, span_len - 1);
            slice_children.push_back(span->slice(split_start, split_end));
        }
        if (end <= span_end) break;
    }
    auto slice_decoration = decoration;
    auto include_start = start == 0;
    auto include_end = end == get_text_length();
    if (include_start && !include_end) {
        slice_decoration = decoration.left();
    } else if (!include_start && include_end) {
        slice_decoration = decoration.right();
    } else if (!include_start && !include_end) {
        slice_decoration = Decoration{
            decoration.background,  // background
            std::nullopt,           // borders
            std::nullopt            // insets
        };
    }
    return std::make_shared<DecorationSpan>(
        slice_children,
        slice_decoration,
        SpanBase{this, base_span.prev_offset + start});
}

int DecorationSpan::get_text_offset_at_position(int position) {
    // TODO
    return 0;
}

}  // namespace aardvark::inline_layout
