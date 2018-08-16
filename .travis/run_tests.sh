#!/bin/sh

unit_tests=(
    concurrent_queue_tests \
    filter_tests \
    frequency_resampler_tests \
    image_tests \
    types_tests \
    utils_tests )

echo "Unit tests:"

test_exit_status=0
for unit_test in "${unit_tests[@]}"
do
    echo "* running ${unit_test}"
    ./tests/${unit_test} &> "./tests/output/${unit_test}.log"
    exit_code=$?
    if [ $exit_code -ne 0 ]; then
        echo "........ FAILED"
        cat ./tests/output/${unit_test}.log
        tests_exit_status=1
    else
        echo "........ OK"
    fi
done

exit ${tests_exit_status}
