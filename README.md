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

The C++ solution was built and tested using Microsoft Visual Studio 2019, on a Windows 10 machine. This project uses the Boost C++ libaries, and SQLite3. Please ensure that they are installed, and that the environment variable `BOOST_DIR` to be set to the root of Boost's installation directory, and `SQLITE_DIR` likewise set for the SQLite3 directory (where the application will look for the code and the DLL). Ensure you build Boost with the correct compiler and C++ version (C++17), otherwise you may experience compilation errors.

Note that we will assume that `SQLITE_DIR` is also on your `PATH`, so that you can use the SQLite command line.

SQLite can be obtained from here: https://www.sqlite.org/download.html and Boost can be obtained from here: https://www.boost.org/

After installation, try building the library and running the unit tests to check everything is working correctly. After running the tests you now need to create a database (see below).

## Setting up the database

Open the command prompt and navigate to the `Data\DB` directory (create it if it doesn't exist already). Type the command `sqlite3 eqlogic.db` to create a new database called `eqlogic` for equational logic.

There is already a script for setting up the database. To run it, type (into the SQLite application, which should now be running): `.read ../Queries/create_eqlogic_db.sql`. This will initialise all the tables etc.

## Running the client

At the moment, the application only supports a single `prove` mode. Usage:

`atp --db <database-filename> --ctx <context-name> --ss <search-settings-name> --prove <statement-filename-or-in-quotes>`

`--ctx` tells the ATP which context you want to prove in - e.g. group theory, ring theory, etc. To get details on what context names are allowed, look at the database's `model_contexts` table. The name is looked-up in here, where the filename will be found. Similarly, `--ss` tells the solver what search settings to use. The different settings can be found in the `search_settings` table in the database.

**Example**: `atp_Releasex64 --db ../Data/DB/eqlogic.db --ctx group-theory --ss ids-uninformed-extended --prove "i(i(x0)) = x0"`

Note that, in the above example, you may need to change `atp` to `atp_Releasex64` or something similar (as in the example), depending on your build settings. Check the `Output` folder to see which versions of the application you have built.

See `Docs` for documentation about context files and search settings files.

## Appendix: Prettier Printing with SQLite

From https://dba.stackexchange.com/questions/40656/how-to-properly-format-sqlite-shell-output/40672 I gather that, if you want prettier SQLite outputs from `SELECT` statements in the shell application, type:

```
sqlite> .mode column
sqlite> .width 50
sqlite> .headers on
```

