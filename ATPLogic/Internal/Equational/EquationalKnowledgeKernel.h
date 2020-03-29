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
	EquationalKnowledgeKernel();

	virtual size_t get_code() const override;
	virtual std::vector<StatementArrayPtr> succs(
		StatementArrayPtr p_stmts) const override;
	virtual std::vector<StatementArrayPtr> prevs(
		StatementArrayPtr p_stmts) const override;
	virtual bool valid(
		StatementArrayPtr p_stmts) const override;
	virtual std::vector<bool> follows(
		StatementArrayPtr p_premise,
		StatementArrayPtr p_concl) const override;

	// precondition: symbol not already defined
	// (warning: if 'name' happens to share a hash code with another
	// symbol name, which is incredibly unlikely, it will throw.)
	void define_symbol(std::string name, size_t arity);

	// TODO: do these in bulk?
	void define_eq_rule(SyntaxNodePtr rule);

	// check if a given identifier has been defined already or not
	bool is_defined(std::string name) const;

	// precondition: is_defined(name)
	size_t symbol_arity(std::string name) const;

	// precondition: is_defined(name)
	// postcondition: returns (basically) a hash of the name. It
	// should be assumed that if two symbol names agree on their
	// hash, they agree on their symbol name too.
	size_t symbol_id(std::string name) const;

private:
	// all defined symbol names, and their arity
	std::map<std::string, size_t> m_symbols;
};


}  // namespace logic
}  // namespace atp


