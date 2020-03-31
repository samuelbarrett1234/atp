#pragma once


/*

Statement.h

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
#include "SyntaxNodes.h"


namespace atp
{
namespace logic
{
namespace equational
{


class ATP_LOGIC_API Statement : public IStatement
{
public:
	// precondition: !equational::needs_free_var_id_rebuild(p_root)
	// and p_root must be an eq node, with no other eq nodes in the
	// tree.
	Statement(KnowledgeKernel& ker,
		SyntaxNodePtr p_root);

	StmtForm form() const override;
	std::string to_str() const override;

	inline SyntaxNodePtr root() const
	{
		return m_root;
	}

private:
	KnowledgeKernel& m_ker;
	SyntaxNodePtr m_root;
};


}  // namespace equational
}  // namespace logic
}  // namespace atp


