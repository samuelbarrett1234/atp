#pragma once


/*

Statement.h

Implementation of the IStatement interface for equational logic. In
equational logic, the main idea is to try to deduce if two things are
equal using a set of equality rules given in a definition file.

If it comes the time to optimise the equational logic statements to
make search faster, here is the place to do it - at the moment the
Statement objects store the syntax trees internally. We do this
because it is simple and convenient, however it is inefficient.
Syntax trees are a good intermediate format (to pass between parsing
and the Statement object).

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
	Statement(const KnowledgeKernel& ker,
		SyntaxNodePtr p_root);

	StmtForm form() const override;
	std::string to_str() const override;

	// for each input rule, try making a substitution!
	// returns an array of at most 4 * rules.size(), because statements
	// have two sides each, and 2*2=4!
	std::shared_ptr<std::vector<Statement>> get_substitutions(
		const std::vector<Statement>& rules
	) const;

	// returns all the possible ways a free variable could be
	// substituted by a user definition (from the kernel) noting
	// that whenever a function is substituted, its arguments
	// are always new free variables.
	// symb_id_to_arity: a mapping from all symbol IDs, to their
	// corresponding arity.
	std::shared_ptr<std::vector<Statement>>
		replace_free_with_def(
			const std::map<size_t, size_t>& symb_id_to_arity) const;

	// returns all the ways of replacing a free variable in this
	// formula with another free variable in this formula (i.e.
	// all the ways you could reduce the number of free variables
	// in this formula by 1). Of course, we are considering
	// distinct unordered pairs of free variables.
	std::shared_ptr<std::vector<Statement>>
		replace_free_with_free() const;

	// returns true iff this statement follows from the premise
	// in any number of steps (we check this by constructing a
	// substitution, if there is one.)
	bool follows_from(const Statement& premise) const;

	// perform type checking against some other kernel
	bool type_check(const KnowledgeKernel& alternative_ker) const;

	// returns the number of (distinct) free variables
	// i.e. if the statement contains "x y z w x" we return 4.
	inline size_t num_free_variables() const
	{
		return m_num_free_vars;
	}

	// checks that the version of the kernel used by this statement
	// agrees with the given kernel (more specifically, it checks
	// the type and the integrity code).
	inline bool check_kernel(const IKnowledgeKernel* p_ker) const
	{
		return (dynamic_cast<const KnowledgeKernel*>(p_ker) != nullptr
			&& p_ker->get_integrity_code() == m_ker.get_integrity_code());
	}

private:
	const KnowledgeKernel& m_ker;
	SyntaxNodePtr m_root;
	SyntaxNodePtr m_left, m_right;  // LHS and RHS of equation
	const size_t m_num_free_vars;
};


}  // namespace equational
}  // namespace logic
}  // namespace atp


