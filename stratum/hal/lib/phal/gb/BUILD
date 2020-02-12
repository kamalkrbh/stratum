#
# Copyright 2020-present STORDIS GmbH
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

licenses(["notice"])  # Apache v2

package(
    default_visibility = ["//visibility:public"],
)

load("//bazel:rules.bzl", "HOST_ARCHES", "stratum_cc_library")

stratum_cc_library(
    name = "gb_wrapper",
    srcs = ["gb_wrapper.cc"],
    hdrs = [
        "gb_wrapper.h",
    ],
    deps = [
         "//stratum/glue/status",
        "@local_sal_bin//:sal_gearbox_lib",
        "@local_sal_bin//:sal_headers",
        "@local_sal_bin//:sal_lib",
        "//stratum/lib:macros",
    ],
)

stratum_cc_library(
    name = "gb_phal_interface",
    hdrs = ["gb_phal_interface.h"],
    deps = [
        "//stratum/glue/status:statusor",
        "//stratum/glue/status",
        "//stratum/hal/lib/common:phal_interface",
        "//stratum/hal/lib/phal:attribute_database",
        "//stratum/hal/lib/phal:phal_backend_interface",
        "//stratum/lib:macros",
    ],
)

stratum_cc_library(
    name = "gb_phal",
    hdrs = ["gb_phal.h"],
    srcs = ["gb_phal.cc"],
    deps = [
        ":gb_phal_interface",
        ":gb_wrapper",
    ],
)