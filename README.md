# atp

An automated theorem prover in C++, geared towards proving mathematical statements, but without limiting itself to human-readable proofs.

## Documentation

Documentation pages can be found at https://samuelbarrett1234.github.io/atp/

## Navigation

Directory list:
- `atp` : the command line application for running the theorem prover,
- `ATPCore` : the project containing processes and operations which are used in the frontend applications,
- `ATPCoreTests` : unit tests for the core library,
- `ATPLogic` : the library which contains the logical inference code, and also the parsers,
- `ATPLogicTests` : the unit tests for the logic library,
- `ATPSearch` : the library of search algorithms for finding proofs,
- `ATPSearchTests` : the unit tests for the search library,
- `ATPDatabase` : the library for the management of proven theorems,
- `ATPStats` : contains statistics and heuristics for usage in the search and core libraries,
- `ATPStatsTests` : unit tests for the statistics library,
- `ATPSite` : contains the website frontend (in PHP),
- `ATPSiteTools` : contains a command line application called by the PHP server for basic utilities,
- `Docs` : specifications, documentation, and example code,
- `Data/Definitions` : axiom and definition files, e.g. group theory,
- `Data/Search` : search settings files,
- `Data/Queries` : SQL queries, including a query to set up the database,
- `Data/Tasks` : lists of statements to try and prove.

Note that when you build the application, the binaries can be found in the `Output` folder, and `Temp` will contain Visual Studio intermediate files.

## Installation

1. Install Microsoft Visual Studio 2019, if you haven't got it already.
2. Download the Boost C++ libraries from [here](https://www.boost.org/), and set the `BOOST_DIR` environment variable to point to the root of the Boost directory (so that when you type `#include <boost/some-boost-library>`, the compiler will be able to find it.)
3. Now you need to build the Boost libraries from source. You may want to follow the official Boost documentation on how to install this, but when you come to running the `b2` command, use this: `b2 toolset=msvc-14.2 cxxflags="/std:c++17" --build-type=complete --abbreviate-paths architecture=x86 address-model=64 define=BOOST_LOG_DYN_LINK stage`. There are a few extra bits to note in that command: we have forced usage of C++17, and we have also specified that the Boost logging library should be built in dynamic-link mode.
4. Check the directory you specified under the environment variable `BOOST_DIR` contains a directory `/stage/lib/` which contains all of the Boost `.lib` and `.dll` files.
5. Copy files of the form `boost_*.dll` to the `Output` folder of the `atp` project (you may need to create the `Output` folder if it's not there already).
6. [Download SQLite3](https://www.sqlite.org/download.html), making sure to get the **binaries and the shell command line**, too.
7. If the binaries you downloaded included a `.exp` file but **not** a `.lib` file, then you need to convert the `.exp` file into a `.lib` file. Visual studio [ships with a tool](https://docs.microsoft.com/en-us/cpp/build/reference/lib-reference?view=vs-2019) to do this. The following command worked for me: `lib /DEF:sqlite3.def /OUT:sqlite3.lib /MACHINE:x64`.
8. Add the folder containing the SQLite source, binaries and command line to your `PATH` and to the environment variable `SQLITE_DIR`.
9. Now you are ready to try compiling the whole `ATP` solution (the easiest way to do this is with a *batch build*).
10. Run all the unit tests to make sure everything passes.
11. Follow the steps in the next section to create the database.
12. *Optional.* If you want to automatically build documentation, [install Doxygen](http://www.doxygen.nl/).
13. *Optional.* [Install XAMPP](https://www.apachefriends.org/index.html) if you want to see the website frontend and don't already have a way to run a local web server. If you are going to do this, check the `ATPSite` README for additional notes on how the config files need to be tweaked.

## Setting up the database

Open the command prompt and navigate to the `Data\DB` directory (create it if it doesn't exist already). Type the command `sqlite3 eqlogic.db` to create a new database called `eqlogic` for equational logic. This database file will then be referenced in the **database configuration file** at `../Queries/dbconfig.json` which contains the path to the SQLite database and also templates for many of the queries executed.

There are already scripts for setting up the database, which are named in the form `create_*.sql`. Run all scripts of this form, starting with the `create_eqlogic_db.sql` one, which sets up the main tables. To run it, type (into the SQLite shell, which should now be running): `.read ../Queries/create_eqlogic_db.sql`.

## Running the application

The command line application supports two modes: `prove` mode and `serve` mode. The former is invoked, executes its command, then exits. The latter mode is intended to be left running, and makes the whole process pretty automated. Usage for the `prove` mode:

`atp --db <database-config-filename> --ctx <context-name> --ss <search-settings-name> --prove <statement-filename-or-in-quotes>`

`--ctx` tells the ATP which context you want to prove in - e.g. group theory, ring theory, etc. To get details on what context names are allowed, look at the database's `model_contexts` table. The name is looked-up in here, where the filename will be found. Similarly, `--ss` tells the solver what search settings to use. The different settings can be found in the `search_settings` table in the database. All three options (`db/ctx/ss`) are necessary for `--prove`.

**Example**: `atp_Releasex64 --db ../Data/Queries/dbconfig.json --ctx group-theory --ss ids-uninformed-extended --prove "i(i(x0)) = x0"`

Note that, in the above example, you may need to change `atp` to `atp_Releasex64` or something similar (as in the example), depending on your build settings. Check the `Output` folder to see which versions of the application you have built.

For the `serve` mode, you can use:

`atp --db <database-config-filename> --serve`

Note that you still need to provide the database, but not the search settings or context, as those are chosen automatically and also change throughout execution. Also note that `serve` mode as of yet does not automatically generate theorems, it only automatically attempts proofs, so you need to ensure there is an ample supply of unproven theorems when running this (or it will spend all of its time trying to prove false theorems to no avail).

See `Docs` for documentation about context files and search settings files. Also, try typing `atp --help` for a full list of commands (for example, if you wanted to do automated conjecturing). Also observe that these applications make extensive use of logging, so you can check the logs for more detail on what the application was doing, how the search trees were explored, exactly what queries were executed, etc.

## Appendix: Prettier Printing with SQLite

From https://dba.stackexchange.com/questions/40656/how-to-properly-format-sqlite-shell-output/40672 I gather that, if you want prettier SQLite outputs from `SELECT` statements in the shell application, type:

```
sqlite> .mode column
sqlite> .width 50
sqlite> .headers on
```

