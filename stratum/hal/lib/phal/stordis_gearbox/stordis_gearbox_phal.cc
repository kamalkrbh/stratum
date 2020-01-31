
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
#include "stratum/glue/status/status.h"
namespace stratum {
    namespace hal {
        namespace phal {
            namespace stordis_gearbox {

                ::util::Status StordisGBPhal::Initialize() {
                    return SAL::startGearBox();
                }
            }  // namespace onlp
        }  // namespace phal
    }  // namespace hal
}  // namespace stordis_gearbox