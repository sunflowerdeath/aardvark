#pragma once

#include <memory>
#include "SkCanvas.h"
#include "../base_types.hpp"
#include "../box_constraints.hpp"
#include "../element.hpp"

namespace aardvark::elements {

class Background : public Element {
 public:
  Background(SkColor color, bool is_repaint_boundary = false);
  SkColor color;
  void set_props(SkColor color);
  bool sized_by_parent = true;
  std::string get_debug_name() override { return "Background"; };
  Size layout(BoxConstraints constraints) override;
  void paint(bool is_changed) override;
};

} // namespace aardvark::elements
