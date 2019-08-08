#include "elements_bindings.hpp"

#include "../elements/elements.hpp"
#include "base_types_bindings.hpp"
#include "bindings_host.hpp"
#include "events_bindings.hpp"
#include "function_wrapper.hpp"

namespace aardvark::js {

// helpers

UnicodeString js_value_to_icu_str(JSContextRef ctx, JSValueRef value) {
    auto js_str =
        JSValueToStringCopy(ctx, value, /* exception */ nullptr);
    auto icu_str = UnicodeString(JSStringGetCharactersPtr(js_str),
                                 JSStringGetLength(js_str));
    JSStringRelease(js_str);
    return icu_str;
}

std::shared_ptr<Element> get_elem(JSContextRef ctx, JSObjectRef object) {
    return BindingsHost::get(ctx)->element_index->get_native_object(object);
}

template <class E>
E* get_elem(JSContextRef ctx, JSObjectRef object) {
    return dynamic_cast<E*>(get_elem(ctx, object).get());
}

// Element

JSValueRef element_get_width(JSContextRef ctx, JSObjectRef object,
                             JSStringRef property_name, JSValueRef* exception) {
    auto elem = get_elem(ctx, object);
    return JSValueMakeNumber(ctx, static_cast<double>(elem->size.width));
}

JSValueRef element_get_height(JSContextRef ctx, JSObjectRef object,
                              JSStringRef property_name,
                              JSValueRef* exception) {
    auto elem = get_elem(ctx, object);
    return JSValueMakeNumber(ctx, static_cast<double>(elem->size.height));
}

JSValueRef element_get_left(JSContextRef ctx, JSObjectRef object,
                            JSStringRef property_name, JSValueRef* exception) {
    auto elem = get_elem(ctx, object);
    return JSValueMakeNumber(ctx, static_cast<double>(elem->abs_position.left));
}

JSValueRef element_get_top(JSContextRef ctx, JSObjectRef object,
                           JSStringRef property_name, JSValueRef* exception) {
    auto elem = get_elem(ctx, object);
    return JSValueMakeNumber(ctx, static_cast<double>(elem->abs_position.top));
}

JSValueRef element_get_parent(JSContextRef ctx, JSObjectRef object,
                              JSStringRef property_name,
                              JSValueRef* exception) {
    auto host = BindingsHost::get(ctx);
    auto elem = host->element_index->get_native_object(object);
    auto parent = elem->parent;
    if (parent == nullptr) {
        return JSValueMakeUndefined(ctx);
    } else {
        return host->element_index->get_js_object(parent);
    }
}

// TODO
// JSValueRef element_get_children() {}

JSValueRef element_append_child(JSContextRef ctx, JSObjectRef function,
                                JSObjectRef object, size_t argument_count,
                                const JSValueRef arguments[],
                                JSValueRef* exception) {
    auto host = BindingsHost::get(ctx);
    auto elem = host->element_index->get_native_object(object);
    auto child = host->element_index->get_native_object(
        JSValueToObject(ctx, arguments[0], exception));
    elem->append_child(child);
    return JSValueMakeUndefined(ctx);
}

JSValueRef element_remove_child(JSContextRef ctx, JSObjectRef function,
                                JSObjectRef object, size_t argument_count,
                                const JSValueRef arguments[],
                                JSValueRef* exception) {
    auto host = BindingsHost::get(ctx);
    auto elem = host->element_index->get_native_object(object);
    auto child = host->element_index->get_native_object(
        JSValueToObject(ctx, arguments[0], exception));
    elem->remove_child(child);
    return JSValueMakeUndefined(ctx);
}

// TODO
// JSValueRef element_insert_child_before(

void element_finalize(JSObjectRef object) {
    ObjectsIndex<Element>::remove(object);
}

JSClassRef element_create_class() {
    auto definition = kJSClassDefinitionEmpty;
    JSStaticFunction static_functions[] = {
        {"appendChild", element_append_child, PROP_ATTR_STATIC},
        {"removeChild", element_remove_child, PROP_ATTR_STATIC},
        {0, 0, 0}};
    JSStaticValue static_values[] = {
        {"width", element_get_width, nullptr, PROP_ATTR_STATIC},
        {"height", element_get_height, nullptr, PROP_ATTR_STATIC},
        {"left", element_get_left, nullptr, PROP_ATTR_STATIC},
        {"top", element_get_top, nullptr, PROP_ATTR_STATIC},
        {"parent", element_get_parent, nullptr, PROP_ATTR_STATIC},
        {0, 0, 0, 0}};
    definition.className = "Element";
    definition.finalize = element_finalize;
    definition.staticFunctions = static_functions;
    definition.staticValues = static_values;
    return JSClassCreate(&definition);
}

//------------------------------------------------------------------------------
// Align
//------------------------------------------------------------------------------

elements::Align* get_align_elem(JSContextRef ctx, JSObjectRef object) {
    auto host = BindingsHost::get(ctx);
    auto elem = host->element_index->get_native_object(object);
    return dynamic_cast<elements::Align*>(elem.get());
}

bool align_element_set_align(JSContextRef ctx, JSObjectRef object,
                             JSStringRef property_name, JSValueRef value,
                             JSValueRef* exception) {
    auto align_elem = get_align_elem(ctx, object);
    auto insets = alignment_from_js(ctx, JSValueToObject(ctx, value, nullptr));
    align_elem->insets = insets;
    align_elem->change();
    return true;
}

JSValueRef align_element_get_align(JSContextRef ctx, JSObjectRef object,
                                   JSStringRef property_name,
                                   JSValueRef* exception) {
    auto align_elem = get_align_elem(ctx, object);
    return alignment_to_js(ctx, align_elem->insets);
}

JSClassRef align_elem_create_class(JSClassRef base_class) {
    auto definition = kJSClassDefinitionEmpty;
    JSStaticValue static_values[] = {
        {"align", align_element_get_align, align_element_set_align,
         kJSPropertyAttributeNone},
        {0, 0, 0, 0}};
    definition.className = "AlignElement";
    definition.parentClass = base_class;
    definition.staticValues = static_values;
    return JSClassCreate(&definition);
}

JSObjectRef align_elem_call_as_constructor(JSContextRef ctx,
                                           JSObjectRef constructor,
                                           size_t argumentCount,
                                           const JSValueRef arguments[],
                                           JSValueRef* exception) {
    auto host = BindingsHost::get(ctx);
    auto insets = elements::EdgeInsets{};
    auto elem = std::make_shared<elements::Align>(
        std::make_shared<elements::Placeholder>(), insets);
    return host->element_index->create_js_object(elem);
}

//------------------------------------------------------------------------------
// Background
//------------------------------------------------------------------------------

JSValueRef background_element_get_background(JSContextRef ctx,
                                             JSObjectRef object,
                                             JSStringRef property_name,
                                             JSValueRef* exception) {
    auto host = BindingsHost::get(ctx);
    auto elem = host->element_index->get_native_object(object);
    auto bg_elem = dynamic_cast<elements::Background*>(elem.get());
    return JSValueMakeNumber(ctx, 123.0);
}

bool background_element_set_background(JSContextRef ctx, JSObjectRef object,
                                       JSStringRef propertyName,
                                       JSValueRef value,
                                       JSValueRef* exception) {
    auto host = BindingsHost::get(ctx);
    auto elem = host->element_index->get_native_object(object);
    auto bg_elem = dynamic_cast<elements::Background*>(elem.get());
    bg_elem->color = SK_ColorRED;
    return true;
}

JSClassRef background_elem_create_class(JSClassRef element_class) {
    auto definition = kJSClassDefinitionEmpty;
    JSStaticValue static_values[] = {
        {"background", background_element_get_background,
         background_element_set_background, PROP_ATTR_STATIC},
        {0, 0, 0, 0}};
    definition.className = "BackgroundElement";
    definition.parentClass = element_class;
    definition.staticValues = static_values;
    return JSClassCreate(&definition);
}

JSObjectRef background_elem_call_as_constructor(JSContextRef ctx,
                                                JSObjectRef constructor,
                                                size_t argument_count,
                                                const JSValueRef arguments[],
                                                JSValueRef* exception) {
    auto host = BindingsHost::get(ctx);
    auto elem = std::make_shared<elements::Background>(SK_ColorRED);
    return host->element_index->create_js_object(elem);
}

//------------------------------------------------------------------------------
// Responder
//------------------------------------------------------------------------------

std::vector<JSValueRef> responder_handler_args_to_js(JSContextRef ctx,
                                                     PointerEvent event,
                                                     int hz) {
    return std::vector<JSValueRef>{pointer_event_to_js(ctx, event),
                                   JSValueMakeNumber(ctx, hz)};
}

void responder_handler_ret_val_from_js(JSContextRef ctx, JSValueRef value){};

JSValueRef responder_elem_set_handler(JSContextRef ctx, JSObjectRef function,
                                      JSObjectRef object, size_t argument_count,
                                      const JSValueRef arguments[],
                                      JSValueRef* exception) {
    auto responder = get_elem<elements::ResponderElement>(ctx, object);
    responder->handler = FunctionWrapper<void, PointerEvent, int>(
        JSContextGetGlobalContext(ctx),    // ctx
        function,                          // function
        responder_handler_args_to_js,      // args_to_js
        responder_handler_ret_val_from_js  // ret_val_from_js
    );
    responder->change();
    return JSValueMakeUndefined(ctx);
}

JSClassRef responder_elem_create_class(JSClassRef element_class) {
    auto definition = kJSClassDefinitionEmpty;
    JSStaticFunction static_functions[] = {
        {"setHandler", responder_elem_set_handler, PROP_ATTR_STATIC},
        {0, 0, 0}};
    definition.className = "ResponderElement";
    definition.parentClass = element_class;
    definition.staticFunctions = static_functions;
    return JSClassCreate(&definition);
}

JSObjectRef responder_elem_call_as_constructor(JSContextRef ctx,
                                               JSObjectRef constructor,
                                               size_t argument_count,
                                               const JSValueRef arguments[],
                                               JSValueRef* exception) {
    auto host = BindingsHost::get(ctx);
    auto elem = std::make_shared<elements::ResponderElement>(
        std::make_shared<elements::Placeholder>(), HitTestMode::PassToParent,
        nullptr);
    return host->element_index->create_js_object(elem);
}

//------------------------------------------------------------------------------
// Stack
//------------------------------------------------------------------------------

JSClassRef stack_elem_create_class(JSClassRef element_class) {
    auto definition = kJSClassDefinitionEmpty;
    definition.className = "Stack";
    definition.parentClass = element_class;
    return JSClassCreate(&definition);
}

JSObjectRef stack_elem_call_as_constructor(JSContextRef ctx,
                                           JSObjectRef constructor,
                                           size_t argument_count,
                                           const JSValueRef arguments[],
                                           JSValueRef* exception) {
    auto host = BindingsHost::get(ctx);
    auto elem = std::make_shared<elements::Stack>();
    return host->element_index->create_js_object(elem);
}

//------------------------------------------------------------------------------
// Text
//------------------------------------------------------------------------------

JSValueRef icu_str_to_js_value(JSContextRef ctx, const UnicodeString& str) {
    auto js_str = JSStringCreateWithCharacters(str.getBuffer(), str.length());
    return JSValueMakeString(ctx, js_str);
}

JSValueRef text_element_get_text(JSContextRef ctx, JSObjectRef object,
                                 JSStringRef property_name,
                                 JSValueRef* exception) {
    auto host = BindingsHost::get(ctx);
    auto elem = host->element_index->get_native_object(object);
    auto text_elem = dynamic_cast<elements::Text*>(elem.get());
    return icu_str_to_js_value(ctx, text_elem->text);
}

bool text_element_set_text(JSContextRef ctx, JSObjectRef object,
                           JSStringRef propertyName, JSValueRef value,
                           JSValueRef* exception) {
    auto host = BindingsHost::get(ctx);
    auto elem = host->element_index->get_native_object(object);
    auto text_elem = dynamic_cast<elements::Text*>(elem.get());
    text_elem->text = js_value_to_icu_str(ctx, value);
    text_elem->change();
    return true;
}

JSClassRef text_elem_create_class(JSClassRef element_class) {
    auto definition = kJSClassDefinitionEmpty;
    JSStaticValue static_values[] = {
        {"text", text_element_get_text, text_element_set_text,
         kJSPropertyAttributeDontDelete},
        {0, 0, 0, 0}};
    definition.className = "TextElement";
    definition.parentClass = element_class;
    definition.staticValues = static_values;
    return JSClassCreate(&definition);
}

JSObjectRef text_elem_call_as_constructor(JSContextRef ctx,
                                          JSObjectRef constructor,
                                          size_t argument_count,
                                          const JSValueRef arguments[],
                                          JSValueRef* exception) {
    auto host = BindingsHost::get(ctx);

    SkPaint paint;
    paint.setColor(SK_ColorRED);
    paint.setTextSize(32);
    paint.setAntiAlias(true);
    paint.setTextEncoding(SkPaint::kUTF16_TextEncoding);

    auto text = js_value_to_icu_str(ctx, arguments[0]);
    auto elem = std::make_shared<elements::Text>(text, paint);

    return host->element_index->create_js_object(elem);
}


//------------------------------------------------------------------------------
// Size
//------------------------------------------------------------------------------

bool sized_elem_set_size_constraints(JSContextRef ctx, JSObjectRef object,
                                       JSStringRef property_name,
                                       JSValueRef value,
                                       JSValueRef* exception) {
    auto elem = get_elem<elements::Sized>(ctx, object);
    elem->size_constraints =
        size_constraints_from_js(ctx, JSValueToObject(ctx, value, nullptr));
    elem->change();
    return true;
}

JSValueRef sized_elem_get_size_constraints(JSContextRef ctx,
                                             JSObjectRef object,
                                             JSStringRef property_name,
                                             JSValueRef* exception) {
    auto elem = get_elem<elements::Sized>(ctx, object);
    return size_constraints_to_js(ctx, elem->size_constraints);
}

JSClassRef sized_elem_create_class(JSClassRef base_class) {
    auto definition = kJSClassDefinitionEmpty;
    JSStaticValue static_values[] = {
        {"sizeConstraints", sized_elem_get_size_constraints,
         sized_elem_set_size_constraints, kJSPropertyAttributeNone},
        {0, 0, 0, 0}};
    definition.className = "Sized";
    definition.parentClass = base_class;
    definition.staticValues = static_values;
    return JSClassCreate(&definition);
}

JSObjectRef sized_elem_call_as_constructor(JSContextRef ctx,
                                          JSObjectRef constructor,
                                          size_t argument_count,
                                          const JSValueRef arguments[],
                                          JSValueRef* exception) {
    auto host = BindingsHost::get(ctx);
    auto size_constrains = elements::SizeConstraints{};
    auto elem = std::make_shared<elements::Sized>(
        std::make_shared<elements::Placeholder>(), size_constrains);
    return host->element_index->create_js_object(elem);
}

}  // namespace aardvark::js
