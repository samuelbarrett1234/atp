#include "EquationalKnowledgeKernel.h"
#include "EquationalMatching.h"
#include "EquationalStatementArray.h"
#include <boost/functional/hash.hpp>


namespace atp
{
namespace logic
{


size_t EquationalKnowledgeKernel::get_integrity_code() const
{
	// this is a temporary and a little bit hacky:
	// we compute the hash of the defined symbols (this is okay) but
	// the way we compute the hash of the user-defined equivalences
	// is a bit crap!

	size_t hash = boost::hash_range(
		m_symb_arity.begin(), m_symb_arity.end());
	
	hash += m_rules.size() * 31;  // this is the hacky, temporary bit

	return hash;
}


std::vector<StatementArrayPtr> EquationalKnowledgeKernel::succs(
	StatementArrayPtr p_stmts) const
{
	ATP_LOGIC_PRECOND(valid(p_stmts));


}


std::vector<StatementArrayPtr> EquationalKnowledgeKernel::prevs(
	StatementArrayPtr p_stmts) const
{
	ATP_LOGIC_PRECOND(valid(p_stmts));


}


bool EquationalKnowledgeKernel::valid(
	StatementArrayPtr _p_stmts) const
{
	auto p_stmts = dynamic_cast<EquationalStatementArray*>(
		_p_stmts.get());

	if (p_stmts == nullptr)
		return false;

	// first, compute a set of IDs:

	const auto& arr = p_stmts->raw();

	std::list<SyntaxNodePtr> stack;
	for (size_t i = 0; i < arr.size(); i++)
	{
		// explore tree i using a stack
		stack.push_back(arr[i].root());
		while (!stack.empty())
		{
			auto p_node = stack.back();
			stack.pop_back();

			// handle constants...
			if (auto p_const = dynamic_cast<ConstantSyntaxNode*>(
				p_node.get()))
			{
				auto id_iter = m_id_to_name.find(
					p_const->get_symbol_id());

				if (id_iter == m_id_to_name.end())
				{
					// identifier not found
					return false;
				}
				// else... check arity

				if (m_symb_arity.at(id_iter->second) != 0)
				{
					// wrong arity
					return false;
				}
			}
			// handle functions...
			else if (auto p_func = dynamic_cast<FuncSyntaxNode*>(
				p_node.get()))
			{
				auto id_iter = m_id_to_name.find(
					p_func->get_symbol_id());

				if (id_iter == m_id_to_name.end())
				{
					// identifier not found
					return false;
				}
				// else... check arity

				if (m_symb_arity.at(id_iter->second) !=
					p_func->get_arity())
				{
					// wrong arity
					return false;
				}

				// otherwise, add children of function node to stack:
				stack.insert(stack.end(), p_func->begin(),
					p_func->end());
			}
			// finally, handle the equals sign:
			else if (auto p_eq = dynamic_cast<EqSyntaxNode*>(
				p_node.get()
				))
			{
				// nothing to check, just examine both children.
				stack.push_back(p_eq->left());
				stack.push_back(p_eq->right());
			}
		}
	}
}


std::vector<bool> EquationalKnowledgeKernel::follows(
	StatementArrayPtr p_premise, StatementArrayPtr p_concl) const
{
	ATP_LOGIC_PRECOND(p_premise->size() == p_concl->size());
	ATP_LOGIC_PRECOND(valid(p_premise));
	ATP_LOGIC_PRECOND(valid(p_concl));

	// TODO:
	// check that, at each step, the left or right hand side of the
	// premise matches the left or right hand side of the conclusion
	// (using the premise as the pattern and the conclusion as the
	// trial). I.e. there will be four checks per element per
	// equality rule.
}


void EquationalKnowledgeKernel::define_eq_rule(SyntaxNodePtr rule)
{
	// rules are all of equation form!
	ATP_LOGIC_PRECOND(rule->get_type() == SyntaxNodeType::EQ);

	auto p_rule = dynamic_cast<EqSyntaxNode*>(rule.get());

	ATP_LOGIC_ASSERT(p_rule != nullptr);

	// again, this is temporary, and we can do better.
	m_rules.push_back(std::make_pair(
		p_rule->left(), p_rule->right()
	));
}


}  // namespace logic
}  // namespace atp


