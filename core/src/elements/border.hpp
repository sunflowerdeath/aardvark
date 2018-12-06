#pragma once

#include <memory>
#include "SkCanvas.h"
#include "../base_types.hpp"
#include "../box_constraints.hpp"
#include "../element.hpp"

namespace aardvark::elements {

struct BorderSide {
  int width = 0;
  SkColor color;
};

struct BoxBorders {
  BorderSide top;
  BorderSide right;
  BorderSide bottom;
  BorderSide left;

  static BoxBorders all(BorderSide side) {
    return BoxBorders{side, side, side, side};
  };
};

struct Radius {
  int width;
  int height;

  // Whether the radius is not rounded
  bool is_square() { return width == 0 && height == 0; };

  static Radius circular(int val) {
    return Radius{val, val};
  };
};

struct BoxRadiuses {
  Radius topLeft;
  Radius topRight;
  Radius bottomRight;
  Radius bottomLeft;

  // Whether all radiuses of the box are square
  bool is_square() {
    return topLeft.is_square() && topRight.is_square() &&
           bottomRight.is_square() && bottomLeft.is_square();
  };

  static BoxRadiuses all(Radius radius) {
    return BoxRadiuses{radius, radius, radius, radius};
  };
};

class Border : public Element {
 public:
  Border(std::shared_ptr<Element> child, BoxBorders borders,
         BoxRadiuses radiuses, bool is_repaint_boundary = false);
  BoxBorders borders;
  BoxRadiuses radiuses;
  std::shared_ptr<Element> child;
  Size layout(BoxConstraints constraints);
  void paint();
 private:
  Layer* layer;
  SkMatrix matrix;
  int rotation;
  void paint_side(BorderSide& prev_side, BorderSide& side,
                  BorderSide& next_side, Radius& left_radius,
                  Radius& right_radius);
  void paint_triangle(Radius& radius, BorderSide& prev_side, BorderSide& side,
                 BorderSide& next_side, int width, bool is_left_edge);
  void paint_arc(Radius& radius, BorderSide& side, int width);
};

} // namespace aardvark::elements
