# Overview

LightNet is a light-weight neural network inference optimizer for different
software/hardware backends.

The deep learning algorithms for various application scenarios are becoming more
and more mature, however, highly real-time algorithms in terminal devices such
as Edge Computing Devices or Autonomous Agents (such as unmanned vehicles and
drones) often require heterogeneous computing systems which are low-power and
highly real-time. Such heterogeneous computing systems generally need to be
developed with specific hardware-related languages ​​(such as CUDA, OpenCL, 
Verilog, C/C++ etc.), which have the disadvantages of low development efficiency
and long iteration period.

In order to overcome those shortcomings, LightNet refers to traditional compiler
optimization technologies, designs and develops a neural network optimizer for
heterogeneous platforms. LightNet can accept neural network models from various 
popular deep learning frameworks, and optimize them according to the user's 
choice of backend platform. 

Further more, LightNet is easy to be extended. There are mainly 3 diversification
problems when designing a NN optimizer for heterogeneous platforms which leads
to enormous coding efforts: 

  * diversification of model formats, such as Tensorflow's checkpoint, 
    Pytorch's pt.
  * diversification of operators, operators are highly diverse and new operators
    are emerging.
  * diversification of operator optimization capabilities of backend 
    platforms.

LightNet provides 3 methods to solve those problems respectively:

  * a unified model format and conversion tools for various frameworks
  * an operator description language and code generation tools
  * an optimizer description language and code generation tools

Combined with its tensor operation library 
[TensorLight](https://github.com/zhaozhixu/TensorLight), LightNet can reuse
existing highly optimized tensor operation libraries to construct neural network
models.
which can realize the rapid construction and automatic
optimization of neural network inference algorithms.

LightNet's core architecture is developed with C, which has the advantages of 
light weight, high efficieny and unified binary interface, which can be easiy 
combined with other languages and deployed in various hardware platforms.
