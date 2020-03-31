#include "Statement.h"
#include "Matching.h"
#include "SyntaxTreeFold.h"
#include "KnowledgeKernel.h"
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/iterator/zip_iterator.hpp>
#include <boost/range.hpp>
#include <boost/bind.hpp>
#include <boost/phoenix.hpp>


namespace atp
{
namespace logic
{
namespace equational
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
std::shared_ptr<std::vector<Statement>> from_trees(
	const KnowledgeKernel& ker,
	std::vector<SyntaxNodePtr> trees
);


Statement::Statement(
	const KnowledgeKernel& ker,
	SyntaxNodePtr p_root) :
	m_ker(ker), m_root(p_root),
	m_num_free_vars(syntax_matching::num_free_vars(p_root))
{
	ATP_LOGIC_PRECOND(m_root != nullptr);
	ATP_LOGIC_PRECOND(
		!syntax_matching::needs_free_var_id_rebuild(m_root));
	ATP_LOGIC_PRECOND(m_root->get_type() == SyntaxNodeType::EQ);

	auto p_eq = dynamic_cast<EqSyntaxNode*>(m_root.get());
	ATP_LOGIC_ASSERT(p_eq != nullptr);
	m_left = p_eq->left();
	m_right = p_eq->right();
}


Statement::Statement(const Statement& other) :
	m_ker(other.m_ker),
	m_root(other.m_root),
	m_left(other.m_left),
	m_right(other.m_right),
	m_num_free_vars(other.m_num_free_vars)
{ }


Statement::Statement(Statement&& other) noexcept :
	m_ker(other.m_ker),
	m_root(std::move(other.m_root)),
	m_left(std::move(other.m_left)),
	m_right(std::move(other.m_right)),
	m_num_free_vars(other.m_num_free_vars)
{ }


Statement& Statement::operator=(const Statement& other)
{
	if (this != &other)
	{
		ATP_LOGIC_PRECOND(&m_ker == &other.m_ker);
		m_root = other.m_root;
		m_left = other.m_left;
		m_right = other.m_right;
		m_num_free_vars = other.m_num_free_vars;
	}
	return *this;
}


StmtForm Statement::form() const
{
	if (syntax_matching::trivially_true(*m_root))
	{
		return StmtForm::CANONICAL_TRUE;
	}
	else return StmtForm::NOT_CANONICAL;

	// equational statements cannot be canonically false
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
		std::list<std::string>::iterator begin,
		std::list<std::string>::iterator end) -> std::string
	{
		return m_ker.symbol_name(symb_id) +
			boost::algorithm::join(
				boost::make_iterator_range(begin, end), ", ");
	};

	return fold_syntax_tree<std::string>(eq_fold, free_var_fold,
		const_fold, func_fold, m_root);
}


std::shared_ptr<std::vector<Statement>>
Statement::get_substitutions(
	const std::vector<Statement>& rules) const
{
	// get both sides of the equation:
	SyntaxNodePtr expr_sides[2] = { m_left, m_right };

	// here we will store the resulting trees
	// we will have at most 4 * num_variables * num_rules
	// substitutions:
	std::vector<SyntaxNodePtr> trees;
	trees.reserve(4 * m_num_free_vars * rules.size());

	// now examine rules:
	for (const auto& r : rules)
	{
		SyntaxNodePtr rule_sides[2] = { r.m_left, r.m_right };

		for (size_t j = 0; j < 4; j++)
		{
			auto maybe_substitution = syntax_matching::try_match(
				rule_sides[j % 2], expr_sides[j / 2]
			);

			if (maybe_substitution.has_value())
			{
				auto sub_result = syntax_matching::get_substitution(
					m_root, maybe_substitution.get()
				);
				trees.push_back(sub_result);
			}
		}
	}

	return from_trees(m_ker, trees);
}


std::shared_ptr<std::vector<Statement>>
Statement::replace_free_with_def(
	const std::map<size_t, size_t>& symb_id_to_arity) const
{
	// now apply the fold!
	// what this does is return a list of syntax trees which
	// represent all of the possible ways one could substitute
	// each free variable with a constant:
	auto trees = fold_syntax_tree<std::vector<SyntaxNodePtr>>(
		&fold_eq_constructor,
		boost::bind(&free_var_def_constructor,
			boost::ref(symb_id_to_arity),
			m_num_free_vars, _1),
		boost::bind(&fold_const_constructor,
			m_num_free_vars * symb_id_to_arity.size(), _1),
		&fold_func_constructor, m_root);

	return from_trees(m_ker, trees);
}


std::shared_ptr<std::vector<Statement>>
Statement::replace_free_with_free() const
{
	// now apply the fold!
	// what this does is return a list of syntax trees which
	// represent all of the possible ways one could substitute
	// each free variable with another free variable (considering
	// unordered distinct pairs only):
	auto trees = fold_syntax_tree<std::vector<SyntaxNodePtr>>(
		&fold_eq_constructor,
		boost::bind(&free_var_free_constructor, m_num_free_vars, _1),
		boost::bind(&fold_const_constructor,
			m_num_free_vars * (m_num_free_vars - 1) / 2, _1),
		&fold_func_constructor, m_root);

	return from_trees(m_ker, trees);
}


bool Statement::follows_from(const Statement& premise) const
{
	SyntaxNodePtr exprs[4] = {
		premise.m_left,
		premise.m_right,
		m_left,
		m_right
	};

	// now check that either side of the conclusion follows from
	// either side of the premise:
	for (size_t j = 0; j < 4; j++)
	{
		auto subs = syntax_matching::try_match(exprs[j / 2],
			exprs[2 + j % 2]);

		if (subs.has_value())
		{
			// we have found a satisfying substitution for the
			// premise! Thus the conclusion DOES follow from the
			// premise...
			return true;
		}
	}

	return false;
}


bool Statement::type_check(
	const KnowledgeKernel& alternative_ker) const
{
	// we will use a fold to check validity!

	// eq is valid iff both its sides are valid:
	auto eq_valid = boost::phoenix::arg_names::arg1
		&& boost::phoenix::arg_names::arg2;

	// can't get a free variable wrong:
	auto free_valid = boost::phoenix::val(true);

	std::function<bool(size_t)> const_valid =
		[&alternative_ker](size_t symb_id) -> bool
	{
		return alternative_ker.id_is_defined(symb_id)
			&& alternative_ker.symbol_arity_from_id(
			symb_id) == 0;
	};

	std::function<bool(size_t, std::list<bool>::iterator,
		std::list<bool>::iterator)> func_valid =
		[&alternative_ker](size_t symb_id,
			std::list<bool>::iterator child_begin,
			std::list<bool>::iterator child_end) -> bool
	{
		const size_t implied_arity = std::distance(child_begin,
			child_end);

		return alternative_ker.id_is_defined(symb_id)
			&& alternative_ker.symbol_arity_from_id(
				symb_id) == implied_arity
			&& std::all_of(child_begin, child_end,
				// use phoenix for an easy identity function
				boost::phoenix::arg_names::arg1);
	};

	return fold_syntax_tree<bool>(eq_valid, free_valid,
		const_valid, func_valid, m_root);
}


bool Statement::check_kernel(const IKnowledgeKernel* p_ker) const
{
	return (dynamic_cast<const KnowledgeKernel*>(p_ker) != nullptr
		&& p_ker->get_integrity_code() ==
		m_ker.get_integrity_code());
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
		[symb_id]() { return std::make_shared<ConstantSyntaxNode>(symb_id); });

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
std::shared_ptr<std::vector<Statement>> from_trees(
	const KnowledgeKernel& ker,
	std::vector<SyntaxNodePtr> trees
)
{
#ifdef ATP_LOGIC_DEFENSIVE
	ATP_LOGIC_PRECOND(std::none_of(trees.begin(),
		trees.end(), &syntax_matching::needs_free_var_id_rebuild));
#endif

	std::shared_ptr<std::vector<Statement>> p_stmt_arr =
		std::make_shared<std::vector<Statement>>();

	p_stmt_arr->reserve(trees.size());

	for (size_t j = 0; j < trees.size(); j++)
	{
		p_stmt_arr->emplace_back(ker, trees[j]);
	}

	return p_stmt_arr;
}


}  // namespace equational
}  // namespace logic
}  // namespace atp


