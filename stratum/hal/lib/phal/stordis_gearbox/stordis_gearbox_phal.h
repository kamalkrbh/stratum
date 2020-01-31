//
// Created by kamal on 30.01.20.
//

#ifndef STRATUM_STORDIS_GEARBOX_PHAL_H
#define STRATUM_STORDIS_GEARBOX_PHAL_H

#include "stratum/hal/lib/common/phal_interface.h"


namespace stratum {
    namespace hal {
        namespace phal {
            namespace stordis_gearbox {
                class StordisGBPhal : public PhalInterface {
                    virtual ::util::Status Initialize();
                };
            }  // namespace onlp
        }  // namespace phal
    }  // namespace hal
}  // namespace stordis_gearbox

#endif //STRATUM_STORDIS_GEARBOX_PHAL_H
