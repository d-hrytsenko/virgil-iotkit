
#include <virgil/iot/tests/helpers.h>
#include <virgil/iot/tests/private/private_helpers.h>
#include <virgil/iot/hsm/hsm_interface.h>
#include <virgil/iot/hsm/hsm_helpers.h>

static uint8_t key_raw[] = {0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
                            0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
                            0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01};

static uint8_t input_raw[] = {0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
                              0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
                              0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
                              0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
                              0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02};

/******************************************************************************/
static bool
_test_hmac_case(vs_hsm_hash_type_e hash_type, const uint8_t *correct, uint16_t correct_sz) {
    uint8_t buf[128];
    uint16_t sz;

    VS_HSM_CHECK_RET(
            vs_hsm_hmac(hash_type, key_raw, sizeof(key_raw), input_raw, sizeof(input_raw), buf, sizeof(buf), &sz),
            "vs_hsm_hmac incorrect result");

    MEMCMP_CHECK_RET(correct, buf, correct_sz);

    return true;
}

/******************************************************************************/
void
test_hmac(void) {

#define TEST_STEP(BITLEN)                                                                                              \
    do {                                                                                                               \
        vs_hsm_hash_type_e hash_type = VS_HASH_SHA_##BITLEN;                                                           \
        const uint8_t *correct = sha##BITLEN##_result_raw;                                                             \
        uint16_t correct_sz = sizeof(sha##BITLEN##_result_raw);                                                        \
        bool not_implemented = false;                                                                                  \
                                                                                                                       \
        TEST_HMAC_NOT_IMPLEMENTED(hash_type);                                                                          \
                                                                                                                       \
        if (not_implemented) {                                                                                         \
            VS_LOG_WARNING("HMAC for SHA_" #BITLEN " algorithm is not implemented");                                   \
        } else {                                                                                                       \
            TEST_CASE_OK(vs_hsm_hash_type_descr(hash_type), _test_hmac_case(hash_type, correct, correct_sz));          \
        }                                                                                                              \
    } while (0)

    uint8_t sha256_result_raw[] = {0x70, 0xa8, 0xe8, 0xa8, 0xb5, 0x24, 0x1b, 0x7e, 0x75, 0x84, 0x93,
                                   0x55, 0x3f, 0x29, 0x21, 0x80, 0x1b, 0x11, 0xd3, 0x6f, 0x47, 0x35,
                                   0xd8, 0xb5, 0xf2, 0x72, 0xa3, 0x46, 0x4b, 0x25, 0x0d, 0x9c};

    uint8_t sha384_result_raw[] = {0x7b, 0x4a, 0xf4, 0x20, 0x98, 0x0c, 0xeb, 0xfa, 0xc1, 0x42, 0xf1, 0x33,
                                   0x66, 0x9d, 0x05, 0xe5, 0x8d, 0x6d, 0x47, 0x49, 0x88, 0xd9, 0x48, 0x22,
                                   0x04, 0xa2, 0xd1, 0x70, 0xf4, 0x59, 0x2c, 0x73, 0xea, 0xd5, 0xc3, 0xf0,
                                   0x8a, 0x8e, 0xe4, 0xf1, 0x9d, 0xfa, 0x13, 0x81, 0x8a, 0xbf, 0xb7, 0xb4};

    uint8_t sha512_result_raw[] = {0x23, 0x14, 0x5c, 0xab, 0x10, 0x24, 0x1b, 0x87, 0xcc, 0x3c, 0x18, 0xb6, 0xfa,
                                   0xe4, 0x61, 0x31, 0xc0, 0x1e, 0x16, 0xef, 0x73, 0xf0, 0x38, 0x5f, 0xc6, 0x0e,
                                   0xa7, 0xc6, 0x14, 0x6d, 0x02, 0x37, 0x9e, 0xb2, 0x29, 0xa0, 0x27, 0xde, 0x37,
                                   0x21, 0xd2, 0x88, 0xac, 0x81, 0x63, 0x17, 0xe9, 0x13, 0x37, 0xb7, 0x4e, 0xde,
                                   0xf1, 0x7a, 0xb5, 0x97, 0xbc, 0x27, 0x0d, 0x23, 0x9a, 0xb8, 0xc8, 0x36};

    START_TEST("HMAC test");

    TEST_STEP(256);
    TEST_STEP(384);
    TEST_STEP(512);

terminate:;

#undef TEST_STEP
}
