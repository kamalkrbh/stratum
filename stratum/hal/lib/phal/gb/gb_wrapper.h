/*
 * Copyright 2020-present STORDIS GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//
// Created by kamal on 07.02.20.
//

#ifndef STRATUM_GB_WRAPPER_H
#define STRATUM_GB_WRAPPER_H

#include "stratum/glue/status/status.h"
#include "absl/synchronization/mutex.h"

namespace stratum {
    namespace hal {
        namespace phal {
            namespace gb {
// A interface for Gearbox calls.
// APIs of SAL (STORDIS's Switch Abstraction layer) are used as an interface to gearbox
// return ::util::Status.
                class GbInterface {
                public:
                    virtual ~GbInterface() {}
                };

                class GbWrapper : public GbInterface {
                public:
                    static GbWrapper *CreateSingleton();
                    ~GbWrapper() override;

                private:
                    GbWrapper();
                    static GbWrapper *singleton_
                    GUARDED_BY(init_lock_);
                    static absl::Mutex init_lock_;
                };
            }
        }
    }
}

#endif //STRATUM_GB_WRAPPER_H
