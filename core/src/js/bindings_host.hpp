#pragma once

#include <optional>
#include <typeindex>
#include <memory>
#include <unordered_map>
#include <string>

#include "JavaScriptCore/JavaScript.h"
#include <nod/nod.hpp>

#include "../document.hpp"
#include "../element.hpp"
#include "../platforms/desktop/desktop_app.hpp"
#include "../platforms/desktop/desktop_window.hpp"
#include "../utils/event_loop.hpp"
#include "../utils/websocket.hpp"
#include "objects_index.hpp"

namespace aardvark::js {

constexpr auto PROP_ATTR_STATIC =
    kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete;

const auto property_attributes_immutable =
    kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete;

typedef JSClassRef (*JSCreateClassCallback)(JSClassRef base_class);

class BindingsHost {
  public:
    BindingsHost();
    ~BindingsHost();

    JSValueRef eval_script(const std::string& src,
                           JSValueRef* exception = nullptr);

    JSGlobalContextRef ctx;
    std::shared_ptr<EventLoop> event_loop = std::make_shared<EventLoop>();
    std::shared_ptr<DesktopApp> app;

    // Use optional to defer initialization
    std::optional<ObjectsIndex<DesktopApp>> desktop_app_window_list_index;
    std::optional<ObjectsIndex<DesktopWindow>> desktop_window_index;
    std::optional<ObjectsIndex<Document>> document_index;
    std::optional<ObjectsIndex<Element>> element_index;
    std::optional<ObjectsIndex<nod::connection>> signal_connection_index;
    std::optional<ObjectsIndex<Websocket>> websocket_index;
    JSClassRef element_class;

    // `type_info` can't be used as map key, it should be wrapped into
    // `type_index`
    std::unordered_map<std::type_index, JSClassRef> elements_classes;

    static BindingsHost* get(JSContextRef ctx);

  private:
    void add_function(
        const char* name, JSObjectCallAsFunctionCallback function,
        JSPropertyAttributes attributes = property_attributes_immutable);

    void add_object(
        const char* name, JSObjectRef object,
        JSPropertyAttributes attributes = property_attributes_immutable);

    void add_constructor(const char* name, JSClassRef jsclass,
                         JSObjectCallAsConstructorCallback call_as_constructor);

    JSClassRef get_element_js_class(Element* elem);

    void add_elem_class(
        const char* name, const std::type_info& elem_type,
        JSCreateClassCallback create_class,
        JSObjectCallAsConstructorCallback call_as_constructor);
};

}  // namespace aardvark::js
