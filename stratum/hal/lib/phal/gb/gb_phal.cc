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

#include "stratum/hal/lib/phal/gb/gb_phal.h"
#include "stratum/hal/lib/phal/gb/gb_wrapper.h"

namespace stratum {
    namespace hal {
        namespace phal {
            namespace gb {
                GbPhal* GbPhal::singleton_ = nullptr;
                ABSL_CONST_INIT absl::Mutex GbPhal::init_lock_(absl::kConstInit);
                GbPhal::GbPhal() : gb_interface_(nullptr) {}
                GbPhal::~GbPhal() {}
                GbPhal *GbPhal::CreateSingleton(GbInterface *gb_interface) {
                    absl::WriterMutexLock l(&init_lock_);
                    if (!singleton_) {
                        singleton_ = new GbPhal();
                    }
                    auto status = singleton_->Initialize(gb_interface);
                    if (!status.ok()) {
                        LOG(ERROR) << "GbPhal failed to initialize: " << status;
                        delete singleton_;
                        singleton_ = nullptr;
                    }
                    return singleton_;
                }

                // Initialize the gb interface and phal DB
                ::util::Status GbPhal::Initialize(GbInterface *gb_interface) {
                    absl::WriterMutexLock l(&config_lock_);
                    if (!initialized_) {
                        CHECK_RETURN_IF_FALSE(gb_interface != nullptr);
                        gb_interface_ = gb_interface;
                        initialized_ = true;
                    }
                    return ::util::OkStatus();
                }

                ::util::Status GbPhal::PushChassisConfig(const ChassisConfig& config) {
                    absl::WriterMutexLock l(&config_lock_);

                    // TODO(unknown): Process Chassis Config here

                    return ::util::OkStatus();
                }

                ::util::Status GbPhal::VerifyChassisConfig(const ChassisConfig& config) {
                    // TODO(unknown): Implement this function.
                    return ::util::OkStatus();
                }

                ::util::Status GbPhal::Shutdown() {
                    absl::WriterMutexLock l(&config_lock_);
                    gb_interface_ = nullptr;
                    initialized_ = false;
                    return ::util::OkStatus();
                }

            }
        }
    }
}