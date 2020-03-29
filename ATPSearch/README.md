# ATPSearch

This project contains the core search functionality. It includes planning algorithms for applying rewrite rules until some canonical form is reached.

The `Interfaces` subfolder contains the main objects this library is concerned with, and will expose to other libraries. `Internal` contains implementations of those interfaces that shouldn't be needed by other libraries except possibly for unit testing.