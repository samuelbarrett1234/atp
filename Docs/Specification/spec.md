# Automated Theorem Prover Specification

## Main ideas and principles

- A theorem prover / knowledge engine which is intended to be left constantly running (on as powerful machines as feasible), and thus building up a large database of knowledge.
- Automatically prioritises knowledge which is useful to store (including deactivating theorems which are not very useful).
- **Not** intended to supply proofs for human checking of its knowledge.
- Eventually the goal is for the system to solve customer problems (which are logical in nature). Customers can supply files containing statements which need to be evaluated as true or false. The file can contain a date by which it should be done, and the system's internal scheduler uses this deadline to estimate the priority of the file.
- Has to balance: training statistical models, making logical inferences in many different contexts, and solving customer problems.
- Can make links between different contexts (e.g. using group theory in complex analysis).
- A team of people constantly formalising new contexts that might be useful to work with (and if this could be automated too, it would be fantastic.)

## Main Applications and Libraries

- Core search functionality and rewrite rule kernel.
- Theorem and logic file parsing.
- Theorem database management.
- A command-line application which ties together the proving software. Can be run in client mode or server mode.
- Web application for remotely monitoring a server's proof processes.
- Statistical model implementations

## Core search functionality and rewrite rule kernel

This library contains algorithms which search through a space of rewrite rules for a reduction of an object to some canonical/normal form. It is independent of the logic and context chosen. It does not include process management, database management, any statistical models, any server applications, etc.

Types of search that it could include are:
- A partial-order planner,
- A Monte-Carlo (MCTS or something else) planner,
- An "aimless" search whichjust tries to construct an 'interesting' statement from a search space. This requires a statistical model of 'interestingness'. It generates the proof (sequence of reduction actions) as it goes along.

It will also need various interfaces for choosing actions and running statistical models.

## Theorem and logic file parsing

This library contains concrete information about what a statement is, how to parse axiom files and logic files, inference rules and axioms and theorems, contains data structures for determining what rules can be applied next. It does not include process management, database management, any statistical models, any server applications, etc.

Essentially, this library revolves around some "knowledge kernel" object which, for a given statement, can either produce all of the statements that can be reached in one step, or which can deduce that statement in one step.

## Theorem database management

This library contains ways of storing masses of theorems into a database file. It will also need to consider different environments, and auxiliary information being associated with theorems. It will also have to store their proofs.

Note that one of the stretch-goals of this application is to make it distributed. This library should be written in anticipation of a distibuted environment, where theorems and proofs are coming in from external sources as well as internal sources.

It should also have a process & resource management system, to handle database locking etc, and to easily facilitate concurrent proving.

It is also quite crucial that this database management system makes available a lot of information about its system (for example, database size, access time, etc) so that the scheduler can update its opinion on how "useful and interesting" a theorem should be in order to be saved to the ever-growing database).

## ATP application

This application ties together the libraries into a tool that can be run from command line. It can be run in client mode or server mode.

In client mode, you could specify something like "launch 3 threads which should each do 4 proofs".

In server mode, you would typically keep the proof processes running (specifying the number of such threads as a command line argument), and a separate thread will monitor requests coming in an out. It can then return `.html` files containing reports, and perhaps also live progress bars. The server mode will also allow remote users to submit model files, to allow the theorem prover to try proving things in a different setting.

The application will have to juggle the following activities:
- forming conjectures
- attempting to prove open conjectures
- "aimless wandering"
- perhaps even suggesting models
- training statistical models (which requires plenty of access to the underlying databases)
- choosing which models to prove things in (we may have many more models than proof processes).

The application may have to report the following:
- various performance metrics
- displaying the proof of an identified, already proven theorem
- links between models (e.g. if the proof of some analysis theorem used group theory, that connection is *very* important and valuable).
- eventually the user will want to input a statement and have the system determine its truth.

## Statistical models implementation

A wrapper around a neural network library (e.g. TensorFlow) which implements the statistical models which are used elsewhere in the application.

All this needs to be is a reasonably loose wrapper around the underlying library, constructing particular models for us (e.g. a transformer) and allowing hyperparameters to be tweaked, etc.
