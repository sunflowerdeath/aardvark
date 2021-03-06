#pragma once

#include <functional>
#include <map>
#include <memory>
#include <tl/expected.hpp>
#include <vector>
#include <string>

namespace aardvark::jsi {

class Context;
class Object;
class Value;

class PointerData {
  public:
    virtual ~PointerData() = default;
    virtual PointerData* copy() = 0;
};

class Pointer {
  public:
    Pointer(Context* ctx, PointerData* ptr) : ctx(ctx), ptr(ptr){};

    // copy
    Pointer(const Pointer& other) : ctx(other.ctx) { ptr = other.ptr->copy(); }

    Pointer& operator=(const Pointer& other) { return *this = Pointer(other); }

    // move
    Pointer(Pointer&& other) noexcept : ctx(other.ctx), ptr(other.ptr) {
        other.ptr = nullptr;
    }

    Pointer& operator=(Pointer&& other) noexcept {
        ctx = other.ctx;
        ptr = std::exchange(other.ptr, nullptr);
        return *this;
    }

    ~Pointer() { delete ptr; }

    Context* ctx;
    PointerData* ptr;
};

class String : public Pointer {
  public:
    using Pointer::Pointer;
    std::string to_utf8() const;
};

struct ErrorLocation {
    std::string source_url;
    int line;
    int column;
};

class Error {
  public:
    Error(Context* ctx, Value* value);
    std::string message();
    Value value();
    std::optional<ErrorLocation> location();
  private:
    std::shared_ptr<Value> value_ptr;
    Context* ctx;
};

tl::unexpected<Error> make_error_result(Context& ctx, std::string message);

template <typename T>
using Result = tl::expected<T, Error>;

using VoidResult = Result<void>;

enum class ValueType {
    undefined,
    null,
    boolean,
    number,
    string,
    object,
    symbol,
    unknown
};

class WeakValue;

class Value : public Pointer {
  public:
    using Pointer::Pointer;
    ValueType get_type() const;
    Result<bool> to_bool() const;
    Result<double> to_number() const;
    Result<String> to_string() const;
    Result<Object> to_object() const;
    bool strict_equal_to(const Value& value) const;
    bool is_error() const;
    WeakValue make_weak() const;
};

class WeakValue : public Pointer {
  public:
    using Pointer::Pointer;
    Value lock() const;
};

class Object : public Pointer {
  public:
    using Pointer::Pointer;

    void set_private_data(void* data) const;
    void* get_private_data() const;

    template <typename T>
    Result<T> get_private_data() const {
        return *static_cast<T*>(get_private_data());
    }

    Value to_value() const;

    Result<Value> get_prototype() const;
    VoidResult set_prototype(const Value& prototype) const;

    std::vector<std::string> get_property_names() const;
    bool has_property(const std::string& name) const;
    Result<Value> get_property(const std::string& name) const;
    VoidResult set_property(const std::string& name, const Value& value) const;
    VoidResult delete_property(const std::string& name) const;

    bool is_function() const;
    Result<Value> call_as_function(
        const Value* jsi_this, const std::vector<Value>& jsi_args) const;

    bool is_constructor() const;
    Result<Object> call_as_constructor(
        const std::vector<Value>& arguments) const;

    bool is_array() const;
    Result<Value> get_property_at_index(size_t index) const;
    VoidResult set_property_at_index(size_t index, const Value& value) const;
};

class Class : public Pointer {
  public:
    using Pointer::Pointer;
};

using Function = std::function<Result<Value>(Value&, std::vector<Value>&)>;

using ClassPropertyGetter = std::function<Result<Value>(Object&)>;

using ClassPropertySetter = std::function<Result<bool>(Object&, Value&)>;

struct ClassPropertyDefinition {
    ClassPropertyGetter get;
    ClassPropertySetter set;
};

using ClassFinalizer = std::function<void(const Object&)>;

struct ClassDefinition {
    std::string name;
    std::optional<Class> base_class = std::nullopt;
    std::map<std::string, Function> methods;
    std::map<std::string, ClassPropertyDefinition> properties;
    ClassFinalizer finalizer;
};

class Context {
  public:
    void* user_pointer;

    virtual Result<Value> eval(
        const std::string& source,
        Object* this_obj,
        const std::string& source_url) = 0;
    virtual void garbage_collect() = 0;
    virtual Object get_global_object() = 0;

    // String
    virtual String string_make_from_utf8(const std::string& str) = 0;
    virtual std::string string_to_utf8(const String&) = 0;

    // Value
    virtual Value value_make_bool(bool value) = 0;
    virtual Value value_make_number(double value) = 0;
    virtual Value value_make_null() = 0;
    virtual Value value_make_undefined() = 0;
    virtual Value value_make_string(const String& str) = 0;
    virtual Value value_make_object(const Object& object) = 0;
    
    virtual WeakValue value_make_weak(const Value& value) = 0;
    virtual Value weak_value_lock(const WeakValue& value) = 0;

    virtual ValueType value_get_type(const Value& value) = 0;
    virtual Result<bool> value_to_bool(const Value& value) = 0;
    virtual Result<double> value_to_number(const Value& value) = 0;
    virtual Result<String> value_to_string(const Value& value) = 0;
    virtual Result<Object> value_to_object(const Value& value) = 0;

    virtual bool value_strict_equal(const Value& a, const Value& b) = 0;

    // Error
    virtual bool value_is_error(const Value& value) = 0;
    virtual Value value_make_error(const std::string& message) = 0;
    virtual std::optional<ErrorLocation> value_get_error_location(
        const Value& value) = 0;

    // Class
    virtual Class class_make(const ClassDefinition& definition) = 0;

    // Object
    virtual Object object_make(const Class* js_class) = 0;
    virtual Object object_make_function(const Function& function) = 0;
    virtual Object object_make_constructor(const Class& js_class) = 0;
    virtual Object object_make_constructor2(
        const Class& js_class, const Function& function) = 0;
    virtual Object object_make_array() = 0;

    virtual Value object_to_value(const Object& object) = 0;

    virtual void object_set_private_data(const Object& object, void* data) = 0;
    virtual void* object_get_private_data(const Object& object) = 0;

    virtual Result<Value> object_get_prototype(const Object& object) = 0;
    virtual VoidResult object_set_prototype(
        const Object& object, const Value& prototype) = 0;

    virtual std::vector<std::string> object_get_property_names(
        const Object& object) = 0;
    virtual bool object_has_property(
        const Object& object, const std::string& name) = 0;
    virtual Result<Value> object_get_property(
        const Object& object, const std::string& name) = 0;
    virtual VoidResult object_delete_property(
        const Object& object, const std::string& name) = 0;
    virtual VoidResult object_set_property(
        const Object& object, const std::string& name, const Value& value) = 0;

    virtual bool object_is_function(const Object& object) = 0;
    virtual Result<Value> object_call_as_function(
        const Object& object,
        const Value* jsi_this,
        const std::vector<Value>& jsi_args) = 0;

    virtual bool object_is_constructor(const Object& object) = 0;
    virtual Result<Object> object_call_as_constructor(
        const Object& object, const std::vector<Value>& arguments) = 0;

    virtual bool object_is_array(const Object& object) = 0;
    virtual Result<Value> object_get_property_at_index(
        const Object& object, size_t index) = 0;
    virtual Result<void> object_set_property_at_index(
        const Object& object, size_t index, const Value& value) = 0;
};

}  // namespace aardvark::jsi
