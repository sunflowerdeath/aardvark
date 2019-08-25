#include "websocket_bindings.hpp"

#include "../utils/websocket.hpp"
#include "bindings_host.hpp"
#include "function_wrapper.hpp"
#include "helpers.hpp"
#include "signal_connection_bindings.hpp"

namespace aardvark::js {

// Methods

JSValueRef websocket_send(JSContextRef ctx, JSObjectRef function,
                          JSObjectRef object, size_t argument_count,
                          const JSValueRef arguments[], JSValueRef* exception) {
    auto host = BindingsHost::get(ctx);
    auto ws = host->websocket_index->get_native_object(object);
    auto js_str = JSValueToStringCopy(ctx, arguments[0], nullptr);
    ws->send(str_from_js(js_str));
    return JSValueMakeUndefined(ctx);
}

JSValueRef websocket_close(JSContextRef ctx, JSObjectRef function,
                           JSObjectRef object, size_t argument_count,
                           const JSValueRef arguments[],
                           JSValueRef* exception) {
    auto host = BindingsHost::get(ctx);
    auto ws = host->websocket_index->get_native_object(object);
    ws->close();
    return JSValueMakeUndefined(ctx);
}

// Handlers

std::vector<JSValueRef> websocket_error_handler_args_to_js(JSContextRef ctx,
                                                           std::string err) {
    auto obj = JSObjectMake(ctx, nullptr, nullptr);
    auto str = JSStringCreateWithUTF8CString("error_text");
    auto val = JSValueMakeString(ctx, str);
    JSStringRelease(str);
    JSObjectSetProperty(ctx, obj, JsStringCache::get("message"), val,
                        kJSPropertyAttributeNone, nullptr);
    return std::vector<JSValueRef>{obj};
}

std::vector<JSValueRef> websocket_message_handler_args_to_js(JSContextRef ctx,
                                                             std::string msg) {
    auto obj = JSObjectMake(ctx, nullptr, nullptr);
    auto str = JSStringCreateWithUTF8CString("message_text");
    auto val = JSValueMakeString(ctx, str);
    JSStringRelease(str);
    JSObjectSetProperty(ctx, obj, JsStringCache::get("data"), val,
                        kJSPropertyAttributeNone, nullptr);
    return std::vector<JSValueRef>{obj};
}

JSValueRef websocket_add_start_handler(JSContextRef ctx, JSObjectRef function,
                                       JSObjectRef object,
                                       size_t argument_count,
                                       const JSValueRef arguments[],
                                       JSValueRef* exception) {
    auto host = BindingsHost::get(ctx);
    auto ws = host->websocket_index->get_native_object(object);
    auto connection =
        ws->start_signal.connect(FunctionWrapper<void>(ctx, arguments[0]));
    return signal_connection_to_js(ctx, std::move(connection));
}

JSValueRef websocket_add_message_handler(JSContextRef ctx, JSObjectRef function,
                                         JSObjectRef object,
                                         size_t argument_count,
                                         const JSValueRef arguments[],
                                         JSValueRef* exception) {
    auto host = BindingsHost::get(ctx);
    auto ws = host->websocket_index->get_native_object(object);
    auto connection =
        ws->message_signal.connect(FunctionWrapper<void, std::string>(
            ctx, arguments[0], websocket_message_handler_args_to_js));
    return signal_connection_to_js(ctx, std::move(connection));
}

JSValueRef websocket_add_error_handler(JSContextRef ctx, JSObjectRef function,
                                       JSObjectRef object,
                                       size_t argument_count,
                                       const JSValueRef arguments[],
                                       JSValueRef* exception) {
    auto host = BindingsHost::get(ctx);
    auto ws = host->websocket_index->get_native_object(object);
    auto connection =
        ws->error_signal.connect(FunctionWrapper<void, std::string>(
            ctx, arguments[0], websocket_error_handler_args_to_js));
    return signal_connection_to_js(ctx, std::move(connection));
}

JSValueRef websocket_add_close_handler(JSContextRef ctx, JSObjectRef function,
                                       JSObjectRef object,
                                       size_t argument_count,
                                       const JSValueRef arguments[],
                                       JSValueRef* exception) {
    auto host = BindingsHost::get(ctx);
    auto ws = host->websocket_index->get_native_object(object);
    auto connection =
        ws->close_signal.connect(FunctionWrapper<void>(ctx, arguments[0]));
    return signal_connection_to_js(ctx, std::move(connection));
}

JSValueRef websocket_get_state(JSContextRef ctx, JSObjectRef object,
                               JSStringRef property_name,
                               JSValueRef* exception) {
    auto host = BindingsHost::get(ctx);
    auto ws = host->websocket_index->get_native_object(object);
    auto state = static_cast<std::underlying_type_t<WebsocketState>>(ws->state);
    return JSValueMakeNumber(ctx, state);
}

void websocket_finalize(JSObjectRef object) {
    ObjectsIndex<Websocket>::remove(object);
}

JSClassRef websocket_create_class() {
    auto definition = kJSClassDefinitionEmpty;
    JSStaticFunction static_functions[] = {
        {"send", websocket_send, PROP_ATTR_STATIC},
        {"close", websocket_close, PROP_ATTR_STATIC},
        {"addStartHandler", websocket_add_start_handler, PROP_ATTR_STATIC},
        {"addMessageHandler", websocket_add_message_handler, PROP_ATTR_STATIC},
        {"addErrorHandler", websocket_add_error_handler, PROP_ATTR_STATIC},
        {"addCloseHandler", websocket_add_close_handler, PROP_ATTR_STATIC},
        {0, 0, 0}};
    JSStaticValue static_values[] = {
        {"state", websocket_get_state, nullptr, PROP_ATTR_STATIC},
        {0, 0, 0, 0}};
    definition.className = "Websocket";
    definition.staticFunctions = static_functions;
    definition.staticValues = static_values;
    definition.finalize = websocket_finalize;
    return JSClassCreate(&definition);
};

JSObjectRef websocket_call_as_constructor(JSContextRef ctx,
                                          JSObjectRef constructor,
                                          size_t argumentCount,
                                          const JSValueRef arguments[],
                                          JSValueRef* exception) {
    auto host = BindingsHost::get(ctx);
    auto ws = std::make_shared<aardvark::Websocket>(host->event_loop->io,
                                                    "echo.websocket.org", "80");
    ws->start();
    return host->websocket_index->create_js_object(ws);
};

}  // namespace aardvark::js
