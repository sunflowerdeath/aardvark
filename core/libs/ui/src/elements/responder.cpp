#include "elements/responder.hpp"

namespace aardvark {

Size ResponderElement::layout(BoxConstraints constraints) {
    auto child_size =
        document->layout_element(child.get(), constraints.make_loose());
    child->size = child_size;
    child->rel_position = Position{0, 0};
    return child_size;
};

void ResponderElement::paint(bool is_changed) {
    document->paint_element(child.get());
};

}  // namespace aardvark
