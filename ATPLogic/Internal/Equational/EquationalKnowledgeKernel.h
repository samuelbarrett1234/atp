#pragma once


/*

EquationalKnowledgeKernel.h

Implementation of the IKnowledgeKernel for equational logic. This
object contains the functions which tell you what is/isn't allowed
in equational logic.

More specifically, the successor statements in equational logic are:
- Replacing either side of an equation with one of the given
  equality rules (just transitivity in disguise; if x=y is
  our statement, and we have a rule of the form y=z, then we
  can turn the statement into x=z.)
- Replacing a free variable with any of: a user-defined
  constant, a user-defined function whose arguments are new
  free variables, or a free variable which already exists
  within the statement.
For example, with the latter rule, if our statement is
"f(x)=g(x)" then we can obtain:
"f(e)=g(e)" for user defined "e", and "f(h(x))=g(h(x))" for
user defined "h", and instead if our statement is
"f(x, y)=g(h(x), y)" then "f(x, x)=g(h(x), x)".

*/


#include <map>
#include <string>
#include <boost/bimap.hpp>
#include "../../ATPLogicAPI.h"
#include "../../Interfaces/IKnowledgeKernel.h"
#include "EquationalSyntaxNodes.h"
#include "EquationalStatementArray.h"


namespace atp
{
namespace logic
{
namespace equational
{


// TODO: a couple of functions for setting/unsetting large
// batches of theorems (loaded from a database). These will
// probably be in a more efficient format (i.e. not a
// SyntaxNodePtr) and we will need to be able to identify
// them so we can get rid of them later.
class ATP_LOGIC_API EquationalKnowledgeKernel :
	public IKnowledgeKernel
{
public:
	size_t get_integrity_code() const override;

	std::vector<StatementArrayPtr> succs(
		StatementArrayPtr p_stmts) const override;

	bool valid(
		StatementArrayPtr p_stmts) const override;

	// warning: this only checks for logical implication based on the
	// theorems which are currently loaded in this knowledge kernel!
	// thus it may return incorrect results if we don't stream in/out
	// the theorems carefully.
	std::vector<bool> follows(
		StatementArrayPtr p_premise,
		StatementArrayPtr p_concl) const override;

	// precondition: !is_defined(name)
	// (warning: if 'name' happens to share a hash code with another
	// symbol name, which is incredibly unlikely, it will throw.)
	inline void define_symbol(std::string name, size_t arity)
	{
		ATP_LOGIC_PRECOND(!is_defined(name));
		m_symb_arity[name] = arity;
		m_id_to_name[symbol_id(name)] = name;
	}

	// this is for rules from the definition file, NOT for theorems
	// loaded from a database!
	// technically this would work as a way of inputting already-
	// proven theorems, but it will be slow and there would be no way
	// of unloading them.
	void define_eq_rule(SyntaxNodePtr rule);

	// check if a given identifier has been defined already or not
	inline bool is_defined(std::string name) const
	{
		return (m_symb_arity.find(name) != m_symb_arity.end());
	}

	// precondition: is_defined(name)
	inline size_t symbol_arity(std::string name) const
	{
		ATP_LOGIC_PRECOND(is_defined(name));
		return m_symb_arity.at(name);
	}

	// precondition: is_defined(name)
	// postcondition: returns (basically) a hash of the name. It
	// should be assumed that if two symbol names agree on their
	// hash, they agree on their symbol name too.
	inline size_t symbol_id(std::string name) const
	{
		ATP_LOGIC_PRECOND(is_defined(name));
		const size_t id = std::hash<std::string>()(name);
#ifdef ATP_LOGIC_DEFENSIVE
		ATP_LOGIC_ASSERT(m_id_to_name.left.find(id)
			!= m_id_to_name.left.end());
#endif
		return id;
	}

	// precondition: 'id' is a valid symbol ID
	inline std::string symbol_name(size_t id) const
	{
		auto iter = m_id_to_name.right.find(id);
		ATP_LOGIC_PRECOND(iter != m_id_to_name.right.end());
		return iter.base()->get_right();
	}

private:
	// return all of the statements which are reachable from the
	// input statements via user-defined equality rules (returns an
	// array B s.t. B[i] are all the statements reachable from arr[i]
	// by the equality rules.)
	std::vector<EquationalStatementArray> adjacent(
		EquationalStatementArray arr) const;

	// for each statement in 'arr',
	// for each free variable in the tree,
	// for each user-defined constant or function,
	// make the substitution (note that function arguments are always
	// new free variables).
	// returns an array B s.t. B[i] are all the results from arr[i]
	// built using a fold - see the fold_* functions for more info.
	std::vector<EquationalStatementArray> replace_free_with_def(
		const EquationalStatementArray& arr) const;

	// for each statement in 'arr',
	// for each distinct unordered pair of free variables in the tree,
	// replace the left element in the pair with the right element
	// (don't need to go the other way around).
	// returns an array B s.t. B[i] are all the results from arr[i]
	// built using a fold - see the fold_* functions for more info.
	std::vector<EquationalStatementArray> replace_free_with_free(
		const EquationalStatementArray& arr) const;

	// for each statement in 'arr'
	// for each equality rule
	// try making substitutions (for the LHS and RHS of both rules,
	// i.e. at most four possibilities per pair)
	std::vector<EquationalStatementArray> make_substitutions(
		const EquationalStatementArray& arr) const;

	// given two arrays of left-hand-sides and right-hand-sides,
	// stitch them together into an array of equality statements
	// precondition: lhss.size() == rhss.size()
	static std::vector<SyntaxNodePtr> fold_eq_constructor(
		const std::vector<SyntaxNodePtr>& lhss,
		const std::vector<SyntaxNodePtr>& rhss);

	// given a symbol ID of a constant, returns N different copies
	// of a constant syntax node with that symbol ID.
	static std::vector<SyntaxNodePtr> fold_const_constructor(
		size_t N, size_t symb_id);

	// given an array of lists of child nodes, and a function symbol
	// ID, create (for each list of children) a function node with
	// those children.
	static std::vector<SyntaxNodePtr> fold_func_constructor(
		size_t symb_id,
		const std::list<std::vector<SyntaxNodePtr>>::iterator&
		children_begin,
		const std::list<std::vector<SyntaxNodePtr>>::iterator&
		children_end);

private:
	// all defined symbol names, and their arity
	std::map<std::string, size_t> m_symb_arity;

	// a mapping from symbol IDs to names
	boost::bimap<size_t, std::string> m_id_to_name;

	// this list stores pairs of expressions which appear on
	// opposite sides of an equals sign.
	// (this is definitely temporary, as it is really inefficient
	// to check against)
	std::list<std::pair<SyntaxNodePtr, SyntaxNodePtr>> m_rules;
};


}  // namespace equational
}  // namespace logic
}  // namespace atp


