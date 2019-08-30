//  Copyright (C) 2015-2019 Virgil Security, Inc.
//
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are
//  met:
//
//      (1) Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//
//      (2) Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in
//      the documentation and/or other materials provided with the
//      distribution.
//
//      (3) Neither the name of the copyright holder nor the names of its
//      contributors may be used to endorse or promote products derived from
//      this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ''AS IS'' AND ANY EXPRESS OR
//  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//  DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
//  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
//  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
//  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
//  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
//  IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//  POSSIBILITY OF SUCH DAMAGE.
//
//  Lead Maintainer: Virgil Security Inc. <support@virgilsecurity.com>

#include <stdint.h>
#include <stddef.h>

#include <update-config.h>

#include <virgil/iot/macros/macros.h>
#include <virgil/iot/update/update.h>
#include <virgil/iot/update/update_interface.h>
#include <virgil/iot/logger/logger.h>
#include <virgil/iot/provision/provision.h>
#include <virgil/iot/hsm/hsm_interface.h>
#include <virgil/iot/hsm/hsm_helpers.h>
#include <virgil/iot/hsm/hsm_sw_sha2_routines.h>

static const vs_key_type_e sign_rules_list[VS_FW_SIGNATURES_QTY] = VS_FW_SIGNER_TYPE_LIST;

/*************************************************************************/
int
vs_update_load_firmware_chunk(vs_firmware_descriptor_t *descriptor,
                              uint32_t offset,
                              uint8_t *data,
                              uint16_t buff_sz,
                              uint16_t *data_sz) {


    CHECK_NOT_ZERO_RET(descriptor, VS_UPDATE_ERR_INVAL);
    CHECK_NOT_ZERO_RET(data, VS_UPDATE_ERR_INVAL);
    CHECK_NOT_ZERO_RET(data_sz, VS_UPDATE_ERR_INVAL);

    return vs_update_read_firmware_data_hal(
            descriptor->info.manufacture_id, descriptor->info.device_type, offset, data, buff_sz, data_sz);
}

/*************************************************************************/
int
vs_update_save_firmware_chunk(vs_firmware_descriptor_t *descriptor,
                              uint8_t *chunk,
                              uint16_t chunk_sz,
                              uint32_t offset) {

    CHECK_NOT_ZERO_RET(descriptor, VS_UPDATE_ERR_INVAL);
    CHECK_NOT_ZERO_RET(chunk, VS_UPDATE_ERR_INVAL);

    return vs_update_write_firmware_data_hal(
            descriptor->info.manufacture_id, descriptor->info.device_type, offset, chunk, chunk_sz);
}

/*************************************************************************/
int
vs_update_save_firmware_footer(vs_firmware_descriptor_t *descriptor, uint8_t *footer) {
    uint16_t footer_sz = sizeof(vs_update_firmware_footer_t);
    vs_update_firmware_footer_t *f = (vs_update_firmware_footer_t *)footer;

    CHECK_NOT_ZERO_RET(descriptor, VS_UPDATE_ERR_INVAL);
    CHECK_NOT_ZERO_RET(footer, VS_UPDATE_ERR_INVAL);

    for (uint8_t i = 0; i < f->signatures_count; ++i) {
        int key_len;
        int sign_len;
        vs_sign_t *sign = (vs_sign_t *)(footer + footer_sz);

        sign_len = vs_hsm_get_signature_len(sign->ec_type);
        key_len = vs_hsm_get_pubkey_len(sign->ec_type);

        CHECK_RET(sign_len > 0 && key_len > 0, VS_UPDATE_ERR_INVAL, "Unsupported signature ec_type")

        footer_sz += sizeof(vs_sign_t) + sign_len + key_len;
    }

    return vs_update_write_firmware_data_hal(descriptor->info.manufacture_id,
                                             descriptor->info.device_type,
                                             descriptor->firmware_length,
                                             footer,
                                             footer_sz);
}

/*************************************************************************/
int
vs_update_load_firmware_footer(vs_firmware_descriptor_t *descriptor,
                               uint8_t *data,
                               uint16_t buff_sz,
                               uint16_t *data_sz) {
    int file_sz;
    CHECK_NOT_ZERO_RET(descriptor, VS_UPDATE_ERR_INVAL);
    CHECK_NOT_ZERO_RET(data, VS_UPDATE_ERR_INVAL);
    CHECK_NOT_ZERO_RET(data_sz, VS_UPDATE_ERR_INVAL);

    *data_sz = 0;

    file_sz = vs_update_get_firmware_image_len_hal(descriptor->info.manufacture_id, descriptor->info.device_type);

    if (file_sz > 0) {

        int32_t footer_sz = file_sz - descriptor->firmware_length;
        CHECK_RET(footer_sz > 0 && footer_sz < UINT16_MAX, VS_UPDATE_ERR_INVAL, "Incorrect footer size")

        *data_sz = (uint16_t)footer_sz;
        CHECK_RET(footer_sz <= buff_sz, VS_UPDATE_ERR_INVAL, "Buffer to small")

        return vs_update_read_firmware_data_hal(descriptor->info.manufacture_id,
                                                descriptor->info.device_type,
                                                descriptor->firmware_length,
                                                data,
                                                footer_sz,
                                                data_sz);
    }
    return VS_UPDATE_ERR_FAIL;
}

/*************************************************************************/
int
vs_update_save_firmware_descriptor(vs_firmware_descriptor_t *descriptor) {
    int file_sz;
    uint8_t *buf = NULL;
    uint8_t *newbuf = NULL;
    uint32_t offset = 0;
    int res;

    CHECK_NOT_ZERO_RET(descriptor, VS_UPDATE_ERR_INVAL);

    file_sz = vs_update_get_firmware_descriptor_table_len_hal();

    if (file_sz > 0) {
        uint16_t read_sz;
        buf = VS_IOT_CALLOC(1, file_sz);
        CHECK_NOT_ZERO_RET(buf, VS_UPDATE_ERR_FAIL);

        if (VS_UPDATE_ERR_OK != vs_update_read_firmware_descriptor_table_hal(buf, file_sz, &read_sz)) {
            VS_IOT_FREE(buf);
            return VS_UPDATE_ERR_FAIL;
        }

        while (offset < file_sz) {
            if (offset + sizeof(vs_firmware_descriptor_t) > file_sz) {
                file_sz = offset;
                break;
            }

            vs_firmware_descriptor_t *ptr = (vs_firmware_descriptor_t *)(buf + offset);

            if (0 == memcmp(ptr->info.manufacture_id, descriptor->info.manufacture_id, MANUFACTURE_ID_SIZE) &&
                0 == memcmp(ptr->info.device_type, descriptor->info.device_type, DEVICE_TYPE_SIZE)) {
                VS_IOT_MEMCPY(ptr, descriptor, sizeof(vs_firmware_descriptor_t));
                newbuf = buf;
                goto save_data;
            }

            offset += sizeof(vs_firmware_descriptor_t);
        }

        newbuf = VS_IOT_CALLOC(1, file_sz + sizeof(vs_firmware_descriptor_t));

        if (NULL == newbuf) {
            VS_IOT_FREE(buf);
            return VS_UPDATE_ERR_FAIL;
        }

        VS_IOT_MEMCPY(newbuf, buf, file_sz);
        VS_IOT_MEMCPY(newbuf + file_sz, descriptor, sizeof(vs_firmware_descriptor_t));
        file_sz += sizeof(vs_firmware_descriptor_t);

        VS_IOT_FREE(buf);
    } else {
        file_sz = sizeof(vs_firmware_descriptor_t);
        newbuf = VS_IOT_CALLOC(1, file_sz);
        CHECK_NOT_ZERO_RET(newbuf, VS_UPDATE_ERR_FAIL);
        VS_IOT_MEMCPY(newbuf, (uint8_t *)descriptor, file_sz);
    }

    save_data:
    res = vs_update_write_firmware_descriptor_table_hal(newbuf, file_sz);
    VS_IOT_FREE(newbuf);

    return res;
}

/*************************************************************************/
int
vs_update_load_firmware_descriptor(uint8_t manufacture_id[MANUFACTURE_ID_SIZE],
                                   uint8_t device_type[DEVICE_TYPE_SIZE],
                                   vs_firmware_descriptor_t *descriptor) {

    int res = VS_UPDATE_ERR_NOT_FOUND;
    int file_sz;
    uint8_t *buf = NULL;
    uint32_t offset = 0;

    CHECK_NOT_ZERO_RET(descriptor, VS_UPDATE_ERR_INVAL);

    file_sz = vs_update_get_firmware_descriptor_table_len_hal();

    if (file_sz <= 0) {
        goto terminate;
    }

    uint16_t read_sz;
    buf = VS_IOT_CALLOC(1, file_sz);
    CHECK_NOT_ZERO_RET(buf, VS_UPDATE_ERR_FAIL);

    if (VS_UPDATE_ERR_OK != vs_update_read_firmware_descriptor_table_hal(buf, file_sz, &read_sz)) {
        res = VS_UPDATE_ERR_FAIL;
        goto terminate;
    }

    while (offset + sizeof(vs_firmware_descriptor_t) <= file_sz) {
        vs_firmware_descriptor_t *ptr = (vs_firmware_descriptor_t *)(buf + offset);

        if (0 == memcmp(ptr->info.manufacture_id, manufacture_id, MANUFACTURE_ID_SIZE) &&
            0 == memcmp(ptr->info.device_type, device_type, DEVICE_TYPE_SIZE)) {
            VS_IOT_MEMCPY(descriptor, ptr, sizeof(vs_firmware_descriptor_t));
            res = VS_UPDATE_ERR_OK;
            break;
        }

        offset += sizeof(vs_firmware_descriptor_t);
    }

    terminate:
    VS_IOT_FREE(buf);

    return res;
}

/*************************************************************************/
int
vs_update_delete_firmware(vs_firmware_descriptor_t *descriptor) {
    int res = VS_UPDATE_ERR_FAIL;
    int file_sz;
    uint8_t *buf = NULL;

    CHECK_NOT_ZERO_RET(descriptor, VS_UPDATE_ERR_INVAL);

    if (VS_UPDATE_ERR_OK !=
        vs_update_remove_firmware_data_hal(descriptor->info.manufacture_id, descriptor->info.device_type)) {
        goto terminate;
    }

    file_sz = vs_update_get_firmware_descriptor_table_len_hal();

    if (file_sz <= 0) {
        goto terminate;
    }

    uint16_t read_sz;
    uint32_t offset = 0;
    buf = VS_IOT_CALLOC(1, file_sz);
    CHECK_NOT_ZERO_RET(buf, VS_UPDATE_ERR_FAIL);

    if (VS_UPDATE_ERR_OK != vs_update_read_firmware_descriptor_table_hal(buf, file_sz, &read_sz)) {
        goto terminate;
    }

    while (offset < file_sz || offset + sizeof(vs_firmware_descriptor_t) > file_sz) {
        vs_firmware_descriptor_t *ptr = (vs_firmware_descriptor_t *)(buf + offset);

        if (0 == memcmp(ptr->info.manufacture_id, descriptor->info.manufacture_id, MANUFACTURE_ID_SIZE) &&
            0 == memcmp(ptr->info.device_type, descriptor->info.device_type, DEVICE_TYPE_SIZE)) {
            VS_IOT_MEMMOVE(buf + offset,
                           buf + offset + sizeof(vs_firmware_descriptor_t),
                           file_sz - offset - sizeof(vs_firmware_descriptor_t));
            file_sz -= sizeof(vs_firmware_descriptor_t);
            break;
        }
        offset += sizeof(vs_firmware_descriptor_t);
    }

    if (VS_UPDATE_ERR_OK != vs_update_remove_firmware_descriptor_table_hal()) {
        goto terminate;
    }

    if (file_sz) {
        res = vs_update_write_firmware_descriptor_table_hal(buf, file_sz);
    }

    terminate:
    VS_IOT_FREE(buf);

    return res;
}

/******************************************************************************/
static bool
_is_rule_equal_to(vs_key_type_e type) {
    uint8_t i;
    for (i = 0; i < VS_FW_SIGNATURES_QTY; ++i) {
        if (sign_rules_list[i] == type) {
            return true;
        }
    }
    return false;
}

/*************************************************************************/
int
vs_update_verify_firmware(vs_firmware_descriptor_t *descriptor) {
    int file_sz;
    uint8_t *pubkey;
    uint16_t sign_len;
    uint16_t key_len;
    uint8_t sign_rules = 0;
    uint16_t i;
    vs_hsm_sw_sha256_ctx ctx;

    // TODO: Need to support all hash types
    uint8_t hash[32];

    CHECK_NOT_ZERO_RET(descriptor, VS_UPDATE_ERR_FAIL);
    file_sz = vs_update_get_firmware_image_len_hal(descriptor->info.manufacture_id, descriptor->info.device_type);

    if (file_sz <= 0) {
        return VS_UPDATE_ERR_FAIL;
    }

    int32_t footer_sz = file_sz - descriptor->firmware_length;
    CHECK_RET(footer_sz > 0 && footer_sz < UINT16_MAX, VS_UPDATE_ERR_INVAL, "Incorrect footer size")

    uint8_t buf[descriptor->chunk_size < footer_sz ? footer_sz : descriptor->chunk_size];
    vs_update_firmware_footer_t *footer = (vs_update_firmware_footer_t *)buf;
    uint32_t offset = 0;
    uint16_t read_sz;

    vs_hsm_sw_sha256_init(&ctx);

    // Update hash by firmware
    while (offset < descriptor->firmware_length) {
        uint32_t fw_rest = descriptor->firmware_length - offset;
        uint32_t required_chunk_size = fw_rest > descriptor->chunk_size ? descriptor->chunk_size : fw_rest;

        if (VS_UPDATE_ERR_OK != vs_update_load_firmware_chunk(descriptor, offset, buf, required_chunk_size, &read_sz)) {
            return VS_UPDATE_ERR_FAIL;
        }

        vs_hsm_sw_sha256_update(&ctx, buf, required_chunk_size);
        offset += required_chunk_size;
    }

    // Calculate fill size
    uint32_t fill_sz = descriptor->app_size - descriptor->firmware_length;
    CHECK_RET(footer_sz <= fill_sz, VS_UPDATE_ERR_FAIL, "Bad fill size of image")
    fill_sz -= footer_sz;
    VS_IOT_MEMSET(buf, 0xFF, descriptor->chunk_size > fill_sz ? fill_sz : descriptor->chunk_size);

    // Update hash by fill
    while (fill_sz) {
        uint16_t sz = descriptor->chunk_size > fill_sz ? fill_sz : descriptor->chunk_size;
        vs_hsm_sw_sha256_update(&ctx, buf, sz);
        fill_sz -= sz;
    }

    // Update hash by footer
    if (VS_UPDATE_ERR_OK != vs_update_load_firmware_footer(descriptor, buf, footer_sz, &read_sz)) {
        return VS_UPDATE_ERR_FAIL;
    }

    vs_hsm_sw_sha256_update(&ctx, buf, sizeof(vs_update_firmware_footer_t));
    vs_hsm_sw_sha256_final(&ctx, hash);

    // First signature
    vs_sign_t *sign = (vs_sign_t *)footer->signatures;

    CHECK_RET(footer->signatures_count >= VS_FW_SIGNATURES_QTY, VS_UPDATE_ERR_FAIL, "There are not enough signatures")

    for (i = 0; i < footer->signatures_count; ++i) {
        CHECK_RET(sign->hash_type == VS_HASH_SHA_256, VS_UPDATE_ERR_FAIL, "Unsupported hash size for sign FW")

        sign_len = vs_hsm_get_signature_len(sign->ec_type);
        key_len = vs_hsm_get_pubkey_len(sign->ec_type);

        CHECK_RET(sign_len > 0 && key_len > 0, VS_UPDATE_ERR_FAIL, "Unsupported signature ec_type")

        // Signer raw key pointer
        pubkey = sign->raw_sign_pubkey + sign_len;

        CHECK_RET(vs_provision_search_hl_pubkey(sign->signer_type, sign->ec_type, pubkey, key_len),
                  VS_UPDATE_ERR_FAIL,
                  "Signer key is wrong")

        if (_is_rule_equal_to(sign->signer_type)) {
            CHECK_RET(VS_HSM_ERR_OK == vs_hsm_ecdsa_verify(sign->ec_type,
                                                           pubkey,
                                                           key_len,
                                                           sign->hash_type,
                                                           hash,
                                                           sign->raw_sign_pubkey,
                                                           sign_len),
                      VS_UPDATE_ERR_FAIL,
                      "Signature is wrong")
            sign_rules++;
        }

        // Next signature
        sign = (vs_sign_t *)(pubkey + key_len);
    }

    VS_LOG_DEBUG("New FW Image. Sign rules is %s", sign_rules >= VS_FW_SIGNATURES_QTY ? "correct" : "wrong");

    return sign_rules >= VS_FW_SIGNATURES_QTY ? VS_UPDATE_ERR_OK : VS_UPDATE_ERR_FAIL;
}

/*************************************************************************/
int
vs_update_install_firmware(vs_firmware_descriptor_t *descriptor) {
    int file_sz;
    CHECK_NOT_ZERO_RET(descriptor, VS_UPDATE_ERR_FAIL);

    CHECK_RET(VS_UPDATE_ERR_OK == vs_update_install_prepare_space_hal(),
              VS_UPDATE_ERR_FAIL,
              "Unable to prepare the install space");

    file_sz = vs_update_get_firmware_image_len_hal(descriptor->info.manufacture_id, descriptor->info.device_type);

    if (file_sz <= 0) {
        return VS_UPDATE_ERR_FAIL;
    }

    int32_t footer_sz = file_sz - descriptor->firmware_length;
    CHECK_RET(footer_sz > 0 && footer_sz < UINT16_MAX, VS_UPDATE_ERR_INVAL, "Incorrect footer size")

    uint8_t buf[descriptor->chunk_size < footer_sz ? footer_sz : descriptor->chunk_size];
    uint32_t offset = 0;
    uint16_t read_sz;

    // Update image by firmware
    while (offset < descriptor->firmware_length) {
        uint32_t fw_rest = descriptor->firmware_length - offset;
        uint32_t required_chunk_size = fw_rest > descriptor->chunk_size ? descriptor->chunk_size : fw_rest;

        if (VS_UPDATE_ERR_OK != vs_update_load_firmware_chunk(descriptor, offset, buf, required_chunk_size, &read_sz)) {
            return VS_UPDATE_ERR_FAIL;
        }

        if (VS_UPDATE_ERR_OK != vs_update_install_append_data_hal(buf, required_chunk_size)) {
            return VS_UPDATE_ERR_FAIL;
        }

        offset += required_chunk_size;
    }

    // Calculate fill size
    uint32_t fill_sz = descriptor->app_size - descriptor->firmware_length;
    CHECK_RET(footer_sz <= fill_sz, VS_UPDATE_ERR_FAIL, "Bad fill size of image")
    fill_sz -= footer_sz;
    VS_IOT_MEMSET(buf, 0xFF, descriptor->chunk_size > fill_sz ? fill_sz : descriptor->chunk_size);

    // Update image by fill
    while (fill_sz) {
        uint16_t sz = descriptor->chunk_size > fill_sz ? fill_sz : descriptor->chunk_size;

        if (VS_UPDATE_ERR_OK != vs_update_install_append_data_hal(buf, sz)) {
            return VS_UPDATE_ERR_FAIL;
        }

        fill_sz -= sz;
    }

    // Update image by footer
    if (VS_UPDATE_ERR_OK != vs_update_load_firmware_footer(descriptor, buf, footer_sz, &read_sz)) {
        return VS_UPDATE_ERR_FAIL;
    }

    return vs_update_install_append_data_hal(buf, read_sz);
}

/*************************************************************************/
int
vs_update_restart_application(void){
    VS_LOG_INFO("Restart application");
    return vs_update_restart_app_hal();
}