#include "elements/padded.hpp"

namespace aardvark {

float PaddedElement::get_intrinsic_height(float width) {
    return padding.height() +
           child->query_intrinsic_height(width - padding.width());
}

float PaddedElement::get_intrinsic_width(float height) {
    return padding.width() +
           child->query_intrinsic_width(height - padding.height());
}

Size PaddedElement::layout(BoxConstraints constraints) {
    auto horiz = padding.left + padding.right;
    auto vert = padding.top + padding.bottom;
    auto child_constraints = BoxConstraints{
        constraints.min_width,          // min_width
        constraints.max_width - horiz,  // max_width
        constraints.min_height,         // min_height
        constraints.max_height - vert   // max_height
    };
    auto child_size = document->layout_element(child.get(), child_constraints);
    child->size = child_size;
    child->rel_position = Position{padding.left, padding.top};
    return Size{child_size.width + horiz, child_size.height + vert};
}

}  // namespace aardvark
