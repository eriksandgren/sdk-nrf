# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#
# Sourced automatically by _debug_gdb.sh (-x). Edit breakpoints here.
# Uncomment the last line to run immediately after GDB starts.

set pagination off

break m_central_blocked_signal_handle
break sched_queue.c:349
ignore 1 10000
ignore 2 10000

run
