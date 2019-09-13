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

#ifndef VS_STORAGE_HAL_H
#define VS_STORAGE_HAL_H

#define VS_STORAGE_ELEMENT_ID_MAX  (32)

typedef enum {
    VS_STORAGE_OK = 0,
    VS_STORAGE_ERROR_GENERAL = -1,
    VS_STORAGE_ERROR_PARAMS = -2,
    VS_STORAGE_ERROR_SMALL_BUFFER = -3,
    VS_STORAGE_ERROR_WRITE = -4,
    VS_STORAGE_ERROR_READ = -5,
    VS_STORAGE_ERROR_NOT_FOUND = -6,
} vs_storage_result_e;

typedef uint8_t vs_storage_element_id_t[VS_STORAGE_ELEMENT_ID_MAX];

/*
 * This context is generated by user call of
 * vs_storage_hal_ctx_t vs_storage_xxx_init( Parameters list depends on implementation of storage );
 *
 */
typedef void *vs_storage_hal_ctx_t;

/*
 * File descriptor depends on implementation of storage
 */
typedef void *vs_storage_file_t;

typedef int (*vs_storage_deinit_hal_t)(
        vs_storage_hal_ctx_t storage_ctx); // After this call storage_ctx is incorrect and must be zeroed.

typedef vs_storage_file_t (*vs_storage_open_hal_t)(
        const vs_storage_hal_ctx_t storage_ctx,
        const vs_storage_element_id_t id);

typedef int (*vs_storage_close_hal_t)(
        const vs_storage_hal_ctx_t storage_ctx,
        vs_storage_file_t file); // After this call file is incorrect and must be zeroed.


typedef int (*vs_storage_save_hal_t)(
        const vs_storage_hal_ctx_t storage_ctx,
        const vs_storage_file_t file,
        size_t offset,
        const uint8_t *in_data,
        size_t data_sz);

typedef int (*vs_storage_load_hal_t)(
        const vs_storage_hal_ctx_t storage_ctx,
        const vs_storage_file_t file,
        size_t offset,
        uint8_t *out_data,
        size_t data_sz);

typedef int (*vs_storage_file_size_hal_t)(
        const vs_storage_hal_ctx_t storage_ctx,
        const vs_storage_element_id_t id);

typedef int (*vs_storage_del_hal_t)(
        const vs_storage_hal_ctx_t storage_ctx,
        const vs_storage_element_id_t id);

// HAL
typedef struct {
    vs_storage_deinit_hal_t deinit;

    vs_storage_open_hal_t open;
    vs_storage_close_hal_t close;

    vs_storage_save_hal_t save;
    vs_storage_load_hal_t load;
    vs_storage_file_size_hal_t size;

    vs_storage_del_hal_t del;
}vs_storage_op_impl_t;

typedef struct {
    // Context if required
    vs_storage_hal_ctx_t storage_ctx;
    size_t file_sz_limit;     // Maximum size of file
    vs_storage_op_impl_t impl;
} vs_storage_op_ctx_t;


#endif //VS_STORAGE_HAL_H
