# atp

An automated theorem prover in C++, geared towards proving mathematical statements, but without limiting itself to human-readable proofs.

## Documentation

Documentation pages can be found at https://samuelbarrett1234.github.io/atp/

## Navigation

Directory list:
- `atp` : the command line application for running the theorem prover,
- `ATPLogic` : the library which contains the logical inference code, and also the parsers,
- `ATPLogicTests` : the unit tests for the logic library,
- `ATPSearch` : the library of search algorithms for finding proofs,
- `ATPSearchTests` : the unit tests for the search library,
- `ATPDatabase` : the library for the management of proven theorems,
- `Docs` : specifications, documentation, and example code,
- `Data` : definition files, theorem databases.

Note that when you build the application, the binaries can be found in the `Output` folder, and `Temp` will contain Visual Studio intermediate files.

## Installation

The C++ solution was built and tested using Microsoft Visual Studio 2019, on a Windows 10 machine. This project uses the Boost C++ libaries, and SQLite3. Please ensure that they are installed, and that the environment variable `BOOST_DIR` to be set to the root of Boost's installation directory, and `SQLITE_DIR` likewise set for the SQLite3 directory (where the application will look for the code and the DLL). Ensure you build Boost with the correct compiler and C++ version, otherwise you may experience compilation errors.

After installation, try building the library and running the unit tests to check everything is working correctly.

## Setting up the database

TODO.

## Running the client

At the moment, the application only supports a single `prove` mode. Usage:

`atp --context <context-filename> --search-settings <search-settings> --prove <statement-filename>`

Note that, in the above example, you may need to change `atp` to `atp_Releasex64` or something similar, depending on your build settings. Check the `Output` folder to see which versions of the application you have built.

See `Docs` for documentation about context files and search settings files.

