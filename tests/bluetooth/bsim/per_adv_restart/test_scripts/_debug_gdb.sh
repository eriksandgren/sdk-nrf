#!/usr/bin/env bash
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#
# Start BabbleSim PHY and run west-built zephyr.exe under gdb.
#
# Prerequisites:
#   - ZEPHYR_BASE, BSIM_OUT_PATH (see nrf52_bsim board docs)
#   - west build -b nrf52_bsim/native tests/bluetooth/bsim/per_adv_restart \
#       -d build/per_adv_restart -- -DCONFIG_NO_OPTIMIZATIONS=y
#
# Usage (from anywhere):
#   ZEPHYR_BASE=... BSIM_OUT_PATH=... \
#     tests/bluetooth/bsim/per_adv_restart/test_scripts/_debug_gdb.sh
#
# Environment overrides:
#   WEST_BUILD_DIR   west build directory (default: <nrf>/build/per_adv_restart)
#   SIM_LENGTH       PHY sim_length in microseconds (default: 120e6)
#   SIMULATION_ID    BabbleSim -s name (default: per_adv_restart)
#   GDB              gdb binary (default: gdb)
#   GDB_INIT_FILE    gdb -x command file (default: ./_debug_gdb.gdb; set to "none" to skip)
#   GDB_AUTO_RUN=1   pass -ex run after the init file
#
# Extra gdb CLI flags (e.g. -ex run) may be passed as script arguments.

set -eu

: "${ZEPHYR_BASE:?ZEPHYR_BASE must be set}"
source "${ZEPHYR_BASE}/tests/bsim/sh_common.source"

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
nrf_root="$(cd "${script_dir}/../../../../.." && pwd)"
WEST_BUILD_DIR="${WEST_BUILD_DIR:-${nrf_root}/tests/bluetooth/bsim/per_adv_restart/build/per_adv_restart}"
EXE="${WEST_BUILD_DIR}/zephyr/zephyr.exe"

verbosity_level=2
simulation_id="${SIMULATION_ID:-per_adv_restart}"
test_id="per_adv_restart"
sim_length="${SIM_LENGTH:-120e6}"
gdb_bin="${GDB:-gdb}"
gdb_init_file="${GDB_INIT_FILE:-${script_dir}/_debug_gdb.gdb}"

if [[ ! -x "${EXE}" ]]; then
	echo "Cannot find ${EXE}" >&2
	echo "Build with:" >&2
	echo "  west build -p always -b nrf52_bsim/native \\" >&2
	echo "    tests/bluetooth/bsim/per_adv_restart -d build/per_adv_restart -- \\" >&2
	echo "    -DCONFIG_NO_OPTIMIZATIONS=y" >&2
	exit 1
fi

if [[ ! -x "${BSIM_OUT_PATH}/bin/bs_2G4_phy_v1" ]]; then
	echo "Cannot find ${BSIM_OUT_PATH}/bin/bs_2G4_phy_v1" >&2
	exit 1
fi

phy_pid=""

cleanup() {
	if [[ -n "${phy_pid}" ]]; then
		kill -TERM "${phy_pid}" 2>/dev/null || true
		wait "${phy_pid}" 2>/dev/null || true
	fi
}
trap cleanup EXIT INT

cd "${BSIM_OUT_PATH}/bin"
./bs_2G4_phy_v1 -v="${verbosity_level}" -s="${simulation_id}" \
	-D=1 -sim_length="${sim_length}" &
phy_pid=$!

device_args=(
	-v="${verbosity_level}"
	-s="${simulation_id}"
	-d=0
	-testid="${test_id}"
)

gdb_cmd=("${gdb_bin}")

if [[ "${gdb_init_file}" != "none" && -f "${gdb_init_file}" ]]; then
	gdb_cmd+=(-x "${gdb_init_file}")
elif [[ "${gdb_init_file}" != "none" ]]; then
	echo "GDB init file not found: ${gdb_init_file}" >&2
	exit 1
fi

if [[ "${GDB_AUTO_RUN:-}" == "1" ]]; then
	gdb_cmd+=(-ex run)
fi

gdb_cmd+=(--args "${EXE}" "${device_args[@]}")

exec "${gdb_cmd[@]}" "$@"
