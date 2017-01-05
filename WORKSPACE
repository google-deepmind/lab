workspace(name = "org_deepmind_lab")

new_git_repository(
    name = "googletest",
    build_file = "googletest.BUILD",
    commit = "d8fe70f477d8a99745b69f7650f75eacf96866f9",
    remote = "https://github.com/google/googletest.git",
)

new_http_archive(
    name = "eigen_archive",
    build_file = "eigen.BUILD",
    sha256 = "7e05dd4b9866ef0aa4498be34752a362596cc5db2f8439cee111e4ea54046b57",
    strip_prefix = "eigen-eigen-1d454915237a",
    url = "http://bitbucket.org/eigen/eigen/get/1d454915237a.tar.gz",
)

new_http_archive(
    name = "glib_archive",
    build_file = "glib.BUILD",
    strip_prefix = "glib-2.38.2",
    sha256 = "056a9854c0966a0945e16146b3345b7a82562a5ba4d5516fd10398732aea5734",
    url = "http://ftp.gnome.org/pub/gnome/sources/glib/2.38/glib-2.38.2.tar.xz",
)

new_http_archive(
    name = "jpeg_archive",
    build_file = "jpeg.BUILD",
    sha256 = "3a753ea48d917945dd54a2d97de388aa06ca2eb1066cbfdc6652036349fe05a7",
    strip_prefix = "jpeg-9a",
    url = "http://www.ijg.org/files/jpegsrc.v9a.tar.gz",
)

new_http_archive(
    name = "libxml_archive",
    build_file = "libxml.BUILD",
    strip_prefix = "libxml2-2.9.4",
    url = "http://xmlsoft.org/sources/libxml2-2.9.4.tar.gz",
)

new_http_archive(
    name = "png_archive",
    build_file = "png.BUILD",
    sha256 = "c35bcc6387495ee6e757507a68ba036d38ad05b415c2553b3debe2a57647a692",
    strip_prefix = "libpng-1.2.53",
    url = "http://github.com/glennrp/libpng/archive/v1.2.53.zip",
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
