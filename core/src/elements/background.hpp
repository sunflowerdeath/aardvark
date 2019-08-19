#pragma once

#include <memory>

#include "../base_types.hpp"
#include "../box_constraints.hpp"
#include "../element.hpp"

namespace aardvark::elements {

class Background : public Element {
  public:
    Background()
        : Element(/* is_repaint_boundary */ false,
                  /* size_depends_on_parent */ true){};
    Background(SkColor color, bool is_repaint_boundary = false);
    SkColor color;
    std::string get_debug_name() override { return "Background"; };
    Size layout(BoxConstraints constraints) override;
    void paint(bool is_changed) override;
};

}  // namespace aardvark::elements
