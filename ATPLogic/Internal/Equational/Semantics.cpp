#include "Semantics.h"
#include "SyntaxTreeFold.h"
#include "SyntaxTreeTraversal.h"
#include <boost/iterator/zip_iterator.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/bimap.hpp>
#include <boost/phoenix.hpp>
#include <boost/bind.hpp>
#include <set>
#include <vector>
#include <list>
#include <map>


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


bool syntax_tree_identical(SyntaxNodePtr a, SyntaxNodePtr b);


// try to apply each rule at the subtree rooted at the given node
// and return the results
std::list<SyntaxNodePtr> immediate_applications(SyntaxNodePtr node,
	const std::vector<Statement>& rules);


// try to obtain a free variable mapping which makes the premise
// expression be identical to the conclusion expression (note
// the word "expression" here is meant to mean "without an = sign").
boost::optional<std::map<size_t, SyntaxNodePtr>>
try_build_map(SyntaxNodePtr expr_premise, SyntaxNodePtr expr_concl);


// a simple function for applying a substitution to a given tree
SyntaxNodePtr substitute_tree(SyntaxNodePtr node,
	const std::map<size_t, SyntaxNodePtr>& free_var_map);


bool equivalent(const Statement& a, const Statement& b)
{
	auto eq_func = phxarg::arg1 &&
		phxarg::arg2;

	// build up a bijection between IDs as we go
	boost::bimap<size_t, size_t> id_map;

	auto free_func = [&id_map](size_t id1, size_t id2)
	{
		auto left_iter = id_map.left.find(id1);
		auto right_iter = id_map.right.find(id2);

		if (left_iter == id_map.left.end() &&
			right_iter == id_map.right.end())
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


StatementArray get_substitutions(const Statement& stmt,
	const std::vector<Statement>& rules)
{
	// this pair can be thought of as a node saying: "<me, and all of
	// the ways the subtree rooted at me look after substitution>"
	typedef std::pair<SyntaxNodePtr, std::list<SyntaxNodePtr>>
		SubResults;

	auto eq_constructor = [&rules](const SubResults& lhs,
		const SubResults& rhs) -> SubResults
	{
		auto me = std::make_shared<EqSyntaxNode>(lhs.first,
			rhs.first);

		std::list<SyntaxNodePtr> sub_results;

		// add left-hand results, keep RHS constant
		std::transform(lhs.second.begin(), lhs.second.end(),
			std::back_inserter(sub_results),
			[&rhs](SyntaxNodePtr lhs)
			{ return std::make_shared<EqSyntaxNode>(lhs, rhs.first); });

		// add right-hand results, keep LHS constant
		std::transform(rhs.second.begin(), rhs.second.end(),
			std::back_inserter(sub_results),
			[&lhs](SyntaxNodePtr rhs)
			{ return std::make_shared<EqSyntaxNode>(lhs.first, rhs); });

		return std::make_pair(me, sub_results);
	};

	auto free_constructor = [&rules](size_t id) -> SubResults
	{
		auto me = std::make_shared<FreeSyntaxNode>(id);

		return std::make_pair(me, immediate_applications(me, rules));
	};

	auto const_constructor = [&rules](size_t id) -> SubResults
	{
		auto me = std::make_shared<ConstantSyntaxNode>(id);

		return std::make_pair(me, immediate_applications(me, rules));
	};

	auto func_constructor = [&rules](size_t id,
		std::list<SubResults>::iterator begin,
		std::list<SubResults>::iterator end) -> SubResults
	{
		// Get the children, unmodified, i.e. without the
		// substitutions. This is just the first element of
		// the pairs of the given list
		auto map_first = boost::bind(&SubResults::first, _1);
		std::list<SyntaxNodePtr> unmodified_children(
			boost::make_transform_iterator(begin, map_first),
			boost::make_transform_iterator(end, map_first));

		auto me = std::make_shared<FuncSyntaxNode>(id,
			unmodified_children.begin(), unmodified_children.end());

		auto sub_results = immediate_applications(me,
			rules);

		for (auto child_iter = begin; child_iter != end;
			++child_iter)
		{
			auto child_dist = std::distance(begin,
				child_iter);

			auto new_child_list = unmodified_children;

			// replace the child at `child_iter` with the
			// substitution `child_sub` in the list, and then
			// build a function syntax node out of it:

			auto replace_iter = new_child_list.begin();
			std::advance(replace_iter, child_dist);

			// finally, go through each substitution and build a new
			// function node out of it:
			for (auto child_sub = child_iter->second.begin();
				child_sub != child_iter->second.end(); ++child_sub)
			{
				// replace it:
				*replace_iter = *child_sub;

				// now build a function node out of it:
				sub_results.push_back(
					std::make_shared<FuncSyntaxNode>(
					id, new_child_list.begin(),
						new_child_list.end()));
			}
		}

		return std::make_pair(me, sub_results);
	};

	auto fold_results = stmt.fold<SubResults>(eq_constructor,
		free_constructor, const_constructor, func_constructor);

	// now we ignore the first element of the pair, and return
	// the second list but as a statement array:

	StatementArray::ArrPtr p_arr = std::make_shared<
		StatementArray::ArrType>();

	std::transform(fold_results.second.begin(),
		fold_results.second.end(),
		std::back_inserter(*p_arr),
		[&stmt](SyntaxNodePtr p_node)
		{
			return Statement(stmt.kernel(), p_node);
		}
	);

	return StatementArray(p_arr);
}


bool implies(const Statement& premise, const Statement& concl)
{
	typedef boost::optional<
		std::map<size_t, SyntaxNodePtr>> FreeVarMap;

	// return true iff the two free variable mappings don't conflict
	// (i.e. if they both map some free variable "x" to some
	// expression, then those expressions are the same.)
	auto try_union = [](const FreeVarMap& a, const FreeVarMap& b)
		-> FreeVarMap
	{
		if (!a.has_value() || !b.has_value())
			return boost::none;

		// build a new mapping, starting with the contents of 'a',
		// and adding the contents of 'b' checking they don't
		// create a conflict
		auto c = a;
		for (const auto& subst : b.get())
		{
			auto iter = c.get().find(subst.first);

			if (iter == c.get().end())
			{
				// okay; 'a' hasn't mapped this variable
				c.get()[subst.first] = subst.second;
			}
			else
			{
				if (!syntax_tree_identical(iter->second,
					subst.second))
					return boost::none;  // CONFLICT!

				// else do nothing
			}
		}
		return c;
	};

	auto eq_constructor = try_union;

	auto free_constructor = [](size_t id_a, size_t id_b)
	{
		// just map id_a to the free variable with id_b
		FreeVarMap map = FreeVarMap::value_type();
		map.get()[id_a] = std::make_shared<FreeSyntaxNode>(id_b);
		return map;
	};

	auto const_constructor = phx::val(FreeVarMap::value_type());

	auto func_constructor = [&try_union](size_t id1, size_t id2,
		std::list<FreeVarMap>::iterator begin,
		std::list<FreeVarMap>::iterator end)
		-> FreeVarMap
	{
		// if the function names don't agree then we can't save
		// ourselves by a substitution
		if (id1 != id2)
			return boost::none;

		// if any child doesn't have a mapping then it's impossible
		if (std::any_of(begin, end,
			!boost::bind(&FreeVarMap::has_value, _1)))
		{
			return boost::none;
		}

		// return the union of all the mappings, or boost::none if
		// they conflict!
		return std::accumulate(begin, end,
			FreeVarMap(FreeVarMap::value_type()), try_union);
	};

	auto default_constructor = [](SyntaxNodePtr a,
		SyntaxNodePtr b) -> FreeVarMap
	{
		if (a->get_type() != SyntaxNodeType::FREE)
			return boost::none;  // cannot do anything here

		auto p_free = dynamic_cast<FreeSyntaxNode*>(a.get());

		ATP_LOGIC_ASSERT(p_free != nullptr);

		// map the free variable to the expression 'b'!
		FreeVarMap map = FreeVarMap::value_type();
		map.get()[p_free->get_free_id()] = b;
		return map;
	};

	// check premise matches to conclusion, OR
	// premise matches to conclusion TRANSPOSE
	// (this is because `fold_pair` matches LHS to LHS and RHS to
	// RHS).
	return premise.fold_pair<FreeVarMap>(eq_constructor,
		free_constructor, const_constructor, func_constructor,
		default_constructor, concl).has_value()
		|| premise.fold_pair<FreeVarMap>(eq_constructor,
			free_constructor, const_constructor, func_constructor,
			default_constructor, transpose(concl)).has_value();
}


std::list<SyntaxNodePtr> immediate_applications(SyntaxNodePtr node,
	const std::vector<Statement>& rules)
{
	ATP_LOGIC_PRECOND(node->get_type() != SyntaxNodeType::EQ);

	// (hacky) idea: build expression syntax trees for the rules,
	// then match them to the node.

	// for most of the tree, this pair only stores something in its
	// .first and its .second is empty. The only exception is the =
	// nodes, which require both.
	typedef std::pair<SyntaxNodePtr, SyntaxNodePtr> PairSyntaxTree;

	auto eq_constructor = [](PairSyntaxTree a,
		PairSyntaxTree b)
	{
		// combine results into a pair, don't create an eq node
		return std::make_pair(a.first, b.first);
	};

	auto free_constructor = [](size_t id)
	{
		return std::make_pair(
			std::make_shared<FreeSyntaxNode>(id),
			SyntaxNodePtr());
	};

	auto const_constructor = [](size_t id)
	{
		return std::make_pair(
			std::make_shared<ConstantSyntaxNode>(id),
			SyntaxNodePtr());
	};

	auto func_constructor = [](size_t id,
		std::list<PairSyntaxTree>::iterator begin,
		std::list<PairSyntaxTree>::iterator end)
	{
		auto map_first = boost::bind(&PairSyntaxTree::first, _1);
		std::list<SyntaxNodePtr> children(
			boost::make_transform_iterator(begin, map_first),
			boost::make_transform_iterator(end, map_first));

		return std::make_pair(
			std::make_shared<FuncSyntaxNode>(id, children.begin(),
				children.end()), SyntaxNodePtr());
	};

	// convert all the rules to expressions representing their LHS
	// and RHS respectively
	std::vector<PairSyntaxTree> rule_exprs;
	rule_exprs.reserve(rules.size());

	std::transform(rules.begin(), rules.end(),
		std::back_inserter(rule_exprs),
		[&eq_constructor, &const_constructor,
		&free_constructor, &func_constructor](const Statement& stmt)
		{
			return stmt.fold<PairSyntaxTree>(eq_constructor,
				free_constructor, const_constructor,
				func_constructor);
		});

	// now match the rule_exprs to the `node`

	std::list<SyntaxNodePtr> results;
	for (auto rule_pair : rule_exprs)
	{
		auto lhs_match = try_build_map(rule_pair.first,
			node);
		auto rhs_match = try_build_map(rule_pair.second,
			node);

		if (lhs_match)
		{
			results.push_back(substitute_tree(rule_pair.second,
				lhs_match.get()));
		}
		if (rhs_match)
		{
			results.push_back(substitute_tree(rule_pair.first,
				rhs_match.get()));
		}
	}

	return results;
}


boost::optional<std::map<size_t, SyntaxNodePtr>>
try_build_map(SyntaxNodePtr expr_premise, SyntaxNodePtr expr_concl)
{
	// we're not doing a fold here, but we're implementing something
	// similar to a fold over pairs of syntax trees.

	std::map<size_t, SyntaxNodePtr> free_var_map;
	std::list<std::pair<SyntaxNodePtr, SyntaxNodePtr>> stack;

	stack.push_back(std::make_pair(expr_premise,
		expr_concl));

	while (!stack.empty())
	{
		auto pair = stack.back();
		stack.pop_back();

		ATP_LOGIC_PRECOND(pair.first->get_type()
			!= SyntaxNodeType::EQ);
		ATP_LOGIC_PRECOND(pair.second->get_type()
			!= SyntaxNodeType::EQ);

		switch (pair.first->get_type())
		{
		case SyntaxNodeType::FREE:
		{
			auto p_first = dynamic_cast<FreeSyntaxNode*>(
				pair.first.get());

			ATP_LOGIC_ASSERT(p_first != nullptr);

			// try adding this free variable substitution to the
			// mapping, but return failure if there is a conflict

			auto id_iter = free_var_map.find(p_first->get_free_id());
			
			if (id_iter == free_var_map.end())
			{
				// safe, as this var hasn't been mapped yet
				free_var_map[p_first->get_free_id()] = pair.second;
			}
			else
			{
				// check for a conflict
				if (!syntax_tree_identical(
					pair.second, id_iter->second))
					// there is a conflict, so building a mapping
					// is impossible
					return boost::none;

				// else do nothing
			}
		}
		break;
		case SyntaxNodeType::CONSTANT:
		{
			// we can only match a constant to a constant
			// (in particular, they can only match if of course
			// they are the same symbol!)
			if (pair.second->get_type() !=
				SyntaxNodeType::CONSTANT)
				return boost::none;

			auto p_first = dynamic_cast<ConstantSyntaxNode*>(
				pair.first.get());
			auto p_second = dynamic_cast<ConstantSyntaxNode*>(
				pair.second.get());

			ATP_LOGIC_ASSERT(p_first != nullptr);
			ATP_LOGIC_ASSERT(p_second != nullptr);

			// check this
			if (p_first->get_symbol_id() !=
				p_second->get_symbol_id())
				return boost::none;

			// nothing else to do if the check passed
		}
		break;
		case SyntaxNodeType::FUNC:
		{
			// we can only match a function to a function
			if (pair.second->get_type() !=
				SyntaxNodeType::FUNC)
				return boost::none;

			auto p_first = dynamic_cast<FuncSyntaxNode*>(
				pair.first.get());
			auto p_second = dynamic_cast<FuncSyntaxNode*>(
				pair.second.get());

			ATP_LOGIC_ASSERT(p_first != nullptr);
			ATP_LOGIC_ASSERT(p_second != nullptr);

			if (p_first->get_symbol_id() !=
				p_second->get_symbol_id())
				return boost::none;

			// if two symbols agree, their arity should agree:
			ATP_LOGIC_ASSERT(p_first->get_arity()
				== p_second->get_arity());

			// now just add the children to the stack

			std::transform(boost::make_zip_iterator(
				boost::make_tuple(p_first->begin(),
					p_second->begin())),

				boost::make_zip_iterator(
					boost::make_tuple(p_first->end(),
						p_second->end())),

				std::back_inserter(stack),

				[](boost::tuple<SyntaxNodePtr, SyntaxNodePtr> tup)
				{ return std::make_pair(tup.get<0>(),
					tup.get<1>()); });
		}
		break;
		}
	}

	return free_var_map;
}


SyntaxNodePtr substitute_tree(SyntaxNodePtr node,
	const std::map<size_t, SyntaxNodePtr>& free_var_map)
{
	auto eq_constructor = [](SyntaxNodePtr lhs, SyntaxNodePtr rhs)
	{
		return std::make_shared<EqSyntaxNode>(lhs, rhs);
	};

	auto free_constructor = [&free_var_map](size_t id)
		-> SyntaxNodePtr
	{
		auto id_iter = free_var_map.find(id);

		if (id_iter == free_var_map.end())
		{
			return std::make_shared<FreeSyntaxNode>(id);
		}
		else
		{
			return id_iter->second;
		}
	};

	auto const_constructor = boost::bind(&std::make_shared<
		ConstantSyntaxNode, size_t>, _1);
	
	auto func_constructor = boost::bind(&std::make_shared<
		FuncSyntaxNode, size_t, std::list<SyntaxNodePtr>::iterator,
		std::list<SyntaxNodePtr>::iterator>, _1, _2, _3);

	return fold_syntax_tree<SyntaxNodePtr>(eq_constructor,
		free_constructor, const_constructor, func_constructor, node);
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

			if (p_first->get_symbol_id() !=
				p_second->get_symbol_id())
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

			if (p_first->get_symbol_id() !=
				p_second->get_symbol_id())
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
				{ return std::make_pair(tup.get<0>(),
					tup.get<1>()); });
		}
		break;
		}
	}

	return true;
}


}  // namespace semantics
}  // namespace equational
}  // namespace logic
}  // namespace atp


