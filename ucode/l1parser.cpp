/*
 * ucode binding for l1parser
 */
#include "l1parser.hpp"

#include <vector>
#include <string>
#include <new>      // for std::nothrow
#include <cstring>  // for strerror

extern "C" {
    #include "ucode/module.h"
    #include "ucode/platform.h"
} // extern "C"

// hold cpp objects directly, use RAII
struct L1Context {
    L1Parser inner;
};

static uc_resource_type_t *l1_ctx_type;

#define err_return(err) do { \
    uc_vm_registry_set(vm, "l1parser.last_error", ucv_int64_new(err)); \
    return NULL; \
} while(0)

/* --- Resource Destructor --- */
static void close_ctx(void *ud) {
    L1Context *ctx = reinterpret_cast<L1Context *>(ud);
    if (ctx) {
        // delete will call destructor inside L1Parser
        delete ctx;
    }
}

/* --- Helper: Convert std::vector<std::string> to ucode Array --- */
static uc_value_t *
vector_to_uc_array(uc_vm_t *vm, const std::vector<std::string>& vec) {
    uc_value_t *arr = ucv_array_new(vm);
    for (const auto &str : vec) {
        ucv_array_push(arr, ucv_string_new(str.c_str()));
    }
    return arr;
}

/* --- Helper: Convert std::unordered_map<std::string, std::string> to ucode Object --- */
static uc_value_t *
map_to_uc_object(uc_vm_t *vm, const std::unordered_map<std::string, std::string>& map) {
    uc_value_t *obj = ucv_object_new(vm);
    for (const auto &kv : map) {
        // put k-v pairs into a ucode object 
        ucv_object_add(obj, kv.first.c_str(), ucv_string_new(kv.second.c_str()));
    }
    return obj;
}

/* --- Methods --- */

static uc_value_t *
uc_l1_get(uc_vm_t *vm, size_t nargs)
{
    L1Context **ctx = reinterpret_cast<L1Context **>(uc_fn_this("l1parser.context"));
    uc_value_t *dev = uc_fn_arg(0);
    uc_value_t *key = uc_fn_arg(1);

    if (!ctx || !*ctx) err_return(EBADF);
    if (ucv_type(dev) != UC_STRING || ucv_type(key) != UC_STRING) err_return(EINVAL);

    return L1_GUARD(({
        auto res = (*ctx)->inner.get_prop(ucv_string_get(dev), ucv_string_get(key));
        res.has_value() ? ucv_string_new(res.value().c_str()) : NULL;
    }));
}

static uc_value_t *
uc_l1_get_all(uc_vm_t *vm, size_t nargs) {
    L1Context **ctx = reinterpret_cast<L1Context **>(uc_fn_this("l1parser.context"));
    if (!ctx || !*ctx) err_return(EBADF);

    return L1_GUARD(({
        // root object
        uc_value_t *root = ucv_object_new(vm);
        
        const auto& devs = (*ctx)->inner.get_all();

        // every k-v pairs in all dev maps
        // here k for dev name, v for L1Entry
        for (const auto& kv : devs) {
            const std::string& dev_key = kv.first;      // e.g. "MT7981_1_1"
            const L1Entry& entry = kv.second;

            // current dev props
            uc_value_t *child_obj = map_to_uc_object(vm, entry.props);

            // root = { dev_key: {dev_props} }
            ucv_object_add(root, dev_key.c_str(), child_obj);
        }
        root;
    }));
}

static uc_value_t *
uc_l1_list(uc_vm_t *vm, size_t nargs)
{
    L1Context **ctx = reinterpret_cast<L1Context **>(uc_fn_this("l1parser.context"));
    if (!ctx || !*ctx) err_return(EBADF);

    return L1_GUARD(vector_to_uc_array(
        vm, (*ctx)->inner.list_devs()
    ));
}

static uc_value_t *
uc_l1_if2zone(uc_vm_t *vm, size_t nargs)
{
    L1Context **ctx = reinterpret_cast<L1Context **>(uc_fn_this("l1parser.context"));
    uc_value_t *val = uc_fn_arg(0);

    if (!ctx || !*ctx) err_return(EBADF);
    if (ucv_type(val) != UC_STRING) err_return(EINVAL);

    return L1_GUARD(({
        auto res = (*ctx)->inner.if2zone(ucv_string_get(val));
        res.has_value() ? ucv_string_new(res.value().c_str()) : NULL;
    }));
}

static uc_value_t *
uc_l1_if2dat(uc_vm_t *vm, size_t nargs)
{
    L1Context **ctx = reinterpret_cast<L1Context **>(uc_fn_this("l1parser.context"));
    uc_value_t *val = uc_fn_arg(0);

    if (!ctx || !*ctx) err_return(EBADF);
    if (ucv_type(val) != UC_STRING) err_return(EINVAL);

    return L1_GUARD(({
        auto res = (*ctx)->inner.if2dat(ucv_string_get(val));
        res.has_value() ? ucv_string_new(res.value().c_str()) : NULL;
    }));
}

static uc_value_t *
uc_l1_zone2if(uc_vm_t *vm, size_t nargs)
{
    L1Context **ctx = reinterpret_cast<L1Context **>(uc_fn_this("l1parser.context"));
    uc_value_t *val = uc_fn_arg(0);

    if (!ctx || !*ctx) err_return(EBADF);
    if (ucv_type(val) != UC_STRING) err_return(EINVAL);

    return L1_GUARD(vector_to_uc_array(
        vm, (*ctx)->inner.zone2if(ucv_string_get(val))
    ));
}

static uc_value_t *
uc_l1_if2dbdcidx(uc_vm_t *vm, size_t nargs)
{
    L1Context **ctx = reinterpret_cast<L1Context **>(uc_fn_this("l1parser.context"));
    uc_value_t *val = uc_fn_arg(0);

    if (!ctx || !*ctx) err_return(EBADF);
    if (ucv_type(val) != UC_STRING) err_return(EINVAL);

    return L1_GUARD(({
        auto res = (*ctx)->inner.if2dbdcidx(ucv_string_get(val));
        res.has_value() ? ucv_string_new(res.value().c_str()) : NULL;
    }));
}

static uc_value_t *
uc_l1_idx2if(uc_vm_t *vm, size_t nargs)
{
    L1Context **ctx = reinterpret_cast<L1Context **>(uc_fn_this("l1parser.context"));
    uc_value_t *idx = uc_fn_arg(0);

    if (!ctx || !*ctx) err_return(EBADF);
    if (ucv_type(idx) != UC_INTEGER) err_return(EINVAL);

    return L1_GUARD(({
        auto res = (*ctx)->inner.idx2if((size_t)ucv_int64_get(idx));
        res.has_value() ? ucv_string_new(res.value().c_str()) : NULL;
    }));
}

static uc_value_t *
uc_l1_close(uc_vm_t *vm, size_t nargs)
{
    L1Context **ctx = reinterpret_cast<L1Context **>(uc_fn_this("l1parser.context"));

    if (!ctx || !*ctx) return ucv_boolean_new(true);

    delete *ctx;
    *ctx = NULL;

    return ucv_boolean_new(true);
}

/* --- Global Open Function --- */

static uc_value_t *
uc_l1_open(uc_vm_t *vm, size_t nargs)
{
    L1Context *ctx = new (std::nothrow) L1Context();

    if (!ctx)
        return NULL;

    if (!ctx->inner.load(L1_DAT_PATH)) {
        delete ctx;
        return NULL;
    }

    return ucv_resource_new(l1_ctx_type, ctx);
}

static uc_value_t *
uc_l1_error(uc_vm_t *vm, size_t nargs)
{
	int last_error = ucv_int64_get(uc_vm_registry_get(vm, "l1parser.last_error"));

	if (last_error == 0)
		return NULL;

	uc_vm_registry_set(vm, "l1parser.last_error", ucv_int64_new(0));
	return ucv_string_new(strerror(last_error));
}

/* --- Definitions --- */

static const uc_function_list_t ctx_fns[] = {
    { "list",           uc_l1_list },
    { "get",            uc_l1_get },
    { "getall",        uc_l1_get_all },
    { "if2zone",        uc_l1_if2zone },
    { "if2dat",         uc_l1_if2dat },
    { "zone2if",        uc_l1_zone2if },
    { "if2dbdcidx",     uc_l1_if2dbdcidx },
    { "idx2if",         uc_l1_idx2if },
    { "close",          uc_l1_close },
};

static const uc_function_list_t global_fns[] = {
    { "open",           uc_l1_open },
    { "error",          uc_l1_error },
};

extern "C" {
    void uc_module_init(uc_vm_t *vm, uc_value_t *scope)
    {
        uc_function_list_register(scope, global_fns);
        l1_ctx_type = uc_type_declare(vm, "l1parser.context", ctx_fns, close_ctx);
    }
} // extern "C"