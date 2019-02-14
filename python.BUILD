# Description:
#   Build rule for Python and Numpy.
#   This rule works for Debian and Ubuntu. Other platforms might keep the
#   headers in different places, cf. 'How to build DeepMind Lab' in build.md.

# MUST ADD: /usr/include/x86_64-linux-gnu/python2.7/pyconfig.h

cc_library(
    name = "python",
    hdrs = glob([
        "include/python2.7/*.h",
        "include/python2.7/numpy/*.h"
    ]),
    includes = [
        "include/python2.7/",
        "include/python2.7/numpy"
    ],
    visibility = ["//visibility:public"],
)
