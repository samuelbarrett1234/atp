#include "ATPLogic.h"
#include "Internal/Equational/EquationalStatementArray.h"


namespace atp
{
namespace logic
{


StatementArrayPtr from_statement(const IStatement& stmt)
{
	StatementArrayPtr p_result;

	// for each statement type, try converting it here:

	p_result = EquationalStatementArray::try_from_stmt(stmt);
	if (p_result != nullptr) return p_result;

	// if we get to here we have failed

	ATP_LOGIC_PRECOND(false && "Invalid statement type!");

	// return as well just incase assertion is disabled
	return StatementArrayPtr();
}


StatementArrayPtr concat(const IStatementArray& l,
	const IStatementArray& r)
{
	StatementArrayPtr p_result;

	// for each statement type, try converting it here:

	p_result = EquationalStatementArray::try_concat(l, r);
	if (p_result != nullptr) return p_result;

	// if we get to here we have failed

	ATP_LOGIC_PRECOND(false && "Invalid statement types!");

	// return as well just incase assertion is disabled
	return StatementArrayPtr();
}


StatementArrayPtr concat(const std::vector<StatementArrayPtr>& stmts)
{
	StatementArrayPtr p_result;

	// for each statement type, try converting it here:

	p_result = EquationalStatementArray::try_concat(stmts);
	if (p_result != nullptr) return p_result;

	// if we get to here we have failed

	ATP_LOGIC_PRECOND(false && "Invalid statement types!");

	// return as well just incase assertion is disabled
	return StatementArrayPtr();
}


}  // namespace logic
}  // namespace atp


