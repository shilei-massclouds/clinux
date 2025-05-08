#!/bin/sh

RED_C='\033[91;1m'
GREEN_C='\033[92;1m'
END_C='\033[0m'

TESTCASES="
    booter
    linux
"

#TESTCASES="
#    lib
#    task
#    early_fdt
#    cgroup
#    cpu
#    memblock
#    params
#	spinlock
#    semaphore
#    early_printk
#    paging
#    bootmem
#    resource
#    of
#    dma
#    sbi
#    riscv_cpu
#"

PASSED=0
FAILED=0
FAILURES=

printf "Clean ...\n"
make clean

printf "Pre-build ...\n"
make -j6

printf "Testing ...\n"

for TEST in $TESTCASES
do
    printf "\n[$TEST]: ...\n"

    set -e
    make TOP=$TEST run > /tmp/output.log

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
