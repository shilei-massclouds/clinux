#!/bin/sh

RED_C='\033[91;1m'
GREEN_C='\033[92;1m'
END_C='\033[0m'

TESTCASES="
    booter
    task
    spinlock
    lib
    params
    early_fdt
    vsprintf
    early_printk
    memblock
    bootmem
    paging
    linux
"

#TESTCASES="
#    cgroup
#    cpu
#    semaphore
#    resource
#    of
#    dma
#    sbi
#    riscv_cpu
#"

PASSED=0
FAILED=0
FAILURES=

#printf "Clean ...\n"
#make clean

printf "Pre-build ...\n"
make -j6

printf "Testing ...\n"

for TEST in $TESTCASES
do
    printf "\n[$TEST]: ...\n"

    if [ "$TEST" = "linux" ]; then
        printf "\nIt may cost several seconds, please wait ...\n"
        printf "We can execve cmdline [./test_all.sh VERBOSE] to check more details.\n\n"
    fi

    set -e
    if [ "$1" = "VERBOSE" ]; then
        make TOP=$TEST run | tee /tmp/output.log
    else
        make TOP=$TEST run > /tmp/output.log
    fi

    set +e
    ret_str=$(cat ${TEST}/expect_output 2>/dev/null)
    if [ -z "${ret_str}" ]; then
        ret_str="\[top_${TEST}\]: init end!"
    fi
    #echo "***** ${ret_str}"

    grep -q "${ret_str}" /tmp/output.log
    if [ $? -eq 0 ]; then
        printf "[$TEST]: ${GREEN_C}PASSED!${END_C}\n"
        PASSED=$(( PASSED + 1 ))
    else
        printf "[$TEST]: ${RED_C}FAILED!${END_C}\n"
        FAILED=$(( FAILED + 1 ))
        FAILURES="\n${TEST}${FAILURES}"
    fi
done

TOTAL=$(( PASSED + FAILED ))

printf "\n"
printf "Summary for tests:\n"
printf "================\n"
printf "  Passed: ${PASSED}\n"
printf "  Failed: ${FAILED}\n"
printf "  Total : ${TOTAL}\n"
printf "================\n"

if [ -n "${FAILURES}" ]; then
    printf "\nFailed tests:${FAILURES}\n" && false
fi
