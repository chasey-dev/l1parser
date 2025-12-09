#include "l1parser.h"
#include "l1parser.hpp"
#include <cstring>

// The extern "C" struct definition
struct L1Context {
    L1Parser inner;
};

// Helper to return malloc'd string for C API
static char* ret_str(std::optional<std::string> s) {
    if (s.has_value()) {
        return strdup(s.value().c_str());
    }
    return nullptr;
}

static std::string safe_str(const char* s) {
    return s ? std::string(s) : std::string();
}

static char** vector_to_c_array(const std::vector<std::string>& vec, size_t* count) {
    *count = vec.size();
    if (vec.empty()) return nullptr;

    char** arr = (char**)malloc(sizeof(char*) * vec.size());
    if (!arr) {
        *count = 0;
        return nullptr;
    }

    for (size_t i = 0; i < vec.size(); ++i) {
        arr[i] = strdup(vec[i].c_str());
    }
    return arr;
}

extern "C" {

L1Context* l1_init() {
    try {
        auto* ctx = new (std::nothrow) L1Context();
        if (!ctx) return nullptr;
        
        if (!ctx->inner.load(L1_DAT_PATH)) {
            delete ctx;
            return nullptr;
        }
        return ctx;
    } catch (...) {
        return nullptr;
    }
}

void l1_free(L1Context* ctx) {
    if (ctx) {
        delete ctx;
    }
}

void l1_free_str_array(char** arr, size_t count) {
    if (!arr) return;
    for (size_t i = 0; i < count; ++i) {
        if (arr[i]) free(arr[i]);
    }
    free(arr);
}

char* l1_get(L1Context* ctx, const char* dev, const char* key) {
    if (!ctx) return nullptr;
    return L1_GUARD(ret_str(ctx->inner.get_prop(safe_str(dev), safe_str(key))));
}

char** l1_list(L1Context* ctx, size_t* count) {
    if (!ctx || !count) return nullptr;
    return L1_GUARD(vector_to_c_array(ctx->inner.list_devs(), count));
}

char* l1_if2zone(L1Context* ctx, const char* ifname) {
    if (!ctx) return nullptr;
    return L1_GUARD(ret_str(ctx->inner.if2zone(safe_str(ifname))));
}

char* l1_if2dat(L1Context* ctx, const char* ifname) {
    if (!ctx) return nullptr;
    return L1_GUARD(ret_str(ctx->inner.if2dat(safe_str(ifname))));
}

char** l1_zone2if(L1Context* ctx, const char* zone, size_t* count) {
    if (!ctx || !count || !zone) return nullptr;
    return L1_GUARD(vector_to_c_array(ctx->inner.zone2if(safe_str(zone)), count));
}

char* l1_if2dbdcidx(L1Context* ctx, const char* ifname) {
    if (!ctx) return nullptr;
    return L1_GUARD(ret_str(ctx->inner.if2dbdcidx(safe_str(ifname))));
}

char* l1_idx2if(L1Context* ctx, size_t idx) {
    if (!ctx) return nullptr;
    return L1_GUARD(ret_str(ctx->inner.idx2if(idx)));
}

/* C API for libiwinfo */
char* l1_get_chip_id_by_devname(L1Context* ctx, const char* dev) {
    if (!ctx) return nullptr;
    return L1_GUARD(ret_str(ctx->inner.get_prop(safe_str(dev), "INDEX")));
}

char* l1_get_chip_id_by_ifname(L1Context* ctx, const char* ifname) {
    try {
        if (!ctx) return nullptr;
        const auto& map = ctx->inner.get_if_map();
        auto it = map.find(safe_str(ifname));
        if (it != map.end()) {
            auto pit = it->second.props.find("INDEX");
            if (pit != it->second.props.end()) {
                return strdup(pit->second.c_str());
            }
        }
        return nullptr;
    } catch (...) { return nullptr; }
}

} // extern "C"