#include "align.hpp"

namespace aardvark::elements {

Align::Align(std::shared_ptr<Element> child, EdgeInsets insets,
             bool adjust_child, bool is_repaint_boundary)
    : SingleChildElement(child, is_repaint_boundary,
                         /* size_depends_on_parent */ true),
      adjust_child(adjust_child),
      insets(insets){};

Size Align::layout(BoxConstraints constraints) {
    auto left = insets.left.calc(constraints.max_width);
    auto right = insets.right.calc(constraints.max_width);
    auto top = insets.top.calc(constraints.max_height);
    auto bottom = insets.bottom.calc(constraints.max_height);
    auto horiz = left + right;
    auto vert = top + bottom;

    auto child_constraints = BoxConstraints{
        0,  // min_width
        adjust_child ? (constraints.max_width - horiz)
                     : constraints.max_width,  // max_width
        0,                                     // min_height
        adjust_child ? (constraints.max_height - vert)
                     : constraints.max_height,  // max_height
    };
    auto size = document->layout_element(child.get(), child_constraints);
    child->size = size;
    child->rel_position = Position{
        insets.left.is_none() ? (constraints.max_width - horiz - size.width)
                              : left,  // left
        insets.top.is_none() ? (constraints.max_height - vert - size.height)
                             : top  // top
    };
    return constraints.max_size();
};

void Align::paint(bool is_changed) { document->paint_element(child.get()); };

bool Align::hit_test(double left, double top) {
    return child->hit_test(left + child->rel_position.left,
                           top + child->rel_position.top);
};

}  // namespace aardvark::elements
