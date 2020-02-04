/*
 * Copyright 2020-present STORDIS GmbH

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

      http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.

 */

//
// Created by kamal on 30.01.20.
//

#include "stordis_gearbox_phal.h"
#include <direct/SAL_Direct.h>


namespace stratum {
    namespace hal {
        namespace phal {
            namespace stordis_gearbox {

                StordisGBPhal* StordisGBPhal::singleton_ = nullptr;
                ABSL_CONST_INIT absl::Mutex StordisGBPhal::init_lock_(absl::kConstInit);

                StordisGBPhal* StordisGBPhal::CreateSingleton() {
                    absl::WriterMutexLock l(&init_lock_);
                    if (!singleton_) {
                        singleton_ = new StordisGBPhal();
                        singleton_->Initialize();
                    }
                    return singleton_;
                }

                ::util::Status StordisGBPhal::Initialize() {
                    SAL::startGearBox();
                    return ::util::OkStatus();

                }

                ::util::Status StordisGBPhal::PushChassisConfig(const ChassisConfig& config) {
                    absl::WriterMutexLock l(&config_lock_);
                    // TODO(unknown): Process Chassis Config here
                    return ::util::OkStatus();
                }

                ::util::Status StordisGBPhal::VerifyChassisConfig(const ChassisConfig& config) {
                    // TODO(unknown): Implement this function.
                    return ::util::OkStatus();
                }

                ::util::Status StordisGBPhal::Shutdown() {
                    absl::WriterMutexLock l(&config_lock_);
                    // TODO(unknown): add clean up code
                    initialized_ = false;

                    return ::util::OkStatus();
                }

                ::util::StatusOr<int> StordisGBPhal::RegisterTransceiverEventWriter(
                        std::unique_ptr<ChannelWriter<TransceiverEvent>> writer,
                        int priority){
                    return 1;
                }

                ::util::Status StordisGBPhal::UnregisterTransceiverEventWriter(int id){
                    return ::util::OkStatus();
                }

                ::util::Status StordisGBPhal::GetFrontPanelPortInfo(
                        int slot, int port, FrontPanelPortInfo* fp_port_info){
                    return ::util::OkStatus();
                }

                ::util::Status StordisGBPhal::SetPortLedState(int slot, int port, int channel,
                                               LedColor color, LedState state){
                    return ::util::OkStatus();
                }
                ::util::Status StordisGBPhal::RegisterSfpConfigurator(
                        int slot, int port, SfpConfigurator* configurator) {
                    return ::util::OkStatus();
                }

            }  // namespace onlp
        }  // namespace phal
    }  // namespace hal
}  // namespace stordis_gearbox