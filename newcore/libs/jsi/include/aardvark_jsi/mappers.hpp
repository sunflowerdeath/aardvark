#pragma once

#include <fmt/format.h>

#include <string>
#include <tl/expected.hpp>
#include <unordered_map>
#include <unordered_set>
#include <variant>

#include "check.hpp"
#include "jsi.hpp"

namespace aardvark::jsi {

template <typename T>
class Mapper {
  public:
    virtual Value to_js(Context& ctx, const T& value) = 0;
    virtual T from_js(Context& ctx, const Value& value) = 0;
    virtual tl::expected<T, std::string> try_from_js(
        Context& ctx,
        const Value& value,
        const CheckErrorParams& err_params) = 0;
};

template <typename T>
using ToJsCallback = std::function<Value(Context&, const T&)>;

template <typename T>
using TryFromJsCallback = std::function<tl::expected<T, std::string>(
    Context&, const Value&, const CheckErrorParams& err_params)>;

template <typename T>
class SimpleMapper : public Mapper<T> {
  public:
    SimpleMapper(ToJsCallback<T> to_js_cb, TryFromJsCallback<T> try_from_js_cb)
        : to_js_cb(to_js_cb), try_from_js_cb(try_from_js_cb){};

    Value to_js(Context& ctx, const T& value) override {
        return to_js_cb(ctx, value);
    }

    T from_js(Context& ctx, const Value& value) override {
        auto err_params = CheckErrorParams{};
        return try_from_js_cb(ctx, value, err_params).value();
    }

    tl::expected<T, std::string> try_from_js(
        Context& ctx,
        const Value& val,
        const CheckErrorParams& err_params) override {
        return try_from_js_cb(ctx, val, err_params);
    }

  private:
    ToJsCallback<T> to_js_cb;
    TryFromJsCallback<T> try_from_js_cb;
};

template <typename T>
class EnumMapper : public Mapper<T> {
    using UnderlyingMapper = Mapper<std::underlying_type_t<T>>;

  public:
    EnumMapper(UnderlyingMapper* mapper) : mapper(mapper){};

    Value to_js(Context& ctx, const T& value) override {
        return mapper->to_js(
            ctx, static_cast<std::underlying_type_t<T>>(value));
    }

    T from_js(Context& ctx, const Value& value) override {
        return static_cast<T>(mapper->from_js(ctx, value));
    }

    tl::expected<T, std::string> try_from_js(
        Context& ctx,
        const Value& value,
        const CheckErrorParams& err_params) override {
        auto res = mapper->try_from_js(ctx, value, err_params);
        return res.map([](auto value) { return static_cast<T>(value); });
    }

  private:
    UnderlyingMapper* mapper;
};

template <class T>
class ObjectsMapper : public Mapper<std::shared_ptr<T>> {
    using ClassGetter = std::function<Class(T*)>;

  public:
    ObjectsMapper(
        std::string type_name, std::variant<Class, ClassGetter> js_class)
        : type_name(type_name), js_class(js_class){};

    Value to_js(
        Context& ctx, const std::shared_ptr<T>& native_object) override {
        auto it = records_map.find(native_object.get());
        if (it != records_map.end()) {
            return it->second.js_value;
        } else {
            return create_js_value(ctx, native_object);
        }
    }

    std::shared_ptr<T> from_js(Context& ctx, const Value& value) override {
        auto record = get_record(value);
        if (records_set.find(record) == records_set.end()) return nullptr;
        return record->native_object;
    }

    tl::expected<std::shared_ptr<T>, std::string> try_from_js(
        Context& ctx,
        const Value& value,
        const CheckErrorParams& err_params) override {
        auto native_object = from_js(ctx, value);
        if (native_object == nullptr) {
            auto error = fmt::format(
                "Invalid {} `{}` supplied to `{}`, expected `{}`.",
                err_params.kind,
                err_params.name,
                err_params.target,
                type_name);
            return tl::make_unexpected(error);
        } else {
            return native_object;
        }
    }

    static void finalize(const Value& value) {
        auto record = get_record(value);
        if (record == nullptr) return;
        auto index = record->index;
        index->records_map.erase(record->native_object.get());
        index->records_set.erase(record);
    }

  private:
    struct Record {
        std::shared_ptr<T> native_object;
        Value js_value;
        ObjectsMapper<T>* index;
    };

    std::string type_name;
    std::variant<Class, ClassGetter> js_class;
    std::unordered_map<T*, Record> records_map;
    std::unordered_set<Record*> records_set;

    Value create_js_value(
        Context& ctx, const std::shared_ptr<T>& native_object) {
        auto ptr = native_object.get();
        auto the_js_class = std::holds_alternative<Class>(js_class)
                                ? std::get<Class>(js_class)
                                : std::get<ClassGetter>(js_class)(ptr);
        auto js_object = ctx.object_make(&the_js_class);
        auto js_value = js_object.to_value();
        auto res =
            records_map.emplace(ptr, Record{native_object, js_value, this});
        auto record_ptr = &(res.first->second);
        records_set.insert(record_ptr);
        js_object.set_private_data(static_cast<void*>(record_ptr));
        return js_value;
    }

    static Record* get_record(const Value& value) {
        return static_cast<Record*>(
            value.to_object().value().get_private_data());
    }
};

extern Mapper<bool>* bool_mapper;
extern Mapper<double>* double_mapper;
extern Mapper<int>* int_mapper;
extern Mapper<std::string>* string_mapper;

}  // namespace aardvark::jsi
