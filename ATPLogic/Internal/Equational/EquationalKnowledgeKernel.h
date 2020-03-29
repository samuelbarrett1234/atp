#pragma once


/*

EquationalKnowledgeKernel.h

Implementation of the IKnowledgeKernel for equational logic.

*/


#include <map>
#include <string>
#include "../../ATPLogicAPI.h"
#include "../../Interfaces/IKnowledgeKernel.h"
#include "EquationalSyntaxTrees.h"


namespace atp
{
namespace logic
{


class ATP_LOGIC_API EquationalKnowledgeKernel :
	public IKnowledgeKernel
{
public:
	virtual size_t get_integrity_code() const override;

	virtual std::vector<StatementArrayPtr> succs(
		StatementArrayPtr p_stmts) const override;

	virtual std::vector<StatementArrayPtr> prevs(
		StatementArrayPtr p_stmts) const override;

	virtual bool valid(
		StatementArrayPtr p_stmts) const override;

	// warning: this only checks for logical implication based on the
	// theorems which are currently loaded in this knowledge kernel!
	// thus it may return incorrect results if we don't stream in/out
	// the theorems carefully.
	virtual std::vector<bool> follows(
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

	// TODO: a couple of functions for setting/unsetting large
	// batches of theorems (loaded from a database). These will
	// probably be in a more efficient format (i.e. not a
	// SyntaxNodePtr) and we will need to be able to identify
	// them so we can get rid of them later.

	// check if a given identifier has been defined already or not
	inline bool is_defined(std::string name) const
	{
		return (m_symb_arity.find(name) != m_symb_arity.end());
	}

	// precondition: is_defined(name)
	size_t symbol_arity(std::string name) const
	{
		ATP_LOGIC_PRECOND(is_defined(name));
		return m_symb_arity.at(name);
	}

	// precondition: is_defined(name)
	// postcondition: returns (basically) a hash of the name. It
	// should be assumed that if two symbol names agree on their
	// hash, they agree on their symbol name too.
	size_t symbol_id(std::string name) const
	{
		ATP_LOGIC_PRECOND(is_defined(name));
		return std::hash<std::string>()(name);
	}

private:
	// all defined symbol names, and their arity
	std::map<std::string, size_t> m_symb_arity;

	// a mapping from symbol IDs to names
	std::map<size_t, std::string> m_id_to_name;

	// this list stores pairs of expressions which appear on
	// opposite sides of an equals sign.
	// (this is definitely temporary, as it is really inefficient
	// to check against)
	std::list<std::pair<SyntaxNodePtr, SyntaxNodePtr>> m_rules;
};


}  // namespace logic
}  // namespace atp


