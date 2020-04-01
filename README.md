# atp

An automated theorem prover in C++, geared towards proving mathematical statements, but without limiting itself to human-readable proofs.

## Navigation

This repository has four projects:
- `atp` : the command line application for running the theorem prover,
- `ATPLogic` : the library which contains the logical inference code, and also the parsers,
- `ATPLogicTests` : the unit tests for the logic library,
- `ATPSearch` : the library of search algorithms for finding proofs,
- `ATPDatabase` : the library for the management of proven theorems and also concurrent process management,
- `Docs` : specifications, documentation, and example code,
- `Data` : definition files, theorem databases.

Note that when you build the application, the binaries can be found in the `Output` folder, and `Temp` will contain Visual Studio intermediate files.

## Installation

The C++ solution was built and tested using Microsoft Visual Studio 2019, on a Windows 10 machine. It depends on the C++ Boost libaries, and it expects the environment variable `BOOST_DIR` to be set to the root of Boost's installation directory.

## Running the client

TODO.

## TODO

At the moment the project is working towards a minimum viable product, using only equational logic and uninformed search strategies. This repository still needs the following:
- a statistics and machine learning library, for modelling the statements and searches, so we can start to use informed search strategies,
- a networking library so the application can be run in a server mode (for delivering reports over the web, or for remote monitoring and control.)
- the `ATPLogic` library needs implementations of other kinds of logic (e.g. first order logic, second order logic.)
- the `ATPSearch` library needs to use some more sophisticated, informed search strategies.
