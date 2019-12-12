#pragma once

#include <quickjs/quickjs.h>

#include "jsi.hpp"

namespace aardvark::jsi {

class Qjs_Context : public Context {
  public:
    static std::shared_ptr<Qjs_Context> create();
    static Qjs_Context* get(JSContext* ctx);
    static JSClassID function_class_id;

    Qjs_Context();
    void init();
    ~Qjs_Context();

    // Helpers
    Value value_from_qjs(const JSValue& value);
    Object object_from_qjs(const JSValue& value);

    JSValue value_get(const Value& value);
    JSValue object_get(const Object& object);
    std::string string_get_str(const String& str);

    Script create_script(
        const std::string& source, const std::string& source_url) override;
    Value eval_script(
        const std::string& script, Object* js_this,
        const std::string& source_url) override;
    void garbage_collect() override;
    Object get_global_object() override;

    // String
    String string_make_from_utf8(const std::string& str) override;
    std::string string_to_utf8(const String&) override;

    // Value
    Value value_make_bool(bool value) override;
    Value value_make_number(double value) override;
    Value value_make_null() override;
    Value value_make_undefined() override;
    Value value_make_string(const String& str) override;
    Value value_make_object(const Object& object) override;

    ValueType value_get_type(const Value& value) override;
    bool value_to_bool(const Value& value) override;
    double value_to_number(const Value& value) override;
    String value_to_string(const Value& value) override;
    Object value_to_object(const Value& value) override;

    bool value_strict_equal(const Value& a, const Value& b) override;

    // Class
    Class class_create(const ClassDefinition& definition) override;

    // Object
    Object object_make(const Class* js_class) override;
    Object object_make_function(const Function& function) override;
    Object object_make_constructor(const Class& js_class) override;
    Object object_make_array() override;

    Value object_to_value(const Object& object) override;

    void object_set_private_data(const Object& object, void* data) override;
    void* object_get_private_data(const Object& object) override;

    Value object_get_prototype(const Object& object) override;
    void object_set_prototype(
        const Object& object, const Value& prototype) override;

    std::vector<std::string> object_get_property_names(
        const Object& object) override;
    bool object_has_property(
        const Object& object, const std::string& name) override;
    Value object_get_property(
        const Object& object, const std::string& name) override;
    void object_delete_property(
        const Object& object, const std::string& name) override;
    void object_set_property(
        const Object& object, const std::string& name,
        const Value& value) override;

    bool object_is_function(const Object& object) override;
    Value object_call_as_function(
        const Object& object, const Value* jsi_this,
        const std::vector<Value>& jsi_args) override;

    bool object_is_constructor(const Object& object) override;
    Value object_call_as_constructor(
        const Object& object, const std::vector<Value>& arguments) override;

    bool object_is_array(const Object& object) override;
    Value object_get_property_at_index(
        const Object& object, size_t index) override;
    void object_set_property_at_index(
        const Object& object, size_t index, const Value& value) override;

    JSRuntime* rt;
    JSContext* ctx;
    std::optional<Object> strict_equal_function;
};

}  // namespace aardvark::jsi
