# Internal

This folder contains all the different types of logic available to use. To create a new type of logic, find `how_to_create_new_logic.md` in the `Docs` folder.

`Equational` contains code for parsing and running equational logic. The version of equational logic implemented here is actually slightly **weaker** than some other forms of equational logic; the solver is only able to use "iff" implications, no unidirectional implications, and cannot use predicate logic tools like "and" or "or". Instead, the prover works by reducing a statement to a form which is *trivially true*. Statements which are trivially true are statements which are symmetric about the equals sign (they are true by reflexivity), or statements which are directly implied by an axiom. At each step the solver is allowed to mutate one side of the equation using rules given in the axioms. All free variables are implicitly universally quantified, which is standard for equational logic.
