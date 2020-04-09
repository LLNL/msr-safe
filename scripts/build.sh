#!/bin/bash

which g++
g++ --version

uname -a
uname -r

export CC=${COMPILER_CC}
export CXX=${COMPILER_CXX}

${CC} --version
${CXX} --version

make VERBOSE=1

sudo insmod msr-safe.ko

sudo chmod o=u /dev/cpu/*/msr_safe
sudo chmod o=u /dev/cpu/msr_*

sleep 2

ls -l /dev/cpu
ls -l /dev/cpu/*/msr_safe

WL=$(printf 'wl_%.2x%x\n' $(lscpu | grep "CPU family:" | awk -F: '{print $2}') $(lscpu | grep "Model:" | awk -F: '{print $2}'))
echo -e "WL:" ${WL}

cat whitelists/${WL} > /dev/cpu/msr_whitelist
cat /dev/cpu/msr_whitelist | head

#
