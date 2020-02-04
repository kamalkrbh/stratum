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

#ifndef STRATUM_STORDIS_TIMESYNC_PHAL_H
#define STRATUM_STORDIS_TIMESYNC_PHAL_H

#include "stratum/hal/lib/common/phal_interface.h"


namespace stratum {
    namespace hal {
        namespace phal {
            namespace stordis_timesync {
                class StordisTimesyncPhal : public PhalInterface {
                public :
                    virtual ::util::Status Initialize();
                    static StordisTimesyncPhal* CreateSingleton() LOCKS_EXCLUDED(config_lock_);

                    ::util::Status PushChassisConfig(const ChassisConfig& config) override
                    LOCKS_EXCLUDED(config_lock_);
                    ::util::Status VerifyChassisConfig(const ChassisConfig& config) override
                    LOCKS_EXCLUDED(config_lock_);
                    ::util::Status Shutdown() override LOCKS_EXCLUDED(config_lock_);
                    ::util::StatusOr<int> RegisterTransceiverEventWriter(
                            std::unique_ptr<ChannelWriter<TransceiverEvent>> writer,
                            int priority) override LOCKS_EXCLUDED(config_lock_);
                    ::util::Status UnregisterTransceiverEventWriter(int id) override
                    LOCKS_EXCLUDED(config_lock_);
                    ::util::Status GetFrontPanelPortInfo(
                            int slot, int port, FrontPanelPortInfo* fp_port_info) override
                    LOCKS_EXCLUDED(config_lock_);
                    ::util::Status SetPortLedState(int slot, int port, int channel,
                                                   LedColor color, LedState state) override
                    LOCKS_EXCLUDED(config_lock_);
                    ::util::Status RegisterSfpConfigurator(
                            int slot, int port, SfpConfigurator* configurator) override;

                private:
                    static absl::Mutex init_lock_;
                    mutable absl::Mutex config_lock_;
                    bool initialized_ GUARDED_BY(config_lock_) = false;
                    static StordisTimesyncPhal* singleton_ GUARDED_BY(init_lock_);
                };
            }  // namespace onlp
        }  // namespace phal
    }  // namespace hal
}  // namespace stordis_timesync

#endif //STRATUM_STORDIS_TIMESYNC_PHAL_H
