# Solvers and Search Settings

## Search Settings File Format

Search settings allow the user to choose a solver, and parameters for that solver, and save them into a persistent configuration file. The format is quite simple; there are some parameters which do not depend on the solver type, and some parameters depend on the type of solver.

The following settings are universal (independent of solver type):
```
{
	"name" : "User Friendly Settings File Name Here",
	"desc" : "A relatively brief description about these settings",
	"type" : "SolverTypeNameGoesHere",
	"step-size" : 1000,
	"max-steps" : 10
}

```

`step-size` is the number of expansions to perform (on each target proof) at each iteration. `max-steps` is the maximum number of updates to perform, although the target statements may be proven earlier than this. Note that all of the above fields are optional (i.e. have default values).

## Iterative Deepening SolverTypeNameGoesHere

Here is a full example of an iterative deepening solver settings file:

```
{
	"name" : "Limited Iterative-Deepening Solver",
	"desc" : "An uninformed iterative deepening solver with low depth settings",
	"type" : "IterativeDeepeningSolver",
	"step-size" : 1000,
	"max-steps" : 10,
	"max-depth" : 5,
	"start-depth" : 3
}
```

In order to create an iterative deepening solver, you must use the type `IterativeDeepeningSolver`. The only new parameters are `max-depth` and `start-depth`. The former is the ultimate depth limit imposed on the iterative deepening search, and `start-depth` is the first depth limit. 

Setting the start depth higher means less wasted work for long proofs, but it makes finding short proofs much less efficient than they otherwise would be.

