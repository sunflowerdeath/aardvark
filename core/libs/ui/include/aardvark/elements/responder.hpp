#pragma once

#include <functional>

#include "../element.hpp"
#include "../pointer_events/responder.hpp"

namespace aardvark {

class ResponderElement : public SingleChildElement {
  public:
    class InnerResponder : public Responder {
      public:
        InnerResponder(ResponderElement* elem) : elem(elem){};
        ResponderElement* elem;
        void handler(
            PointerEvent event, ResponderEventType event_type) override {
            if (elem->handler) elem->handler(event, event_type);
        };
    };

    ResponderElement()
        : SingleChildElement(
              nullptr,
              /* is_repaint_boundary */ false,
              /* size_by_parent */ false),
          responder(InnerResponder(this)){};

    ResponderElement(
        std::shared_ptr<Element> child,
        HitTestMode hit_test_mode,
        std::function<void(PointerEvent, ResponderEventType)> handler,
        bool is_repaint_boundary = false)
        : SingleChildElement(
              std::move(child),
              /* is_repaint_boundary */ is_repaint_boundary,
              /* size_by_parent */ false),
          hit_test_mode(hit_test_mode),
          handler(std::move(handler)),
          responder(InnerResponder(this)){};

    std::string get_debug_name() override { return "Responder"; };
    Size layout(BoxConstraints constraints) override;
    void paint(bool is_changed) override;
    HitTestMode get_hit_test_mode() override { return hit_test_mode; };
    Responder* get_responder() override { return &responder; };

    // These are not ELEMENT_PROP's, because they don't cause rerender
    HitTestMode hit_test_mode = HitTestMode::PassToParent;
    std::function<void(PointerEvent, ResponderEventType)> handler;

  private:
    InnerResponder responder;
};

}  // namespace aardvark
