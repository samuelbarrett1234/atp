#pragma once


/**

\file

\author Samuel Barrett

\brief Implementation of the IKnowledgeKernel for equational logic

\detailed The Knowledge Kernel object contains the functions which
    tell you what is/isn't allowed in equational logic, and functions
	for logical inference etc. Currently, proofs in equational logic
	operate only in "iff" steps, so proofs can be read in both
	directions. This is for simplicity but can be a bit limiting.
	Furthermore, we don't support propositional operators (yet) so
	there is no support for and/or/not. Finally, there is no support
	for showing that two expressions are NOT equal; in other words,
	we cannot prove a statement to be false, only find a proof if it
	is true.

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

	/**
	\pre !is_defined(name)

	\warning if `name` happens to share a hash code with another
	    already-defined symbol name (which is incredibly unlikely)
		then it will throw.
	*/
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

	/**
	\pre valid(p_rules)

	\todo as of yet, there is no way to "undefine" these rules, which
	    is useful for streaming theorems in and out of memory, for
		example.
	*/
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

	// \pre is_defined(name)
	inline size_t symbol_arity_from_name(std::string name) const
	{
		ATP_LOGIC_PRECOND(is_defined(name));
		return m_name_to_arity.at(name);
	}
	
	// \pre id_is_defined(id)
	inline size_t symbol_arity_from_id(size_t id) const
	{
		ATP_LOGIC_PRECOND(id_is_defined(id));
		return m_id_to_arity.at(id);
	}

	/**
	\pre is_defined(name)
	
	\post returns a hash-code like value of the name (if two symbols
	    agree on their hash code, they agree on their string name.)
	*/
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

	// \pre 'id' is a valid symbol ID
	inline std::string symbol_name(size_t id) const
	{
		auto iter = m_id_to_name.left.find(id);
		ATP_LOGIC_PRECOND(iter != m_id_to_name.left.end());
		return iter.base()->get_right();
	}

	// \returns a list of all constant symbol IDs
	std::vector<size_t> constant_symbol_ids() const;

private:
	// returns true iff `stmt` is equivalent to one of the given
	// equality rules, allowing substitutions in the rules
	bool is_equivalent_to_a_rule(const Statement& stmt) const;

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


