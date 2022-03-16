#ifndef SID_PAL_MFG_STORE_IFC_H
#define SID_PAL_MFG_STORE_IFC_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum { SID_PAL_MFG_STORE_INVALID_OFFSET = UINT32_MAX };

/* Values available to all users of the manufacturing store.
 */
typedef enum {
    SID_PAL_MFG_STORE_DEVID = 1, // use sid_pal_mfg_store_dev_id_get
   /* Version is stored in network order */
    SID_PAL_MFG_STORE_VERSION = 2, // use sid_pal_mfg_store_get_version
    SID_PAL_MFG_STORE_SERIAL_NUM = 3, // use sid_pal_mfg_store_dev_id_get
    SID_PAL_MFG_STORE_SMSN = 4,
    SID_PAL_MFG_STORE_APP_PUB_ED25519 = 5,
    SID_PAL_MFG_STORE_DEVICE_PRIV_ED25519 = 6,
    SID_PAL_MFG_STORE_DEVICE_PUB_ED25519 = 7,
    SID_PAL_MFG_STORE_DEVICE_PUB_ED25519_SIGNATURE = 8,
    SID_PAL_MFG_STORE_DEVICE_PRIV_P256R1 = 9,
    SID_PAL_MFG_STORE_DEVICE_PUB_P256R1 = 10,
    SID_PAL_MFG_STORE_DEVICE_PUB_P256R1_SIGNATURE = 11,
    SID_PAL_MFG_STORE_DAK_PUB_ED25519 = 12,
    SID_PAL_MFG_STORE_DAK_PUB_ED25519_SIGNATURE = 13,
    SID_PAL_MFG_STORE_DAK_ED25519_SERIAL = 14,
    SID_PAL_MFG_STORE_DAK_PUB_P256R1 = 15,
    SID_PAL_MFG_STORE_DAK_PUB_P256R1_SIGNATURE = 16,
    SID_PAL_MFG_STORE_DAK_P256R1_SERIAL = 17,
    SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519 = 18,
    SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519_SIGNATURE = 19,
    SID_PAL_MFG_STORE_PRODUCT_ED25519_SERIAL = 20,
    SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1 = 21,
    SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1_SIGNATURE = 22,
    SID_PAL_MFG_STORE_PRODUCT_P256R1_SERIAL = 23,
    SID_PAL_MFG_STORE_MAN_PUB_ED25519 = 24,
    SID_PAL_MFG_STORE_MAN_PUB_ED25519_SIGNATURE = 25,
    SID_PAL_MFG_STORE_MAN_ED25519_SERIAL = 26,
    SID_PAL_MFG_STORE_MAN_PUB_P256R1 = 27,
    SID_PAL_MFG_STORE_MAN_PUB_P256R1_SIGNATURE = 28,
    SID_PAL_MFG_STORE_MAN_P256R1_SERIAL = 29,
    SID_PAL_MFG_STORE_SW_PUB_ED25519 = 30,
    SID_PAL_MFG_STORE_SW_PUB_ED25519_SIGNATURE = 31,
    SID_PAL_MFG_STORE_SW_ED25519_SERIAL = 32,
    SID_PAL_MFG_STORE_SW_PUB_P256R1 = 33,
    SID_PAL_MFG_STORE_SW_PUB_P256R1_SIGNATURE = 34,
    SID_PAL_MFG_STORE_SW_P256R1_SERIAL = 35,
    SID_PAL_MFG_STORE_AMZN_PUB_ED25519 = 36,
    SID_PAL_MFG_STORE_AMZN_PUB_P256R1 = 37,
    SID_PAL_MFG_STORE_APID = 38,

    // This arbitrary value is the number of value identifiers
    // reserved by Sidewalk. The range of these value identifiers is:
    // [0, SID_PAL_MFG_STORE_CORE_VALUE_MAX].
    // Applications may use identifiers outside of that range.
    SID_PAL_MFG_STORE_CORE_VALUE_MAX = 4000
} sid_pal_mfg_store_value_t;


/* Value sizes in bytes */
typedef enum {
    SID_PAL_MFG_STORE_DEVID_SIZE                        = 5,
    SID_PAL_MFG_STORE_VERSION_SIZE                      = 4,
    SID_PAL_MFG_STORE_SERIAL_NUM_SIZE                   = 17,
    SID_PAL_MFG_STORE_SMSN_SIZE                         = 32,
    SID_PAL_MFG_STORE_APP_PUB_ED25519_SIZE              = 32,
    SID_PAL_MFG_STORE_DEVICE_PRIV_ED25519_SIZE          = 32,
    SID_PAL_MFG_STORE_DEVICE_PUB_ED25519_SIZE           = 32,
    SID_PAL_MFG_STORE_DEVICE_PUB_ED25519_SIGNATURE_SIZE = 64,
    SID_PAL_MFG_STORE_DEVICE_PRIV_P256R1_SIZE           = 32,
    SID_PAL_MFG_STORE_DEVICE_PUB_P256R1_SIZE            = 64,
    SID_PAL_MFG_STORE_DEVICE_PUB_P256R1_SIGNATURE_SIZE  = 64,
    SID_PAL_MFG_STORE_DAK_PUB_ED25519_SIZE              = 32,
    SID_PAL_MFG_STORE_DAK_PUB_ED25519_SIGNATURE_SIZE    = 64,
    SID_PAL_MFG_STORE_DAK_ED25519_SERIAL_SIZE           = 4,
    SID_PAL_MFG_STORE_DAK_PUB_P256R1_SIZE               = 64,
    SID_PAL_MFG_STORE_DAK_PUB_P256R1_SIGNATURE_SIZE     = 64,
    SID_PAL_MFG_STORE_DAK_P256R1_SERIAL_SIZE            = 4,
    SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519_SIZE          = 32,
    SID_PAL_MFG_STORE_PRODUCT_PUB_ED25519_SIGNATURE_SIZE= 64,
    SID_PAL_MFG_STORE_PRODUCT_ED25519_SERIAL_SIZE       = 4,
    SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1_SIZE           = 64,
    SID_PAL_MFG_STORE_PRODUCT_PUB_P256R1_SIGNATURE_SIZE = 64,
    SID_PAL_MFG_STORE_PRODUCT_P256R1_SERIAL_SIZE        = 4,
    SID_PAL_MFG_STORE_MAN_PUB_ED25519_SIZE              = 32,
    SID_PAL_MFG_STORE_MAN_PUB_ED25519_SIGNATURE_SIZE    = 64,
    SID_PAL_MFG_STORE_MAN_ED25519_SERIAL_SIZE           = 4,
    SID_PAL_MFG_STORE_MAN_PUB_P256R1_SIZE               = 64,
    SID_PAL_MFG_STORE_MAN_PUB_P256R1_SIGNATURE_SIZE     = 64,
    SID_PAL_MFG_STORE_MAN_P256R1_SERIAL_SIZE            = 4,
    SID_PAL_MFG_STORE_SW_PUB_ED25519_SIZE               = 32,
    SID_PAL_MFG_STORE_SW_PUB_ED25519_SIGNATURE_SIZE     = 64,
    SID_PAL_MFG_STORE_SW_ED25519_SERIAL_SIZE            = 4,
    SID_PAL_MFG_STORE_SW_PUB_P256R1_SIZE                = 64,
    SID_PAL_MFG_STORE_SW_PUB_P256R1_SIGNATURE_SIZE      = 64,
    SID_PAL_MFG_STORE_SW_P256R1_SERIAL_SIZE             = 4,
    SID_PAL_MFG_STORE_AMZN_PUB_ED25519_SIZE             = 32,
    SID_PAL_MFG_STORE_AMZN_PUB_P256R1_SIZE              = 64,
    SID_PAL_MFG_STORE_APID_SIZE                         = 4,
} sid_pal_mfg_store_value_size_t;


#define SID_PAL_MFG_STORE_MAX_FLASH_WRITE_LEN 64


typedef uint32_t (*sid_pal_mfg_store_app_value_to_offset_t)(int value);

/* Type which holds the start and end addresses of the manufacturing store */
typedef struct {
    uintptr_t addr_start;
    uintptr_t addr_end;
    /*
     * This function allows applications to extend the manufacturing store to be
     * be used for non-Sidewalk values. Applications should provide an
     * implementation of this function if they wish to extend the manufacturing
     * store for their own use. Its responsibility is to convert a value identifier
     * to an offset (in bytes) from the beginning of the manufacturing store.
     * Sidewalk owns the first SID_PAL_MFG_STORE_CORE_VALUE_MAX identifiers, So this
     * function's input should be greater than SID_PAL_MFG_STORE_CORE_VALUE_MAX and
     * its output should a valid offset below the mfg_store end address.
     *  If no mapping from the value identifier to an offset can be found, this
     *  function should return SID_PAL_MFG_STORE_INVALID_OFFSET value, which will
     *  cause the manufacturing store implementation to reject any operation on the
     *  provided value.
     */
    sid_pal_mfg_store_app_value_to_offset_t app_value_to_offset;
} sid_pal_mfg_store_region_t;


void sid_pal_mfg_store_init(sid_pal_mfg_store_region_t mfg_store_region);
int32_t sid_pal_mfg_store_erase(void);
bool sid_pal_mfg_store_is_empty(void);
int32_t sid_pal_mfg_store_write(int value, const uint8_t *buffer, uint8_t length);
void sid_pal_mfg_store_read(int value, uint8_t *buffer, uint8_t length);
uint32_t sid_pal_mfg_store_get_version(void);
bool sid_pal_mfg_store_dev_id_get(uint8_t dev_id[SID_PAL_MFG_STORE_DEVID_SIZE]);
bool sid_pal_mfg_store_serial_num_get(uint8_t serial_num[SID_PAL_MFG_STORE_SERIAL_NUM_SIZE]);

#endif
