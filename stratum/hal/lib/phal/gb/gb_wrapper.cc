//
// Created by kamal on 10.02.20.
//

#include "stratum/hal/lib/phal/gb/gb_wrapper.h"
#include "absl/memory/memory.h"
#include "stratum/glue/status/status.h"
#include "stratum/lib/macros.h"
#include <direct/SAL_Direct.h>
#include <SALTypes.h>

namespace stratum {
    namespace hal {
        namespace phal {
            namespace gb {
                GbWrapper* GbWrapper::singleton_ = nullptr;
                ABSL_CONST_INIT absl::Mutex GbWrapper::init_lock_(absl::kConstInit);
                GbWrapper* GbWrapper::CreateSingleton() {
                    absl::WriterMutexLock l(&init_lock_);
                    if (!singleton_) {
                        singleton_ = new GbWrapper();
                    }

                    return singleton_;
                }

                GbWrapper::GbWrapper() {
                    LOG(INFO) << "Initializing Gearbox.";
                    if (SAL::startGearBox()!=SAL::SAL_OK) {
                        LOG(FATAL) << "Failed to initialize Gearbox.";
                    }
                }

                GbWrapper::~GbWrapper() {
                    //TODO: Need to check for the procedures in Gearbox APIs for deinit.
                }
            }
        }
    }

}