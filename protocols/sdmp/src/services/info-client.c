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

#include <virgil/iot/protocols/sdmp/info-client.h>
#include <virgil/iot/protocols/sdmp/info-private.h>
#include <virgil/iot/protocols/sdmp.h>
#include <virgil/iot/status_code/status_code.h>
#include <virgil/iot/logger/logger.h>
#include <virgil/iot/macros/macros.h>
#include <stdlib-config.h>
#include <global-hal.h>
#include <virgil/iot/trust_list/trust_list.h>
#include <virgil/iot/trust_list/tl_structs.h>
#include <endian-config.h>

// External functions for access to upper level implementations
static vs_sdmp_info_impl_t _info_impl = {0};

static vs_sdmp_service_t _info_client = {0};

static vs_sdmp_info_device_t *_devices_list = 0;
static size_t _devices_list_max = 0;
static size_t _devices_list_cnt = 0;

// Callbacks for devices polling
static vs_sdmp_info_callbacks_t _callbacks = {NULL, NULL, NULL};

/******************************************************************************/
int
vs_sdmp_info_enum_devices(const vs_netif_t *netif,
                          vs_sdmp_info_device_t *devices,
                          size_t devices_max,
                          size_t *devices_cnt,
                          uint32_t wait_ms) {

    VS_IOT_ASSERT(_info_impl.wait_func);

    // Set storage for ENUM request
    _devices_list = devices;
    _devices_list_max = devices_max;
    _devices_list_cnt = 0;
    *devices_cnt = 0;

    // Send request
    if (0 != vs_sdmp_send_request(netif, 0, VS_INFO_SERVICE_ID, VS_INFO_ENUM, NULL, 0)) {
        return -1;
    }

    // Wait request
    vs_global_hal_msleep(wait_ms);

    *devices_cnt = _devices_list_cnt;

    return 0;
}

/******************************************************************************/

int
vs_sdmp_info_set_polling(const vs_netif_t *netif,
                         const vs_mac_addr_t *mac,
                         uint32_t elements, // Multiple vs_sdmp_info_element_mask_e
                         bool enable,
                         uint16_t period_seconds) {
    vs_info_poll_request_t request;
    const vs_netif_t *default_netif = vs_sdmp_default_netif();
    const vs_mac_addr_t *dst_mac;

    // Set destination mac
    dst_mac = mac ? mac : vs_sdmp_broadcast_mac();

    // Fill request fields
    request.elements = elements;
    request.enable = enable ? 1 : 0;
    request.period_seconds = period_seconds;
    if (default_netif && default_netif->mac_addr) {
        default_netif->mac_addr(&request.recipient_mac);
    } else {
        VS_IOT_MEMSET(request.recipient_mac.bytes, 0xFF, ETH_ADDR_LEN);
    }

    // Send request
    if (0 !=
        vs_sdmp_send_request(netif, dst_mac, VS_INFO_SERVICE_ID, VS_INFO_POLL, (uint8_t *)&request, sizeof(request))) {
        return -1;
    }

    return 0;
}

/******************************************************************************/
static int
_snot_request_processor(const uint8_t *request,
                        const uint16_t request_sz,
                        uint8_t *response,
                        const uint16_t response_buf_sz,
                        uint16_t *response_sz) {
    vs_info_enum_response_t *enum_request = (vs_info_enum_response_t *)request;
    vs_sdmp_info_device_t device_info;

    // Check is callback present
    if (!_callbacks.device_start_cb) {
        return VS_SDMP_COMMAND_NOT_SUPPORTED;
    }

    // Check input parameters
    CHECK_RET(request, VS_CODE_ERR_INCORRECT_PARAMETER, "SDMP:SNOT error on a remote device");
    CHECK_RET(sizeof(vs_info_enum_response_t) == request_sz, VS_CODE_ERR_INCORRECT_ARGUMENT, "Wrong data size");

    // Get data from packed structure
    VS_IOT_MEMSET(&device_info, 0, sizeof(device_info));
    VS_IOT_MEMCPY(device_info.mac, enum_request->mac.bytes, ETH_ADDR_LEN);
    device_info.device_roles = enum_request->device_roles;

    // Invoke callback
    _callbacks.device_start_cb(&device_info);

    return VS_SDMP_COMMAND_NOT_SUPPORTED;
}

/******************************************************************************/
static int
_ginf_request_processor(const uint8_t *request,
                        const uint16_t request_sz,
                        uint8_t *response,
                        const uint16_t response_buf_sz,
                        uint16_t *response_sz) {

    vs_info_ginf_response_t *ginf_request = (vs_info_ginf_response_t *)request;
    vs_info_general_t general_info;

    // Check is callback present
    if (!_callbacks.general_info_cb) {
        return VS_CODE_OK;
    }

    // Check input parameters
    CHECK_RET(request, VS_CODE_ERR_INCORRECT_PARAMETER, "SDMP:GINF error on a remote device");
    CHECK_RET(sizeof(vs_info_ginf_response_t) == request_sz, VS_CODE_ERR_INCORRECT_ARGUMENT, "Wrong data size");

    // Get data from packed structure
    VS_IOT_MEMSET(&general_info, 0, sizeof(general_info));
    VS_IOT_MEMCPY(general_info.manufacture_id, ginf_request->manufacture_id, MANUFACTURE_ID_SIZE);
    VS_IOT_MEMCPY(general_info.device_type, ginf_request->device_type, DEVICE_TYPE_SIZE);
    VS_IOT_MEMCPY(general_info.default_netif_mac, ginf_request->default_netif_mac.bytes, ETH_ADDR_LEN);
    general_info.fw_major = ginf_request->fw_version.major;
    general_info.fw_minor = ginf_request->fw_version.minor;
    general_info.fw_patch = ginf_request->fw_version.patch;
    general_info.fw_dev_milestone = ginf_request->fw_version.dev_milestone;
    general_info.fw_dev_build = ginf_request->fw_version.dev_build;
    general_info.fw_timestamp = ginf_request->fw_version.timestamp;
    general_info.tl_version = ginf_request->tl_version;
    general_info.device_roles = ginf_request->device_roles;

    // Invoke callback function
    _callbacks.general_info_cb(&general_info);

    *response_sz = 0;

    return VS_CODE_OK;
}

/******************************************************************************/
static int
_stat_request_processor(const uint8_t *request,
                        const uint16_t request_sz,
                        uint8_t *response,
                        const uint16_t response_buf_sz,
                        uint16_t *response_sz) {

    vs_info_stat_response_t *stat_request = (vs_info_stat_response_t *)request;
    vs_info_statistics_t stat_info;

    // Check is callback present
    if (!_callbacks.statistics_cb) {
        return VS_CODE_OK;
    }

    // Check input parameters
    CHECK_RET(request, VS_CODE_ERR_INCORRECT_PARAMETER, "SDMP:STAT error on a remote device");
    CHECK_RET(sizeof(vs_info_stat_response_t) == request_sz, VS_CODE_ERR_INCORRECT_ARGUMENT, "Wrong data size");

    // Get data from packed structure
    VS_IOT_MEMSET(&stat_info, 0, sizeof(stat_info));
    VS_IOT_MEMCPY(stat_info.default_netif_mac, stat_request->mac.bytes, ETH_ADDR_LEN);
    stat_info.received = stat_request->received;
    stat_info.sent = stat_request->sent;

    // Invoke callback function
    _callbacks.statistics_cb(&stat_info);

    *response_sz = 0;

    return VS_CODE_OK;
}

/******************************************************************************/
static int
_enum_response_processor(bool is_ack, const uint8_t *response, const uint16_t response_sz) {

    vs_info_enum_response_t *enum_response = (vs_info_enum_response_t *)response;

    CHECK_RET(is_ack, VS_CODE_ERR_INCORRECT_PARAMETER, "ENUM error on a remote device");
    CHECK_RET(response, VS_CODE_ERR_INCORRECT_ARGUMENT, 0);
    CHECK_RET(sizeof(vs_info_enum_response_t) == response_sz, VS_CODE_ERR_INCORRECT_ARGUMENT, "Wrong data size");

    if (_devices_list && _devices_list_cnt < _devices_list_max) {
        VS_IOT_MEMCPY(_devices_list[_devices_list_cnt].mac, enum_response->mac.bytes, ETH_ADDR_LEN);
        _devices_list[_devices_list_cnt].device_roles = enum_response->device_roles;
        _devices_list_cnt++;

        return 0;
    }

    return VS_CODE_OK;
}

/******************************************************************************/
static int
_poll_response_processor(bool is_ack, const uint8_t *response, const uint16_t response_sz) {
    return VS_CODE_OK;
}

/******************************************************************************/
static int
_info_client_request_processor(const struct vs_netif_t *netif,
                               vs_sdmp_element_t element_id,
                               const uint8_t *request,
                               const uint16_t request_sz,
                               uint8_t *response,
                               const uint16_t response_buf_sz,
                               uint16_t *response_sz) {
    (void)netif;

    *response_sz = 0;

    switch (element_id) {

    case VS_INFO_SNOT:
        return _snot_request_processor(request, request_sz, response, response_buf_sz, response_sz);

    case VS_INFO_ENUM:
    case VS_INFO_POLL:
        return VS_SDMP_COMMAND_NOT_SUPPORTED;

    case VS_INFO_GINF:
        return _ginf_request_processor(request, request_sz, response, response_buf_sz, response_sz);

    case VS_INFO_STAT:
        return _stat_request_processor(request, request_sz, response, response_buf_sz, response_sz);

    default:
        VS_LOG_ERROR("Unsupported INFO command");
        VS_IOT_ASSERT(false);
        return VS_SDMP_COMMAND_NOT_SUPPORTED;
    }
}

/******************************************************************************/
static int
_info_client_response_processor(const struct vs_netif_t *netif,
                                vs_sdmp_element_t element_id,
                                bool is_ack,
                                const uint8_t *response,
                                const uint16_t response_sz) {
    (void)netif;

    switch (element_id) {

    case VS_INFO_SNOT:
    case VS_INFO_GINF:
    case VS_INFO_STAT:
        return VS_SDMP_COMMAND_NOT_SUPPORTED;

    case VS_INFO_ENUM:
        return _enum_response_processor(is_ack, response, response_sz);

    case VS_INFO_POLL:
        return _poll_response_processor(is_ack, response, response_sz);

    default:
        VS_LOG_ERROR("Unsupported INFO command");
        VS_IOT_ASSERT(false);
        return VS_SDMP_COMMAND_NOT_SUPPORTED;
    }
}

/******************************************************************************/
const vs_sdmp_service_t *
vs_sdmp_info_client(vs_sdmp_info_impl_t impl, vs_sdmp_info_callbacks_t callbacks) {
    // Save implementation
    VS_IOT_MEMCPY(&_info_impl, &impl, sizeof(_info_impl));

    _info_client.user_data = 0;
    _info_client.id = VS_INFO_SERVICE_ID;
    _info_client.request_process = _info_client_request_processor;
    _info_client.response_process = _info_client_response_processor;
    _info_client.periodical_process = NULL;

    // Save callbacks
    VS_IOT_MEMCPY(&_callbacks, &callbacks, sizeof(callbacks));

    return &_info_client;
}

/******************************************************************************/