# ATPCore

This project contains processes and operations used by the frontend command line applications. It relies on the logic/database/search libraries. The reason it is its own library, and not part of the `atp` project, is because (i) being in a separate library makes it easier to test, and (ii) it can be reused between different command line applications if necessary.
