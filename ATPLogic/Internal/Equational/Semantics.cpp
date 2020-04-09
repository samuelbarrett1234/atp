/**

\file

\author Samuel Barrett

*/


#include "Semantics.h"
#include <list>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/phoenix.hpp>
#include <boost/bind.hpp>
#include <boost/bimap.hpp>
#include "SemanticsHelper.h"
#include "SyntaxTreeFold.h"
#include "SyntaxTreeTraversal.h"


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


StatementArray get_successors(const Statement& stmt,
	const std::vector<Statement>& rules)
{
	// this pair can be thought of as a node saying: "<me, and all of
	// the ways the subtree rooted at me look after substitution>"
	typedef std::pair<SyntaxNodePtr, std::list<SyntaxNodePtr>>
		SubResults;

	SubstitutionInfo sub_info(stmt.kernel(), rules,
		stmt.free_var_ids());

	auto eq_constructor = [](const SubResults& lhs,
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

	auto free_constructor = [&sub_info]
		(size_t id) -> SubResults
	{
		auto me = std::make_shared<FreeSyntaxNode>(id);

		return std::make_pair(me,
			immediate_applications(me, sub_info));
	};

	auto const_constructor = [&sub_info]
		(size_t id) -> SubResults
	{
		auto me = std::make_shared<ConstantSyntaxNode>(id);

		return std::make_pair(me,
			immediate_applications(me, sub_info));
	};

	auto func_constructor = [&sub_info]
		(size_t id, std::list<SubResults>::iterator begin,
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
			sub_info);

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


}  // namespace semantics
}  // namespace equational
}  // namespace logic
}  // namespace atp


