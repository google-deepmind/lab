# Description:
#   Build rule for Python and Numpy.
#   This rule works for Debian and Ubuntu. Other platforms might keep the
#   headers in different places, cf. 'How to build DeepMind Lab' in build.md.

cc_library(
    name = "python",
    hdrs = glob([
        "include/python2.7/*.h",
        # numpy
        "local/lib/python2.7/site-packages/numpy/core/include/**/*.h",
        # Homebrew numpy
        "local/opt/numpy/lib/python2.7/site-packages/numpy/core/include/**/*.h",
    ]),
    includes = [
        "include/python2.7",
        # numpy
        "local/lib/python2.7/site-packages/numpy/core/include",
        # Homebrew numpy
        "local/opt/numpy/lib/python2.7/site-packages/numpy/core/include",
    ],
    visibility = ["//visibility:public"],
)
