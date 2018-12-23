#include "gesture_responder.hpp"

namespace aardvark::elements {

GestureResponder::GestureResponder(std::shared_ptr<Element> child,
                                   bool is_repaint_boundary)
    : SingleChildElement(child, is_repaint_boundary,
                         /* size_depends_on_parent */ true){};

Size GestureResponder::layout(BoxConstraints constraints) {
  auto child_size =
      document->layout_element(child.get(), constraints.make_loose());
  child->size = child_size;
  child->rel_position = Position{0, 0};
  return constraints.max_size();
};

void GestureResponder::paint(bool is_changed) {
  document->paint_element(child.get());
};

}  // namespace aardvark::elements
