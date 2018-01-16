#!/bin/bash

# Copyright (C) 2016 Google Inc.
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
#
################################################################################

# Driver script for compiling a map in .map format into a .pk3 archive
# (containing a .bsp and a .aas file) that can be loaded by DeepMind Lab.
#
# You may replace this script with a different implementation as long as
# it has the same interface, which is as follows:
#
# compile_map.sh [-a] /path/to/mymap
#
# This expects:
#   * A file /path/to/mymap.map.
#   * The directory /path/to is writable.
#
# Effects:
#   * Creates a package file /path/to/mymap.pk3 which contains the files
#     maps/mymap.bsp and, if -a is specified, maps/mymap.aas.
#
# This implementation uses Bash and requires the bspc and q3map2 tools to exist
# in fixed locations relative to this script, and the "zip" command must work.

set -e

function usage() {
  echo "Usage: ${0} [-a] [-m map_path] path/to/mymap"
  echo "  -a: Generate AAS data."
  echo "  -m: Path to map location."
  exit 1
}

GENERATE_AAS=false
MAP_LOCATION=

while getopts ":am:" opt; do
  case $opt in
    a ) GENERATE_AAS=true ;;
    m ) MAP_LOCATION="${OPTARG}" ;;
    \? ) usage ;;
  esac
done

shift $((OPTIND - 1))

readonly MAPBASE="${1}"

readonly BASE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
readonly Q3MP="${BASE}/../../q3map2/q3map2"
readonly BSPC="${BASE}/../../bspc"

readonly DIR="$(dirname "${MAPBASE}")"
readonly MPF="$(basename "${MAPBASE}")"
readonly MPB="${MPF/.map/}"

if [[ -n $MAP_LOCATION ]]; then
  mkdir --parents -- "${DIR}"
  cp --force -- "${MAP_LOCATION}" "${MAPBASE}.map"
  chmod 660 -- "${MAPBASE}.map"
fi

function die {
  echo "Error: ${1}"
  exit 1
}

function check_exe {
  [[ -x "${1}" ]] || die "could not find ${2} tool"
}

## Sanity checking

check_exe "${Q3MP}" "q3map2"
check_exe "${BSPC}" "bspc"
[[ -n "${MAPBASE}" ]] || die "missing map argument"
[[ -f "${MAPBASE}.map" ]] || die "no map file '${MAPBASE}.map'"

## Main logic

function clean_up {
  rm --force -- "${DIR}/${MPB}."{bsp,map,prt,srf}
}

# Step 1: q3map2 to generate the BSP
${Q3MP} -fs_basepath "${BASE}/../.." -fs_game baselab -meta -patchmeta -threads 1 "${MAPBASE}"
${Q3MP} -fs_basepath "${BASE}/../.." -fs_game baselab -vis -threads 8 "${MAPBASE}"
${Q3MP} -fs_basepath "${BASE}/../.." -fs_game baselab -light -threads 8 -fast \
        -patchshadows -samples 2 -bounce 3 -gamma 2 -compensate 4 -dirty \
        "${MAPBASE}"

# Step 2: bscp to generate the AAS

if [[ $GENERATE_AAS = true ]]
then
  ${BSPC} -optimize -forcesidesvisible -bsp2aas "${MAPBASE}.map" -output "${DIR}"
fi

# Step 3: Zip .bsp and .aas into a .pk3 archive.

mkdir --parents -- "${DIR}/maps"
if [[ $GENERATE_AAS = true ]]; then
  mv --target-directory="${DIR}/maps" -- "${DIR}/${MPB}.bsp" "${DIR}/${MPB}.aas"
else
  mv --target-directory="${DIR}/maps" -- "${DIR}/${MPB}.bsp"
fi

rm -f -- "${DIR}/${MPB}.pk3"
if [[ $GENERATE_AAS = true ]]; then
  (cd -- "${DIR}" && zip "${MPB}.pk3" -- "maps/${MPB}.bsp" "maps/${MPB}.aas")
  rm -- "${DIR}/maps/${MPB}.bsp" "${DIR}/maps/${MPB}.aas"
else
  (cd -- "${DIR}" && zip "${MPB}.pk3" -- "maps/${MPB}.bsp")
  rm -- "${DIR}/maps/${MPB}.bsp"
fi

rmdir -- "${DIR}/maps"
clean_up

# Done!

echo "Created map pack ${DIR}/${MPB}.pk3"
