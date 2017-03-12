# Description:
#   Build rule for Python and Numpy.
#   This rule works for Debian and Ubuntu. Other platforms might keep the
#   headers in different places, cf. 'How to build DeepMind Lab' in build.md.

cc_library(
    name = "python3",
    hdrs = glob(["include/python3.5/*.h"]),
    includes = ["include/python3.5"],
    visibility = ["//visibility:public"],
)
