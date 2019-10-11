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

#ifndef VS_FIRMWARE_H
#define VS_FIRMWARE_H

#include <virgil/iot/storage_hal/storage_hal.h>
#include <virgil/iot/status_code/status_code.h>
#include <virgil/iot/provision/provision.h>

typedef struct __attribute__((__packed__)) {
    vs_file_info_t info;
    uint8_t padding;
    uint16_t chunk_size;
    uint32_t firmware_length;
    uint32_t app_size; // firmware_length + fill_size + footer
} vs_firmware_descriptor_t;

typedef struct __attribute__((__packed__)) {
    uint8_t signatures_count;
    vs_firmware_descriptor_t descriptor;
    uint8_t signatures[];
} vs_firmware_footer_t;

vs_status_e
vs_firmware_init(const vs_storage_op_ctx_t *ctx);

vs_status_e
vs_firnware_deinit(const vs_storage_op_ctx_t *ctx);

vs_status_e
vs_firmware_save_firmware_chunk(const vs_storage_op_ctx_t *ctx,
                              const vs_firmware_descriptor_t *descriptor,
                              const uint8_t *chunk,
                              size_t chunk_sz,
                              size_t offset);

vs_status_e
vs_firmware_save_firmware_footer(const vs_storage_op_ctx_t *ctx, const vs_firmware_descriptor_t *descriptor, const uint8_t *footer);

vs_status_e
vs_firmware_load_firmware_chunk(const vs_storage_op_ctx_t *ctx,
                              const vs_firmware_descriptor_t *descriptor,
                              uint32_t offset,
                              uint8_t *data,
                              size_t buf_sz,
                              size_t *data_sz);

vs_status_e
vs_firmware_load_firmware_footer(const vs_storage_op_ctx_t *ctx,
                               const vs_firmware_descriptor_t *descriptor,
                               uint8_t *data,
                               size_t buff_sz,
                                 size_t *data_sz);

vs_status_e
vs_firmware_verify_firmware(const vs_storage_op_ctx_t *ctx, const vs_firmware_descriptor_t *descriptor);

vs_status_e
vs_firmware_save_firmware_descriptor(const vs_storage_op_ctx_t *ctx, const vs_firmware_descriptor_t *descriptor);

vs_status_e
vs_firmware_get_own_firmware_descriptor(vs_firmware_descriptor_t *descriptor);

vs_status_e
vs_firmware_load_firmware_descriptor(const vs_storage_op_ctx_t *ctx,
                                   const uint8_t manufacture_id[VS_DEVICE_MANUFACTURE_ID_SIZE],
                                   const uint8_t device_type[VS_DEVICE_TYPE_SIZE],
                                   vs_firmware_descriptor_t *descriptor);

vs_status_e
vs_firmware_delete_firmware(const vs_storage_op_ctx_t *ctx, const vs_firmware_descriptor_t *descriptor);

vs_status_e
vs_firmware_install_firmware(const vs_storage_op_ctx_t *ctx, const vs_firmware_descriptor_t *descriptor);

char *
vs_firmware_describe_version(const vs_file_version_t *fw_ver, char *buffer, size_t buf_size);

#endif // VS_FIRMWARE_H
