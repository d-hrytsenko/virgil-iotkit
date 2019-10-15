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
#include <stdbool.h>

#include <stdlib-config.h>

#include <virgil/iot/hsm/hsm.h>
#include <virgil/iot/hsm/hsm_helpers.h>
#include <virgil/iot/macros/macros.h>
#include <virgil/iot/logger/logger.h>
#include <virgil/iot/protocols/sdmp/prvs/prvs-structs.h>
#include <virgil/iot/provision/provision.h>
#include <virgil/iot/trust_list/trust_list.h>

static const size_t rec_key_slot[PROVISION_KEYS_QTY] = {REC1_KEY_SLOT, REC2_KEY_SLOT};

static const size_t auth_key_slot[PROVISION_KEYS_QTY] = {AUTH1_KEY_SLOT, AUTH2_KEY_SLOT};

static const size_t tl_key_slot[PROVISION_KEYS_QTY] = {TL1_KEY_SLOT, TL2_KEY_SLOT};

static const size_t fw_key_slot[PROVISION_KEYS_QTY] = {FW1_KEY_SLOT, FW2_KEY_SLOT};

static vs_hsm_impl_t *_hsm = NULL;

/******************************************************************************/
static vs_status_e
_get_pubkey_slot_num(vs_key_type_e key_type, uint8_t index, vs_iot_hsm_slot_e *slot) {
    const size_t *ptr;

    switch (key_type) {
    case VS_KEY_RECOVERY:
        ptr = rec_key_slot;
        break;
    case VS_KEY_AUTH:
        ptr = auth_key_slot;
        break;
    case VS_KEY_TRUSTLIST:
        ptr = tl_key_slot;
        break;
    case VS_KEY_FIRMWARE:
        ptr = fw_key_slot;
        break;
    default:
        VS_LOG_ERROR("Incorrect key type %d", key_type);
        return VS_CODE_ERR_INCORRECT_ARGUMENT;
    }

    *slot = ptr[index];

    return VS_CODE_OK;
}

/******************************************************************************/
vs_status_e
vs_provision_get_slot_num(vs_provision_element_id_e id, uint16_t *slot) {
    size_t index;
    const size_t *ptr;

    BOOL_CHECK_RET(NULL != slot, "Invalid args");

    switch (id) {
    case VS_PROVISION_PBR1:
        ptr = rec_key_slot;
        index = 0;
        break;
    case VS_PROVISION_PBR2:
        ptr = rec_key_slot;
        index = 1;
        break;
    case VS_PROVISION_PBA1:
        ptr = auth_key_slot;
        index = 0;
        break;
    case VS_PROVISION_PBA2:
        ptr = auth_key_slot;
        index = 1;
        break;
    case VS_PROVISION_PBT1:
        ptr = tl_key_slot;
        index = 0;
        break;
    case VS_PROVISION_PBT2:
        ptr = tl_key_slot;
        index = 1;
        break;
    case VS_PROVISION_PBF1:
        ptr = fw_key_slot;
        index = 0;
        break;
    case VS_PROVISION_PBF2:
        ptr = fw_key_slot;
        index = 1;
        break;
    case VS_PROVISION_SGNP:
        *slot = SIGNATURE_SLOT;
        return VS_CODE_OK;
    default:
        VS_LOG_ERROR("Incorrect provision element %d", id);
        return VS_CODE_ERR_INCORRECT_ARGUMENT;
    }

    *slot = ptr[index];

    return VS_CODE_OK;
}

/******************************************************************************/
vs_status_e
vs_provision_search_hl_pubkey(vs_key_type_e key_type, vs_hsm_keypair_type_e ec_type, uint8_t *key, uint16_t key_sz) {
    vs_iot_hsm_slot_e slot;
    uint8_t i = 0;
    int ref_key_sz;
    // TODO: Fix buffer constant size
    uint8_t buf[512];
    vs_pubkey_dated_t *ref_key = (vs_pubkey_dated_t *)buf;
    uint16_t _sz;
    vs_status_e ret_code;

    VS_IOT_ASSERT(_hsm);

    for (i = 0; i < PROVISION_KEYS_QTY; ++i) {

        STATUS_CHECK_RET(_get_pubkey_slot_num(key_type, i, &slot), "Unable to get public key from slot");
        STATUS_CHECK_RET(_hsm->slot_load(slot, buf, sizeof(buf), &_sz), "Unable to load slot data");

        ref_key_sz = vs_hsm_get_pubkey_len(ref_key->pubkey.ec_type);

        if (ref_key_sz < 0) {
            return VS_CODE_ERR_INCORRECT_PARAMETER;
        }

        if (ref_key->pubkey.key_type == key_type && ref_key->pubkey.ec_type == ec_type && ref_key_sz == key_sz &&
            0 == VS_IOT_MEMCMP(key, ref_key->pubkey.pubkey, key_sz)) {
            return vs_provision_verify_hl_key(buf, _sz);
        }
    }

    return VS_CODE_ERR_NOT_FOUND;
}

/******************************************************************************/
vs_status_e
vs_provision_verify_hl_key(const uint8_t *key_to_check, uint16_t key_size) {

    int key_len;
    int sign_len;
    int hash_size;
    uint16_t signed_data_sz;
    uint16_t res_sz;
    uint8_t *pubkey;
    vs_sign_t *sign;
    vs_status_e ret_code;

    VS_IOT_ASSERT(_hsm);

    BOOL_CHECK_RET(NULL != key_to_check, "Invalid args");
    BOOL_CHECK_RET(key_size > sizeof(vs_pubkey_dated_t), "key stuff is too small");

    vs_pubkey_dated_t *key = (vs_pubkey_dated_t *)key_to_check;

    // Recovery key doesn't have signature
    if (VS_KEY_RECOVERY == key->pubkey.key_type) {
        return VS_CODE_OK;
    }

    key_len = vs_hsm_get_pubkey_len(key->pubkey.ec_type);

    CHECK_RET(key_len > 0, VS_CODE_ERR_CRYPTO, "Unsupported key ec_type");

    // Determine stuff size under signature
    signed_data_sz = sizeof(vs_pubkey_dated_t) + key_len;

    CHECK_RET(key_size > signed_data_sz + sizeof(vs_sign_t), VS_CODE_ERR_CRYPTO, "key stuff is too small");

    // Signature pointer
    sign = (vs_sign_t *)(key_to_check + signed_data_sz);

    CHECK_RET(VS_KEY_RECOVERY == sign->signer_type, VS_CODE_ERR_CRYPTO, "Signer type must be RECOVERY");

    sign_len = vs_hsm_get_signature_len(sign->ec_type);
    key_len = vs_hsm_get_pubkey_len(sign->ec_type);

    CHECK_RET(sign_len > 0 && key_len > 0, VS_CODE_ERR_CRYPTO, "Unsupported signature ec_type");
    CHECK_RET(key_size == signed_data_sz + sizeof(vs_sign_t) + sign_len + key_len,
              VS_CODE_ERR_CRYPTO,
              "key stuff is wrong");

    // Calculate hash of stuff under signature
    hash_size = vs_hsm_get_hash_len(sign->hash_type);
    CHECK_RET(hash_size > 0, VS_CODE_ERR_CRYPTO, "Unsupported hash type");

    uint8_t hash[hash_size];

    STATUS_CHECK_RET(_hsm->hash(sign->hash_type, key_to_check, signed_data_sz, hash, hash_size, &res_sz),
                     "Error hash create");

    // Signer raw key pointer
    pubkey = sign->raw_sign_pubkey + sign_len;

    STATUS_CHECK_RET(vs_provision_search_hl_pubkey(sign->signer_type, sign->ec_type, pubkey, key_len),
                     "Signer key is not present");

    STATUS_CHECK_RET(
            _hsm->ecdsa_verify(sign->ec_type, pubkey, key_len, sign->hash_type, hash, sign->raw_sign_pubkey, sign_len),
            "Signature is wrong");

    return VS_CODE_OK;
}

/******************************************************************************/
vs_status_e
vs_provision_init(vs_storage_op_ctx_t *tl_storage_ctx, vs_hsm_impl_t *hsm) {
    CHECK_NOT_ZERO_RET(hsm, VS_CODE_ERR_NULLPTR_ARGUMENT);
    _hsm = hsm;

    // TrustList module
    // TODO: Fix errors detection
    vs_tl_init(tl_storage_ctx, hsm);

    return VS_CODE_OK;
}

/******************************************************************************/
