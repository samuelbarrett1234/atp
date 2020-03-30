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


// TODO: do better than a recursion!


// check if p_a and p_b are equivalent (return true iff they are) and
// keep track of which free variables have been paired
bool equivalent_from_mapping(const ISyntaxNode* p_a,
	const ISyntaxNode* p_b, FreeIdMatching& free_id_map);


// check if the trial can be created from p_a by making substitutions
// for the pattern's free variables, to create a tree identical to
// the trial's. We also have to keep track of substitutions made.
bool build_mapping(SyntaxNodePtr pattern, SyntaxNodePtr trial,
	FreeVarSubstitution& substitutions);


boost::optional<FreeVarSubstitution> try_match(SyntaxNodePtr pattern,
	SyntaxNodePtr trial)
{
	FreeVarSubstitution subs;

	if (build_mapping(pattern, trial, subs))
	{
		return subs;
	}
	else return boost::optional<FreeVarSubstitution>();
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


}
}  // namespace logic
}  // namespace atp


