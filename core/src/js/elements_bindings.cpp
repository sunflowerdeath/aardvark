#include "elements_bindings.hpp"

#include "../elements/elements.hpp"
#include "helpers.hpp"
#include "base_types_bindings.hpp"
#include "bindings_host.hpp"
#include "events_bindings.hpp"
#include "function_wrapper.hpp"

#define ADV_DEFINE_ELEM_CONSTRUCTOR(NAME, TYPE)                           \
    JSObjectRef NAME##_elem_call_as_constructor(                          \
        JSContextRef ctx, JSObjectRef constructor, size_t argument_count, \
        const JSValueRef arguments[], JSValueRef* exception) {            \
        return BindingsHost::get(ctx)->element_index->create_js_object(   \
            std::make_shared<TYPE>());                                    \
    }

namespace aardvark::js {

// helpers

std::shared_ptr<Element> get_elem(JSContextRef ctx, JSObjectRef object) {
    return BindingsHost::get(ctx)->element_index->get_native_object(object);
}

template <class E>
E* get_elem(JSContextRef ctx, JSObjectRef object) {
    return dynamic_cast<E*>(get_elem(ctx, object).get());
}

JSClassDefinition create_elem_class_definition(const char* name,
                                               JSClassRef parent_class) {
    auto definition = kJSClassDefinitionEmpty;
    definition.className = name;
    definition.parentClass = parent_class;
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

    static auto target = std::string("Element.appendChild()");
    auto checker =
        check_types::make_arguments({{"child", host->typedefs->element}});
    auto result = checker(ctx, argument_count, arguments, target);
    if (check_types::to_exception(result, ctx, exception)) {
        return JSValueMakeUndefined(ctx);
    }

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

    // TODO check types
  
    auto elem = host->element_index->get_native_object(object);
    auto child = host->element_index->get_native_object(
        JSValueToObject(ctx, arguments[0], exception));
    elem->remove_child(child);
    return JSValueMakeUndefined(ctx);
}

JSValueRef element_insert_before_child(JSContextRef ctx, JSObjectRef function,
                                       JSObjectRef object,
                                       size_t argument_count,
                                       const JSValueRef arguments[],
                                       JSValueRef* exception) {
    auto host = BindingsHost::get(ctx);

    /*
    auto checker =
        check_types::make_arguments({{"child", host->typedefs->element},
                                     {"beforeChild", host->typedefs->element}});
    if (check_types::to_exception(checker, ctx, argument_count, arguments,
                                  "Element.insertBeforeChild()", exception)) {
        return JSValueMakeUndefined(ctx);
    }
    */

    auto elem = host->element_index->get_native_object(object);
    auto child = get_elem(ctx, JSValueToObject(ctx, arguments[0], exception));
    auto before_child =
        get_elem(ctx, JSValueToObject(ctx, arguments[1], exception));
    elem->insert_before_child(child, before_child);
    return JSValueMakeUndefined(ctx);
}

void element_finalize(JSObjectRef object) {
    ObjectsIndex<Element>::remove(object);
}

JSClassRef element_create_class() {
    auto definition = kJSClassDefinitionEmpty;
    JSStaticFunction static_functions[] = {
        {"appendChild", element_append_child, PROP_ATTR_STATIC},
        {"removeChild", element_remove_child, PROP_ATTR_STATIC},
        {"insertBeforeChild", element_insert_before_child, PROP_ATTR_STATIC},
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

JSObjectRef element_call_as_constructor(JSContextRef ctx,
                                        JSObjectRef constructor,
                                        size_t argument_count,
                                        const JSValueRef arguments[],
                                        JSValueRef* exception) {
    auto message =
        str_to_js(ctx, "Element's constructor can't be called directly.");
    *exception = JSObjectMakeError(ctx, 1, &message, nullptr);
    return nullptr;
}

//------------------------------------------------------------------------------
// Align
//------------------------------------------------------------------------------

bool align_element_set_align(JSContextRef ctx, JSObjectRef object,
                             JSStringRef property_name, JSValueRef value,
                             JSValueRef* exception) {
    auto host = BindingsHost::get(ctx);

    // TODO check types
    /*
    auto params = check_types::params{"property", "alignment", "AlignElement"};
    if (check_types::to_exception(host->typedefs->alignment, ctx, value, params,
                                  exception)) {
        return false;
    }
    */

    auto elem = get_elem<elements::Align>(ctx, object);
    auto insets = alignment_from_js(ctx, JSValueToObject(ctx, value, nullptr));
    elem->insets = insets;
    elem->change();
    return true;
}

JSValueRef align_element_get_align(JSContextRef ctx, JSObjectRef object,
                                   JSStringRef property_name,
                                   JSValueRef* exception) {
    auto elem = get_elem<elements::Align>(ctx, object);
    return alignment_to_js(ctx, elem->insets);
}

JSClassRef align_elem_create_class(JSClassRef parent_class) {
    auto definition =
        create_elem_class_definition("AlignElement", parent_class);
    JSStaticValue static_values[] = {
        {"align", align_element_get_align, align_element_set_align,
         kJSPropertyAttributeNone},
        {0, 0, 0, 0}};
    definition.staticValues = static_values;
    return JSClassCreate(&definition);
}

ADV_DEFINE_ELEM_CONSTRUCTOR(align, elements::Align);

//------------------------------------------------------------------------------
// Background
//------------------------------------------------------------------------------

JSValueRef background_element_get_color(JSContextRef ctx, JSObjectRef object,
                                        JSStringRef property_name,
                                        JSValueRef* exception) {
    auto elem = get_elem<elements::Background>(ctx, object);
    return color_to_js(ctx, elem->color);
}

bool background_element_set_color(JSContextRef ctx, JSObjectRef object,
                                  JSStringRef property_name, JSValueRef value,
                                  JSValueRef* exception) {
    auto host = BindingsHost::get(ctx);

    static auto params =
        check_types::ErrorParams{"property", "color", "BackgroundElement"};
    auto error = host->typedefs->color(ctx, value, params);
    if (check_types::to_exception(error, ctx, exception)) return false;

    auto elem = get_elem<elements::Background>(ctx, object);
    elem->color = color_from_js(ctx, value);
    return true;
}

JSClassRef background_elem_create_class(JSClassRef parent_class) {
    auto definition =
        create_elem_class_definition("BackgroundElement", parent_class);
    JSStaticValue static_values[] = {
        {"color", background_element_get_color, background_element_set_color,
         kJSPropertyAttributeNone},
        {0, 0, 0, 0}};
    definition.staticValues = static_values;
    return JSClassCreate(&definition);
}

ADV_DEFINE_ELEM_CONSTRUCTOR(background, elements::Background);

//------------------------------------------------------------------------------
// Center
//------------------------------------------------------------------------------

JSClassRef center_elem_create_class(JSClassRef parent_class) {
    auto definition =
        create_elem_class_definition("CenterElement", parent_class);
    return JSClassCreate(&definition);
}

ADV_DEFINE_ELEM_CONSTRUCTOR(center, elements::Center);

//------------------------------------------------------------------------------
// IntrinsicHeight
//------------------------------------------------------------------------------

JSClassRef intrinsic_height_elem_create_class(JSClassRef parent_class) {
    auto definition =
        create_elem_class_definition("IntrinsicHeightElement", parent_class);
    return JSClassCreate(&definition);
}

ADV_DEFINE_ELEM_CONSTRUCTOR(intrinsic_height, elements::IntrinsicHeight);

//------------------------------------------------------------------------------
// Flex
//------------------------------------------------------------------------------

JSClassRef flex_elem_create_class(JSClassRef parent_class) {
    auto definition = create_elem_class_definition("FlexElement", parent_class);
    return JSClassCreate(&definition);
}

ADV_DEFINE_ELEM_CONSTRUCTOR(flex, elements::Flex);

//------------------------------------------------------------------------------
// Responder
//------------------------------------------------------------------------------

std::vector<JSValueRef> responder_handler_args_to_js(
    JSContextRef ctx, PointerEvent event, ResponderEventType event_type) {
    auto event_type_num =
        static_cast<std::underlying_type_t<ResponderEventType>>(event_type);
    return std::vector<JSValueRef>{pointer_event_to_js(ctx, event),
                                   JSValueMakeNumber(ctx, event_type_num)};
}

void responder_handler_ret_val_from_js(JSContextRef ctx, JSValueRef value){};

JSValueRef responder_elem_get_handler(JSContextRef ctx, JSObjectRef object,
                                      JSStringRef property_name,
                                      JSValueRef* exception) {
    return JSValueMakeUndefined(ctx);
}

bool responder_elem_set_handler(JSContextRef ctx, JSObjectRef object,
                                JSStringRef property_name, JSValueRef value,
                                JSValueRef* exception) {
    auto host = BindingsHost::get(ctx);
    auto responder = get_elem<elements::ResponderElement>(ctx, object);
    responder->handler =
        FunctionWrapper<void, PointerEvent, ResponderEventType>(
            host->ctx,                          // ctx
            value,                              // function
            responder_handler_args_to_js,       // args_to_js
            responder_handler_ret_val_from_js,  // ret_val_from_js
            [host](JSContextRef ctx, JSValueRef ex) {
                host->module_loader->handle_exception(ex);
            });
    return true;
}

JSClassRef responder_elem_create_class(JSClassRef parent_class) {
    auto definition =
        create_elem_class_definition("ResponderElement", parent_class);
    JSStaticValue static_values[] = {
        {"handler", responder_elem_get_handler, responder_elem_set_handler,
         kJSPropertyAttributeNone},
        {0, 0, 0, 0}};
    definition.staticValues = static_values;
    return JSClassCreate(&definition);
}

ADV_DEFINE_ELEM_CONSTRUCTOR(responder, elements::ResponderElement);

//------------------------------------------------------------------------------
// Stack
//------------------------------------------------------------------------------

JSClassRef stack_elem_create_class(JSClassRef parent_class) {
    auto definition =
        create_elem_class_definition("StackElement", parent_class);
    return JSClassCreate(&definition);
}

ADV_DEFINE_ELEM_CONSTRUCTOR(stack, elements::Stack);

//------------------------------------------------------------------------------
// Text
//------------------------------------------------------------------------------

JSValueRef text_element_get_text(JSContextRef ctx, JSObjectRef object,
                                 JSStringRef property_name,
                                 JSValueRef* exception) {
    auto host = BindingsHost::get(ctx);
    auto elem = host->element_index->get_native_object(object);
    auto text_elem = dynamic_cast<elements::Text*>(elem.get());
    return icu_str_to_js(ctx, text_elem->text);
}

bool text_element_set_text(JSContextRef ctx, JSObjectRef object,
                           JSStringRef propertyName, JSValueRef value,
                           JSValueRef* exception) {
    auto host = BindingsHost::get(ctx);
    auto elem = host->element_index->get_native_object(object);
    auto text_elem = dynamic_cast<elements::Text*>(elem.get());
    text_elem->text = icu_str_from_js(ctx, value);
    text_elem->change();
    return true;
}

JSClassRef text_elem_create_class(JSClassRef parent_class) {
    auto definition =
        create_elem_class_definition("TextElement", parent_class);
    JSStaticValue static_values[] = {
        {"text", text_element_get_text, text_element_set_text,
         kJSPropertyAttributeDontDelete},
        {0, 0, 0, 0}};
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
    paint.setColor(SK_ColorWHITE);
    paint.setTextSize(32);
    paint.setAntiAlias(true);
    paint.setTextEncoding(SkPaint::kUTF16_TextEncoding);

    auto text = UnicodeString((UChar*)u"");
    // auto text = js_value_to_icu_str(ctx, arguments[0]);
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

JSClassRef sized_elem_create_class(JSClassRef parent_class) {
    auto definition =
        create_elem_class_definition("SizedElement", parent_class);
    JSStaticValue static_values[] = {
        {"sizeConstraints", sized_elem_get_size_constraints,
         sized_elem_set_size_constraints, kJSPropertyAttributeNone},
        {0, 0, 0, 0}};
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
