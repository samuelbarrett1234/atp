#include "EquationalMatching.h"
#include "EquationalSyntaxTreeFold.h"
#include "EquationalSyntaxTreeTraversal.h"
#include <boost/iterator/zip_iterator.hpp>
#include <boost/bimap.hpp>
#include <boost/mpl/identity.hpp>
#include <set>


namespace atp
{
namespace logic
{
namespace eq_matching
{


typedef boost::bimap<size_t, size_t> FreeIdMatching;


// TODO: do better than recursion!


// check if p_a and p_b are equivalent (return true iff they are) and
// keep track of which free variables have been paired
bool equivalent_from_mapping(const ISyntaxNode* p_a,
	const ISyntaxNode* p_b, FreeIdMatching& free_id_map);


// check if the trial can be created from p_a by making substitutions
// for the pattern's free variables, to create a tree identical to
// the trial's. We also have to keep track of substitutions made.
bool build_mapping(SyntaxNodePtr pattern, SyntaxNodePtr trial,
	FreeVarSubstitution& substitutions);


// get all of the free variable IDs in the tree rooted at this node
std::set<size_t> get_free_var_ids(SyntaxNodePtr p_node);


boost::optional<FreeVarSubstitution> try_match(SyntaxNodePtr pattern,
	SyntaxNodePtr trial)
{
	FreeVarSubstitution subs;

	if (build_mapping(pattern, trial, subs))
	{
		return subs;
	}
	else return boost::none;
}


SyntaxNodePtr get_substitution(SyntaxNodePtr p_node,
	FreeVarSubstitution subs)
{
	// it turns out that substitution is just a fold!

	// warning: we need to make sure there are no free variable
	// clashes when we make the substitution! To solve this, we will
	// just offset all of the free variables in p_node which are not
	// being substituted, by the highest ID in p_node, to make sure
	// they remain unique. right at the end we will rebuild the IDs
	// anyway.

	const auto var_ids = get_free_var_ids(p_node);
	const auto max_id = *std::max_element(var_ids.begin(), var_ids.end());

	auto fold_eq_constructor = boost::bind(
		std::make_shared<EqSyntaxNode>, _1);
	auto fold_const_constructor = boost::bind(
		std::make_shared<ConstantSyntaxNode>, _1);
	auto fold_func_constructor = boost::bind(
		std::make_shared<FuncSyntaxNode>, _1, _2, _3);

	auto fold_free_constructor = [&subs, max_id](size_t free_var_id)
		-> SyntaxNodePtr
	{
		auto iter = subs.find(free_var_id);

		// if this is a free variable we're not interested in
		// substituting...
		if (iter == subs.end())
			return std::make_shared<FreeSyntaxNode>(
				// see earlier comment for why we add these
				free_var_id + max_id);

		else  // else return the substitution...
			return iter->second;
	};

	auto result_tree = fold_syntax_tree<SyntaxNodePtr>(
		fold_eq_constructor, fold_free_constructor,
		fold_const_constructor, fold_func_constructor,
		p_node
	);

	rebuild_free_var_ids(result_tree);

	return result_tree;
}


void rebuild_free_var_ids(SyntaxNodePtr p_node)
{
	auto var_ids = get_free_var_ids(p_node);

	// mapping from old IDs to new IDs
	std::map<size_t, size_t> id_map;
	size_t next_id = 0;

	for (size_t id : var_ids)
	{
		id_map[id] = next_id;
		next_id++;
	}

	std::list<SyntaxNodePtr> stack;
	stack.push_back(p_node);

	while (!stack.empty())
	{
		p_node = stack.back();
		stack.pop_back();

		if (p_node->get_type() == SyntaxNodeType::FREE)
		{
			auto p_free = dynamic_cast<FreeSyntaxNode*>(
				p_node.get());
			ATP_LOGIC_ASSERT(p_free != nullptr);

			p_free->rebuild_free_id(id_map.at(
				p_free->get_free_id()));
		}
	}
}


bool needs_free_var_id_rebuild(SyntaxNodePtr p_node)
{
	auto var_ids = get_free_var_ids(p_node);

	std::vector<bool> id_bitmap;

	for (size_t id : var_ids)
	{
		// ensure that the bitmap is large enough so that
		// id_bitmap[id] exists - but no larger than necessary
		if (id >= id_bitmap.size())
			id_bitmap.resize(id + 1, false);

		id_bitmap[id] = true;
	}

	ATP_LOGIC_ASSERT(id_bitmap.empty() == var_ids.empty());

	// either there are no free variables, or the free variable with
	// largest ID is at the back, thus the back must be true
	ATP_LOGIC_ASSERT(id_bitmap.empty() || id_bitmap.back());

	// we need a rebuild iff any element in this vector is false
	return std::all_of(id_bitmap.begin(), id_bitmap.end(),
		boost::mpl::identity<bool>());
}


size_t num_free_vars(SyntaxNodePtr p_node)
{
	return get_free_var_ids(p_node).size();
}


bool equivalent(const ISyntaxNode& a, const ISyntaxNode& b)
{
	// we will (greedily) build a mapping from free variable IDs in
	// 'a' to free variable IDs in 'b':
	FreeIdMatching free_id_map;

	return equivalent_from_mapping(&a, &b,
		free_id_map);
}


bool identical(const ISyntaxNode& a, const ISyntaxNode& b)
{
	// we will (greedily) build a mapping from free variable IDs in
	// 'a' to free variable IDs in 'b', but then we will assert that
	// mapping is just the identity mapping!

	FreeIdMatching free_id_map;

	const bool equiv = equivalent_from_mapping(&a, &b,
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


bool trivially_true(const ISyntaxNode& _eq)
{
	ATP_LOGIC_PRECOND(_eq.get_type() == SyntaxNodeType::EQ);

	auto p_eq = dynamic_cast<const EqSyntaxNode*>(&_eq);

	ATP_LOGIC_ASSERT(p_eq != nullptr);

	// check if LHS is equal to RHS!
	return identical(*p_eq->left(), *p_eq->right());
}


bool equivalent_from_mapping(const ISyntaxNode* p_a,
	const ISyntaxNode* p_b,
	FreeIdMatching& free_id_map)
{
	if (p_a->get_type() != p_b->get_type())
		return false;

	switch (p_a->get_type())
	{
	case SyntaxNodeType::EQ:
	{
		auto p_a_eq = dynamic_cast<const EqSyntaxNode*>(p_a);
		auto p_b_eq = dynamic_cast<const EqSyntaxNode*>(p_b);

		ATP_LOGIC_ASSERT(p_a_eq != nullptr);
		ATP_LOGIC_ASSERT(p_b_eq != nullptr);

		return equivalent_from_mapping(p_a_eq->left().get(),
			p_b_eq->left().get(), free_id_map) &&
			equivalent_from_mapping(p_a_eq->right().get(),
				p_b_eq->right().get(), free_id_map);
	}
	case SyntaxNodeType::FREE:
	{
		auto p_a_free = dynamic_cast<const FreeSyntaxNode*>(p_a);
		auto p_b_free = dynamic_cast<const FreeSyntaxNode*>(p_b);

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
		auto p_a_const =
			dynamic_cast<const ConstantSyntaxNode*>(p_a);
		auto p_b_const =
			dynamic_cast<const ConstantSyntaxNode*>(p_b);

		ATP_LOGIC_ASSERT(p_a_const != nullptr);
		ATP_LOGIC_ASSERT(p_b_const != nullptr);

		// just check that they are the same symbol
		return (p_a_const->get_symbol_id() ==
			p_b_const->get_symbol_id());
	}
	case SyntaxNodeType::FUNC:
	{
		auto p_a_func = dynamic_cast<const FuncSyntaxNode*>(p_a);
		auto p_b_func = dynamic_cast<const FuncSyntaxNode*>(p_b);

		ATP_LOGIC_ASSERT(p_a_func != nullptr);
		ATP_LOGIC_ASSERT(p_b_func != nullptr);

		// just check that they are the same symbol and that all
		// their children also match:

		if (p_a_func->get_symbol_id() != p_b_func->get_symbol_id())
			return false;

		if (p_a_func->get_arity() != p_b_func->get_arity())
			return false;

		ATP_LOGIC_ASSERT(std::distance(p_a_func->begin(),
			p_a_func->end()) == std::distance(p_b_func->begin(),
				p_b_func->end()));

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


bool build_mapping(SyntaxNodePtr pattern, SyntaxNodePtr trial,
	FreeVarSubstitution& substitutions)
{
	// free can be turned into anything!
	if (pattern->get_type() != trial->get_type() &&
		pattern->get_type() != SyntaxNodeType::FREE)
		return false;

	// in this case, we have to make a substitution:
	if (pattern->get_type() == SyntaxNodeType::FREE)
	{
		auto p_free = dynamic_cast<FreeSyntaxNode*>(pattern.get());
		ATP_LOGIC_ASSERT(p_free != nullptr);
		auto free_var_iter = substitutions.find(p_free->get_free_id());

		if (free_var_iter == substitutions.end())
		{
			// ideal case; this free variable hasn't been used yet so
			// we can make a substitution as we like.
			substitutions[p_free->get_free_id()] = trial;

			return true;  // always a succes
		}
		else
		{
			// otherwise we have to check that the trial is
			// equivalent to the substitution that has already been
			// made:
			return identical(*trial, *free_var_iter->second);
		}
	}

	// otherwise we simply have to check equality, after making
	// the substitutions:
	ATP_LOGIC_ASSERT(pattern->get_type() == trial->get_type());
	switch (pattern->get_type())
	{
	case SyntaxNodeType::EQ:
	{
		auto pattern_eq = dynamic_cast<const EqSyntaxNode*>(pattern.get());
		auto trial_eq = dynamic_cast<const EqSyntaxNode*>(trial.get());

		ATP_LOGIC_ASSERT(pattern_eq != nullptr);
		ATP_LOGIC_ASSERT(trial_eq != nullptr);

		// build a mapping for both sides
		return build_mapping(pattern_eq->left(),
			trial_eq->left(), substitutions) &&
			build_mapping(pattern_eq->right(),
				trial_eq->right(), substitutions);
	}
	case SyntaxNodeType::CONSTANT:
	{
		auto pattern_const =
			dynamic_cast<const ConstantSyntaxNode*>(pattern.get());
		auto trial_const =
			dynamic_cast<const ConstantSyntaxNode*>(trial.get());

		ATP_LOGIC_ASSERT(pattern_const != nullptr);
		ATP_LOGIC_ASSERT(trial_const != nullptr);

		// just check that they are the same symbol
		return (pattern_const->get_symbol_id() ==
			trial_const->get_symbol_id());
	}
	case SyntaxNodeType::FUNC:
	{
		auto pattern_func = dynamic_cast<const FuncSyntaxNode*>(
			pattern.get());
		auto trial_func = dynamic_cast<const FuncSyntaxNode*>(
			trial.get());

		ATP_LOGIC_ASSERT(pattern_func != nullptr);
		ATP_LOGIC_ASSERT(trial_func != nullptr);

		// just check that they are the same symbol and that all
		// their children also match:

		if (pattern_func->get_symbol_id() !=
			trial_func->get_symbol_id())
			return false;

		if (pattern_func->get_arity() != trial_func->get_arity())
			return false;

		ATP_LOGIC_ASSERT(std::distance(pattern_func->begin(),
			pattern_func->end()) == std::distance(trial_func->begin(),
				trial_func->end()));

		auto begin = boost::make_zip_iterator(boost::make_tuple(
			pattern_func->begin(), trial_func->begin()));
		auto end = boost::make_zip_iterator(boost::make_tuple(
			pattern_func->end(), trial_func->end()));

		for (auto iter = begin; iter != end; iter++)
		{
			if (!build_mapping(
				iter->get<0>(),
				iter->get<1>(),
				substitutions
			))
				return false;  // children don't agree
		}

		// all checks passed
		return true;
	}
#ifdef ATP_LOGIC_DEFENSIVE
	default:
		ATP_LOGIC_ASSERT(false && "invalid syntax node type.");
		return false;
#endif
	}
}


// get all of the free variable IDs in the tree rooted at this node
std::set<size_t> get_free_var_ids(SyntaxNodePtr p_node)
{
	std::list<SyntaxNodePtr> stack;
	std::set<size_t> var_ids;

	stack.push_back(p_node);

	while (!stack.empty())
	{
		p_node = stack.back();
		stack.pop_back();

		apply_to_syntax_node(
			[&stack](EqSyntaxNode& node) -> void
			{
				stack.push_back(node.left());
				stack.push_back(node.right());
			},
			[&var_ids](FreeSyntaxNode& node) -> void
			{
				var_ids.insert(node.get_free_id());
			},
			[](ConstantSyntaxNode&) -> void {},
			[&stack](FuncSyntaxNode& node) -> void
			{
				stack.insert(stack.end(), node.begin(), node.end());
			},
			*p_node
		);
	}
}


}  // namespace eq_matching
}  // namespace logic
}  // namespace atp


