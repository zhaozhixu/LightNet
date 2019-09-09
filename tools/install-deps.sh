#! /bin/bash

set -ev

sudo apt-get install build-essential perl git pkg-config check python3-pip python3-setuptools libjpeg-dev
sudo cpan install JSON Sort::strverscmp
sudo pip3 install mkdocs markdown>=3.1.1 pygments opencv-python

wget https://github.com/zhaozhixu/LightNet/archive/master.zip -O TensorLight.zip
unzip TensorLight.zip
(cd TensorLight && ./configure && make && make test && make install)
