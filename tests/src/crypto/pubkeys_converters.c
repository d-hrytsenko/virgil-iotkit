#include <virgil/iot/tests/helpers.h>
#include <virgil/iot/converters/crypto_format_converters.h>

static const uint8_t virgil_SECP192R1_pubkey[] = {
        0x30, 0x49, 0x30, 0x13, 0x06, 0x07, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x02, 0x01, 0x06, 0x08,
        0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x03, 0x01, 0x01, 0x03, 0x32, 0x00, 0x04, 0x10, 0xB2, 0x9F,
        0xFA, 0x22, 0x5E, 0x36, 0x63, 0x82, 0xD4, 0xB5, 0xA4, 0xCE, 0x26, 0x6A, 0x2F, 0xB9, 0x72,
        0xF2, 0x17, 0x2D, 0xEA, 0xCB, 0x31, 0x65, 0x76, 0x5E, 0x6D, 0x1C, 0x72, 0xB5, 0x03, 0x00,
        0xE6, 0xD1, 0x28, 0x92, 0x0A, 0xEC, 0xB0, 0x62, 0x68, 0x48, 0x51, 0x40, 0x11, 0x91, 0x02};
static const uint8_t raw_SECP192R1_pubkey[] = {
        0x04, 0x10, 0xB2, 0x9F, 0xFA, 0x22, 0x5E, 0x36, 0x63, 0x82, 0xD4, 0xB5, 0xA4, 0xCE, 0x26, 0x6A, 0x2F,
        0xB9, 0x72, 0xF2, 0x17, 0x2D, 0xEA, 0xCB, 0x31, 0x65, 0x76, 0x5E, 0x6D, 0x1C, 0x72, 0xB5, 0x03, 0x00,
        0xE6, 0xD1, 0x28, 0x92, 0x0A, 0xEC, 0xB0, 0x62, 0x68, 0x48, 0x51, 0x40, 0x11, 0x91, 0x02};
static const uint8_t virgil_SECP224R1_pubkey[] = {
        0x30, 0x4E, 0x30, 0x10, 0x06, 0x07, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x02, 0x01, 0x06, 0x05, 0x2B,
        0x81, 0x04, 0x00, 0x21, 0x03, 0x3A, 0x00, 0x04, 0xFA, 0xAE, 0x30, 0xA1, 0x13, 0x50, 0x6B, 0xFD,
        0xF1, 0x8F, 0xEE, 0xA5, 0x10, 0x6D, 0x0B, 0x1E, 0xD2, 0x55, 0xD4, 0x66, 0x88, 0x96, 0x20, 0x22,
        0x7A, 0x0F, 0xD9, 0x45, 0xB5, 0xCE, 0x4A, 0xE9, 0x86, 0xD3, 0xA9, 0xE3, 0xF4, 0x85, 0x60, 0xBE,
        0x4A, 0x7E, 0x85, 0xC8, 0xC2, 0x98, 0x0C, 0xF4, 0x84, 0x81, 0x36, 0xD1, 0x38, 0x6B, 0x19, 0xC2};
static const uint8_t raw_SECP224R1_pubkey[] = {0x04, 0xFA, 0xAE, 0x30, 0xA1, 0x13, 0x50, 0x6B, 0xFD, 0xF1, 0x8F, 0xEE,
                                               0xA5, 0x10, 0x6D, 0x0B, 0x1E, 0xD2, 0x55, 0xD4, 0x66, 0x88, 0x96, 0x20,
                                               0x22, 0x7A, 0x0F, 0xD9, 0x45, 0xB5, 0xCE, 0x4A, 0xE9, 0x86, 0xD3, 0xA9,
                                               0xE3, 0xF4, 0x85, 0x60, 0xBE, 0x4A, 0x7E, 0x85, 0xC8, 0xC2, 0x98, 0x0C,
                                               0xF4, 0x84, 0x81, 0x36, 0xD1, 0x38, 0x6B, 0x19, 0xC2};
static const uint8_t virgil_SECP256R1_pubkey[] = {
        0x30, 0x59, 0x30, 0x13, 0x06, 0x07, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x02, 0x01, 0x06, 0x08, 0x2A,
        0x86, 0x48, 0xCE, 0x3D, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00, 0x04, 0x97, 0x5A, 0x89, 0x38, 0xE6,
        0xE8, 0x9A, 0x4B, 0x4D, 0x12, 0xB7, 0x18, 0x37, 0x5F, 0x91, 0xAA, 0x9D, 0xB5, 0x49, 0x8A, 0xE8,
        0x3A, 0xAD, 0x8B, 0x4C, 0x79, 0xF4, 0x94, 0x00, 0x98, 0x5F, 0xD2, 0xF8, 0xE3, 0x2C, 0x6C, 0x0B,
        0xF1, 0x8B, 0x03, 0x73, 0xD7, 0x4E, 0x75, 0xFF, 0x41, 0xEA, 0x96, 0x87, 0x7C, 0x60, 0x96, 0x65,
        0x83, 0x50, 0xF9, 0x99, 0x10, 0x13, 0x45, 0xD3, 0x44, 0x9F, 0xD5};
static const uint8_t raw_SECP256R1_pubkey[] = {
        0x04, 0x97, 0x5A, 0x89, 0x38, 0xE6, 0xE8, 0x9A, 0x4B, 0x4D, 0x12, 0xB7, 0x18, 0x37, 0x5F, 0x91, 0xAA,
        0x9D, 0xB5, 0x49, 0x8A, 0xE8, 0x3A, 0xAD, 0x8B, 0x4C, 0x79, 0xF4, 0x94, 0x00, 0x98, 0x5F, 0xD2, 0xF8,
        0xE3, 0x2C, 0x6C, 0x0B, 0xF1, 0x8B, 0x03, 0x73, 0xD7, 0x4E, 0x75, 0xFF, 0x41, 0xEA, 0x96, 0x87, 0x7C,
        0x60, 0x96, 0x65, 0x83, 0x50, 0xF9, 0x99, 0x10, 0x13, 0x45, 0xD3, 0x44, 0x9F, 0xD5};
static const uint8_t virgil_SECP384R1_pubkey[] = {
        0x30, 0x76, 0x30, 0x10, 0x06, 0x07, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x02, 0x01, 0x06, 0x05, 0x2B, 0x81, 0x04,
        0x00, 0x22, 0x03, 0x62, 0x00, 0x04, 0x7B, 0xDF, 0x91, 0xC9, 0xC9, 0x03, 0x55, 0x1C, 0x9E, 0x3C, 0x95, 0x57,
        0xB2, 0x5C, 0x0D, 0xC4, 0xD3, 0x8A, 0xD8, 0xB3, 0x66, 0x24, 0xA3, 0x43, 0xA9, 0x11, 0x14, 0x3D, 0xB2, 0x61,
        0x98, 0x07, 0x85, 0x5A, 0x8E, 0x02, 0x62, 0xF7, 0xB9, 0xEA, 0xEC, 0x43, 0x0C, 0x82, 0x51, 0x5A, 0x6D, 0xF8,
        0xE4, 0x76, 0x48, 0x4B, 0x81, 0x7B, 0x03, 0x70, 0xDE, 0x1D, 0xCB, 0x80, 0xEC, 0x50, 0xD7, 0xB0, 0x0D, 0x08,
        0x8A, 0x01, 0x9D, 0x14, 0x48, 0x96, 0x87, 0x38, 0xCC, 0x6C, 0x29, 0x70, 0xBF, 0x0B, 0x21, 0xEA, 0xCD, 0x18,
        0xEB, 0xBA, 0x1C, 0x8D, 0x18, 0x61, 0x79, 0xB8, 0xE1, 0x8C, 0x7E, 0x27};
static const uint8_t raw_SECP384R1_pubkey[] = {
        0x04, 0x7B, 0xDF, 0x91, 0xC9, 0xC9, 0x03, 0x55, 0x1C, 0x9E, 0x3C, 0x95, 0x57, 0xB2, 0x5C, 0x0D, 0xC4,
        0xD3, 0x8A, 0xD8, 0xB3, 0x66, 0x24, 0xA3, 0x43, 0xA9, 0x11, 0x14, 0x3D, 0xB2, 0x61, 0x98, 0x07, 0x85,
        0x5A, 0x8E, 0x02, 0x62, 0xF7, 0xB9, 0xEA, 0xEC, 0x43, 0x0C, 0x82, 0x51, 0x5A, 0x6D, 0xF8, 0xE4, 0x76,
        0x48, 0x4B, 0x81, 0x7B, 0x03, 0x70, 0xDE, 0x1D, 0xCB, 0x80, 0xEC, 0x50, 0xD7, 0xB0, 0x0D, 0x08, 0x8A,
        0x01, 0x9D, 0x14, 0x48, 0x96, 0x87, 0x38, 0xCC, 0x6C, 0x29, 0x70, 0xBF, 0x0B, 0x21, 0xEA, 0xCD, 0x18,
        0xEB, 0xBA, 0x1C, 0x8D, 0x18, 0x61, 0x79, 0xB8, 0xE1, 0x8C, 0x7E, 0x27};
static const uint8_t virgil_SECP521R1_pubkey[] = {
        0x30, 0x81, 0x9B, 0x30, 0x10, 0x06, 0x07, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x02, 0x01, 0x06, 0x05, 0x2B, 0x81,
        0x04, 0x00, 0x23, 0x03, 0x81, 0x86, 0x00, 0x04, 0x00, 0x20, 0x7F, 0x27, 0x4D, 0x9D, 0x26, 0x79, 0x45, 0xD2,
        0x00, 0x9C, 0xD0, 0x0B, 0x53, 0x3D, 0x52, 0x66, 0x4E, 0xE9, 0x73, 0xCE, 0x05, 0x04, 0x2D, 0x27, 0x58, 0x4E,
        0x1A, 0x3E, 0xDB, 0xC0, 0xDF, 0x7D, 0xBC, 0xFB, 0x4D, 0xF6, 0x38, 0x8D, 0x8B, 0x3A, 0x67, 0x9F, 0x53, 0x90,
        0x39, 0xEE, 0x1C, 0xFE, 0xF0, 0xFA, 0x9E, 0x11, 0x76, 0x3C, 0x6B, 0x3A, 0x92, 0x9F, 0x1B, 0xD7, 0xCF, 0x7B,
        0xC5, 0xA6, 0x00, 0xE0, 0xFF, 0x4E, 0xF9, 0x50, 0x6A, 0xD7, 0x03, 0x8A, 0x05, 0xB0, 0x22, 0x59, 0xD7, 0x45,
        0x53, 0x86, 0x9F, 0xCF, 0xAA, 0x66, 0xA4, 0xE0, 0xED, 0xBD, 0xE9, 0x9E, 0xF2, 0x91, 0x49, 0x66, 0xF5, 0xAD,
        0x6D, 0x37, 0x34, 0x7A, 0xC1, 0x3B, 0xBB, 0x10, 0xEE, 0xCE, 0x33, 0xAB, 0x03, 0x4C, 0xF4, 0x0E, 0x78, 0x6F,
        0x80, 0xAD, 0x11, 0x09, 0xD9, 0xC5, 0x85, 0xAD, 0xC5, 0x11, 0xF1, 0x5E, 0xF1, 0xBA};
static const uint8_t raw_SECP521R1_pubkey[] = {
        0x04, 0x00, 0x20, 0x7F, 0x27, 0x4D, 0x9D, 0x26, 0x79, 0x45, 0xD2, 0x00, 0x9C, 0xD0, 0x0B, 0x53, 0x3D,
        0x52, 0x66, 0x4E, 0xE9, 0x73, 0xCE, 0x05, 0x04, 0x2D, 0x27, 0x58, 0x4E, 0x1A, 0x3E, 0xDB, 0xC0, 0xDF,
        0x7D, 0xBC, 0xFB, 0x4D, 0xF6, 0x38, 0x8D, 0x8B, 0x3A, 0x67, 0x9F, 0x53, 0x90, 0x39, 0xEE, 0x1C, 0xFE,
        0xF0, 0xFA, 0x9E, 0x11, 0x76, 0x3C, 0x6B, 0x3A, 0x92, 0x9F, 0x1B, 0xD7, 0xCF, 0x7B, 0xC5, 0xA6, 0x00,
        0xE0, 0xFF, 0x4E, 0xF9, 0x50, 0x6A, 0xD7, 0x03, 0x8A, 0x05, 0xB0, 0x22, 0x59, 0xD7, 0x45, 0x53, 0x86,
        0x9F, 0xCF, 0xAA, 0x66, 0xA4, 0xE0, 0xED, 0xBD, 0xE9, 0x9E, 0xF2, 0x91, 0x49, 0x66, 0xF5, 0xAD, 0x6D,
        0x37, 0x34, 0x7A, 0xC1, 0x3B, 0xBB, 0x10, 0xEE, 0xCE, 0x33, 0xAB, 0x03, 0x4C, 0xF4, 0x0E, 0x78, 0x6F,
        0x80, 0xAD, 0x11, 0x09, 0xD9, 0xC5, 0x85, 0xAD, 0xC5, 0x11, 0xF1, 0x5E, 0xF1, 0xBA};
static const uint8_t virgil_ED25519_pubkey[] = {0x30, 0x2A, 0x30, 0x05, 0x06, 0x03, 0x2B, 0x65, 0x70, 0x03, 0x21,
                                                0x00, 0xEB, 0xFC, 0x9D, 0xD0, 0x59, 0x45, 0x82, 0x16, 0xBD, 0xD4,
                                                0xD7, 0xD4, 0x65, 0x02, 0x28, 0xA2, 0x91, 0xFB, 0x3C, 0x49, 0x26,
                                                0xA0, 0x4B, 0xF2, 0x93, 0x53, 0x7B, 0xAA, 0x1A, 0x3F, 0xFB, 0xFB};
static const uint8_t raw_ED25519_pubkey[] = {0xEB, 0xFC, 0x9D, 0xD0, 0x59, 0x45, 0x82, 0x16, 0xBD, 0xD4, 0xD7,
                                             0xD4, 0x65, 0x02, 0x28, 0xA2, 0x91, 0xFB, 0x3C, 0x49, 0x26, 0xA0,
                                             0x4B, 0xF2, 0x93, 0x53, 0x7B, 0xAA, 0x1A, 0x3F, 0xFB, 0xFB};
static const uint8_t virgil_CURVE25519_pubkey[] = {0x30, 0x2A, 0x30, 0x05, 0x06, 0x03, 0x2B, 0x65, 0x6E, 0x03, 0x21,
                                                   0x00, 0x0E, 0x5D, 0xB3, 0x72, 0x55, 0x21, 0x0D, 0x3B, 0x47, 0x25,
                                                   0x19, 0xAC, 0x67, 0xC8, 0x96, 0x53, 0x0B, 0x31, 0xC5, 0x11, 0x3E,
                                                   0xA7, 0xAA, 0x35, 0x76, 0x04, 0x3A, 0x0F, 0xB1, 0x11, 0x3E, 0x2D};
static const uint8_t raw_CURVE25519_pubkey[] = {0x0E, 0x5D, 0xB3, 0x72, 0x55, 0x21, 0x0D, 0x3B, 0x47, 0x25, 0x19,
                                                0xAC, 0x67, 0xC8, 0x96, 0x53, 0x0B, 0x31, 0xC5, 0x11, 0x3E, 0xA7,
                                                0xAA, 0x35, 0x76, 0x04, 0x3A, 0x0F, 0xB1, 0x11, 0x3E, 0x2D};

/******************************************************************************/
static bool
_test_case_converters_pubkey_pass(vs_secmodule_keypair_type_e keypair_type,
                                  const uint8_t *virgil_pubkey,
                                  uint16_t virgil_pubkey_sz,
                                  const uint8_t *raw_pubkey,
                                  uint16_t raw_pubkey_sz) {
    uint8_t result_buf[RESULT_BUF_SIZE];
    uint16_t result_sz;

    BOOL_CHECK_RET(vs_converters_pubkey_to_raw(
                           keypair_type, virgil_pubkey, virgil_pubkey_sz, result_buf, sizeof(result_buf), &result_sz),
                   "ERROR while convert pubkey virgil to raw");

    BOOL_CHECK_RET(result_sz == raw_pubkey_sz, "ERROR raw pubkey has wrong size");
    MEMCMP_CHECK_RET(result_buf, raw_pubkey, raw_pubkey_sz, false);

    BOOL_CHECK_RET(vs_converters_pubkey_to_virgil(
                           keypair_type, raw_pubkey, raw_pubkey_sz, result_buf, sizeof(result_buf), &result_sz),
                   "ERROR while convert pubkey raw to virgil");

    BOOL_CHECK_RET(result_sz == virgil_pubkey_sz, "ERROR virgil signature has wrong size");
    MEMCMP_CHECK_RET(result_buf, virgil_pubkey, virgil_pubkey_sz, false);

    return true;
}


/******************************************************************************/
uint16_t
test_pubkeys_converters(void) {
    uint16_t failed_test_result = 0;

#define TEST_CONVERTERS_PUBKEY_PASS(KEY, VIRGIL_PUBKEY, VIRGIL_PUBKEY_SZ, RAW_PUBKEY, RAW_PUBKEY_SZ)                   \
    TEST_CASE_OK("key " #KEY,                                                                                          \
                 _test_case_converters_pubkey_pass(KEY, VIRGIL_PUBKEY, VIRGIL_PUBKEY_SZ, RAW_PUBKEY, RAW_PUBKEY_SZ))

    START_TEST("pubkeys converters");

    TEST_CONVERTERS_PUBKEY_PASS(VS_KEYPAIR_EC_SECP192R1,
                                virgil_SECP192R1_pubkey,
                                sizeof(virgil_SECP192R1_pubkey),
                                raw_SECP192R1_pubkey,
                                sizeof(raw_SECP192R1_pubkey));

    TEST_CONVERTERS_PUBKEY_PASS(VS_KEYPAIR_EC_SECP224R1,
                                virgil_SECP224R1_pubkey,
                                sizeof(virgil_SECP224R1_pubkey),
                                raw_SECP224R1_pubkey,
                                sizeof(raw_SECP224R1_pubkey));

    TEST_CONVERTERS_PUBKEY_PASS(VS_KEYPAIR_EC_SECP256R1,
                                virgil_SECP256R1_pubkey,
                                sizeof(virgil_SECP256R1_pubkey),
                                raw_SECP256R1_pubkey,
                                sizeof(raw_SECP256R1_pubkey));

    TEST_CONVERTERS_PUBKEY_PASS(VS_KEYPAIR_EC_SECP384R1,
                                virgil_SECP384R1_pubkey,
                                sizeof(virgil_SECP384R1_pubkey),
                                raw_SECP384R1_pubkey,
                                sizeof(raw_SECP384R1_pubkey));

    TEST_CONVERTERS_PUBKEY_PASS(VS_KEYPAIR_EC_SECP521R1,
                                virgil_SECP521R1_pubkey,
                                sizeof(virgil_SECP521R1_pubkey),
                                raw_SECP521R1_pubkey,
                                sizeof(raw_SECP521R1_pubkey));

    TEST_CONVERTERS_PUBKEY_PASS(VS_KEYPAIR_EC_ED25519,
                                virgil_ED25519_pubkey,
                                sizeof(virgil_ED25519_pubkey),
                                raw_ED25519_pubkey,
                                sizeof(raw_ED25519_pubkey));

    TEST_CONVERTERS_PUBKEY_PASS(VS_KEYPAIR_EC_CURVE25519,
                                virgil_CURVE25519_pubkey,
                                sizeof(virgil_CURVE25519_pubkey),
                                raw_CURVE25519_pubkey,
                                sizeof(raw_CURVE25519_pubkey));

terminate:
    return failed_test_result;

#undef TEST_CONVERTERS_PUBKEY_PASS
}
