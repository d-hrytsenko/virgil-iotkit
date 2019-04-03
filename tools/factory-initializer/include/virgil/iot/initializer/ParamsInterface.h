/**
 * Copyright (C) 2017 Virgil Security Inc.
 *
 * Lead Maintainer: Virgil Security Inc. <support@virgilsecurity.com>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     (1) Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *     (2) Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *
 *     (3) Neither the name of the copyright holder nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ''AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef VIRGIL_IOT_DEVICE_INITIALIZER_PARAMSINTERFACE_H
#define VIRGIL_IOT_DEVICE_INITIALIZER_PARAMSINTERFACE_H

#include <virgil/sdk/crypto/Crypto.h>
#include <virgil/iot/initializer/ProvisioningInfo.h>

using virgil::iot::initializer::ProvisioningInfo;
using virgil::sdk::crypto::Crypto;
using virgil::crypto::VirgilByteArray;

namespace virgil {
namespace iot {
    namespace initializer {
        class ParamsInterface {
        public:
            virtual ~ParamsInterface() = default;
            
            // Used for encryption of file with requests for Cards registration
            virtual VirgilByteArray fileEncryptionPrivateKey() const = 0;
            virtual std::string fileEncryptionPrivateKeyPassword() const = 0;
            virtual VirgilByteArray fileRecipientPublicKey() const = 0;
            virtual std::string exportFile() const = 0;
            virtual std::string deviceInfoFile() const = 0;
            
            // Info for device provisioning
            virtual ProvisioningInfo provisioningInfo() const = 0;
            
            // Credantials for signature of public key of device
            virtual VirgilByteArray deviceSignPrivateKey() const = 0;
            virtual std::string deviceSignPrivateKeyPassword() const = 0;
            virtual std::string factoryPrivateKey() const = 0;
            
        };
    }
}
}

#endif //VIRGIL_IOT_DEVICE_INITIALIZER_PARAMSINTERFACE_H
