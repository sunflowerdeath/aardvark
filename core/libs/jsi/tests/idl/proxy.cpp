#include "proxy.hpp"

Result<Value> class_prop_get_proxy(
    Context& ctx, std::shared_ptr<ProxyClass>& this_val, Mapper<int>& mapper) {
    return ctx.value_make_number(2);
}

Result<bool> class_prop_set_proxy(
    Context& ctx,
    std::shared_ptr<ProxyClass>& this_val,
    Value& val,
    Mapper<int>& mapper) {
    this_val->prop = 2;
    return true;
}

Value class_method_return_proxy(Context& ctx, Value& val, Mapper<int>& mapper) {
    return ctx.value_make_number(2);
}
