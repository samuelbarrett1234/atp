#include "Semantics.h"
#include "SyntaxTreeFold.h"
#include "SyntaxTreeTraversal.h"
#include <boost/iterator/zip_iterator.hpp>
#include <boost/bimap.hpp>
#include <boost/phoenix.hpp>
#include <set>


namespace phx = boost::phoenix;
namespace phxarg = phx::arg_names;


namespace atp
{
namespace logic
{
namespace equational
{
namespace semantics
{


// the free variable constructor for substituting it with definitions
// for use in a fold
std::vector<SyntaxNodePtr> free_var_def_constructor(
	const std::map<size_t, size_t>& symb_id_to_arity,
	size_t num_free_vars, size_t my_var_id
);

// the free variable constructor for substituting it with other free
// variables, for use in a fold
std::vector<SyntaxNodePtr> free_var_free_constructor(
	size_t num_free_vars, size_t my_var_id
);

// constructor method for equality nodes
std::vector<SyntaxNodePtr> fold_eq_constructor(
	const std::vector<SyntaxNodePtr>& lhss,
	const std::vector<SyntaxNodePtr>& rhss);

// constructor method for constant nodes
std::vector<SyntaxNodePtr>
fold_const_constructor(size_t N,
	size_t symb_id);

// constructor method for function nodes
std::vector<SyntaxNodePtr>
fold_func_constructor(size_t symb_id,
	const std::list<std::vector<SyntaxNodePtr>>::iterator&
	children_begin,
	const std::list<std::vector<SyntaxNodePtr>>::iterator&
	children_end);

// helper function for converting fold results;
// precond: none of the trees need their free variable IDs rebuilding
StatementArray from_trees(
	const KnowledgeKernel& ker,
	std::vector<SyntaxNodePtr> trees
);


typedef boost::optional<std::map<size_t, SyntaxNodePtr>>
OptionalFreeVarSubstitution;

// try to obtain a free variable mapping which, when substituted
// into the LHS of the pattern, gives the LHS of the trial (we
// completely ignore the RHS of both the pattern and the trial!)
OptionalFreeVarSubstitution try_build_lhs_mapping(
	const Statement& pattern, const Statement& trial);


SyntaxNodePtr get_substitution(const Statement& pattern,
	std::map<size_t, SyntaxNodePtr> sub, size_t max_id_in_trial);


bool syntax_tree_identical(SyntaxNodePtr a, SyntaxNodePtr b);


bool equivalent(const Statement& a, const Statement& b)
{
	auto eq_func = phxarg::arg1 &&
		phxarg::arg2;

	// build up a bijection between IDs as we go
	boost::bimap<size_t, size_t> id_map;

	auto free_func = [&id_map](size_t id1, size_t id2)
	{
		auto left_iter = id_map.left.find(id1);
		auto right_iter = id_map.left.find(id2);

		if (left_iter == id_map.left.end() &&
			right_iter == id_map.left.end())
		{
			id_map.left.insert(std::make_pair(id1, id2));
			return true;
		}
		else if (left_iter != id_map.left.end())
		{
			return left_iter->second == id2;
		}
		else
		{
			return right_iter->second == id1;
		}
	};

	auto const_func = (phxarg::arg1 == phxarg::arg2);

	auto f_func = [](size_t id1, size_t id2,
		std::list<bool>::iterator begin,
		std::list<bool>::iterator end)
	{
		return id1 == id2 && std::all_of(begin, end,
			phxarg::arg1);
	};

	auto default_func = phx::val(false);

	return a.fold_pair<bool>(eq_func, free_func, const_func,
		f_func, default_func, b) ||
	a.fold_pair<bool>(eq_func, free_func, const_func,
		f_func, default_func, transpose(b));
}


bool identical(const Statement& a, const Statement& b)
{
	auto eq_func = phxarg::arg1 &&
		phxarg::arg2;

	auto free_func = (phxarg::arg1 == phxarg::arg2);

	auto const_func = (phxarg::arg1 == phxarg::arg2);

	auto f_func = [](size_t id1, size_t id2,
		std::list<bool>::iterator begin,
		std::list<bool>::iterator end)
	{
		return id1 == id2 && std::all_of(begin, end,
			phxarg::arg1);
	};

	auto default_func = phx::val(false);

	return a.fold_pair<bool>(eq_func, free_func, const_func,
		f_func, default_func, b);
}


Statement transpose(const Statement& stmt)
{
	// reflect the statement about the equals sign

	auto eq_func = [](SyntaxNodePtr lhs, SyntaxNodePtr rhs)
	{ return std::make_shared<EqSyntaxNode>(rhs, lhs); };

	auto free_func = boost::bind(
		&std::make_shared<FreeSyntaxNode, size_t>, _1);
	
	auto const_func = boost::bind(
		&std::make_shared<ConstantSyntaxNode, size_t>, _1);

	auto f_func = boost::bind(
		&std::make_shared<FuncSyntaxNode, size_t,
		std::list<SyntaxNodePtr>::iterator,
		std::list<SyntaxNodePtr>::iterator>, _1, _2, _3);

	auto syntax_tree = stmt.fold<SyntaxNodePtr>(eq_func,
		free_func, const_func, f_func);

	return Statement(stmt.kernel(), syntax_tree);
}


bool true_by_reflexivity(const Statement& stmt)
{
	return identical(stmt, transpose(stmt));
}


OptionalFreeVarSubstitution try_build_lhs_mapping(
	const Statement& pattern, const Statement& trial)
{
	auto eq_func = [](OptionalFreeVarSubstitution lhs,
		OptionalFreeVarSubstitution rhs)
	{
		return lhs;
	};

	auto free_func = [](size_t id_pattern, size_t id_trial)
		-> OptionalFreeVarSubstitution
	{
		std::map<size_t, SyntaxNodePtr> subs;
		subs[id_pattern] =
			std::make_shared<FreeSyntaxNode>(id_trial);
		return subs;
	};

	auto const_func = [](size_t symb_pattern, size_t symb_trial)
		-> OptionalFreeVarSubstitution
	{
		if (symb_pattern == symb_trial)
			return std::map<size_t, SyntaxNodePtr>();
		else
			return boost::none;
	};

	auto f_func = [](size_t symb_pattern, size_t symb_trial,
		std::list<OptionalFreeVarSubstitution>::iterator begin,
		std::list<OptionalFreeVarSubstitution>::iterator end)
		-> OptionalFreeVarSubstitution
	{
		// if the function names don't agree, it's impossible
		if (symb_pattern != symb_trial)
			return boost::none;

		// if any of them don't have a value, it's impossible
		if (std::any_of(begin, end,
			!boost::bind(&OptionalFreeVarSubstitution::has_value, _1)))
			return boost::none;

		// else try to combine the substitutions
		std::map<size_t, SyntaxNodePtr> sub;
		for (auto iter = begin; iter != end; ++iter)
		{
			ATP_LOGIC_ASSERT(iter->has_value());

			for (auto sub_iter = iter->get().begin();
				sub_iter != iter->get().end();
				++sub_iter)
			{
				// we are trying to make a substitution for the
				// variable with ID sub_iter->first

				auto sub_search = sub.find(sub_iter->first);

				if (sub_search != sub.end())
				{
					// if we already have a substitution for that free
					// variable we need to check that it is identical
					// to the one we are about to suggest!

					if (!syntax_tree_identical(sub_search->second,
						sub_iter->second))
					{
						// if they don't agree, it's impossible
						return boost::none;
					}
					// else do nothing
				}
				else
				{
					// else this is a new free variable so we are free
					// to make this substitution
					sub.insert(*sub_iter);
				}
			}
		}

		return sub;
	};

	auto default_func = [](SyntaxNodePtr p_pattern, SyntaxNodePtr p_trial)
		-> OptionalFreeVarSubstitution
	{
		if (p_pattern->get_type() != SyntaxNodeType::FREE)
			return boost::none;

		const FreeSyntaxNode* p_free = dynamic_cast<
			const FreeSyntaxNode*>(p_pattern.get());

		ATP_LOGIC_ASSERT(p_free != nullptr);

		std::map<size_t, SyntaxNodePtr> subs;
		subs[p_free->get_free_id()] = p_trial;
		return subs;
	};

	return pattern.fold_pair<OptionalFreeVarSubstitution>(eq_func,
		free_func, const_func, f_func, default_func, trial);
}


SyntaxNodePtr get_substitution(const Statement& pattern,
	std::map<size_t, SyntaxNodePtr> sub, size_t max_id_in_trial)
{
	auto eq_func = boost::bind(
		&std::make_shared<EqSyntaxNode, SyntaxNodePtr, SyntaxNodePtr>,
		_1, _2);

	auto free_func = [&sub, max_id_in_trial](size_t id)
		-> SyntaxNodePtr
	{
		auto iter = sub.find(id);

		if (iter != sub.end())
			return iter->second;  // return the substitution

		// else just return the free variable, but we need to offset
		// its ID to prevent the remaining free variables from clashing
		// with the IDs we are substituting for
		return std::make_shared<FreeSyntaxNode>(
			id + max_id_in_trial + 1);
	};

	auto const_func = boost::bind(
		&std::make_shared<ConstantSyntaxNode, size_t>, _1);

	auto f_func = boost::bind(
		&std::make_shared<FuncSyntaxNode, size_t,
		std::list<SyntaxNodePtr>::iterator,
		std::list<SyntaxNodePtr>::iterator>, _1, _2, _3);

	return pattern.fold<SyntaxNodePtr>(eq_func, free_func,
		const_func, f_func);
}


StatementArray get_substitutions(const Statement& stmt,
	const std::vector<Statement>& rules)
{
	// here we will store the resulting trees
	// we will have at most 4 * num_variables * num_rules
	// substitutions:
	std::vector<SyntaxNodePtr> trees;
	trees.reserve(4 * stmt.num_free_vars() * rules.size());

	auto stmt_T = transpose(stmt);

	// we also need to compute the largest free variable
	// ID in the statement:

	auto eq_func = boost::bind(&std::max<size_t>, _1, _2);
	auto free_func = phxarg::arg1;
	auto const_func = phx::val(0);  // smallest possible value
	auto f_func = [](size_t _,
		std::list<size_t>::iterator begin,
		std::list<size_t>::iterator end) -> size_t
	{
		auto mx = std::max_element(begin, end);
		ATP_LOGIC_ASSERT(mx != end);
		return *mx;
	};
	const size_t max_id = stmt.fold<size_t>(eq_func,
		free_func, const_func, f_func);

	// now examine rules:
	for (const auto& r : rules)
	{
		auto r_T = transpose(r);

		auto sub1 = try_build_lhs_mapping(r, stmt);
		auto sub2 = try_build_lhs_mapping(r_T, stmt);
		auto sub3 = try_build_lhs_mapping(r, stmt_T);
		auto sub4 = try_build_lhs_mapping(r_T, stmt_T);

		if (sub1.has_value())
		{
			trees.emplace_back(
				get_substitution(r, sub1.get(), max_id));
		}
		if (sub2.has_value())
		{
			trees.emplace_back(
				get_substitution(r, sub2.get(), max_id));
		}
		if (sub3.has_value())
		{
			trees.emplace_back(
				get_substitution(r, sub3.get(), max_id));
		}
		if (sub4.has_value())
		{
			trees.emplace_back(
				get_substitution(r, sub4.get(), max_id));
		}
	}

	return from_trees(stmt.kernel(), trees);
}


StatementArray replace_free_with_def(const Statement& stmt,
	const std::map<size_t, size_t>& symb_id_to_arity)
{
	// now apply the fold!
	// what this does is return a list of syntax trees which
	// represent all of the possible ways one could substitute
	// each free variable with a constant:
	auto trees = stmt.fold<std::vector<SyntaxNodePtr>>(
		&fold_eq_constructor,
		boost::bind(&free_var_def_constructor,
			boost::ref(symb_id_to_arity),
			stmt.num_free_vars(), _1),
		boost::bind(&fold_const_constructor,
			stmt.num_free_vars() * symb_id_to_arity.size(), _1),
		&fold_func_constructor);

	return from_trees(stmt.kernel(), trees);
}


StatementArray replace_free_with_free(const Statement& stmt)
{
	// now apply the fold!
	// what this does is return a list of syntax trees which
	// represent all of the possible ways one could substitute
	// each free variable with another free variable (considering
	// unordered distinct pairs only):
	auto trees = stmt.fold<std::vector<SyntaxNodePtr>>(
		&fold_eq_constructor,
		boost::bind(&free_var_free_constructor,
			stmt.num_free_vars(), _1),
		boost::bind(&fold_const_constructor,
			stmt.num_free_vars() * (stmt.num_free_vars() - 1) / 2, _1),
		&fold_func_constructor);

	return from_trees(stmt.kernel(), trees);
}


bool follows_from(const Statement& premise,
	const Statement& concl)
{
	auto subs = get_substitutions(concl, { premise });
	
	return std::any_of(subs.begin(), subs.end(),
		boost::bind(&equivalent, boost::ref(concl), _1));
}


bool syntax_tree_identical(SyntaxNodePtr a, SyntaxNodePtr b)
{
	// don't need a fold for this (although we could do it with
	// one; however that would involve writing a fold-pairs function
	// for syntax trees.)

	std::list<std::pair<SyntaxNodePtr, SyntaxNodePtr>> stack;

	stack.push_back(std::make_pair(a, b));

	while (!stack.empty())
	{
		auto pair = stack.back();
		stack.pop_back();

		if (pair.first->get_type() != pair.second->get_type())
			return false;

		switch (pair.first->get_type())
		{
		case SyntaxNodeType::EQ:
		{
			auto p_first = dynamic_cast<EqSyntaxNode*>(
				pair.first.get());
			auto p_second = dynamic_cast<EqSyntaxNode*>(
				pair.second.get());

			ATP_LOGIC_ASSERT(p_first != nullptr);
			ATP_LOGIC_ASSERT(p_second != nullptr);

			stack.emplace_back(std::make_pair(
				p_first->left(), p_second->left()));
			stack.emplace_back(std::make_pair(
				p_first->right(), p_second->right()));
		}
			break;
		case SyntaxNodeType::FREE:
		{
			auto p_first = dynamic_cast<FreeSyntaxNode*>(
				pair.first.get());
			auto p_second = dynamic_cast<FreeSyntaxNode*>(
				pair.second.get());

			ATP_LOGIC_ASSERT(p_first != nullptr);
			ATP_LOGIC_ASSERT(p_second != nullptr);

			if (p_first->get_free_id() != p_second->get_free_id())
				return false;
		}
		break;
		case SyntaxNodeType::CONSTANT:
		{
			auto p_first = dynamic_cast<ConstantSyntaxNode*>(
				pair.first.get());
			auto p_second = dynamic_cast<ConstantSyntaxNode*>(
				pair.second.get());

			ATP_LOGIC_ASSERT(p_first != nullptr);
			ATP_LOGIC_ASSERT(p_second != nullptr);

			if (p_first->get_symbol_id() != p_second->get_symbol_id())
				return false;
		}
		break;
		case SyntaxNodeType::FUNC:
		{
			auto p_first = dynamic_cast<FuncSyntaxNode*>(
				pair.first.get());
			auto p_second = dynamic_cast<FuncSyntaxNode*>(
				pair.second.get());

			ATP_LOGIC_ASSERT(p_first != nullptr);
			ATP_LOGIC_ASSERT(p_second != nullptr);

			if (p_first->get_symbol_id() != p_second->get_symbol_id())
				return false;

			// these statements have been type-checked, thus if two
			// functions agree on their symbols, they must also agree
			// on their arity!
			ATP_LOGIC_ASSERT(p_first->get_arity() ==
				p_second->get_arity());

			// add all pairs of children to the stack
			std::transform(boost::make_zip_iterator(
				boost::make_tuple(p_first->begin(),
					p_second->begin())),

				boost::make_zip_iterator(
					boost::make_tuple(p_first->end(),
						p_second->end())),

				std::back_inserter(stack),

				[](boost::tuple<SyntaxNodePtr, SyntaxNodePtr> tup)
				{ return std::make_pair(tup.get<0>(), tup.get<1>()); });
		}
		break;
		}
	}

	return true;
}


// Implementations of fold constructors:


std::vector<SyntaxNodePtr> free_var_def_constructor(
	const std::map<size_t, size_t>& symb_id_to_arity,
	size_t num_free_vars, size_t my_var_id
)
{
	// create results for each symbol and for each free variable
	// (i.e. substitution candidate.)
	std::vector<SyntaxNodePtr> result;
	result.reserve(symb_id_to_arity.size() * num_free_vars);

	for (auto symb_id_arity : symb_id_to_arity)
	{
		for (size_t i = 0; i < num_free_vars; i++)
		{
			// if it is my turn to be substituted...
			if (i == my_var_id)
			{
				if (symb_id_arity.second == 0)
				{
					// it's a constant:
					result.push_back(
						std::make_shared<ConstantSyntaxNode>(
							symb_id_arity.first)
					);
				}
				else
				{
					// it's a function, so create its arguments:
					std::list<SyntaxNodePtr> func_args;
					for (size_t i = 0; i < symb_id_arity.second; i++)
					{
						func_args.push_back(
							std::make_shared<FreeSyntaxNode>(
								num_free_vars + i)
						);
					}

					// now create the function node:
					result.push_back(
						std::make_shared<FuncSyntaxNode>(
							symb_id_arity.first, func_args.begin(),
							func_args.end())
					);
				}
			}
			else  // not my turn to be substituted
			{
				result.push_back(
					std::make_shared<FreeSyntaxNode>(my_var_id)
				);
			}
		}
	}

	// cull excess memory (remark that, when we called reserve()
	// earlier, we overestimated the number of substitutions)
	result.shrink_to_fit();

	return result;
}


std::vector<SyntaxNodePtr> free_var_free_constructor(
	size_t num_free_vars, size_t my_var_id
)
{
	// create results for each unordered distinct pair of free
	// variable IDs
	std::vector<SyntaxNodePtr> result;

	// no chance of overflow here because n*(n-1)/2 is always an
	// integer:
	result.reserve(num_free_vars * (num_free_vars - 1) / 2);

	for (size_t i = 0; i < num_free_vars; i++)
	{
		for (size_t j = i + 1; j < num_free_vars; j++)
		{
			// if it is my turn to be substituted...
			if (i == my_var_id)
			{
				// replace with j
				result.push_back(
					std::make_shared<FreeSyntaxNode>(j)
				);
			}
			else  // not my turn to be substituted
			{
				result.push_back(
					std::make_shared<FreeSyntaxNode>(my_var_id)
				);
			}
		}
	}

	return result;
}


std::vector<SyntaxNodePtr> fold_eq_constructor(
	const std::vector<SyntaxNodePtr>& lhss,
	const std::vector<SyntaxNodePtr>& rhss)
{
	ATP_LOGIC_PRECOND(lhss.size() == rhss.size());

	auto begin = boost::make_zip_iterator(
		boost::make_tuple(lhss.begin(), rhss.begin())
	);
	auto end = boost::make_zip_iterator(
		boost::make_tuple(lhss.end(), rhss.end())
	);

	std::vector<SyntaxNodePtr> result;
	result.reserve(lhss.size());

	std::transform(begin, end, std::back_inserter(result),
		[](boost::tuple<SyntaxNodePtr, SyntaxNodePtr> tup)
		{ return std::make_shared<EqSyntaxNode>(tup.get<0>(), tup.get<1>()); });

	return result;
}


std::vector<SyntaxNodePtr>
fold_const_constructor(size_t N,
	size_t symb_id)
{
	std::vector<SyntaxNodePtr> result;
	result.reserve(N);

	std::generate_n(result.begin(), N,
		[symb_id]()
		{
			return std::make_shared<ConstantSyntaxNode>(symb_id);
		});

	return result;
}


std::vector<SyntaxNodePtr>
fold_func_constructor(size_t symb_id,
	const std::list<std::vector<SyntaxNodePtr>>::iterator&
	children_begin,
	const std::list<std::vector<SyntaxNodePtr>>::iterator&
	children_end)
{
	ATP_LOGIC_PRECOND(children_begin != children_end);

	auto num_children = std::distance(children_begin, children_end);
	auto num_poss = children_begin->size();

#ifdef ATP_LOGIC_DEFENSIVE
	ATP_LOGIC_PRECOND(std::all_of(children_begin, children_end,
		[num_poss](auto vec) { return vec.size() == num_poss; }));
#endif

	std::vector<SyntaxNodePtr> result;
	result.reserve(num_poss);

	// warning: we basically have to do a transpose on the 2D array
	// given as input! (we want a vector of lists not a list of
	// vectors!)

	for (size_t i = 0; i < num_poss; i++)
	{
		// the children for this possibility
		std::list<SyntaxNodePtr> children_for_poss;
		for (auto iter = children_begin; iter != children_end; ++iter)
		{
			children_for_poss.push_back(iter->at(i));
		}
		result.push_back(std::make_shared<FuncSyntaxNode>(
			symb_id, children_for_poss.begin(),
			children_for_poss.end()
			));
	}

	return result;
}


// helper function for converting fold results:
StatementArray from_trees(const KnowledgeKernel& ker,
	std::vector<SyntaxNodePtr> trees
)
{
	std::shared_ptr<std::vector<Statement>> p_stmt_arr =
		std::make_shared<std::vector<Statement>>();

	p_stmt_arr->reserve(trees.size());

	for (size_t j = 0; j < trees.size(); j++)
	{
		p_stmt_arr->emplace_back(ker, trees[j]);
	}

	return StatementArray(p_stmt_arr);
}


}  // namespace semantics
}  // namespace equational
}  // namespace logic
}  // namespace atp


