#pragma once


/*

EquationalStatement.h

Implementation of the IStatement interface for equational logic. In
equational logic, the main idea is to try to deduce if two things are
equal using a set of equality rules given in a definition file.

Note that an equational statement is trivially true if and only if it
is of the form "x = x", with some substitution for "x". Thus, to check
if a statement is trivial, we check if the left hand side and right
hand side are identical (without allowing free variables to be
swapped; this is obvious because f(x,y) /= f(y,x) in general.)

*/


#include "../../ATPLogicAPI.h"
#include "../../Interfaces/IStatement.h"
#include "EquationalSyntaxTrees.h"


namespace atp
{
namespace logic
{


class ATP_LOGIC_API EquationalStatement : public IStatement
{
public:
	// precondition: !eq_matching::needs_free_var_id_rebuild(p_root).
	EquationalStatement(SyntaxNodePtr p_root);

	virtual StmtForm form() const override;
	virtual std::string to_str() const override;

	inline SyntaxNodePtr root() const
	{
		return m_root;
	}

private:
	SyntaxNodePtr m_root;
};


}  // namespace logic
}  // namespace atp


