#include "EquationalKnowledgeKernel.h"
#include "EquationalMatching.h"
#include "EquationalStatementArray.h"
#include "EquationalSyntaxTreeFold.h"
#include <boost/functional/hash.hpp>
#include <boost/iterator/zip_iterator.hpp>
#include <boost/bind.hpp>
#include <boost/mpl/identity.hpp>
#include <functional>


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
	StatementArrayPtr _p_stmts) const
{
	ATP_LOGIC_PRECOND(valid(_p_stmts));  // expensive :(

	// The successor statements in equational logic are:
	// - Replacing either side of an equation with one of the given
	//   equality rules (just transitivity in disguise; if x=y is
	//   our statement, and we have a rule of the form y=z, then we
	//   can turn the statement into x=z.)
	// - Replacing a free variable with any of: a user-defined
	//   constant, a user-defined function whose arguments are new
	//   free variables, or a free variable which already exists
	//   within the statement.
	// For example, with the latter rule, if our statement is
	// "f(x)=g(x)" then we can obtain:
	// "f(e)=g(e)" for user defined "e", and "f(h(x))=g(h(x))" for
	// user defined "h", and instead if our statement is
	// "f(x, y)=g(h(x), y)" then "f(x, x)=g(h(x), x)".

	auto p_stmts = dynamic_cast<EquationalStatementArray*>(
		_p_stmts.get());

	auto results1 = replace_free_with_def(*p_stmts);
	auto results2 = replace_free_with_free(*p_stmts);
	auto results3 = make_substitutions(*p_stmts);

	ATP_LOGIC_ASSERT(results1.size() == results2.size());
	ATP_LOGIC_ASSERT(results1.size() == results3.size());
	ATP_LOGIC_ASSERT(results1.size() == p_stmts->size());

	std::vector<StatementArrayPtr> results;
	results.reserve(results1.size());

	// now we want to zip together the arrays results1 and results2
	// by concatenating their elements pairwise; so results[i] ==
	// concat(results1[i], results2[i])

	std::transform(boost::make_zip_iterator(
		boost::make_tuple(results1.begin(), results2.begin(),
			results3.begin())
	), boost::make_zip_iterator(
		boost::make_tuple(results1.end(), results2.end(),
			results3.end())
	), std::back_inserter(results),

		[](auto iter_pair)
		{
			// concatenate!
			auto result = EquationalStatementArray::try_concat(
				iter_pair->get<0>(), iter_pair->get<1>()
			);

			// this should work as no typing issues
			ATP_LOGIC_ASSERT(result != nullptr);

			// concatenate again!
			result = EquationalStatementArray::try_concat(
				*result, iter_pair->get<2>()
			);

			// this should work as no typing issues
			ATP_LOGIC_ASSERT(result != nullptr);

			// now create a pointer to an EquationalStatementArray
			// object
			return std::make_shared<EquationalStatementArray>(
				result);
		});

	return results;
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
		auto id_iter = m_id_to_name.left.find(symb_id);

		if (id_iter == m_id_to_name.left.end())
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
		auto id_iter = m_id_to_name.left.find(symb_id);

		if (id_iter == m_id_to_name.left.end())
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
			boost::mpl::identity<bool>());
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
	StatementArrayPtr _p_premise, StatementArrayPtr _p_concl) const
{
	ATP_LOGIC_PRECOND(_p_premise->size() == _p_concl->size());

#ifdef ATP_LOGIC_DEFENSIVE
	// these are quite expensive checks:
	ATP_LOGIC_PRECOND(valid(_p_premise));
	ATP_LOGIC_PRECOND(valid(_p_concl));
#endif

	// check that, at each step, the left or right hand side of the
	// premise matches the left or right hand side of the conclusion
	// (using the premise as the pattern and the conclusion as the
	// trial). I.e. there will be four checks per element per
	// equality rule.

	auto p_premise = dynamic_cast<EquationalStatementArray*>(
		_p_premise.get());
	auto p_concl = dynamic_cast<EquationalStatementArray*>(
		_p_concl.get());

	ATP_LOGIC_ASSERT(p_premise != nullptr);
	ATP_LOGIC_ASSERT(p_concl != nullptr);

	const auto& arr_premise = p_premise->raw();
	const auto& arr_concl = p_concl->raw();

	const size_t n = arr_premise.size();
	ATP_LOGIC_ASSERT(arr_concl.size() == n);

	// here is where we will store success/failure for each (premise,
	// conclusion) pair; it will start as false, and we will set it
	// to true if we find a satisfying substitution in the premise:
	std::vector<bool> follows_result(n, false);
	ATP_LOGIC_ASSERT(follows_result.size() == n);

	// for each (premise, conclusion) pair in the two arrays...
	for (size_t i = 0; i < n; i++)
	{
		// obtain the LHS and RHS of each equality sign

		auto p_prem_root = dynamic_cast<const EqSyntaxNode*>(
			arr_premise[i].root().get());
		auto p_concl_root = dynamic_cast<const EqSyntaxNode*>(
			arr_concl[i].root().get());

		ATP_LOGIC_ASSERT(p_prem_root != nullptr);
		ATP_LOGIC_ASSERT(p_concl_root != nullptr);

		SyntaxNodePtr exprs[4] = {
			p_prem_root->left(),
			p_prem_root->right(),
			p_concl_root->left(),
			p_concl_root->right()
		};

		// now check that either side of the conclusion follows from
		// either side of the premise:
		for (size_t j = 0; j < 4; j++)
		{
			auto subs = eq_matching::try_match(exprs[j / 2],
				exprs[2 + j % 2]);

			if (subs.has_value())
			{
				// we have found a satisfying substitution for the
				// premise! Thus the conclusion DOES follow from the
				// premise...
				follows_result[i] = true;
				break;
			}
		}
	}

	return follows_result;
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


std::vector<EquationalStatementArray>
EquationalKnowledgeKernel::replace_free_with_def(
	const EquationalStatementArray& arr) const
{
	// get the symbol ID of every user-defined constant or function
	std::list<size_t> symb_ids;
	std::transform(m_id_to_name.left.begin(), m_id_to_name.left.end(),
		std::back_inserter(symb_ids),
		[](std::pair<size_t, std::string> a) { return a.first; });

	// this is the constructor for the FreeSyntaxNodes which we will
	// pass to the fold function
	std::function<std::vector<SyntaxNodePtr>(size_t, size_t)>
		fold_free_constructor = [&symb_ids, this]
		(size_t num_free_vars, size_t my_var_id)
		-> std::vector<SyntaxNodePtr>
	{
		// create results for each symbol and for each free variable
		// (i.e. substitution candidate.)
		std::vector<SyntaxNodePtr> result;
		result.reserve(symb_ids.size() * num_free_vars);

		for (auto symb_id : symb_ids)
		{
			for (size_t i = 0; i < num_free_vars; i++)
			{
				// if it is my turn to be substituted...
				if (i == my_var_id)
				{
					// get the arity so we can determine whether it's
					// a function or a constant:

					ATP_LOGIC_ASSERT(m_id_to_name.left.find(symb_id)
						!= m_id_to_name.left.end());
					const auto name = m_id_to_name.left.at(symb_id);
					ATP_LOGIC_ASSERT(m_symb_arity.find(name)
						!= m_symb_arity.end());
					const auto arity = m_symb_arity.at(name);

					if (arity == 0)
					{
						// it's a constant:
						result.push_back(
							std::make_shared<ConstantSyntaxNode>(
								symb_id)
						);
					}
					else
					{
						// it's a function, so create its arguments:
						std::list<SyntaxNodePtr> func_args;
						for (size_t i = 0; i < arity; i++)
						{
							func_args.push_back(
								std::make_shared<FreeSyntaxNode>(
									num_free_vars + i)
							);
						}

						// now create the function node:
						result.push_back(
							std::make_shared<FuncSyntaxNode>(symb_id,
								func_args)
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
	};

	std::vector<EquationalStatementArray> results;
	results.reserve(arr.size());

	for (size_t i = 0; i < arr.size(); i++)
	{
		auto p_root = arr.raw()[i].root();

		// compute the number of free variables in this statement
		// which will then be bound to the fold_free_constructor
		// below.
		const size_t num_free_vars =
			eq_matching::num_free_vars(p_root);

		// NOTE: we are assuming that the free var IDs are contiguous
		ATP_LOGIC_PRECOND(!eq_matching::needs_free_var_id_rebuild(
			p_root));

		// now apply the fold!
		// what this does is return a list of syntax trees which
		// represent all of the possible ways one could substitute
		// each free variable with a constant:
		auto trees = fold_syntax_tree<std::vector<SyntaxNodePtr>>(
			&fold_eq_constructor,
			boost::bind(fold_free_constructor, num_free_vars, _1),
			&fold_const_constructor, &fold_func_constructor,
			p_root);

		// now we want to turn this array of trees into an array of
		// EquationalStatements, which then needs to be turned into
		// an EquationalStatementArray:

		EquationalStatementArray::ArrPtr p_stmt_arr =
			std::make_shared<EquationalStatementArray::ArrType>();
		p_stmt_arr->reserve(trees.size());

		for (size_t j = 0; j < trees.size(); j++)
		{
			p_stmt_arr->emplace_back(trees[i]);
		}

		results.emplace_back(p_stmt_arr);
	}

	return results;
}


std::vector<EquationalStatementArray>
EquationalKnowledgeKernel::replace_free_with_free(
	const EquationalStatementArray& arr) const
{
	// this is the constructor for the FreeSyntaxNodes which we will
	// pass to the fold function
	std::function<std::vector<SyntaxNodePtr>(size_t, size_t)>
		fold_free_constructor = []
		(size_t num_free_vars, size_t my_var_id)
		-> std::vector<SyntaxNodePtr>
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
	};

	std::vector<EquationalStatementArray> results;
	results.reserve(arr.size());

	for (size_t i = 0; i < arr.size(); i++)
	{
		auto p_root = arr.raw()[i].root();

		// compute the number of free variables in this statement
		// which will then be bound to the fold_free_constructor
		// below.
		const size_t num_free_vars =
			eq_matching::num_free_vars(p_root);

		// NOTE: we are assuming that the free var IDs are contiguous
		ATP_LOGIC_PRECOND(!eq_matching::needs_free_var_id_rebuild(
			p_root));

		// now apply the fold!
		// what this does is return a list of syntax trees which
		// represent all of the possible ways one could substitute
		// each free variable with a constant:
		auto trees = fold_syntax_tree<std::vector<SyntaxNodePtr>>(
			&fold_eq_constructor,
			boost::bind(fold_free_constructor, num_free_vars, _1),
			&fold_const_constructor, &fold_func_constructor,
			p_root);

		// now we want to turn this array of trees into an array of
		// EquationalStatements, which then needs to be turned into
		// an EquationalStatementArray:

		EquationalStatementArray::ArrPtr p_stmt_arr =
			std::make_shared<EquationalStatementArray::ArrType>();
		p_stmt_arr->reserve(trees.size());

		for (size_t j = 0; j < trees.size(); j++)
		{
			p_stmt_arr->emplace_back(trees[i]);
		}

		results.emplace_back(p_stmt_arr);
	}

	return results;
}


std::vector<EquationalStatementArray>
EquationalKnowledgeKernel::make_substitutions(
	const EquationalStatementArray& arr) const
{
	std::vector<EquationalStatementArray> results;
	results.reserve(arr.size());

	for (size_t i = 0; i < arr.size(); i++)
	{
		auto p_root = arr.raw()[i].root();
		SyntaxNodePtr expr_sides[2] = { nullptr, nullptr };

		// get both sides of the equation:
		ATP_LOGIC_PRECOND(p_root->get_type() == SyntaxNodeType::EQ);
		auto p_eq = dynamic_cast<EqSyntaxNode*>(p_root.get());
		ATP_LOGIC_ASSERT(p_eq != nullptr);
		expr_sides[0] = p_eq->left();
		expr_sides[1] = p_eq->right();

		// here we will store the resulting trees
		// we will have at most 4 * num_variables * num_rules
		// substitutions:
		std::vector<SyntaxNodePtr> trees;
		trees.reserve(4 * eq_matching::num_free_vars(p_root)
			* m_rules.size());

		// now examine rules:
		for (auto r : m_rules)
		{
			SyntaxNodePtr rule_sides[2] = { r.first, r.second };

			for (size_t j = 0; j < 4; j++)
			{
				auto maybe_substitution = eq_matching::try_match(
					rule_sides[j % 2], expr_sides[j / 2]
				);

				if (maybe_substitution.has_value())
				{
					auto sub_result = eq_matching::get_substitution(
						p_root, maybe_substitution.get()
					);
					trees.push_back(sub_result);
				}
			}
		}

		// now we want to turn this array of trees into an array of
		// EquationalStatements, which then needs to be turned into
		// an EquationalStatementArray:

		EquationalStatementArray::ArrPtr p_stmt_arr =
			std::make_shared<EquationalStatementArray::ArrType>();
		p_stmt_arr->reserve(trees.size());

		for (size_t j = 0; j < trees.size(); j++)
		{
			p_stmt_arr->emplace_back(trees[i]);
		}

		results.emplace_back(p_stmt_arr);
	}

	return results;
}


std::vector<SyntaxNodePtr>
EquationalKnowledgeKernel::fold_eq_constructor(
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
		[](boost::tuple<SyntaxNodePtr, SyntaxNodePtr>& tup)
		{ return std::make_shared<EqSyntaxNode>(tup.get<0>(), tup.get<1>()); });

	return result;
}


std::vector<SyntaxNodePtr>
EquationalKnowledgeKernel::fold_const_constructor(size_t N,
	size_t symb_id)
{
	std::vector<SyntaxNodePtr> result;

	std::generate_n(result.end(), N, boost::bind(
		std::make_shared<ConstantSyntaxNode>, symb_id));

	return result;
}


std::vector<SyntaxNodePtr>
EquationalKnowledgeKernel::fold_func_constructor(size_t symb_id,
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
		for (auto iter = children_begin; iter != children_end; iter++)
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


}  // namespace logic
}  // namespace atp


