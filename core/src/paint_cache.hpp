#pragma once

#include <functional>
#include "SkPicture.h"
#include "SkPictureRecorder.h"
#include "SkCanvas.h"

namespace aardvark {

class PaintCache {
 public:
  void restart(bool is_changed);
  void paint(SkCanvas* real_canvas, std::function<void(SkCanvas*)> painter);

 private:
  bool is_changed;
  int current_index;
  std::vector<sk_sp<SkPicture>> records;
};

} // namespace aardvark
