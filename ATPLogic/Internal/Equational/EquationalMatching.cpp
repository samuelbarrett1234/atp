#include "EquationalMatching.h"
#include <boost/iterator/zip_iterator.hpp>
#include <boost/bimap.hpp>


namespace atp
{
namespace logic
{
namespace eq_matching
{


typedef boost::bimap<size_t, size_t> FreeIdMatching;


// TODO: do better than a recursion...
bool equivalent_from_mapping(ISyntaxNode* p_a, ISyntaxNode* p_b,
	FreeIdMatching& free_id_map);


bool equivalent(SyntaxNodePtr p_a, SyntaxNodePtr p_b)
{
	// we will (greedily) build a mapping from free variable IDs in
	// 'a' to free variable IDs in 'b':
	FreeIdMatching free_id_map;

	return equivalent_from_mapping(p_a.get(), p_b.get(),
		free_id_map);
}


bool trivially_equal(SyntaxNodePtr p_a, SyntaxNodePtr p_b)
{
	// we will (greedily) build a mapping from free variable IDs in
	// 'a' to free variable IDs in 'b', but then we will assert that
	// mapping is just the identity mapping!

	FreeIdMatching free_id_map;

	const bool equiv = equivalent_from_mapping(p_a.get(), p_b.get(),
		free_id_map);

	if (!equiv)
		return false;  // they are definitely not equivalent

	// note that if equiv is true, it is necessarily the case that
	// both expressions have the same number of free variables!

	// else check that free_id_map is the identity:

	for (auto iter = free_id_map.begin(); iter != free_id_map.end();
		iter++)
	{
		if (iter->get_left() != iter->get_right())
			return false;  // doesn't map left to itself
	}
}


bool trivially_true(SyntaxNodePtr _p_eq)
{
	ATP_LOGIC_PRECOND(_p_eq->get_type() == SyntaxNodeType::EQ);

	auto p_eq = dynamic_cast<EqSyntaxNode*>(_p_eq.get());

	ATP_LOGIC_ASSERT(p_eq != nullptr);

	// check if LHS is equal to RHS!
	return trivially_equal(p_eq->left(), p_eq->right());
}


bool equivalent_from_mapping(ISyntaxNode* p_a, ISyntaxNode* p_b,
	FreeIdMatching& free_id_map)
{
	if (p_a->get_type() != p_b->get_type())
		return false;

	switch (p_a->get_type())
	{
	case SyntaxNodeType::EQ:
	{
		auto p_a_eq = dynamic_cast<EqSyntaxNode*>(p_a);
		auto p_b_eq = dynamic_cast<EqSyntaxNode*>(p_b);

		ATP_LOGIC_ASSERT(p_a_eq != nullptr);
		ATP_LOGIC_ASSERT(p_b_eq != nullptr);

		return equivalent_from_mapping(p_a_eq->left().get(),
			p_b_eq->left().get(), free_id_map) &&
			equivalent_from_mapping(p_a_eq->right().get(),
				p_b_eq->right().get(), free_id_map);
	}
	case SyntaxNodeType::FREE:
	{
		auto p_a_free = dynamic_cast<FreeSyntaxNode*>(p_a);
		auto p_b_free = dynamic_cast<FreeSyntaxNode*>(p_b);

		ATP_LOGIC_ASSERT(p_a_free != nullptr);
		ATP_LOGIC_ASSERT(p_b_free != nullptr);

		const size_t a_id = p_a_free->get_free_id();
		const size_t b_id = p_b_free->get_free_id();

		auto left_iter = free_id_map.left.find(a_id);
		auto right_iter = free_id_map.right.find(b_id);

		// if variables are, as of yet, unpaired...
		if (left_iter == free_id_map.left.end() &&
			right_iter == free_id_map.right.end())
		{
			// greedily set this, as we haven't seen either variable
			// before
			free_id_map.left[a_id] = b_id;
			return true;
		}
		else if (left_iter->get_right() == b_id &&
			right_iter->get_left() == a_id)
		{
			return true;  // variables have already been paired
		}
		// variables have been paired already with other variables
		else return false;
	}
	case SyntaxNodeType::CONSTANT:
	{
		auto p_a_const = dynamic_cast<ConstantSyntaxNode*>(p_a);
		auto p_b_const = dynamic_cast<ConstantSyntaxNode*>(p_b);

		ATP_LOGIC_ASSERT(p_a_const != nullptr);
		ATP_LOGIC_ASSERT(p_b_const != nullptr);

		// just check that they are the same symbol
		return (p_a_const->get_symbol_id() ==
			p_b_const->get_symbol_id());
	}
	case SyntaxNodeType::FUNC:
	{
		auto p_a_func = dynamic_cast<FuncSyntaxNode*>(p_a);
		auto p_b_func = dynamic_cast<FuncSyntaxNode*>(p_b);

		ATP_LOGIC_ASSERT(p_a_func != nullptr);
		ATP_LOGIC_ASSERT(p_b_func != nullptr);

		// just check that they are the same symbol and that all
		// their children also match:

		if (p_a_func->get_symbol_id() != p_b_func->get_symbol_id())
			return false;

		if (p_a_func->get_arity() != p_b_func->get_arity())
			return false;

		auto begin = boost::make_zip_iterator(boost::make_tuple(
			p_a_func->begin(), p_b_func->begin()));
		auto end = boost::make_zip_iterator(boost::make_tuple(
			p_a_func->end(), p_b_func->end()));

		for (auto iter = begin; iter != end; iter++)
		{
			if (!equivalent_from_mapping(
				iter->get<0>().get(),
				iter->get<1>().get(),
				free_id_map
			))
				return false;  // children don't agree
		}

		// all checks passed
		return true;
	}
#ifdef ATP_LOGIC_DEFENSIVE
	default:
		ATP_LOGIC_ASSERT(false && "invalid syntax node type - memory"
			" corruption?");
		return false;
#endif
	}
}


}
}  // namespace logic
}  // namespace atp


