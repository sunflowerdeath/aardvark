#pragma once

#include <memory>
#include <map>
#include <vector>
#include "../document.hpp"
#include "hit_tester.hpp"
#include "responder.hpp"
#include "responder_reconciler.hpp"

namespace aardvark {

class Element;
class Document;
class HitTester;
class ResponderReconciler;

// Class that controls handling of document pointer events
class PointerEventManager {
  public:
    PointerEventManager(Document* document);

    // Registers handler for all pointer events of the document.
    // When `after` is true, handler will be called after all other handlers.
    void add_handler(PointerEventHandler handler, bool after = false);

    // Removes handler of pointer events
    void remove_handler(PointerEventHandler handler, bool after = false);

    // Register handler to track all events of the specified pointer
    void start_tracking_pointer(int pointer_id, PointerEventHandler handler);

    // Stops tracking pointer by the handler
    void stop_tracking_pointer(int pointer_id, PointerEventHandler handler);

    void handle_event(PointerEvent event);

  private:
    Document* document;
    std::unique_ptr<HitTester> hit_tester;
    std::unique_ptr<ResponderReconciler> reconciler;
    std::vector<PointerEventHandler> before_handlers;
    std::vector<PointerEventHandler> after_handlers;
    std::map<int, std::vector<PointerEventHandler>> tracked_pointers_handlers;
};

}
