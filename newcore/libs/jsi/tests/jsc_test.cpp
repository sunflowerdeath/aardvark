#include <Catch2/catch.hpp>

#ifdef ADV_JSI_JSC
#include <aardvark_jsi/jsc.hpp>
#endif

#ifdef ADV_JSI_QJS
#include <aardvark_jsi/qjs.hpp>
#endif

using namespace aardvark::jsi;

TEMPLATE_TEST_CASE(
    "Context", "[Context]"
#ifdef ADV_JSI_QJS
    ,Qjs_Context
#endif
#ifdef ADV_JSI_JSC
    ,Jsc_Context
#endif
) {
    auto create_context = []() { return TestType::create(); };

    SECTION("context eval") {
        auto ctx = create_context();

        auto res = ctx->eval_script("2 + 3", nullptr, "source_url");
        REQUIRE(res.to_number() == 5);
    }

    SECTION("context eval this") {
        auto ctx = create_context();

        auto this_obj = ctx->object_make(nullptr);
        this_obj.set_property("a", ctx->value_make_number(2));
        auto res = ctx->eval_script("this.a + 3", &this_obj, "source_url");
        REQUIRE(res.to_number() == 5);
    }

    SECTION("context eval exception") {
        auto ctx = create_context();

        auto did_throw = false;
        try {
            ctx->eval_script("a/b/", nullptr, "source_url");
        } catch (JsError& ex) {
            did_throw = true;
        }
        REQUIRE(did_throw);
    }

    SECTION("string") {
        auto ctx = create_context();

        auto str = ctx->string_make_from_utf8("test");
        REQUIRE(str.to_utf8() == "test");
    }

    SECTION("value") {
        auto ctx = create_context();

        auto null_val = ctx->value_make_null();
        REQUIRE(null_val.get_type() == ValueType::null);

        auto undef_val = ctx->value_make_undefined();
        REQUIRE(undef_val.get_type() == ValueType::undefined);

        auto num_val = ctx->value_make_number(1.5);
        REQUIRE(num_val.to_number() == 1.5);
        REQUIRE(num_val.get_type() == ValueType::number);

        auto bool_val = ctx->value_make_bool(true);
        REQUIRE(bool_val.to_bool());
        REQUIRE(bool_val.get_type() == ValueType::boolean);

        auto str = ctx->string_make_from_utf8("test");
        auto str_val = ctx->value_make_string(str);
        REQUIRE(str_val.get_type() == ValueType::string);
        REQUIRE(str_val.to_string().to_utf8() == "test");
    }

    SECTION("object props") {
        auto ctx = create_context();

        auto obj = ctx->object_make(nullptr);
        auto val = ctx->value_make_number(1);
        obj.set_property("a", val);
        REQUIRE(obj.has_property("a") == true);
        REQUIRE(obj.has_property("b") == false);
        auto prop_names = obj.get_property_names();
        REQUIRE(prop_names.size() == 1);
        REQUIRE(prop_names[0] == "a");
        REQUIRE(obj.get_property("a").to_number() == 1);
        obj.delete_property("a");
        REQUIRE(obj.has_property("a") == false);
    }

    SECTION("object proto") {
        auto ctx = create_context();

        auto obj = ctx->object_make(nullptr);
        auto proto = ctx->object_make(nullptr);
        auto proto_val = ctx->value_make_object(proto);

        obj.set_prototype(proto_val);
        auto curr_proto = obj.get_prototype();
        REQUIRE(curr_proto.get_type() == ValueType::object);
        REQUIRE(curr_proto.strict_equal_to(proto_val));
    }

    SECTION("object function") {
        auto ctx = create_context();

        auto is_called = false;
        auto out_this = std::optional<Value>();
        auto out_args = std::vector<Value>();
        auto in_ret_val = ctx->value_make_object(ctx->object_make(nullptr));
        auto func = [&](const Value& xthis, const std::vector<Value>& args) {
            is_called = true;
            out_this = xthis;
            out_args = args;
            return in_ret_val;
        };
        auto obj = ctx->object_make_function(func);

        REQUIRE(obj.is_function() == true);
        auto in_this = ctx->value_make_object(ctx->object_make(nullptr));
        auto in_arg = ctx->value_make_object(ctx->object_make(nullptr));
        auto out_ret_val = obj.call_as_function(&in_this, {in_arg});
        REQUIRE(is_called);
        REQUIRE(out_this->strict_equal_to(in_this));
        REQUIRE(out_args.size() == 1);
        REQUIRE(out_args[0].strict_equal_to(in_arg));
    }

    SECTION("object function exception") {
        auto ctx = create_context();

        auto func = [&](const Value& xthis, const std::vector<Value>& args) {
            return ctx->eval_script("a/b", nullptr, "sourceurl");
        };
        auto obj = ctx->object_make_function(func);

        auto did_throw = false;
        try {
            obj.call_as_function(nullptr, {});
        } catch (JsError& ex) {
            did_throw = true;
        }
        REQUIRE(did_throw);
    }

    SECTION("object private") {
        auto ctx = create_context();

        auto definition = ClassDefinition();
        definition.name = "TestClass";
        auto cls = ctx->class_create(definition);
        auto instance = ctx->object_make(&cls);
        auto data = 25;
        instance.set_private_data(static_cast<void*>(&data));
        auto get_data = instance.template get_private_data<int>(); // WTF
        REQUIRE(get_data == 25);
    }

    SECTION("object class") {
        auto ctx = create_context();

        auto prop_value = 1;
        auto get = [&](Object& object) {
            return ctx->value_make_number(prop_value);
        };
        auto set = [&](Object& object, Value& value) {
            prop_value = value.to_number();
            return true;
        };
        auto method = [&](Value js_this, std::vector<Value>& args) {
            prop_value = args[0].to_number();
            return args[0];
        };
        auto finalizer_called = false;
        auto finalizer = [&](const Object& object) { finalizer_called = true; };

        auto definition = ClassDefinition();
        definition.name = "TestClass";
        definition.methods = {{"method", method}};
        definition.properties = {{"prop", ClassPropertyDefinition{get, set}}};
        definition.finalizer = finalizer;
        auto cls = ctx->class_create(definition);
        auto instance = ctx->object_make(&cls);

        auto getter_res = instance.get_property("prop");
        REQUIRE(getter_res.to_number() == 1);

        instance.set_property("prop", ctx->value_make_number(2));
        REQUIRE(prop_value == 2);

        auto ret_val =
            ctx->eval_script("this.method(3)", &instance, "sourceurl");
        REQUIRE(prop_value == 3);
        REQUIRE(ret_val.to_number() == 3);

        ctx.reset();
        REQUIRE(finalizer_called == true);
    }

    SECTION("object class exceptions") {
        auto ctx = create_context();

        auto get = [&](Object& object) {
            return ctx->eval_script("a/b", nullptr, "sourceurl");
        };
        auto set = [&](Object object, Value& value) {
            ctx->eval_script("a/b", nullptr, "sourceurl");
            return true;
        };
        auto method = [&](Value& js_this, std::vector<Value>& args) {
            return ctx->eval_script("a/b", nullptr, "sourceurl");
        };

        auto definition = new ClassDefinition();
        definition->name = "TestClass";
        definition->methods = {{"method", method}};
        definition->properties = {{"prop", ClassPropertyDefinition{get, set}}};
        auto cls = ctx->class_create(*definition);
        delete definition;  // Ensure that definition is not used
        auto instance = ctx->object_make(&cls);

        auto getter_did_throw = false;
        try {
            instance.get_property("prop");
        } catch (JsError& ex) {
            getter_did_throw = true;
        }
        REQUIRE(getter_did_throw);

        auto setter_did_throw = false;
        try {
            instance.set_property("prop", ctx->value_make_number(1));
        } catch (JsError& ex) {
            setter_did_throw = true;
        }
        REQUIRE(setter_did_throw);

        auto method_did_throw = false;
        try {
            ctx->eval_script("this.method()", &instance, "sourceurl");
        } catch (JsError& ex) {
            method_did_throw = true;
        }
        REQUIRE(method_did_throw);
    }

    SECTION("object array") {
        auto ctx = create_context();

        auto not_arr = ctx->object_make(nullptr);
        REQUIRE(not_arr.is_array() == false);

        auto arr = ctx->object_make_array();
        REQUIRE(arr.is_array() == true);

        arr.set_property_at_index(0, ctx->value_make_number(1));
        arr.set_property_at_index(1, ctx->value_make_number(2));
        REQUIRE(arr.get_property("length").to_number() == 2);
        REQUIRE(arr.get_property_at_index(1).to_number() == 2);
        REQUIRE(arr.get_property("1").to_number() == 2);
    }
}
