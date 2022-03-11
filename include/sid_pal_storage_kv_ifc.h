#ifndef SID_PAL_STORAGE_KV_IFC_H
#define SID_PAL_STORAGE_KV_IFC_H

#include <sid_error.h>

sid_error_t sid_pal_storage_kv_init(void);
sid_error_t sid_pal_storage_kv_record_get(uint16_t group, uint16_t key, void *p_data, uint32_t len);
sid_error_t sid_pal_storage_kv_record_set(uint16_t group, uint16_t key, void const *p_data, uint32_t len);
sid_error_t sid_pal_storage_kv_record_get_len(uint16_t group, uint16_t key, uint32_t *p_len);
sid_error_t sid_pal_storage_kv_record_delete(uint16_t group, uint16_t key);
sid_error_t sid_pal_storage_kv_group_delete(uint16_t group);

#endif
