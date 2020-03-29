#pragma once


/*

IKnowledgeKernel.h

This file contains an interface to a class representing the core
inference rules of the logical system. It aims to be vectorised and
lazy where possible.
In summary, the knowledge kernel contains all logical rules, axioms,
and available theorems.
Note that every kernel state is associated with a kind of hash-code;
this is useful because the code is designed such that if two kernels
agree on their code, they agree on the return results for all of
their functions (assuming no hash collisions).

*/


#include <memory>
#include <vector>
#include "../ATPLogicAPI.h"
#include "IStatement.h"
#include "IStatementArray.h"


namespace atp
{
namespace logic
{


/// <summary>
/// Object for logical inference on statement arrays.
/// In the comments below, "one step" refers to a single application
/// of a logical rule, axiom, or available theorem.
/// Knowledge kernels are always tied to a particular language.
/// </summary>
class ATP_LOGIC_API IKnowledgeKernel
{
public:
	virtual ~IKnowledgeKernel() = default;

	// If two knowledge kernels agree on their code, then they agree
	// on the return results of all the functions below. (This code
	// is computed by hashing all of the available syntax and axioms
	// etc.)
	virtual size_t get_code() const = 0;

	// Precondition: valid(p_stmts)
	// Postcondition: for each statement in the input array, we
	// return an array of statements which follow in one step
	// from it.
	virtual std::vector<StatementArrayPtr> succs(
		StatementArrayPtr p_stmts) const = 0;

	// Postcondition: for each statement in the input array, we
	// return an array of statements which deduce the statement
	// in one step.
	virtual std::vector<StatementArrayPtr> prevs(
		StatementArrayPtr p_stmts) const = 0;

	// Postcondition: for each statement, check whether it is valid
	// in terms of the logic, axioms and definitions in this
	// knowledge kernel. Statements are only allowed to be used if
	// they are valid. Return true iff all statements in the array
	// are valid.
	// Note that a necessary condition for validity is that they
	// must be using the same language as that which created this
	// knowledge kernel.
	virtual bool valid(
		StatementArrayPtr p_stmts) const = 0;

	// Precondition: p_premise->size() == p_concl.size() and
	// valid(p_premise) and valid(p_concl)
	// Postcondition: for each premise and conclusion, returns
	// true (in the output array) if the conclusion follows from
	// the premise in a single step.
	virtual std::vector<bool> follows(
		StatementArrayPtr p_premise,
		StatementArrayPtr p_concl) const = 0;
};


typedef std::shared_ptr<IKnowledgeKernel> KnowledgeKernelPtr;


}  // namespace logic
}  // namespace atp


