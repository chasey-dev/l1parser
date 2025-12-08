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

char* l1_get(L1Context* ctx, const char* dev, const char* key) {
    try {
        if (!ctx) return nullptr;
        return ret_str(ctx->inner.get_prop(safe_str(dev), safe_str(key)));
    } catch (...) { return nullptr; }
}

char* l1_list(L1Context* ctx) {
    try {
        if (!ctx) return nullptr;
        return strdup(ctx->inner.list_devs().c_str());
    } catch (...) { return nullptr; }
}

char* l1_if2zone(L1Context* ctx, const char* ifname) {
    try {
        if (!ctx) return nullptr;
        return ret_str(ctx->inner.if2zone(safe_str(ifname)));
    } catch (...) { return nullptr; }
}

char* l1_if2dat(L1Context* ctx, const char* ifname) {
    try {
        if (!ctx) return nullptr;
        return ret_str(ctx->inner.if2dat(safe_str(ifname)));
    } catch (...) { return nullptr; }
}

char* l1_zone2if(L1Context* ctx, const char* zone) {
    try {
        if (!ctx) return nullptr;
        return ret_str(ctx->inner.zone2if(safe_str(zone)));
    } catch (...) { return nullptr; }
}

char* l1_if2dbdcidx(L1Context* ctx, const char* ifname) {
    try {
        if (!ctx) return nullptr;
        return ret_str(ctx->inner.if2dbdcidx(safe_str(ifname)));
    } catch (...) { return nullptr; }
}

char* l1_idx2if(L1Context* ctx, int idx) {
    try {
        if (!ctx) return nullptr;
        return ret_str(ctx->inner.idx2if(idx));
    } catch (...) { return nullptr; }
}

/* C API for libiwinfo */
char* l1_get_chip_id_by_devname(L1Context* ctx, const char* dev) {
    try {
        if (!ctx) return nullptr;
        return ret_str(ctx->inner.get_prop(safe_str(dev), "INDEX"));
    } catch (...) { return nullptr; }
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