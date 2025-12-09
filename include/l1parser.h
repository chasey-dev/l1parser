#ifndef _MTK_L1PARSER_H
#define _MTK_L1PARSER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Opaque pointer to the C++ class */
typedef struct L1Context L1Context;

/* Initialization and Cleanup */
L1Context* l1_init();
void l1_free(L1Context* ctx);
void l1_free_str_array(char** arr, size_t count);

/* Core API. Returned char* is strictly owned by the caller and must be freed using free() */
char* l1_get(L1Context* ctx, const char* dev, const char* key);
char** l1_list(L1Context* ctx, size_t* count);
char* l1_if2zone(L1Context* ctx, const char* ifname);
char* l1_if2dat(L1Context* ctx, const char* ifname);
char** l1_zone2if(L1Context* ctx, const char* zone, size_t* count);
char* l1_if2dbdcidx(L1Context* ctx, const char* ifname);
char* l1_idx2if(L1Context* ctx, int idx);

/* Helper functions for iwinfo */
char* l1_get_chip_id_by_devname(L1Context* ctx, const char* dev);
char* l1_get_chip_id_by_ifname(L1Context* ctx, const char* ifname);

#ifdef __cplusplus
}
#endif

#endif