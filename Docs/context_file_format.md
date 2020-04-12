# Context File Format

A context file specifies a kind of "proof environment", which includes both axioms and definitions. Context files are written in JSON. The axioms are just represented by a list of strings, where each string is a valid statement representing that axiom's rule, and the definitions are pairs containing the name and arity of the definition.

For example, here is a group theory context file:
```
{
	"name" : "Group Theory",
	"definitions" : [
		{ "name" : "e", "arity" : 0 },
		{ "name" : "i", "arity" : 1 },
		{ "name" : "*", "arity" : 2 }
	],
	"axioms" : [
		"*(*(x, y), z) = *(x, *(y, z))",
		"*(x, e) = x",
		"*(e, x) = x",
		"*(x, i(x)) = e",
		"*(i(x), x) = e"
	]
}
```

Notice how we have also provided a name for the whole context.

