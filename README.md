A light weight neural network optimizer framework for different software/hardware
backends. The first version is still under development.

## Overview

LightNet is a lightweight neural network optimizer framework for different 
software/hardware backends.

See [Introduction](https://zhaozhixu.github.io/LightNet/Documentation/Introduction.html)
for the detailed introduction.
See [LightNet](https://zhaozhixu.github.io/LightNet) for its full documentation.

## Installation

### Requirements

The following steps have been tested for Ubuntu 16.04 but should work with
other distros as well. 

Most required packages can be installed using the following commands
(`sudo` permission may be required):

    apt-get install build-essential perl git pkg-config check
    cpan install JSON Sort::strverscmp

This project also depends on [TensorLight](https://github.com/zhaozhixu/TensorLight),
a lightweight tensor operation library. Install it according to 
[its repository](https://github.com/zhaozhixu/TensorLight) before continuing to
build LightNet.

* (Optional) Packages for building documents

    Use the following commands to install the packages for building documents:
    
        apt-get install python3-pip
        pip3 install mkdocs markdown>=3.1.1 pygments

* (Optional) Packages for building python packages

    Use the following commands to install the packages for building python packages 
    (python3 for example):
    
        apt-get install python3-setuptools

* (Optional) CUDA dependency

    You also have to install CUDA 8.0 (or later) according to its website
    [CUDA Toolkit](http://docs.nvidia.com/cuda/cuda-installation-guide-linux/index.html)
    if you want to build with CUDA support.
    Remember to put `nvcc` (usually in `/usr/local/cuda/bin`) in environment
    variable `PATH`.

* (Optional) cuDNN dependency

    You also have to install CUDA 8.0 (or later) and cuDNN 7.0 (or later)
    libraries, according to their websites
    [CUDA Toolkit](http://docs.nvidia.com/cuda/cuda-installation-guide-linux/index.html) and 
    [cuDNN](https://docs.nvidia.com/deeplearning/sdk/cudnn-install/index.html) 
    if you want to build with cuDNN support.

* (Optional) TensorRT dependency

    You also have to install CUDA 8.0 (or later) and TensorRT 3.0 (or later) 
    libraries, according to their websites
    [CUDA Toolkit](http://docs.nvidia.com/cuda/cuda-installation-guide-linux/index.html) and 
    [TensorRT](https://docs.nvidia.com/deeplearning/sdk/tensorrt-install-guide/index.html)
    if you want to build with TensorRT support.

### Building and Installation

1. Clone this repository to your local directory.

        cd <my_working_directory>
        git clone https://github.com/zhaozhixu/LightNet.git
        cd LightNet

2. Configure and build

    First, configure your installation using:
    
        chmod +x configure
        ./configure
    
    There are options to custom your building and installation process.
    You can append them after `./configure`. For example, use
    
        ./configure --install-dir=DIR
        
    to set the installation directory (default is `/usr/local`). Use
    
        ./configure --with-cuda=yes
        
    if you want to build with CUDA support.
    
    Detailed `./configure` options can be displayed using `./configure -h`.

    After that, use `make` to compile the binaries, and run the test.
    Finally, use `make install` to install the build directory into
    the installation directory.

3. Other `make` options

    Use `make info` to see other `make` options.
    Especially, you can use `make clean` to clean up the build directory and all
    object files, and `make uninstall` to remove installed files from
    the installation directory.

## Usage

After compilation, the following components will be installed:

- `lightnet`: LightNet command tool
- `liblightnet.so`: LightNet runtime library
- `ir2json.pl`: LightNet intermediate language (IL) interpreter
- `pylightnet` (optional): LightNet python wrapper package

LightNet provides 3 interfaces which can be used by developers and users:

- [Command line](#command-line)
- [C API Demo](#c-api-demo)
- [Python API Demo](#python-api-demo)

### Command line

From the command line, type `lightnet -h` can display the program usage, as 
follows.

```
Usage: lightnet [OPTION...] SOURCE
Apply compilation procedures to SOURCE according to the options.
If SOURCE is -, read standard input.

Options:
  -h, --help             display this message
  -v, --version          display version information
  -o, --outfile=FILE     specify output file name; if FILE is -, print to
                         standard output; if FILE is !, do not print;
                         (default: out.json)
  -t, --target=TARGET    specify target platform (default: cpu)
  -f, --datafile=FILE    specify tensor data file
  -c, --compile          compile only; do not run
  -r, --run              run only; do not compile; SOURCE should have been
                         memory-planned
  -Wwarn                 display warnings (default)
  -w, -Wno-warn          do not display warnings
  -Winter                display internal warnings (default)
  -Wno-inter             do not display internal warnings
  -debug                 display debug messages (only works with LN_DEBUG
                         defined when compiling)
```

### C API Demo

An example using the C API is in `example/object-detect.c`. 

This example performs an single-object detection algorithm using 
[TensorRT](https://docs.nvidia.com/deeplearning/sdk/tensorrt-install-guide/index.html),
with [ShuffleNetV2](https://arxiv.org/abs/1807.11164) as back-bone conv-net
and [SqueezeDet](https://arxiv.org/abs/1612.01051)'s detection part as feature
expression algorithm. 
    
To play with this demo, LightNet should be configured with

    --with-tensorrt=yes

And to minimalize dependencies, this demo uses 
[libjpeg](https://libjpeg-turbo.org) to read JPEG files, which requires
`libjpeg-dev` package to be installed, and can be installed in Ubuntu via

    apt install libjpeg-dev
    
After the compilation and installation of LightNet, you need to enter the
`example` directory and compile the demo:

    cd example
    make
    
Then, in the `example` directory, enter the following commands (the first
command is to translate the *.net model file into a JSON file that the program
can read):

    ir2json.pl data/shuffledet_dac.net -o data/out.json
    ./object-detect data/out.json data/shuffledet_dac.wts data/images

And you should get a series of bounding boxes coordinates (xmin, ymin,
xmax, ymax), one for an input image, printed in the terminal like this:

    [197.965088, 163.387146, 275.853577, 323.011322]
    [197.478195, 161.533936, 276.260590, 322.812714]
    [197.014648, 158.862747, 276.261810, 322.054504]
    [196.676514, 160.987122, 275.435303, 322.000763]
    [196.797455, 160.380035, 274.323181, 320.533020]
    [196.917221, 161.277679, 273.463776, 319.580872]
    ......
    frames per second of detection: 245.948881

In real projects, developers may draw the bounding boxes in the original image
with any libraries they like (such as OpenCV, GTK...).

### Python API Demo

An example using the python API is in `example/detect.py` and demoed with
`example/object-detect.py`. This example does the same detection algorithm as
the [C API demo](#c-api-demo).

To play with this demo, LightNet should be configured with

    --with-tensorrt=yes --with-python=yes
    
And [OpenCV](https://opencv.org) for Python3 should be installed. A possible
command for installation:

    pip3 install opencv-python

After compilation and installation, enter the following
commands in the `example` directory (the first command is to translate
the *.net model file into a JSON file that the program can read):

    ir2json.pl data/shuffledet_dac.net -o data/out.json
    ./object-detect.py data/out.json data/shuffledet_dac.wts data/images

Then you should get a dection window with bouding boxes detecting the images
in `example/data/images` dynamicly like the following screenshot, 
and a series of bounding boxes coordinates (xmin, ymin, xmax, ymax),
one for an input image, printed in the terminal.

![Demo](docs/img/demo.png)
