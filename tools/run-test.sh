#! /bin/bash

set -ev

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib

ncpu=`cat /proc/cpuinfo| grep "processor"| wc -l`

echo $ncpu

./configure --with-python=yes && make -j$ncpu && make test -j$ncpu \
    && make doc && sudo make install && sudo ldconfig
