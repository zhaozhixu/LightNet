# Building Options

## Configuration

The LightNet source code must be configured before being built. 
This process uses a `configure` script in the project root.

Various configuration options can be passed to `configure` on the command line.
It then checks dependencies and print makefile variables in 
`config.mk`, which will then be included by other makefiles.

Here is a list of the configuration options, which can also be printed by
`./configure -h`. Default options are showed in brackets.

* `-h, --help`                     print this information
* `--target=<name>`                target name [`lightnet`]
* `--abbr=<abbr>`                  abbreviation name [`ln`]
* `--build-dir=<path>`             building directory [`build`]
* `--install-dir=<path>`           installation directory [`/usr/local`]
* `--pkgconfig-dir=<path>`         pkgconfig directory [`/usr/local/lib/pkgconfig`]
* `--with-cuda=<bool>`             set to yes if build with CUDA [`no`]
* `--with-cudnn=<bool>`            set to yes if build with cudnn library [`no`]
* `--cuda-install-dir=<path>`      cuda installation directory [`/usr/local/cuda`]
* `--with-tensorrt=<bool>`         set to yes if build with TensorRT [`no`]
* `--tensorrt-install-dir=<path>`  tensorrt installation directory [`/usr`]
* `--debug=<bool>`                 set to yes when debugging [`no`]
* `--doc=<bool>`                   set to yes if build the documents too [`yes`]

## Make Options
