#include "inline_layout/utils.hpp"

#include "elements/aligned.hpp"
#include "elements/sized.hpp"

namespace aardvark::inline_layout {

float measure_text_width(
    const UnicodeString& text,
    const SkPaint& paint,
    std::optional<int> num_chars) {
    auto byte_length =
        (num_chars == std::nullopt ? text.length() : num_chars.value()) * 2;
    return paint.measureText(text.getBuffer(), byte_length);
};

int break_text(
    const UnicodeString& text,
    const SkPaint& paint,
    float max_width,
    float* measured_width) {
    auto fit_bytes = paint.breakText(
        text.getBuffer(), text.length() * 2, max_width, measured_width);
    return text.countChar32(0, fit_bytes / 2);
};

std::string icu_to_std_string(const UnicodeString& text) {
    std::string std_string;
    text.toUTF8String(std_string);
    return std_string;
};

LineMetrics calc_combined_metrics(
    const std::vector<std::shared_ptr<Span>>& spans,
    const LineMetrics& default_metrics) {
    float max_ascent = default_metrics.baseline;
    float max_descent = default_metrics.height - default_metrics.baseline;
    for (auto& span : spans) {
        if (span->metrics.baseline > max_ascent) {
            max_ascent = span->metrics.baseline;
        }
        auto descent = span->metrics.height - span->metrics.baseline;
        if (descent > max_descent) max_descent = descent;
    }
    return LineMetrics{
        max_ascent + max_descent,  // height
        max_ascent,                // baseline
        default_metrics.x_height   // x_height
    };
}

void render_spans(
    const std::vector<std::shared_ptr<Span>>& spans,
    const LineMetrics& metrics,
    const Position& offset,
    std::vector<std::shared_ptr<Element>>* container,
    Element* parent) {
    auto current_width = 0.0f;
    for (auto& span : spans) {
        auto elem = span->render();
        auto sizeConstraints = SizeConstraints{
            Value::abs(span->width), Value::abs(span->metrics.height)};
        auto align = Alignment::top_left(
            Value::abs(span->vert_align(metrics, span->metrics) + offset.top),
            Value::abs(current_width + offset.left));
        auto aligned = std::make_shared<AlignedElement>(
            std::make_shared<SizedElement>(elem, sizeConstraints), align);
        aligned->parent = parent;
        container->emplace_back(aligned);
        current_width += span->width;
    }
}

SkPaint make_default_paint() {
    SkPaint paint;
    paint.setColor(SK_ColorBLACK);
    paint.setTextSize(16);
    paint.setAntiAlias(true);
    paint.setTextEncoding(SkPaint::kUTF16_TextEncoding);
    return paint;
}

}  // namespace aardvark::inline_layout
