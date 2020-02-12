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

def _impl(repository_ctx):
    if "SAL_HOME" not in repository_ctx.os.environ:
        repository_ctx.file("BUILD", """
""")
        return
    sal_home = repository_ctx.os.environ["SAL_HOME"]
    repository_ctx.symlink(sal_home, "sal-home")
    repository_ctx.file("BUILD", """
package(
    default_visibility = ["//visibility:public"],
)

cc_library(
    name = "sal_headers",
    hdrs = glob(["sal-home/include/**/*.h",
    "sal-home/src/include/*.h",
    "sal-home/src/include/**/*.h"]),
    includes=["sal-home/include","sal-home/src/include"],
    linkopts = ["-ldl",],
)

cc_import(
  name = "sal_lib",
  hdrs = [],  # see cc_library rule above
  static_library = "sal-home/build/libsal.a",
  alwayslink = 1,
)

cc_import(
  name = "sal_gearbox_lib",
  hdrs = [],  # see cc_library rule above
  static_library = "sal-home/lib/libgearbox.a",
  alwayslink = 1,
)

""")

gb_configure = repository_rule(
    implementation = _impl,
    local = True,
    environ = ["SAL_HOME"],
)