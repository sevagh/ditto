#!/usr/bin/env bash

test_bin_name="dittotest"
bench_bin_name="dittobench"
test_bin=$(find . -type f -executable -name "${test_bin_name}")
bench_bin=$(find . -type f -executable -name "${bench_bin_name}")
bin=""
bin_name=""

if [ "${1}" == "test" ]; then
	bin=${test_bin}
	bin_name=${test_bin_name}
elif [ "${1}" == "bench" ]; then
	bin=${bench_bin}
	bin_name=${bench_bin_name}
else
	exit 1
fi


adb push ${bin} /data/local/tmp
adb shell chmod +x /data/local/tmp/${bin_name}
adb shell /data/local/tmp/${bin_name}
