#pragma once

#include "../element.hpp"

namespace aardvark::elements {

class IntrinsicHeight : public SingleChildElement {
  public:
    IntrinsicHeight()
        : SingleChildElement(/* child */ nullptr,
                             /* is_repaint_boundary */ false,
                             /* size_depends_on_parent */ false){};

    IntrinsicHeight(std::shared_ptr<Element> child,
                    bool is_repaint_boundary = false)
        : SingleChildElement(child, is_repaint_boundary,
                             /* size_depends_on_parent */ false){};

    std::string get_debug_name() override { return "IntrinsicHeight"; };

    Size layout(BoxConstraints constraints) override {
        auto size = Size{constraints.max_width, child->get_intrinsic_height()};
        auto child_constraints =
            BoxConstraints::from_size(size, /* tight */ true);
        auto child_size =
            document->layout_element(child.get(), child_constraints);
        child->size = child_size;
        child->rel_position = Position{0, 0};
        return child_size;
    };
};

}  // namespace aardvark::elements