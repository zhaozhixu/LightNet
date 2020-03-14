#! /bin/bash

set -ev

sudo apt update
sudo apt-get install -y build-essential perl git pkg-config check libjpeg-dev \
     unzip python3-pip python3-setuptools

pip3 install -U --no-cache-dir mkdocs markdown>=3.1.1 pygments opencv-python

mkdocs --version

export PERL_MM_USE_DEFAULT=1
sudo cpan install JSON Sort::strverscmp

ncpu=`cat /proc/cpuinfo| grep "processor"| wc -l`

wget https://github.com/zhaozhixu/TensorLight/archive/master.zip -O TensorLight.zip
unzip TensorLight.zip
(cd TensorLight-master && ./configure && make -j$ncpu && make test -j$ncpu \
     && sudo make install && sudo ldconfig)
