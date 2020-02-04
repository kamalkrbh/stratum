#
# Copyright 2018 Google LLC
# Copyright 2018-present Open Networking Foundation
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

load("//bazel:rules.bzl", "HOST_ARCHES", "stratum_cc_binary", "stratum_cc_library")

stratum_cc_binary(
    name = "stratum_bf",
    srcs = [
        "main.cc",
    ],
    arches = HOST_ARCHES,
    deps = select({
        "//conditions:default": [
            "bf_lib",
        ],
        "//stratum:stordis_bf2556x_1t_profile": [
            "bf_lib",
            "stordis_ts_lib",
            "stordis_gb_lib",
        ],
        "//stratum:stordis_bf6064x_t_profile": [
            "bf_lib",
            "//stratum/hal/lib/phal/stordis_timesync:stordis_timesync_phal",
        ],
        "//stratum:barefoot_profile": ["bf_lib"],
    }),
)

stratum_cc_library(
    name = "stordis_ts_lib",
    copts = ["-DSTORDIS_TS"],
    deps = ["//stratum/hal/lib/phal/stordis_timesync:stordis_timesync_phal"],
)

stratum_cc_library(
    name = "stordis_gb_lib",
    copts = ["-DSTORDIS_GB"],
    deps = ["//stratum/hal/lib/phal/stordis_gearbox:stordis_gearbox_phal"],
)

stratum_cc_library(
    name = "bf_lib",
    data = [
        "platforms/x86-64-accton-wedge100bf-32x-r0.json",
        "platforms/x86-64-delta-ag9064v1-r0.json",
        "platforms/x86-64-inventec-d5254-r0.json",
        "tofino_skip_p4.conf",
        "tofino_skip_p4_no_bsp.conf",
    ],
    deps = [
        "//stratum/glue:init_google",
        "//stratum/glue:logging",
        "//stratum/hal/lib/barefoot:bf_chassis_manager",
        "//stratum/hal/lib/barefoot:bf_pal_wrapper",
        "//stratum/hal/lib/barefoot:bf_switch",
        "//stratum/hal/lib/common:hal",
        "//stratum/hal/lib/phal:phal_sim",
        "//stratum/hal/lib/phal/onlp:onlpphal",
        "//stratum/hal/lib/pi:pi_node_bf",
        "//stratum/lib/security:auth_policy_checker",
        "//stratum/lib/security:credentials_manager",
        "@com_github_google_glog//:glog",
        "@local_barefoot_bin//:bf_drivers",
        "@local_barefoot_bin//:bf_headers",
        "@local_barefoot_bin//:bf_switchd",
        "@local_barefoot_bin//:bf_sys",
        "@local_barefoot_bin//:bf_utils",
    ],
)
