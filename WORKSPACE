workspace(name = "org_deepmind_lab")

http_archive(
    name = "com_google_googletest",
    strip_prefix = "googletest-master",
    urls = ["https://github.com/google/googletest/archive/master.zip"],
)

http_archive(
    name = "com_google_absl",
    strip_prefix = "abseil-cpp-master",
    urls = ["https://github.com/abseil/abseil-cpp/archive/master.zip"],
)

new_http_archive(
    name = "eigen_archive",
    build_file = "eigen.BUILD",
    sha256 = "9a01fed6311df359f3f9af119fcf298a3353aef7d1b1bc86f6c8ae0ca6a2f842",
    strip_prefix = "/eigen-eigen-5d5dd50b2eb6",
    url = "https://bitbucket.org/eigen/eigen/get/5d5dd50b2eb6.zip",
)

new_http_archive(
    name = "glib_archive",
    build_file = "glib.BUILD",
    sha256 = "0cbb3d31c9d181bbcc97cba3d9dbe3250f75e2da25e5f7c8bf5a993fe54baf6a",
    strip_prefix = "glib-2.55.1",
    url = "http://ftp.gnome.org/pub/gnome/sources/glib/2.55/glib-2.55.1.tar.xz",
)

new_http_archive(
    name = "jpeg_archive",
    build_file = "jpeg.BUILD",
    sha256 = "650250979303a649e21f87b5ccd02672af1ea6954b911342ea491f351ceb7122",
    strip_prefix = "jpeg-9c",
    url = "http://www.ijg.org/files/jpegsrc.v9c.tar.gz",
)

new_http_archive(
    name = "libxml_archive",
    build_file = "libxml.BUILD",
    sha256 = "f63c5e7d30362ed28b38bfa1ac6313f9a80230720b7fb6c80575eeab3ff5900c",
    strip_prefix = "libxml2-2.9.7",
    url = "http://xmlsoft.org/sources/libxml2-2.9.7.tar.gz",
)

new_http_archive(
    name = "png_archive",
    build_file = "png.BUILD",
    sha256 = "7ffa5eb8f9f3ed23cf107042e5fec28699718916668bbce48b968600475208d3",
    strip_prefix = "libpng-1.6.34",
    url = "http://github.com/glennrp/libpng/archive/v1.6.34.zip",
)

new_http_archive(
    name = "zlib_archive",
    build_file = "zlib.BUILD",
    sha256 = "36658cb768a54c1d4dec43c3116c27ed893e88b02ecfcb44f2166f9c0b7f2a0d",
    strip_prefix = "zlib-1.2.8",
    url = "http://bazel-mirror.storage.googleapis.com/zlib.net/zlib-1.2.8.tar.gz",
)

# TODO: Replace with hermetic build
new_local_repository(
    name = "lua_system",
    build_file = "lua.BUILD",
    path = "/usr",
)

new_local_repository(
    name = "sdl_system",
    build_file = "sdl.BUILD",
    path = "/usr",
)

new_local_repository(
    name = "python_system",
    build_file = "python.BUILD",
    path = "/usr",
)
