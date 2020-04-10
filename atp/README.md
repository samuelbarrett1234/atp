# atp

This is the main command-line application for running the theorem prover.

## Contexts

Each proof process operates in a *context*. This consists of a logical language (e.g. first order logic) and a collection of definitions and axioms (e.g. set theory and ZFC, respectively.)

See `ILanguage` and `IKnowledgeKernel` in the `ATPLogic` library for more details - but a context file basically sets up those two object types.

In `Data/Definitions` see `group_theory.txt` and `reg_expr.txt` for examples of contexts.

## Modes of Execution

At the moment, the application only supports a single `prove` mode. Usage:

`atp --context <context-filename> --prove <statement-filename>`

Note that, in the above example, you may need to change `atp` to `atp_Releasex64` or something similar, depending on your build settings. Check the `Output` folder to see which versions of the application you have built.
