# ATPLogic

This project contains the core "statement" objects, contains code for parsing logic, axiom and definition files, and contains code for performing inferences (such as, what is deducable from a given statement in a single step? What would deduce this theorem in a single step?).

The `Interfaces` subfolder contains the main objects this library is concerned with, and will expose to other libraries. `Internal` contains implementations of those interfaces that shouldn't be needed by other libraries except possibly for unit testing.

In the `Internal` folder, you will see folders corresponding to each type of logic implemented by this library (so far just `Equational`). Each of the types of logic implements all of the interfaces in the `Interfaces` folder, including parsing the language it is written in (so, for example, equational logic statements are written in a particular way, so they need their own parser to read.)
