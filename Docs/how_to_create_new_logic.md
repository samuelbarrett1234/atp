# How to create a new type of logic?

This document contains instructions for creating a new type of logic, and rationale behind the architecture decisions.

## Main components

- Parsing,
- Syntax tree building and type checking,
- The `Statement` and `StatementArray` objects,
- The `Language` and `KnowledgeKernel` objects.

## Parsing

You will need to be able to parse statements which appear in the language, for example "\forall x . \exists y . P(x, y)". You might also want a parser for definition files, where the user defines constants and functions, if applicable.

## Syntax tree building and type checking

This will involve things like: checking functions have the right arity, checking we're not quantifying over a user-defined constant, etc.

## Statements

This will involve implementing the `IStatement` and `IStatementArray` interfaces for your kind of logic.

## Languages and KnowledgeKernel

This will involve implementing the `ILanguage` and `IKnowledgeKernel` interfaces for your kind of logic. In the implementation of the language, you will likely be invoking your parsers mentioned earlier.

