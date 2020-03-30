#include "EquationalKnowledgeKernel.h"
#include "EquationalMatching.h"
#include "EquationalStatementArray.h"
#include "EquationalSyntaxTreeFold.h"
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

	// TODO
}


std::vector<StatementArrayPtr> EquationalKnowledgeKernel::prevs(
	StatementArrayPtr p_stmts) const
{
	ATP_LOGIC_PRECOND(valid(p_stmts));

	// TODO
}


bool EquationalKnowledgeKernel::valid(
	StatementArrayPtr _p_stmts) const
{
	// we will use a fold to check validity!

	std::function<bool(bool, bool)> eq_valid =
		[](bool lhs, bool rhs) -> bool
	{
		return lhs && rhs;
	};
	std::function<bool(size_t)> free_valid =
		[](size_t free_id) -> bool
	{
		return true;  // can't get a free variable wrong
	};
	std::function<bool(size_t)> const_valid =
		[this](size_t symb_id) -> bool
	{
		auto id_iter = m_id_to_name.find(symb_id);

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

		// passed check
		return true;
	};
	std::function<bool(size_t, std::list<bool>::iterator,
		std::list<bool>::iterator)> func_valid =
		[this](size_t symb_id,
		std::list<bool>::iterator child_begin,
		std::list<bool>::iterator child_end) -> bool
	{
		auto id_iter = m_id_to_name.find(symb_id);

		if (id_iter == m_id_to_name.end())
		{
			// identifier not found
			return false;
		}
		// else... check arity

		const size_t implied_arity = std::distance(child_begin,
			child_end);

		if (m_symb_arity.at(id_iter->second) != implied_arity)
		{
			// wrong arity
			return false;
		}

		// if the above didn't fail, then our subtree is valid
		// iff all children are valid:
		return std::all_of(child_begin, child_end,
			[](bool x) { return x; });
	};
	auto check_stmt = [&eq_valid, &free_valid, &const_valid,
		&func_valid](const EquationalStatement& stmt) -> bool
	{
		return fold_syntax_tree<bool>(
			eq_valid,
			free_valid,
			const_valid,
			func_valid, stmt.root());
	};

	auto p_stmts = dynamic_cast<EquationalStatementArray*>(
		_p_stmts.get());

	if (p_stmts == nullptr)
		return false;

	const auto& arr = p_stmts->raw();

	return std::all_of(arr.begin(), arr.end(),
		check_stmt);
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


