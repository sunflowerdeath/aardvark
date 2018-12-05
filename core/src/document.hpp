#pragma once

#include <memory>
#include <unordered_set>

#include "base_types.hpp"
#include "box_constraints.hpp"
#include "compositing.hpp"
#include "element.hpp"
#include "layer_tree.hpp"

namespace aardvark {

// forward declaration due to circular includes
class Element;
class LayerTree;

using ElementsSet = std::unordered_set<Element*>;

class Document {
 public:
  Document(Compositor& compositor, std::shared_ptr<Element> root);

  // Sets new root element
  void set_root(std::shared_ptr<Element> new_root);

  // Notify document that element was changed
  void change_element(Element* elem);

  // Paints document
  void paint();

  // Elements should call this function to layout its children
  Size layout_element(Element* elem, BoxConstraints constraints);

  // Elements should call this function to paint its children
  void paint_element(Element* elem, bool is_repaint_root = false,
                     bool clip = false);

  // Elements should call this method to obtain layer to paint itself
  Layer* get_layer();

  // Creates layer and adds it to the current layer tree, reusing layers from
  // previous repaint if possible.
  Layer* create_layer(Size size);

  std::shared_ptr<Layer> screen;
 private:
  Compositor compositor;
  std::shared_ptr<Element> root;
  ElementsSet changed_elements;
  bool is_initial_paint;

  // These members are used during the paint phase

  // Currently painted element
  Element* current_element = nullptr;
	// Layer tree of the current repaint boundary element
  LayerTree* current_layer_tree = nullptr;
	// Previous layer tree of the current repaint boundary element
  LayerTree* prev_layer_tree = nullptr;
	// Layer that is currently used for painting
  Layer* current_layer = nullptr;
  bool current_clip;

  void initial_paint();
  void repaint();
  void compose_layers();
  void paint_layer_tree(LayerTree* tree);
};

}  // namespace aardvark
