#pragma once

#include <aardvark_jsi/jsi.hpp>
#include <aardvark_jsi/mappers.hpp>

using namespace aardvark::jsi;

struct ProxyClass {
  public:
    int prop = 1;
    int method() { return 1; }
};

Result<Value> class_prop_get_proxy(
    Context& ctx, std::shared_ptr<ProxyClass>& this_val, Mapper<int>& mapper);

Result<bool> class_prop_set_proxy(
    Context& ctx,
    std::shared_ptr<ProxyClass>& this_val,
    Value& val,
    Mapper<int>& mapper,
    CheckErrorParams& err_params);

Result<Value> class_method_return_proxy(
    Context& ctx, int& val, Mapper<int>& mapper);
