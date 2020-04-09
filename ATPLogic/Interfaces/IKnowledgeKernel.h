#pragma once


/**

\file

\author Samuel Barrett

\brief Contains the IKnowledgeKernel interface, which represents a
    collection of definitions and axioms, and can perform logical
	deduction.

\detailed This file contains an interface representing the core
    inference rules of the logical system. It aims to be vectorised
	and lazy where possible.

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


/**

\interface IKnowledgeKernel

\brief Represents a collection of definitions and axioms, associated
    with a particular ILanguage, and performs logical deduction.

\detailed The knowledge kernel contains all logical rules,
	axioms, and available theorems.
	Note that every kernel state is associated with a kind of
	hash-code; this is useful because the code is designed such that
	if two kernels agree on their code, they agree on the return
	results for all of their functions (assuming no hash collisions).
	Knowledge kernels are also associated with a language object.

*/
class ATP_LOGIC_API IKnowledgeKernel
{
public:
	virtual ~IKnowledgeKernel() = default;

	/**

	\brief Characterises the information provided to this knowledge
	    kernel in a single integer code.

	\detailed If two knowledge kernels agree on their code, then
	    they agree on the return results of all the functions
		below. (This code is computed similarly to a hash code of all
		the user definitions, axioms, and available theorems.)

	*/
	virtual size_t get_integrity_code() const = 0;

	/**
	\brief Get the next logical steps of an array of statements.

	\pre valid(p_stmts)

	\post For each statement in the input array, we return an array
	    of statements which logically follow in a single step from
		it.

	*/
	virtual std::vector<StatementArrayPtr> succs(
		StatementArrayPtr p_stmts) const = 0;

	/**
	\brief Type-check the given statements

	\post for each statement, check whether it is valid
	    in terms of the logic, axioms and definitions in this
	    knowledge kernel. Statements are only allowed to be used if
	    they are valid. Return true iff ALL statements in the array
	    are valid.
	    Note that a necessary condition for validity is that they
	    must be using the same language as that which created this
	    knowledge kernel.
	*/
	virtual bool valid(
		StatementArrayPtr p_stmts) const = 0;

	/**

	\brief Check if the conclusions follow in one logical step from
	    the corresponding premises

	\pre p_premise->size() == p_concl.size() and
	    valid(p_premise) and valid(p_concl)

	\post For each (premise, conclusion) pair, returns true if that
	    conclusion follows from that premise in exactly one step
		(equivalently, if the conclusion is a successor of the
		premise.)
	*/
	virtual std::vector<bool> follows(
		StatementArrayPtr p_premise,
		StatementArrayPtr p_concl) const = 0;

	/**
	\brief Determine what form each statement is in (trivial or
	    nontrivial, etc.)

	\pre valid(p_stmts)

	\post returns an array of size p_stmts->size()
	    which contains the form of each statement in the array
	    (telling you if the statements are trivially true /
	    trivially false / not canonical.)
	*/
	virtual std::vector<StmtForm> get_form(
		StatementArrayPtr p_stmts) const = 0;
};


typedef std::shared_ptr<IKnowledgeKernel> KnowledgeKernelPtr;


}  // namespace logic
}  // namespace atp


