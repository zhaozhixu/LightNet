# Intermediate Language

## IL Format

The [Intermediate Representation](Intermediate-Representation.md) may seems
simple at the first glance. But when we're writing a real neural network,
the model file in JSON IR format is easily getting tens of thousands of lines,
which is still piece of cake for computers, but a nightmare for humans.

Thus, LightNet provides a "concise version" of JSON IR format, which contains
the exactly the same information as JSON IR, called LightNet Intermediate
Language (IL), conforming to the following format:
