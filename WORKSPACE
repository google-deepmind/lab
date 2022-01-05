workspace(name = "org_deepmind_lab")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@//:python_system.bzl", "python_repo")

http_archive(
    name = "com_google_googletest",
    strip_prefix = "googletest-main",
    urls = ["https://github.com/google/googletest/archive/main.zip"],
)

http_archive(
    name = "bazel_skylib",
    strip_prefix = "bazel-skylib-main",
    urls = ["https://github.com/bazelbuild/bazel-skylib/archive/main.zip"],
)

http_archive(
    name = "com_google_absl",
    strip_prefix = "abseil-cpp-master",
    urls = ["https://github.com/abseil/abseil-cpp/archive/master.zip"],
)

http_archive(
    name = "com_google_absl_py",
    strip_prefix = "abseil-py-main",
    urls = ["https://github.com/abseil/abseil-py/archive/main.zip"],
)

http_archive(
    name = "enum34_archive",
    build_file = "@com_google_absl_py//third_party:enum34.BUILD",
    sha256 = "8ad8c4783bf61ded74527bffb48ed9b54166685e4230386a9ed9b1279e2df5b1",
    urls = [
        "https://mirror.bazel.build/pypi.python.org/packages/bf/3e/31d502c25302814a7c2f1d3959d2a3b3f78e509002ba91aea64993936876/enum34-1.1.6.tar.gz",
        "https://pypi.python.org/packages/bf/3e/31d502c25302814a7c2f1d3959d2a3b3f78e509002ba91aea64993936876/enum34-1.1.6.tar.gz",
    ],
)

http_archive(
    name = "funcsigs_archive",
    build_file = "@//bazel:funcsigs.BUILD",
    strip_prefix = "funcsigs-1.0.2",
    urls = [
        "https://pypi.python.org/packages/94/4a/db842e7a0545de1cdb0439bb80e6e42dfe82aaeaadd4072f2263a4fbed23/funcsigs-1.0.2.tar.gz",
    ],
)

http_archive(
    name = "eigen_archive",
    build_file = "@//bazel:eigen.BUILD",
    sha256 = "9a01fed6311df359f3f9af119fcf298a3353aef7d1b1bc86f6c8ae0ca6a2f842",
    strip_prefix = "/eigen-eigen-5d5dd50b2eb6",
    urls = [
        "https://mirror.bazel.build/bitbucket.org/eigen/eigen/get/5d5dd50b2eb6.zip",
        "https://bitbucket.org/eigen/eigen/get/5d5dd50b2eb6.zip",
    ],
)

http_archive(
    name = "glib_archive",
    build_file = "@//bazel:glib.BUILD",
    sha256 = "0cbb3d31c9d181bbcc97cba3d9dbe3250f75e2da25e5f7c8bf5a993fe54baf6a",
    strip_prefix = "glib-2.55.1",
    urls = [
        "https://mirror.bazel.build/ftp.gnome.org/pub/gnome/sources/glib/2.55/glib-2.55.1.tar.xz",
        "https://ftp.gnome.org/pub/gnome/sources/glib/2.55/glib-2.55.1.tar.xz",
    ],
)

http_archive(
    name = "jpeg_archive",
    build_file = "@//bazel:jpeg.BUILD",
    sha256 = "2303a6acfb6cc533e0e86e8a9d29f7e6079e118b9de3f96e07a71a11c082fa6a",
    strip_prefix = "jpeg-9d",
    urls = ["http://www.ijg.org/files/jpegsrc.v9d.tar.gz"],
)

http_archive(
    name = "libxml_archive",
    build_file = "@//bazel:libxml.BUILD",
    sha256 = "f63c5e7d30362ed28b38bfa1ac6313f9a80230720b7fb6c80575eeab3ff5900c",
    strip_prefix = "libxml2-2.9.7",
    urls = [
        "https://mirror.bazel.build/xmlsoft.org/sources/libxml2-2.9.7.tar.gz",
        "http://xmlsoft.org/sources/libxml2-2.9.7.tar.gz",
    ],
)

http_archive(
    name = "png_archive",
    build_file = "@//bazel:png.BUILD",
    sha256 = "c2c50c13a727af73ecd3fc0167d78592cf5e0bca9611058ca414b6493339c784",
    strip_prefix = "libpng-1.6.37",
    urls = [
        "https://mirror.bazel.build/github.com/glennrp/libpng/archive/v1.6.37.zip",
        "https://github.com/glennrp/libpng/archive/v1.6.37.zip",
    ],
)

http_archive(
    name = "zlib_archive",
    build_file = "@//bazel:zlib.BUILD",
    sha256 = "c3e5e9fdd5004dcb542feda5ee4f0ff0744628baf8ed2dd5d66f8ca1197cb1a1",
    strip_prefix = "zlib-1.2.11",
    urls = [
        "https://mirror.bazel.build/zlib.net/zlib-1.2.11.tar.gz",
        "https://zlib.net/zlib-1.2.11.tar.gz",
    ],
)

http_archive(
    name = "six_archive",
    build_file = "@//bazel:six.BUILD",
    sha256 = "30639c035cdb23534cd4aa2dd52c3bf48f06e5f4a941509c8bafd8ce11080259",
    strip_prefix = "six-1.15.0",
    urls = [
        "https://mirror.bazel.build/pypi.python.org/packages/source/s/six/six-1.15.0.tar.gz",
        "https://pypi.python.org/packages/source/s/six/six-1.15.0.tar.gz",
    ],
)

http_archive(
    name = "lua_archive",
    build_file = "@//bazel:lua.BUILD",
    sha256 = "2640fc56a795f29d28ef15e13c34a47e223960b0240e8cb0a82d9b0738695333",
    strip_prefix = "lua-5.1.5/src",
    urls = [
        "https://mirror.bazel.build/www.lua.org/ftp/lua-5.1.5.tar.gz",
        "https://www.lua.org/ftp/lua-5.1.5.tar.gz",
    ],
)

http_archive(
    name = "dm_env_archive",
    build_file = "@//bazel:dm_env.BUILD",
    strip_prefix = "dm_env-master",
    urls = ["https://github.com/deepmind/dm_env/archive/master.zip"],
)

http_archive(
    name = "tree_archive",
    build_file = "@//bazel:tree.BUILD",
    strip_prefix = "tree-master",
    urls = ["https://github.com/deepmind/tree/archive/master.zip"],
)

http_archive(
    name = "pybind11_archive",
    build_file = "@//bazel:pybind11.BUILD",
    strip_prefix = "pybind11-master",
    urls = ["https://github.com/pybind/pybind11/archive/master.zip"],
)

# TODO: Replace with hermetic build
new_local_repository(
    name = "sdl_system",
    build_file = "@//bazel:sdl.BUILD",
    path = "/usr",
)

python_repo(
    name = "python_system",
    py_version = "PY3",
)
