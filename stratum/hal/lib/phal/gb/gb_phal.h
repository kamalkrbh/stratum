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

#ifndef STRATUM_GB_PHAL_H
#define STRATUM_GB_PHAL_H
#include "stratum/hal/lib/common/phal_interface.h"
#include "stratum/hal/lib/phal/gb/gb_phal_interface.h"
#include "stratum/hal/lib/phal/gb/gb_wrapper.h"

namespace stratum {
namespace hal {
namespace phal {
namespace gb {
class GbPhal final : public GbPhalInterface {
 public:
   ~GbPhal() override;

   // PhalInterface public methods.
   ::util::Status PushChassisConfig(const ChassisConfig &config) override LOCKS_EXCLUDED(config_lock_);

   ::util::Status VerifyChassisConfig(const ChassisConfig &config) override LOCKS_EXCLUDED(config_lock_);

   ::util::Status Shutdown() override LOCKS_EXCLUDED(config_lock_);

   // Creates the singleton instance. Expected to be called once to initialize
   // the instance.
   static GbPhal* CreateSingleton(GbInterface* gb_interface) LOCKS_EXCLUDED(config_lock_, init_lock_);

   // GbPhal is neither copyable nor movable.
   GbPhal(const GbPhal &) = delete;

   GbPhal &operator=(const GbPhal &) = delete;

 private:
   //private constructor
   GbPhal();

   // Calls all the one time start initialisations
   ::util::Status Initialize(GbInterface* gb_interface) LOCKS_EXCLUDED(config_lock_);
   // Internal mutex lock for protecting the internal maps and initializing the
   // singleton instance.
   static absl::Mutex init_lock_;
   // The singleton instance.
   static GbPhal *singleton_ GUARDED_BY(init_lock_);

   // Mutex lock for protecting the internal state when config is pushed or the
   // class is initialized so that other threads do not access the state while
   // the are being changed.
   mutable absl::Mutex config_lock_;

   // Determines if PHAL is fully initialized.
   bool initialized_ GUARDED_BY(config_lock_) = false;

   // Not owned by this class.
   GbInterface *gb_interface_ GUARDED_BY(config_lock_);
};

}
}
}
}


#endif //STRATUM_GB_PHAL_H
