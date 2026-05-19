# ./os os_0_mlq_paging  > input/os_0_mlq_paging.output
# ./os os_1_mlq_paging > input/os_1_mlq_paging.output
# ./os os_1_mlq_paging_small_1K > input/os_1_mlq_paging_small_1K.output
# ./os os_1_mlq_paging_small_4K > input/os_1_mlq_paging_small_4K.output
# ./os os_1_singleCPU_mlq > input/os_1_singleCPU_mlq.output
# ./os os_1_singleCPU_mlq_paging > input/os_1_singleCPU_mlq_paging.output
# ./os sched > input/sched.output
# ./os sched_0 > input/sched_0.output
# ./os sched_1 > input/sched_1.output
# ./os os_sc > input/os_sc.output
# ./os os_syscall > input/os_syscall.output
# ./os os_syscall_list > input/os_syscall_list.output
# mv input/*.output output

// This script runs all the test cases and saves their outputs to the output directory.
TMP_OUT_DIR="/tmp/ossim_caitoa_run_outputs"
rm -rf "$TMP_OUT_DIR"
mkdir -p "$TMP_OUT_DIR"

run_case() {
	local cfg="$1"
	local out_file="$2"
	echo "[run.sh] running $cfg"
	./os "$cfg" > "$out_file"
}

run_case os_0_mlq_paging "$TMP_OUT_DIR/os_0_mlq_paging.output"
run_case os_1_mlq_paging "$TMP_OUT_DIR/os_1_mlq_paging.output"
run_case os_1_mlq_paging_small_1K "$TMP_OUT_DIR/os_1_mlq_paging_small_1K.output"
run_case os_1_mlq_paging_small_4K "$TMP_OUT_DIR/os_1_mlq_paging_small_4K.output"
run_case os_1_singleCPU_mlq "$TMP_OUT_DIR/os_1_singleCPU_mlq.output"
run_case os_1_singleCPU_mlq_paging "$TMP_OUT_DIR/os_1_singleCPU_mlq_paging.output"
run_case sched "$TMP_OUT_DIR/sched.output"
run_case sched_0 "$TMP_OUT_DIR/sched_0.output"
run_case sched_1 "$TMP_OUT_DIR/sched_1.output"
run_case os_sc "$TMP_OUT_DIR/os_sc.output"
run_case os_syscall "$TMP_OUT_DIR/os_syscall.output"
run_case os_syscall_list "$TMP_OUT_DIR/os_syscall_list.output"
mv "$TMP_OUT_DIR"/*.output output
