# Automated Conjecturing: HMM Conjecturer Model

This document briefly explains how the automated conjecturing works, using a Hidden Markov model to generate the conjectures.

Hidden Markov models (HMMs) are discrete time filtering algorithms, and perform something called *recursive Bayesian estimation*. A HMM has a fixed set of discrete *hidden states* that it can be in, and at each timestep, the state gives of an observation (also called an *emission*) and then transitions randomly to the next state.

HMMs are useful because they allow us to develop unsupervised models of the sequences of characters that form a *logical statement*. More specifically, we view a logical statement as a syntax tree, and the HMM interprets this syntax tree in its [pre-order traversal](https://en.wikipedia.org/wiki/Tree_traversal#Pre-order_(NLR)) order. This allows the HMM to interpret a syntax tree in a relatively natural way, even though they are only designed to work with sequences (not trees). Each observation is a single element of the formula (e.g. a constant symbol, a function symbol, a free variable, etc.)

Note that there is a slight complication with free variable IDs: there are infinitely many possibilities. I decided to model free variable IDs using a geometric distribution, which I don't think is too unreasonable.

The HMM is then trained by learning the parameters of the model from a dataset consisting of **theorems with proofs** (of course, we do not want to learn the statistics of theorems without a proof, because we only want to generate conjectures that resemble those that are true).

