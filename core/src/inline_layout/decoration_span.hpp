#pragma once

#include <optional>
#include <memory>
#include "../element.hpp"
#include "../elements/border.hpp"
#include "../elements/background.hpp"
#include "inline_layout.hpp"

namespace aardvark::inline_layout {

struct Decoration {
    std::optional<SkColor> background;
    std::optional<BoxBorders> borders;
    std::optional<Alignment> padding;
};

class DecorationSpan : public Span {
  public:
    DecorationSpan(std::vector<std::shared_ptr<Span>> content,
                   Decoration decoration,
                   std::optional<SpanBase> base_span = std::nullopt);
    InlineLayoutResult layout(InlineConstraints constraints) override;
    std::shared_ptr<Element> render(
        std::optional<SpanSelection> selection) override;
    std::vector<std::shared_ptr<Span>> content;
    Decoration decoration;
  private:
    std::unique_ptr<DecorationSpan> fit_span;
    std::unique_ptr<DecorationSpan> remainder_span;
};

}  // namespace aardvark::inline_layout

