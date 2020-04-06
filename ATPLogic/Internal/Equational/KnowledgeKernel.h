#pragma once


/*

KnowledgeKernel.h

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
#include <vector>
#include <list>
#include <boost/bimap.hpp>
#include "../../ATPLogicAPI.h"
#include "../../Interfaces/IKnowledgeKernel.h"
#include "Statement.h"
#include "StatementArray.h"


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
class ATP_LOGIC_API KnowledgeKernel :
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

	std::vector<StmtForm> get_form(
		StatementArrayPtr p_stmts) const override;

	// precondition: !is_defined(name)
	// (warning: if 'name' happens to share a hash code with another
	// symbol name, which is incredibly unlikely, it will throw.)
	inline void define_symbol(std::string name, size_t arity)
	{
		ATP_LOGIC_PRECOND(!is_defined(name));
		const size_t id = _get_id_from_str(name);

		// check we have no hash collisions:
		ATP_LOGIC_ASSERT(!id_is_defined(id));

		m_name_to_arity[name] = arity;
		m_id_to_name.left.insert(std::make_pair(id, name));
		m_id_to_arity[id] = arity;
	}

	// note: as of yet, there is no way of "undefining" these rules,
	// if you wanted to stream theorems in and out of memory (for
	// example.)
	// precondition: valid(p_rules)
	void define_eq_rules(StatementArrayPtr p_rules);

	// check if a given identifier has been defined already or not
	inline bool is_defined(std::string name) const
	{
		return (m_name_to_arity.find(name) != m_name_to_arity.end());
	}

	// check if a given ID has been defined already or not
	inline bool id_is_defined(size_t id) const
	{
		return (m_id_to_name.left.find(id) != m_id_to_name.left.end());
	}

	// precondition: is_defined(name)
	inline size_t symbol_arity_from_name(std::string name) const
	{
		ATP_LOGIC_PRECOND(is_defined(name));
		return m_name_to_arity.at(name);
	}
	
	// precondition: id_is_defined(id)
	inline size_t symbol_arity_from_id(size_t id) const
	{
		ATP_LOGIC_PRECOND(id_is_defined(id));
		return m_id_to_arity.at(id);
	}

	// precondition: is_defined(name)
	// postcondition: returns (basically) a hash of the name. It
	// should be assumed that if two symbol names agree on their
	// hash, they agree on their symbol name too.
	inline size_t symbol_id(std::string name) const
	{
		ATP_LOGIC_PRECOND(is_defined(name));
		const size_t id = _get_id_from_str(name);
#ifdef ATP_LOGIC_DEFENSIVE
		ATP_LOGIC_ASSERT(m_id_to_name.left.find(id)
			!= m_id_to_name.left.end());
#endif
		return id;
	}

	// precondition: 'id' is a valid symbol ID
	inline std::string symbol_name(size_t id) const
	{
		auto iter = m_id_to_name.left.find(id);
		ATP_LOGIC_PRECOND(iter != m_id_to_name.left.end());
		return iter.base()->get_right();
	}

private:
	// returns true iff `stmt` is equivalent to one of the given
	// equality rules, allowing substitutions in the rules
	bool is_equivalent_to_a_rule(const Statement& stmt) const;

private:
	// a version of "symbol_id" which doesn't check for
	// is_defined(str) or anything like that
	inline size_t _get_id_from_str(std::string str) const
	{
		return std::hash<std::string>()(str);
	}

private:
	// mapping from symbol names to arity
	std::map<std::string, size_t> m_name_to_arity;

	// mapping from symbol IDs to arity
	std::map<size_t, size_t> m_id_to_arity;

	// a mapping from symbol IDs to names
	boost::bimap<size_t, std::string> m_id_to_name;

	// all equality rules (given as axioms)
	// use vector not StatementArray because we need mutability
	// (we need to .push_back as we go):
	std::vector<Statement> m_rules;
};


}  // namespace equational
}  // namespace logic
}  // namespace atp


