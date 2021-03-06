#pragma once

#include <fmt/format.h>

#include <string>
#include <tl/expected.hpp>
#include <typeindex>
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
        : to_js_cb(std::move(to_js_cb)),
          try_from_js_cb(std::move(try_from_js_cb)){};

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
        : type_name(std::move(type_name)), js_class(std::move(js_class)){};

    Value to_js(
        Context& ctx, const std::shared_ptr<T>& native_object) override {
        auto it = records.find(native_object.get());
        if (it != records.end()) {
            return it->second.js_value.lock();
        } else {
            return create_js_value(ctx, native_object);
        }
    }

    Value create_js_value(
        Context& ctx, const std::shared_ptr<T>& native_object) {
        auto ptr = native_object.get();
        auto the_js_class = std::holds_alternative<Class>(js_class)
                                ? std::get<Class>(js_class)
                                : std::get<ClassGetter>(js_class)(ptr);
        auto js_obj = ctx.object_make(&the_js_class);
        auto js_val = js_obj.to_value();
        auto res = records.emplace(
            ptr, Record{native_object, js_val.make_weak(), this});
        auto record_ptr = &(res.first->second);
        js_obj.set_private_data(static_cast<void*>(record_ptr));
        return js_val;
    }

    std::shared_ptr<T> from_js(Context& ctx, const Value& value) override {
        auto record = get_record(value);
        return record->native_object;
    }

    tl::expected<std::shared_ptr<T>, std::string> try_from_js(
        Context& ctx,
        const Value& value,
        const CheckErrorParams& err_params) override {
        auto err = check_type(ctx, value, "object", err_params);
        if (err.has_value()) return tl::make_unexpected(err.value());
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
        index->records.erase(record->native_object.get());
    }

  private:
    struct Record {
        std::shared_ptr<T> native_object;
        WeakValue js_value;
        ObjectsMapper<T>* index;
    };

    std::string type_name;
    std::variant<Class, ClassGetter> js_class;
    std::unordered_map<T*, Record> records;

    static Record* get_record(const Value& value) {
        auto ptr = value.to_object().value().get_private_data();
        return static_cast<Record*>(ptr);
    }
};

extern Mapper<bool>* bool_mapper;
extern Mapper<float>* float_mapper;
extern Mapper<double>* double_mapper;
extern Mapper<int>* int_mapper;
extern Mapper<std::string>* string_mapper;

template <typename T>
class ObjectsIndex {
  public:
    ObjectsIndex(
        std::string type_name,
        std::unordered_map<std::type_index, Class>* class_map)
        : type_name(std::move(type_name)), class_map(class_map){};

    Value to_js(Context& ctx, const std::shared_ptr<T>& native_object) {
        auto it = records.find(native_object.get());
        if (it != records.end()) {
            return it->second.js_value.lock();
        } else {
            return create_js_value(ctx, native_object);
        }
    }

    Value create_js_value(
        Context& ctx, const std::shared_ptr<T>& native_object) {
        auto ptr = native_object.get();
        auto typeidx = std::type_index(typeid(*ptr));
        auto js_class_rec = class_map->find(typeidx);
        // TODO better solution
        auto& js_class =
            js_class_rec != class_map->end()
                ? js_class_rec->second
                : class_map->find(std::type_index(typeid(T)))->second;
        auto js_obj = ctx.object_make(&js_class);
        auto js_val = js_obj.to_value();
        auto res = records.emplace(
            ptr, Record{native_object, js_val.make_weak(), this});
        auto record_ptr = &(res.first->second);
        js_obj.set_private_data(static_cast<void*>(record_ptr));
        return js_val;
    }

    template <typename DerivedT>
    std::shared_ptr<DerivedT> from_js(Context& ctx, const Value& value) {
        auto rec = get_record(value);
        if (rec == nullptr) return nullptr;
        return std::dynamic_pointer_cast<DerivedT>(rec->native_object);
    }

    template <typename DerivedT>
    tl::expected<std::shared_ptr<DerivedT>, std::string> try_from_js(
        Context& ctx, const Value& value, const CheckErrorParams& err_params) {
        auto err = check_type(ctx, value, "object", err_params);
        if (err.has_value()) return tl::make_unexpected(err.value());
        auto native_object = from_js<DerivedT>(ctx, value);
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
        index->records.erase(record->native_object.get());
    }

    std::unordered_map<std::type_index, Class>* class_map;

  private:
    struct Record {
        std::shared_ptr<T> native_object;
        WeakValue js_value;
        ObjectsIndex<T>* index;
    };

    std::string type_name; // TODO
    std::unordered_map<T*, Record> records;

    static Record* get_record(const Value& value) {
        auto ptr = value.to_object().value().get_private_data();
        return static_cast<Record*>(ptr);
    }
};

template <typename T, typename BaseT>
class ObjectsMapper2 : public Mapper<std::shared_ptr<T>> {
  public:
    ObjectsMapper2(ObjectsIndex<BaseT>* index) : index(index) {};
    
    Value to_js(
        Context& ctx, const std::shared_ptr<T>& native_object) override {
        if (native_object == nullptr) return ctx.value_make_undefined();
        return index->to_js(ctx, native_object);
    }

    std::shared_ptr<T> from_js(Context& ctx, const Value& value) override {
        return index->template from_js<T>(ctx, value);
    }
    
    tl::expected<std::shared_ptr<T>, std::string> try_from_js(
        Context& ctx,
        const Value& value,
        const CheckErrorParams& err_params) override {
        return index->template try_from_js<T>(ctx, value, err_params);
    }
    
    ObjectsIndex<BaseT>* index;
  private:
};

template <typename T>
class OptionalMapper : public Mapper<std::optional<T>> {
  public:
    OptionalMapper(Mapper<T>* mapper) : mapper(mapper){};

    Value to_js(Context& ctx, const std::optional<T>& value) override {
        if (!value.has_value()) return ctx.value_make_null();
        return mapper->to_js(ctx, value.value());
    }

    std::optional<T> from_js(Context& ctx, const Value& value) override {
        if (value.get_type() == ValueType::null) return std::nullopt;
        return static_cast<T>(mapper->from_js(ctx, value));
    }

    tl::expected<std::optional<T>, std::string> try_from_js(
        Context& ctx,
        const Value& value,
        const CheckErrorParams& err_params) override {
        if (value.get_type() == ValueType::null) return std::nullopt;
        auto res = mapper->try_from_js(ctx, value, err_params);
        return res.map([](auto value) { return static_cast<T>(value); });
    }

  private:
    Mapper<T>* mapper;
};

template <typename T>
class ArrayMapper : public Mapper<std::vector<T>> {
  public:
    ArrayMapper(Mapper<T>* mapper) : mapper(mapper){};

    Value to_js(Context& ctx, const std::vector<T>& value) override {
        auto res = ctx.object_make_array();
        for (auto i = 0; i < value.size(); i++) {
            res.set_property_at_index(i, mapper->to_js(ctx, value[i]));
        }
        return res.to_value();
    }

    std::vector<T> from_js(Context& ctx, const Value& value) override {
        auto res = std::vector<T>();
        auto obj = value.to_object().value();
        auto length = obj.get_property("length").value().to_number().value();
        for (auto i = 0; i < length; i++) {
            res.push_back(
                mapper->from_js(ctx, obj.get_property_at_index(i).value()));
        }
        return res;
    }

    tl::expected<std::vector<T>, std::string> try_from_js(
        Context& ctx,
        const Value& value,
        const CheckErrorParams& err_params) override {
        auto res = std::vector<T>();
        auto err = check_type(ctx, value, "array", err_params);
        if (err.has_value()) return tl::make_unexpected(err.value());
        auto obj = value.to_object().value();
        auto length = obj.get_property("length").value().to_number().value();
        for (auto i = 0; i < length; i++) {
            auto item_err_params = CheckErrorParams{
                err_params.kind,
                err_params.name + "[" + std::to_string(i) + "]",
                err_params.target};
            auto item_res = mapper->try_from_js(
                ctx,
                obj.get_property_at_index(i).value(),  // TODO check?
                item_err_params);
            if (item_res.has_value()) {
                res.push_back(item_res.value());
            } else {
                return tl::make_unexpected(item_res.error());
            }
        }
        return res;
    }

  private:
    Mapper<T>* mapper;
};

template <typename T>
class MapMapper : public Mapper<std::unordered_map<std::string, T>> {
  public:
    MapMapper(Mapper<T>* mapper) : mapper(mapper){};

    Value to_js(Context& ctx, const std::unordered_map<std::string, T>& value)
        override {
        auto res = ctx.object_make(nullptr);
        for (auto it : value) {
            res.set_property(it.first, mapper->to_js(ctx, it.second));
        }
        return res.to_value();
    };

    std::unordered_map<std::string, T> from_js(
        Context& ctx, const Value& value) override {
        auto res = std::unordered_map<std::string, T>();
        auto obj = value.to_object().value();
        auto prop_names = obj.get_property_names();
        for (auto& prop : prop_names) {
            res.insert(
                {prop, mapper->from_js(ctx, obj.get_property(prop).value())});
        }
        return res;
    };

    tl::expected<std::unordered_map<std::string, T>, std::string> try_from_js(
        Context& ctx,
        const Value& value,
        const CheckErrorParams& err_params) override {
        auto res = std::unordered_map<std::string, T>();
        auto err = check_type(ctx, value, "object", err_params);
        if (err.has_value()) return tl::make_unexpected(err.value());
        auto obj = value.to_object().value();
        auto prop_names = obj.get_property_names();
        for (auto& prop : prop_names) {
            auto prop_val = obj.get_property(prop).value(); // TODO check?
            auto prop_err_params = CheckErrorParams{
                err_params.kind,
                err_params.name + "." + prop,
                err_params.target};
            auto prop_res = mapper->try_from_js(ctx, prop_val, prop_err_params);
            if (prop_res.has_value()) {
                res.insert({ prop, prop_res.value() });
            } else {
                return tl::make_unexpected(prop_res.error());
            }
        }
        return res;
    };
    
  private:
    Mapper<T>* mapper;
};

}  // namespace aardvark::jsi
