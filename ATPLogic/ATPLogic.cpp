#include "ATPLogic.h"
#include "Internal/Equational/StatementArray.h"
#include "Internal/Equational/Language.h"


namespace atp
{
namespace logic
{


LanguagePtr create_language(LangType lt)
{
	switch (lt)
	{
	case LangType::EQUATIONAL_LOGIC:
		return std::make_shared<equational::Language>();
	default:
		ATP_LOGIC_PRECOND(false && "invalid language type.");
		return LanguagePtr();
	}
}


StatementArrayPtr from_statement(const IStatement& stmt)
{
	StatementArrayPtr p_result;

	// for each statement type, try converting it here:

	p_result = equational::StatementArray::try_from_stmt(stmt);
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

	p_result = equational::StatementArray::try_concat(l, r);
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

	p_result = equational::StatementArray::try_concat(stmts);
	if (p_result != nullptr) return p_result;

	// if we get to here we have failed

	ATP_LOGIC_PRECOND(false && "Invalid statement types!");

	// return as well just incase assertion is disabled
	return StatementArrayPtr();
}


}  // namespace logic
}  // namespace atp


