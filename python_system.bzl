# Copyright 2021 DeepMind Technologies Limited.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
# ============================================================================

"""Generates a local repository that points at the system's Python installation."""

_BUILD_FILE = '''# Description:
#   Build rule for Python

exports_files(["defs.bzl"])

cc_library(
    name = "python_headers",
    hdrs = select({
        "@bazel_tools//tools/python:PY2": glob(["python2/**/*.h", "numpy2/**/*.h"]),
        "@bazel_tools//tools/python:PY3": glob(["python3/**/*.h", "numpy3/**/*.h"]),
    }),
    includes = select({
        "@bazel_tools//tools/python:PY2": ["python2", "numpy2"],
        "@bazel_tools//tools/python:PY3": ["python3", "numpy3"],
    }),
    visibility = ["//visibility:public"],
)
'''

_GET_PYTHON_INCLUDE_DIR = """
import sys
from distutils.sysconfig import get_python_inc
from numpy import get_include
sys.stdout.write("{}\\n{}\\n".format(get_python_inc(), get_include()))
""".strip()

_GET_PYTHON_SOABI = """
from packaging import tags
tag = next(iter(tags.sys_tags()))
print(f'struct(interpreter = "{tag.interpreter}", abi = "{tag.abi}", platform = "{tag.platform}")')
""".strip()

def _python_repo_impl(repository_ctx):
    """Creates external/<reponame>/BUILD, a python3 symlink, and other files."""

    repository_ctx.file("BUILD", _BUILD_FILE)
    py2_tags_def = """struct(interpreter = "py2", abi = "none", platform = "any")"""
    py3_tags_def = """struct(interpreter = "py3", abi = "none", platform = "any")"""

    if repository_ctx.attr.py_version in ["PY2", "PY2AND3"]:
        result = repository_ctx.execute(["python2", "-c", _GET_PYTHON_INCLUDE_DIR])
        if result.return_code:
            fail("Failed to run local Python2 interpreter: %s" % result.stderr)
        pypath, nppath = result.stdout.splitlines()
        repository_ctx.symlink(pypath, "python2")
        repository_ctx.symlink(nppath, "numpy2")

    if repository_ctx.attr.py_version in ["PY3", "PY2AND3"]:
        result = repository_ctx.execute(["python3", "-c", _GET_PYTHON_INCLUDE_DIR])
        if result.return_code:
            fail("Failed to run local Python3 interpreter: %s" % result.stderr)
        pypath, nppath = result.stdout.splitlines()
        repository_ctx.symlink(pypath, "python3")
        repository_ctx.symlink(nppath, "numpy3")

        result = repository_ctx.execute(["python3", "-c", _GET_PYTHON_SOABI])
        if result.return_code:
            fail("Failed to run local Python3 interpreter: %s" % result.stderr)
        py3_tags_def = result.stdout.strip()

    tags_def = """
def py_tags(py_ver):
    if py_ver == "py2":
        return {}
    elif py_ver == "py3":
        return {}
    else:
        fail("Argument must be either 'py2' or 'py3', was: '{{}}'.".format(py_ver))
""".format(py2_tags_def, py3_tags_def)
    repository_ctx.file("defs.bzl", tags_def.strip())

python_repo = repository_rule(
    implementation = _python_repo_impl,
    configure = True,
    local = True,
    attrs = {"py_version": attr.string(default = "PY2AND3", values = ["PY2", "PY3", "PY2AND3"])},
)
