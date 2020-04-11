/**

\file

\author Samuel Barrett

*/


#include "Expression.h"
#include <boost/iterator/zip_iterator.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/range.hpp>
#include <boost/bind.hpp>
#include <boost/phoenix.hpp>
#include "SyntaxTreeFold.h"


namespace phx = boost::phoenix;


namespace atp
{
namespace logic
{
namespace equational
{


ExpressionPtr Expression::construct(const KnowledgeKernel& ker,
	SyntaxNodePtr p_root)
{
	return std::make_shared<Expression>(ker, p_root);
}


Expression::Expression(const KnowledgeKernel& ker,
	SyntaxNodePtr p_root) :
	m_ker(ker)
{
	ATP_LOGIC_PRECOND(p_root->get_type() !=
		SyntaxNodeType::EQ);

	auto root_data = add_tree_data(p_root);

	m_root = root_data.first;
	m_root_type = root_data.second;
}


Expression::Expression(const Expression& other) :
	m_ker(other.m_ker)
{
	*this = other;
}


Expression::Expression(Expression&& other) noexcept :
	m_ker(other.m_ker)
{
	*this = std::move(other);
}


Expression& Expression::operator=(const Expression& other)
{
	if (this != &other)
	{
		ATP_LOGIC_PRECOND(&m_ker == &other.m_ker);
		m_root = other.m_root;
		m_root_type = other.m_root_type;
		m_func_symb_ids = other.m_func_symb_ids;
		m_func_arity = other.m_func_arity;
		m_func_children = other.m_func_children;
		m_func_child_types = other.m_func_child_types;
		m_free_var_ids = other.m_free_var_ids;
	}
	return *this;
}


Expression& Expression::operator=(Expression&& other) noexcept
{
	if (this != &other)
	{
		ATP_LOGIC_PRECOND(&m_ker == &other.m_ker);
		m_root = other.m_root;
		m_root_type = other.m_root_type;
		m_func_symb_ids = std::move(other.m_func_symb_ids);
		m_func_arity = std::move(other.m_func_arity);
		m_func_children = std::move(other.m_func_children);
		m_func_child_types = std::move(other.m_func_child_types);
		m_free_var_ids = std::move(other.m_free_var_ids);
	}
	return *this;
}


Expression Expression::map_free_vars(const std::map<size_t,
	SyntaxNodePtr> free_map) const
{
	// check that this map is total
	ATP_LOGIC_PRECOND(std::all_of(m_free_var_ids.begin(),
		m_free_var_ids.end(),
		[&free_map](size_t id)
		{ return free_map.find(id) != free_map.end(); }));

	Expression new_expr = *this;

	// clear this, as it will automatically be rebuilt below during
	// the substitutions
	new_expr.m_free_var_ids.clear();

	// firstly, create a new mapping which doesn't use syntax trees:
	std::map<size_t, std::pair<size_t, SyntaxNodeType>> our_free_map;

	// build the new map:

	for (const auto& sub : free_map)
	{
		// we are only concerned with free variable mappings that are
		// assigned to free variables that actually exist; see the
		// unit test:
		// `test_extra_mapping_doesnt_fool_free_var_tracking`

		if (m_free_var_ids.find(sub.first)
			!= m_free_var_ids.end())
		{
			ATP_LOGIC_ASSERT(sub.second->get_type()
				!= SyntaxNodeType::EQ);

			// note that the call to add_tree_data below will do a few
			// other things to `new_stmt`:
			// - it will add free variables to the free variable ID set,
			// - it will (recursively) add function data to the table
			//   so it can be referenced below.

			our_free_map[sub.first] =
				new_expr.add_tree_data(sub.second);
		}
	}

	// now it's a relatively easy case of just directly mapping the
	// results over the arrays in `new_expr`!

	// note: of course, we don't want to apply the mapping to the new
	// functions we just created! Since the new functions are all
	// guaranteed to be at the end of the array, simply only update
	// the ones that came directly from the `this` object:
	const size_t num_funcs_to_update = m_func_symb_ids.size();
	for (size_t i = 0; i < num_funcs_to_update; ++i)
	{
		// for each child of the function
		for (size_t j = 0; j < new_expr.m_func_arity[i]; ++j)
		{
			if (new_expr.m_func_child_types[i][j]
				== SyntaxNodeType::FREE)
			{
				// find the substitution for this free variable
				auto sub_iter = our_free_map.find(
					new_expr.m_func_children[i][j]);

				// mapping should be total
				ATP_LOGIC_ASSERT(sub_iter != our_free_map.end());

				// substitution data
				const size_t new_id = sub_iter->second.first;
				const SyntaxNodeType new_type =
					sub_iter->second.second;

				// update `new_expr`

				new_expr.m_func_children[i][j] = new_id;
				new_expr.m_func_child_types[i][j] = new_type;
			}
		}
	}

	// and make sure to handle root as a special case:

	if (new_expr.m_root_type == SyntaxNodeType::FREE)
	{
		// find the substitution for this free variable
		auto sub_iter = our_free_map.find(m_root);

		// mapping should be total
		ATP_LOGIC_ASSERT(sub_iter != our_free_map.end());

		new_expr.m_root = sub_iter->second.first;
		new_expr.m_root_type = sub_iter->second.second;
	}

	// done:
	return new_expr;
}


std::pair<size_t, SyntaxNodeType>
Expression::add_tree_data(SyntaxNodePtr tree)
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


SyntaxNodePtr Expression::to_syntax_tree(size_t id,
	SyntaxNodeType type) const
{
	// \todo don't use recursion here

	switch (type)
	{
	case SyntaxNodeType::FREE:
		return FreeSyntaxNode::construct(id);
	case SyntaxNodeType::CONSTANT:
		return ConstantSyntaxNode::construct(id);
	case SyntaxNodeType::FUNC:
	{
		const size_t arity = m_func_arity[id];

		std::vector<SyntaxNodePtr> children;
		children.reserve(arity);
		for (size_t i = 0; i < arity; ++i)
		{
			children.push_back(to_syntax_tree(
				m_func_children[id][i],
				m_func_child_types[id][i]));
		}

		return FuncSyntaxNode::construct(
			m_func_symb_ids[id],
			children.begin(), children.end());
	}
	default:
		ATP_LOGIC_ASSERT(false && "invalid type.");
		return SyntaxNodePtr();
	}
}


}  // namespace equational
}  // namespace logic
}  // namespace atp


