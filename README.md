# VLearn

VLearn is a C++ library containing all sort of stuff out of the box to make cool projects machine learning projects.
Every aspect of VLearn is separated in a different folder.

While VLearn contains machine learning features, it also has methods for file manipulation, networking and serialization,
to help you build awesome deep learning projects.


VLearn is the framework used for the Distributed Machine Learning project.

# Performance

We are faster than tensorflow. No more waiting for days to train your model.

# Building and Usage.

VLearn compiles to a static library (vlearn.a) that you can then link against.
When linking vlearn, you should also link with dbghelp on windows to access the debugging features.

VLearn is compiled with the vapm build system and the MinGW compiler. VLearn aims to be compatible with Windows and Linux.
I won't add compatibility with the Microsoft Visual C++ Compiler but I will accept merge requests adding this compatibility.

# Presentation of the various modules

## Machine learning

The machine learning related code is in the ml folder.
We provide code to build and train a neural network, with methods similar to the keras ones.
We provide various Layer types, Optimizers (I mean, we have adam, you won't need anything else) and backends.

## Networking

## File manipulation

VToolbox has some methods to make reading / writing various file types easier.
This includes images, audio and text files.
The methods are similar to the ones in Python.
Check out the documentation (`doc/index.html`) for more info.

## Debugging

VCrash is a part of VToolbox, checkout vcrash here: https://github.com/vanyle/vcrash

## Documentation

Checkout `/doc/` for detailed documentation of every package.
`/doc/` contains an `index.html` with auto-generated searchable documentation for every package.
For examples, see the `/test/` folder, it contains working code examples of most features of vtoolbox.

# Distributed Machine Learning

How does it work ?

Well, let's consider a NeuralNetwork object.
Some ideas to parallelism training:

1. Every node computes the gradient with a subset of the training data and for a step,
we take the average gradient.

2. (Get a quick start): Have nodes starts the training alone with random
weights and pick the best node (with the smallest loss) after a few iteration.

3. In case of adverserial networks (GAN), the 2 opposing networks can run of separate machines.

Those ideas work but don't provide a big performance gain. To have a bigger speedup, we need to change the
overall architecture of the learning algorithm.
We need to think about how humans learn. A human can learn several thing at once very quickly.

Maybe what we could do is to train multiple smaller independent learning algorithms at once.
The challenge is to make sure that all those networks have to be independent.

# References

We used the following lecture to implement gradient descent:
https://web.stanford.edu/class/cs224n/readings/gradient-notes.pdf

We used the adam paper to implement adam:
https://arxiv.org/abs/1412.6980

We used the following paper as a parallelisation reference for neural networks:
https://papers.nips.cc/paper/2006/file/77ee3bc58ce560b86c2b59363281e914-Paper.pdf