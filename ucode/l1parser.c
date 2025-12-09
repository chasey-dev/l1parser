/*
 * ucode binding for l1parser
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "ucode/module.h"
#include "ucode/platform.h"
#include "l1parser.h"

static uc_resource_type_t *l1_ctx_type;

#define err_return(err) do { \
    uc_vm_registry_set(vm, "l1parser.last_error", ucv_int64_new(err)); \
    return NULL; \
} while(0)

/* --- Helper: Free string returned by C API and return ucode string --- */
static uc_value_t *
ret_string_and_free(char *str)
{
    uc_value_t *val;

    if (!str)
        return NULL;

    val = ucv_string_new(str);
    free(str); /* l1parser C API returns malloc'd strings */
    return val;
}

/* --- Helper: Free str array returned by C API and return ucode array --- */
static uc_value_t *
ret_array_and_free(char **arr, size_t count, uc_vm_t *vm)
{
    uc_value_t *uc_arr = ucv_array_new(vm);
    if (arr) {
        for (size_t i = 0; i < count; i++) {
            if (arr[i]) ucv_array_push(uc_arr, ucv_string_new(arr[i]));
        }
        l1_free_str_array(arr, count);
    }
    return uc_arr;
}

/* --- Resource Destructor --- */
static void close_ctx(void *ud)
{
    L1Context *ctx = (L1Context *)ud;
    if (ctx)
        l1_free(ctx);
}

/* --- Methods --- */

static uc_value_t *
uc_l1_get(uc_vm_t *vm, size_t nargs)
{
    L1Context **ctx = uc_fn_this("l1parser.context");
    uc_value_t *dev = uc_fn_arg(0);
    uc_value_t *key = uc_fn_arg(1);

    if (!ctx || !*ctx) err_return(EBADF);
    if (ucv_type(dev) != UC_STRING || ucv_type(key) != UC_STRING) err_return(EINVAL);

    return ret_string_and_free(l1_get(*ctx, ucv_string_get(dev), ucv_string_get(key)));
}

static uc_value_t *
uc_l1_list(uc_vm_t *vm, size_t nargs)
{
    L1Context **ctx = uc_fn_this("l1parser.context");
    
    if (!ctx || !*ctx) err_return(EBADF);

    size_t count = 0;
    char **arr = l1_list(*ctx, &count);
    return ret_array_and_free(arr, count, vm);
}

static uc_value_t *
uc_l1_if2zone(uc_vm_t *vm, size_t nargs)
{
    L1Context **ctx = uc_fn_this("l1parser.context");
    uc_value_t *val = uc_fn_arg(0);

    if (!ctx || !*ctx) err_return(EBADF);
    if (ucv_type(val) != UC_STRING) err_return(EINVAL);

    return ret_string_and_free(l1_if2zone(*ctx, ucv_string_get(val)));
}

static uc_value_t *
uc_l1_if2dat(uc_vm_t *vm, size_t nargs)
{
    L1Context **ctx = uc_fn_this("l1parser.context");
    uc_value_t *val = uc_fn_arg(0);

    if (!ctx || !*ctx) err_return(EBADF);
    if (ucv_type(val) != UC_STRING) err_return(EINVAL);

    return ret_string_and_free(l1_if2dat(*ctx, ucv_string_get(val)));
}

static uc_value_t *
uc_l1_zone2if(uc_vm_t *vm, size_t nargs)
{
    L1Context **ctx = uc_fn_this("l1parser.context");
    uc_value_t *val = uc_fn_arg(0);

    if (!ctx || !*ctx) err_return(EBADF);
    if (ucv_type(val) != UC_STRING) err_return(EINVAL);

    size_t count = 0;
    char **arr = l1_zone2if(*ctx, ucv_string_get(val), &count);
    return ret_array_and_free(arr, count, vm);
}

static uc_value_t *
uc_l1_if2dbdcidx(uc_vm_t *vm, size_t nargs)
{
    L1Context **ctx = uc_fn_this("l1parser.context");
    uc_value_t *val = uc_fn_arg(0);

    if (!ctx || !*ctx) err_return(EBADF);
    if (ucv_type(val) != UC_STRING) err_return(EINVAL);

    return ret_string_and_free(l1_if2dbdcidx(*ctx, ucv_string_get(val)));
}

static uc_value_t *
uc_l1_idx2if(uc_vm_t *vm, size_t nargs)
{
    L1Context **ctx = uc_fn_this("l1parser.context");
    uc_value_t *idx = uc_fn_arg(0);

    if (!ctx || !*ctx) err_return(EBADF);
    if (ucv_type(idx) != UC_INTEGER) err_return(EINVAL);

    return ret_string_and_free(l1_idx2if(*ctx, (int)ucv_int64_get(idx)));
}

static uc_value_t *
uc_l1_close(uc_vm_t *vm, size_t nargs)
{
    L1Context **ctx = uc_fn_this("l1parser.context");

    if (!ctx || !*ctx) return ucv_boolean_new(true);

    l1_free(*ctx);
    *ctx = NULL;

    return ucv_boolean_new(true);
}

/* --- Global Open Function --- */

static uc_value_t *
uc_l1_open(uc_vm_t *vm, size_t nargs)
{
    L1Context *ctx = l1_init();

    if (!ctx)
        return NULL;

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

void uc_module_init(uc_vm_t *vm, uc_value_t *scope)
{
    uc_function_list_register(scope, global_fns);
    l1_ctx_type = uc_type_declare(vm, "l1parser.context", ctx_fns, close_ctx);
}