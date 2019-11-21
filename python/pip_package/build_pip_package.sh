#!/bin/bash
#
# This script creates a temporary directory with the structure required by
# setuptools to build the deepmind_lab package and generates a binary
# distribution in the directory specified.
#
# This should be invoked directly and not via "bazel run" since the working
# directory has to be the root of the build tree.

set -e

function main() {
  if [[ ${#} -lt 1 ]]; then
    echo "No destination dir provided"
    exit 1
  fi

  DEST="${1}"
  TMPDIR=$(mktemp --directory -t tmp.XXXXXXXXXX)

  if [[ ! -d bazel-bin ]]; then
    echo "Could not find bazel-bin. Did you run from the root of the build tree?"
    exit 1
  fi

  if [[ ! -d bazel-bin/python/pip_package/build_pip_package.runfiles/org_deepmind_lab ]]; then
    # Old-style runfiles structure without the org name.
    cp --dereference --recursive -- \
        bazel-bin/python/pip_package/build_pip_package.runfiles \
        "${TMPDIR}/deepmind_lab"
  else
    # New-style runfiles structure
    cp --dereference --recursive -- \
        bazel-bin/python/pip_package/build_pip_package.runfiles/org_deepmind_lab \
        "${TMPDIR}/deepmind_lab"
  fi

  cp -- "python/pip_package/README.md" "${TMPDIR}"
  cp -- "python/pip_package/setup.py" "${TMPDIR}"
  cp -- "python/pip_package/__init__.py" "${TMPDIR}/deepmind_lab/__init__.py"
  cp -- "python/dmenv_module.py" "${TMPDIR}/deepmind_lab/dmenv_module.py"

  MANIFEST_IN="${TMPDIR}/MANIFEST.in"
  echo "include README.md" >> "${MANIFEST_IN}"
  cd "${TMPDIR}" && find deepmind_lab -type f | awk '$0="include "$0' >> "${MANIFEST_IN}"

  if [[ -z "${PYTHON_BIN_PATH}" ]]; then
    PYTHON_BIN_PATH=$(which python || which python3 || true)
  fi
  if [[ ! -e "${PYTHON_BIN_PATH}" ]]; then
    echo "Invalid python path. ${PYTHON_BIN_PATH} cannot be found" 1>&2
    exit 1
  fi

  pushd "${TMPDIR}" > /dev/null
  echo $(date) : "=== Building wheel"
  "${PYTHON_BIN_PATH}" setup.py bdist_wheel > /dev/null
  mkdir --parents -- "${DEST}"
  cp -- dist/* "${DEST}"
  popd > /dev/null
  rm --recursive --force -- "${TMPDIR}"
  echo $(date) : "=== Output wheel file is in: ${DEST}"
}

main "$@"
