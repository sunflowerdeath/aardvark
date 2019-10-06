#pragma once

#include <memory>
#include <optional>
#include <unordered_set>
#include <functional>

#include "GrContext.h"
#include "SkCanvas.h"
#include "SkRegion.h"

#include "base_types.hpp"
#include "box_constraints.hpp"
#include "element.hpp"
#include "layer.hpp"
#include "layer_tree.hpp"
#include "pointer_events/pointer_event_manager.hpp"
#include "pointer_events/signal_event_sink.hpp"

namespace aardvark {

// forward declaration due to circular includes
class Element;
class LayerTree;
class PointerEventManager;

// TODO maybe weak ptr
using ElementsSet = std::unordered_set<Element*>;

class Document {
  public:
    Document(std::shared_ptr<Element> root = nullptr);

    // Sets new root element
    void set_root(std::shared_ptr<Element> new_root);

    // Notify document that element was changed
    void change_element(Element* elem);

    // Paints document
    bool paint();

    // Elements should call this function to layout its children
    Size layout_element(Element* elem, BoxConstraints constraints);

    // Elements should call this function to paint its children
    void paint_element(Element* elem, bool is_repaint_root = false);

    // Elements should call this method to obtain layer to paint itself
    Layer* get_layer();

    // Sets translate and clip for painting element on the layer
    void setup_layer(Layer* layer, Element* elem);

    // Creates layer and adds it to the current layer tree, reusing layers from
    // previous repaint if possible.
    Layer* create_layer(Size size);

    std::shared_ptr<Layer> screen;

    bool is_initial_paint;

    std::unique_ptr<PointerEventManager> pointer_event_manager;
    SignalEventSink<KeyEvent> key_event_sink;
    SignalEventSink<ScrollEvent> scroll_event_sink;

    std::shared_ptr<Element> root;

    bool need_recompose = false;

    void compose_layers();

  private:
    sk_sp<GrContext> gr_context;
    ElementsSet changed_elements;
    // Currently painted element
    Element* current_element = nullptr;
    // Layer tree of the current repaint boundary element
    LayerTree* current_layer_tree = nullptr;
    // Previous layer tree of the current repaint boundary element
    LayerTree* prev_layer_tree = nullptr;
    // Layer that is currently used for painting
    Layer* current_layer = nullptr;
    std::optional<SkPath> current_clip = std::nullopt;
    // Whether the current element or some of its parent is changed since last
    // repaint
    bool inside_changed = false;
    void initial_paint();
    bool repaint();
    void paint_layer_tree(LayerTree* tree);
};

}  // namespace aardvark
