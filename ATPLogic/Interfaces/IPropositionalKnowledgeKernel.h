#pragma once


/**

\file

\author Samuel Barrett

\brief Contains a common specialisation of the IKnowledgeKernel
    interface.

\detailed Most logics support at least propositional logic (and /
    or / not), thus we provide this extended interface for such
	logics, to support special search algorithms which require
	this structure.

*/


#include <memory>
#include <vector>
#include "../ATPLogicAPI.h"
#include "IKnowledgeKernel.h"
#include "IStatement.h"
#include "IStatementArray.h"


namespace atp
{
namespace logic
{


/**

\interface IPropositionalKnowledgeKernel

\detailed A knowledge kernel which supports propositional logic, thus
    provides functions for the search algorithms to manipulate
    statements using propositional logic.
*/
class ATP_LOGIC_API IPropositionalKnowledgeKernel :
	public IKnowledgeKernel
{
public:
	virtual ~IPropositionalKnowledgeKernel() = default;

	/**
	\brief Returns the logical negation of each statement
	
	\pre valid(p_stmts)
	*/
	virtual StatementArrayPtr negate_stmts(
		StatementArrayPtr p_stmts) const = 0;

	/**
	\brief Returns the logical and-ing of the pairs of statements

	\pre valid(p_stmts) and l->size() == r->size()
	*/
	virtual StatementArrayPtr and_stmts(
		StatementArrayPtr l, StatementArrayPtr r) const = 0;

	/**
	\brief Returns the logical or-ing of the pairs of statements

	\pre valid(p_stmts) and l->size() == r->size()
	*/
	virtual StatementArrayPtr or_stmts(
		StatementArrayPtr l, StatementArrayPtr r) const = 0;
};


}  // namespace logic
}  // namespace atp


