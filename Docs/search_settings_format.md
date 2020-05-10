# Solvers and Search Settings

## Search Settings File Format

Search settings allow the user to choose a solver, and parameters for that solver, and save them into a persistent configuration file. The format is quite simple; there are some parameters which do not depend on the solver type, and some parameters depend on the type of solver.

The following settings are universal (independent of solver type):
```
{
	"name" : "Limited Iterative-Deepening Solver",
	"desc" : "An uninformed iterative deepening solver with low depth settings",
	"step-size" : 1000,
	"max-steps" : 10,
	"num-helper-theorems" : 25,
	"seed" : "time"
}
```

`step-size` is the number of expansions to perform (on each target proof) at each iteration. `max-steps` is the maximum number of updates to perform, although the target statements may be proven earlier than this. Note that all of the above fields are optional (i.e. have default values). `num-helper-theorems` is the number of theorems to load from the database to aid in the proof ("lemmas", if you will). The higher this number is, the higher the branching factor, but the more it leverages existing knowledge.

`no-repeats` is a flag indicating whether the search algorithm should avoid repeated states while searching. The benefit of this flag depends on the tradeoff between state space size, and time required to check for repeats. `randomised` is a flag telling the search to try to evaluate successors in a random order.

`seed` is the seed value used for a random number generator. You can either set this equal to the string `"time"`, which seeds it based on the current clock time when the program is launched, or you can give it a fixed integer value for reproducability. Any other string value is an error.

## Solvers

Solvers support a set of flags which governs how the proof states iterate over their successors. These are represented by the `no-repeats` and `randomised` flags. When `no-repeats` is true, the proof states don't allow successors which have already been seen, where this is possible to implement reasonably. There is of course a tradeoff here, in that this test demands extra checks for each successor, but there will be fewer of them. When `randomised` is true, the successors are enumerated in an order which is as random as is reasonably possible. This is useful for statistical estimation, which might assume the successors are IID, however this might negatively impact cache behaviour.

### Iterative Deepening Solver

The iterative deepening solver is specified using the `IterativeDeepeningSolver` type, and has two parameters `max-depth` and `starting-depth` which govern the depth to which it searches to. Here is a partial example of an iterative deepening solver settings file:

```
{
	"name" : "Iterative-Deepening Solver",
	"desc" : "Your description here.",
	"solver" : {
		"type" : "IterativeDeepeningSolver",
		"max-depth" : 5,
		"starting-depth" : 3,
		"no-repeats" : true,
		"randomised" : true
	},
	...
}
```

In order to create an iterative deepening solver, you must use the type `IterativeDeepeningSolver`. The new parameters are `max-depth` and `start-depth`. The former is the ultimate depth limit imposed on the iterative deepening search, and `start-depth` is the first depth limit. 

Setting the start depth higher means less wasted work for long proofs, but it makes finding short proofs much less efficient than they otherwise would be.

Iterative deepening search will make use of any successor modifiers (e.g. stopping strategies) that are specified in the settings file (see below).

## Heuristics

As of yet there are no heuristics implemented.

## Stopping Strategies

Stopping strategies are a class of statistical strategies for using a heuristic function to enumerate through successors. This is useful because there may be a great many successors to a given statement, and it is costly to even generate them all. Hence we may want to generate a few at a time and pick the best.

Stopping strategies only affect the order in which successors are enumerated, and not the results of the iterators. A stopping strategy cannot be successful without a good heuristic, first and foremost.

### Fixed Stopping Strategy

This is a very elementary stopping strategy. It always keeps `N` successors in memory at a time, and returns the best one. When it advances, it removes the best one from the list, and obtains the next successor. When there are no successors left, it enumerates the `N` elements in memory in nonincreasing order.

Here is an example of how to specify it in a search settings file:

```
{
	"name" : "Solver with a Fixed Stopping Strategy",
	"desc" : "Your description here.",
	"stopping-strategy" : {
		"type" : "FixedStoppingStrategy",
		"size" : 5
	},
	...
}
```

### Basic Stopping Strategy

This stopping strategy is statistical in nature. It models the *cost* and *benefit* of each successor as the time it took to compute, and the heuristic value, respectively. It approximates these values with a normal distribution (estimating the parameters from the data seen so far), and computes the probability that one more evaluation would result in a better benefit for a small increase in cost.

More precisely, if `X` is a normally-distributed random variable representing the cost, `Y` is a normally-distributed random variable representing the benefit heuristic, `s` is the sum of the costs of the states held in memory so far, and `m` is the maximum benefit held in the states in memory at the moment, then we choose to stop evaluating successors when:

` P ( lambda * (Y - m) > X + s ) < alpha `

For parameters `lambda > 0` and `alpha \in (0,1)`.

Note that, in order to help gather data to estimate the distributions in the early moments of the successor enumeration, this stopping strategy inputs an integer `initial-size` and it starts by adding in this many successors, regardless to their cost/benefit values. This helps get reliable estimates for the distribution parameters first.

Finally, note that since this is a statistical-based strategy, it will assume the successors are IID, hence it is better to use `randomised = true` whenever this stopping strategy is used.

Here is an example of how to specify it in a search settings file:

```
{
	"name" : "Solver with a Fixed Stopping Strategy",
	"desc" : "Your description here.",
	"stopping-strategy" : {
		"type" : "BasicStoppingStrategy",
		"initial-size" : 10,
		"lambda" : 10.0,
		"alpha" : 0.05
	},
	...
}
```
