/**

\file

\author Samuel Barrett

*/


#include "Statement.h"
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/iterator/zip_iterator.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/range.hpp>
#include <boost/bind.hpp>
#include <boost/phoenix.hpp>
#include "SyntaxTreeFold.h"
#include "KnowledgeKernel.h"
#include "SemanticsHelper.h"


namespace phx = boost::phoenix;


namespace atp
{
namespace logic
{
namespace equational
{


Statement::Statement(
	const KnowledgeKernel& ker,
	SyntaxNodePtr p_root) :
	m_ker(ker)
{
	ATP_LOGIC_PRECOND(p_root->get_type() ==
		SyntaxNodeType::EQ);

	auto p_eq = dynamic_cast<EqSyntaxNode*>(p_root.get());

	ATP_LOGIC_ASSERT(p_eq != nullptr);

	auto left_pair = add_tree_data(p_eq->left());
	auto right_pair = add_tree_data(p_eq->right());

	m_left = left_pair.first;
	m_left_type = left_pair.second;
	m_right = right_pair.first;
	m_right_type = right_pair.second;
}


Statement::Statement(const Statement& other) :
	m_ker(other.m_ker)
{
	*this = other;
}


Statement::Statement(Statement&& other) noexcept :
	m_ker(other.m_ker)
{
	*this = std::move(other);
}


Statement& Statement::operator=(const Statement& other)
{
	if (this != &other)
	{
		ATP_LOGIC_PRECOND(&m_ker == &other.m_ker);
		m_left = other.m_left;
		m_left_type = other.m_left_type;
		m_right = other.m_right;
		m_right_type = other.m_right_type;
		m_func_symb_ids = other.m_func_symb_ids;
		m_func_arity = other.m_func_arity;
		m_func_children = other.m_func_children;
		m_func_child_types = other.m_func_child_types;
		m_free_var_ids = other.m_free_var_ids;
	}
	return *this;
}


Statement& Statement::operator=(Statement&& other) noexcept
{
	if (this != &other)
	{
		ATP_LOGIC_PRECOND(&m_ker == &other.m_ker);
		m_left = std::move(other.m_left);
		m_left_type = std::move(other.m_left_type);
		m_right = std::move(other.m_right);
		m_right_type = std::move(other.m_right_type);
		m_func_symb_ids = std::move(other.m_func_symb_ids);
		m_func_arity = std::move(other.m_func_arity);
		m_func_children = std::move(other.m_func_children);
		m_func_child_types = std::move(other.m_func_child_types);
		m_free_var_ids = std::move(other.m_free_var_ids);
	}
	return *this;
}


std::string Statement::to_str() const
{
	// this is a fold!

	auto eq_fold = [](std::string lhs, std::string rhs)
		-> std::string
	{
		return lhs + " = " + rhs;
	};
	auto free_var_fold = [](size_t free_var_id) -> std::string
	{
		return "x" + boost::lexical_cast<std::string>(free_var_id);
	};
	auto const_fold = [this](size_t symb_id) -> std::string
	{
		return m_ker.symbol_name(symb_id);
	};
	auto func_fold = [this](size_t symb_id,
		std::vector<std::string>::iterator begin,
		std::vector<std::string>::iterator end) -> std::string
	{
		return m_ker.symbol_name(symb_id) + '(' +
			boost::algorithm::join(
				boost::make_iterator_range(begin, end), ", ") + ')';
	};

	return fold<std::string>(eq_fold, free_var_fold,
		const_fold, func_fold);
}


Statement Statement::transpose() const
{
	Statement tr = *this;

	std::swap(tr.m_left, tr.m_right);
	std::swap(tr.m_left_type, tr.m_right_type);

	return tr;
}


std::pair<SyntaxNodePtr, SyntaxNodePtr> Statement::get_sides() const
{
	return std::make_pair(to_syntax_tree(m_left, m_left_type),
		to_syntax_tree(m_right, m_right_type));
}


Statement Statement::adjoin_rhs(const Statement& other) const
{
	ATP_LOGIC_PRECOND(&m_ker == &other.m_ker);

	// \todo: this could be more efficient I think, but it would
	// be a considerable amount of effort (perhaps a couple of
	// hundred lines?)

	auto my_sides = get_sides();
	auto other_sides = other.get_sides();

	auto new_eq = EqSyntaxNode::construct(my_sides.first,
		other_sides.second);

	return Statement(m_ker, new_eq);
}


Statement Statement::map_free_vars(const std::map<size_t,
	SyntaxNodePtr> free_map) const
{
	// check that this map is total
	ATP_LOGIC_PRECOND(std::all_of(m_free_var_ids.begin(),
		m_free_var_ids.end(),
		[&free_map](size_t id)
		{ return free_map.find(id) != free_map.end(); }));

	Statement new_stmt = *this;

	// clear this, as it will automatically be rebuilt below during
	// the substitutions
	new_stmt.m_free_var_ids.clear();

	// firstly, create a new mapping which doesn't use syntax trees:
	std::map<size_t, std::pair<size_t, SyntaxNodeType>> our_free_map;

	// build the new map:

	for (const auto& sub : free_map)
	{
		ATP_LOGIC_ASSERT(sub.second->get_type()
			!= SyntaxNodeType::EQ);

		// note that the call to add_tree_data below will do a few
		// other things to `new_stmt`:
		// - it will add free variables to the free variable ID set,
		// - it will (recursively) add function data to the table
		//   so it can be referenced below.

		our_free_map[sub.first] =
			new_stmt.add_tree_data(sub.second);
	}

	// now it's a relatively easy case of just directly mapping the
	// results over the arrays in `new_stmt`!

	// note: of course, we don't want to apply the mapping to the new
	// functions we just created! Since the new functions are all
	// guaranteed to be at the end of the array, simply only update
	// the ones that came directly from the `this` object:
	const size_t num_funcs_to_update = m_func_symb_ids.size();
	for (size_t i = 0; i < num_funcs_to_update; ++i)
	{
		// for each child of the function
		for (size_t j = 0; j < new_stmt.m_func_arity[i]; ++j)
		{
			if (new_stmt.m_func_child_types[i][j]
				== SyntaxNodeType::FREE)
			{
				// find the substitution for this free variable
				auto sub_iter = our_free_map.find(
					new_stmt.m_func_children[i][j]);

				// mapping should be total
				ATP_LOGIC_ASSERT(sub_iter != our_free_map.end());

				// substitution data
				const size_t new_id = sub_iter->second.first;
				const SyntaxNodeType new_type =
					sub_iter->second.second;

				// update `new_stmt`

				new_stmt.m_func_children[i][j] = new_id;
				new_stmt.m_func_child_types[i][j] = new_type;
			}
		}
	}

	// and make sure to handle left/right as a special case:
	
	if (new_stmt.m_left_type == SyntaxNodeType::FREE)
	{
		// find the substitution for this free variable
		auto sub_iter = our_free_map.find(m_left);

		// mapping should be total
		ATP_LOGIC_ASSERT(sub_iter != our_free_map.end());

		new_stmt.m_left = sub_iter->second.first;
		new_stmt.m_left_type = sub_iter->second.second;
	}
	if (new_stmt.m_right_type == SyntaxNodeType::FREE)
	{
		// find the substitution for this free variable
		auto sub_iter = our_free_map.find(m_right);

		// mapping should be total
		ATP_LOGIC_ASSERT(sub_iter != our_free_map.end());

		new_stmt.m_right = sub_iter->second.first;
		new_stmt.m_right_type = sub_iter->second.second;
	}

	// done:
	return new_stmt;
}


SyntaxNodePtr Statement::to_syntax_tree(size_t idx,
	SyntaxNodeType type) const
{
	ATP_LOGIC_PRECOND(type != SyntaxNodeType::EQ);

	switch (type)
	{
	case SyntaxNodeType::FREE:
		return FreeSyntaxNode::construct(idx);
	case SyntaxNodeType::CONSTANT:
		return ConstantSyntaxNode::construct(idx);
	case SyntaxNodeType::FUNC:
	{
		// eek recursion
		// \todo use a stack (a fold-like operation) here

		std::vector<SyntaxNodePtr> children;
		children.reserve(m_func_arity[idx]);

		for (size_t i = 0; i < m_func_arity[idx]; ++i)
		{
			children.push_back(to_syntax_tree(
				m_func_children[idx][i],
				m_func_child_types[idx][i]));
		}

		return FuncSyntaxNode::construct(m_func_symb_ids[idx],
			children.begin(), children.end());
	}
	default:
		ATP_LOGIC_ASSERT(false && "invalid type");
		return SyntaxNodePtr();
	}
}


std::pair<size_t, SyntaxNodeType>
Statement::add_tree_data(SyntaxNodePtr tree)
{
	ATP_LOGIC_PRECOND(tree->get_type() !=
		SyntaxNodeType::EQ);

	typedef std::pair<size_t, SyntaxNodeType> NodePair;

	auto eq_func = [](NodePair l, NodePair r) -> NodePair
	{
		ATP_LOGIC_PRECOND(false && "no eq allowed!");

		// we have to return something to avoid compiler
		// errors
		return std::make_pair(0, SyntaxNodeType::EQ);
	};

	auto free_func = [this](size_t free_id) -> NodePair
	{
		m_free_var_ids.insert(free_id);
		return std::make_pair(free_id, SyntaxNodeType::FREE);
	};

	auto const_func = [](size_t symb_id) -> NodePair
	{
		return std::make_pair(symb_id, SyntaxNodeType::CONSTANT);
	};

	auto f_func = [this](size_t symb_id,
		std::vector<NodePair>::iterator begin,
		std::vector<NodePair>::iterator end) -> NodePair
	{
		// when we add a new element to all the vectors, the index
		// will be the newsize-1, which is just the oldsize.
		const size_t func_idx = m_func_children.size();

		const size_t arity = std::distance(begin, end);

		// arity limit!!!
		ATP_LOGIC_PRECOND(arity < MAX_ARITY);

		m_func_arity.push_back(arity);
		m_func_symb_ids.push_back(symb_id);

		auto map_first = boost::bind(&NodePair::first, _1);
		auto map_second = boost::bind(&NodePair::second, _1);

		m_func_children.emplace_back();
		m_func_child_types.emplace_back();

		std::copy(boost::make_transform_iterator(begin, map_first),
			boost::make_transform_iterator(end, map_first),
			m_func_children.back().begin());

		std::copy(boost::make_transform_iterator(begin, map_second),
			boost::make_transform_iterator(end, map_second),
			m_func_child_types.back().begin());

		return std::make_pair(func_idx, SyntaxNodeType::FUNC);
	};

	return fold_syntax_tree<NodePair>(eq_func, free_func, const_func,
		f_func, tree);
}


}  // namespace equational
}  // namespace logic
}  // namespace atp


