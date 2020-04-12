# How to create a new type of logic?

This document contains instructions for creating a new type of logic, and rationale behind the architecture decisions.

## Main components

- Parsing,
- Syntax tree building and type checking,
- The `Statement` and `StatementArray` objects,
- The `Language`, `ModelContext` and `KnowledgeKernel` objects,
- The `ProofState` objects.

## Parsing

You will need to be able to parse statements which appear in the language, for example `"\forall x . \exists y . P(x, y)"`.

## Syntax tree building and type checking

This will involve type checking; for example: checking functions have the right arity, checking we're not quantifying over a user-defined constant, etc.

## Statements

This will involve implementing the `IStatement` and `IStatementArray` interfaces for your kind of logic. For a first version, it suffices to just use a syntax tree here (directly from the output of the above), however one may find this to be lacking in performance, so a more optimised storage might be necessary.

Statements are always associated with a particular model context.

## Language, ModelContext and KnowledgeKernel

This will involve implementing the `ILanguage`, `IModelContext` and `IKnowledgeKernel` interfaces for your kind of logic.

Language objects basically act as a factory object for all of the logic objects. It allows the user of the library to parse statements and context files.

Model Context objects encapsulate the combination of user-definitions and axioms. It does not perform syntax and type checks of the axiom statements. Context files are in JSON; their format is documented elsewhere. Examples of contexts include: Group Theory, or Kozen's Regular Expressions.

Knowledge Kernels add to the idea of a context, the ability to load (and unload) already-proven statements. They may wish to store these axioms and already-proven statements in an optimised format, so that during search, one can quickly find successors of a statement.

## Proof States

A `ProofState` encapsulates the idea of a complete, or work-in-progress, proof. These objects have a target statement (what they're trying to prove), must be able to enumerate their successor states. A proof state might be a proof tree (as seen often in logic, where the premises are at the top as the leaves of the tree and the root is the target statement), or in simpler cases it might just be a list of statements, each one following the last. Proof states must also be able to self-assess their completion (i.e. is this a complete proof? has this proof state reached a dead-end? is the proof otherwise unfinished?)

