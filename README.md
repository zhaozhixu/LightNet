A light weight neural network frontend framework for different software/hardware
backends. The first version is still under development.

## Getting Started

### Prerequisites
The following steps have been tested for Ubuntu 16.04 but should work with
other distros as well. 
Required packages can be installed using the following command:

```
sudo apt-get install build-essential perl git pkg-config check
```
#### Build with CUDA
If you want to build with CUDA support, you also have to install CUDA 8.0
(or later) according to their website [CUDA Toolkit](http://docs.nvidia.com/cuda/cuda-installation-guide-linux/index.html).
Remember to put `nvcc` (usually in `/usr/local/cuda/bin`) in environment
variable `PATH`.

#### Build with cuDNN
If you want to build with cuDNN support, you also have to install CUDA 8.0 
(or later) and cuDNN 7.0 (or later) libraries, according to their websites [CUDA Toolkit](http://docs.nvidia.com/cuda/cuda-installation-guide-linux/index.html)
and [cuDNN](https://docs.nvidia.com/deeplearning/sdk/cudnn-install/index.html).

#### Build with TensorRT
If you want to build with TensorRT support, you also have to install CUDA 8.0 
(or later) and TensorRT 3.0 (or later) libraries, according to their websites [CUDA Toolkit](http://docs.nvidia.com/cuda/cuda-installation-guide-linux/index.html)
and [TensorRT](https://docs.nvidia.com/deeplearning/sdk/tensorrt-install-guide/index.html).

### Building and Installation
1.  Clone this repository to your local directory.

    ```
    cd <my_working_directory>
    git clone https://github.com/zhaozhixu/LightNet.git
    cd LightNet
    ```

2.  Build and install

    First, configure your installation using:
    
    ```
    chmod +x configure
    ./configure
    ```
    There are options to custom your building and installation process.
    You can append them after `./configure`. For example, use
    ```
    ./configure --install-dir=DIR
    ```
    to set the installation directory (default is `/usr/local`); use
    ```
    ./configure --with-cuda=yes
    ```
    if you want to build with CUDA support.
    Detailed `./configure` options can be displayed using `./configure -h`.

    After that, use `make` to compile the library, and optionally, run the tests with
    `make test`. Then `make install` to copy the library files and headers into 
    the installation directory, or `sudo make install` if you don't have the
    permissions with that directory.

3.  Other `make` options

    Use `make info` to see other `make` options.
    Especially, you can use `make clean` to clean up the build directory and all
    object files, and `make uninstall` to remove library files and headers from
    the installation directory.

### Usage
After compilation, use `lightnet -h` to learn the usage for this program, as follows.
```
Usage: lightnet [OPTION...] SOURCE
Apply compilation procedures to SOURCE according to the options.
When SOURCE is -, read standard input.

Options:
  -h, --help             display this message
  -v, --version          display version information
  -o, --outfile=FILE     specify output file name; when FILE is -, print to
                         standard output; when FILE is !, do not print
                         (default: out.json)
  -t, --target=TARGET    specify target platform (default: cpu)
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

