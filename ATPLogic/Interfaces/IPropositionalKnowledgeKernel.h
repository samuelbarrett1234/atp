#pragma once


/*

IPropositionalKnowledgeKernel.h

Most logics support at least propositional logic (and/or/not), thus
we provide this extended interface for such logics, to support
special search algorithms which require this structure.

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


/// <summary>
/// A knowledge kernel which supports propositional logic, thus
/// provides functions for the search algorithms to manipulate
/// statements using propositional logic.
/// </summary>
class ATP_LOGIC_API IPropositionalKnowledgeKernel :
	public IKnowledgeKernel
{
public:
	virtual ~IPropositionalKnowledgeKernel() = default;

	// Precondition: valid(p_stmts)
	// Postcondition: returns the negation of each statement in the
	// array (this is always valid because propositional logic is
	// built into the system).
	virtual StatementArrayPtr negate_stmts(
		StatementArrayPtr p_stmts) const = 0;

	// Precondition: valid(p_stmts) and l->size() == r->size()
	// Postcondition: returns the and-ing of each pair of statements
	// in the two arrays (this is always valid because propositional
	// logic is built into the system).
	virtual StatementArrayPtr and_stmts(
		StatementArrayPtr l, StatementArrayPtr r) const = 0;

	// Precondition: valid(p_stmts) and l->size() == r->size()
	// Postcondition: returns the or-ing of each pair of statements
	// in the two arrays (this is always valid because propositional
	// logic is built into the system).
	virtual StatementArrayPtr or_stmts(
		StatementArrayPtr l, StatementArrayPtr r) const = 0;
};


}  // namespace logic
}  // namespace atp


