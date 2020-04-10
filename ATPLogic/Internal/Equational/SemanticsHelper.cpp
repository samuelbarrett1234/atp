/**

\file

\author Samuel Barrett

*/


#include "SemanticsHelper.h"
#include <boost/iterator/zip_iterator.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/range/adaptor/indexed.hpp>
#include <boost/range/combine.hpp>
#include <boost/bimap.hpp>
#include <boost/phoenix.hpp>
#include <boost/bind.hpp>
#include "SyntaxTreeFold.h"
#include "SyntaxTreeTraversal.h"
#include "KnowledgeKernel.h"


namespace atp
{
namespace logic
{
namespace equational
{
namespace semantics
{


SubstitutionInfo::SubstitutionInfo(const KnowledgeKernel& kernel,
	const std::vector<Statement>& rules,
	const std::set<size_t>& stmt_free_var_ids) :
	kernel(kernel), free_var_ids(stmt_free_var_ids),
	rule_exprs(get_statement_sides(rules)),
	const_symbol_ids(kernel.constant_symbol_ids())
{
	// compute the free variable IDs used in each rule
	rule_free_vars.reserve(rules.size());
	std::transform(rules.begin(), rules.end(),
		std::back_inserter(rule_free_vars),
		boost::bind(&Statement::free_var_ids, _1));
}


std::list<SyntaxNodePtr> immediate_applications(
	SyntaxNodePtr node,
	const SubstitutionInfo& sub_info)
{
	ATP_LOGIC_PRECOND(node->get_type()
		!= SyntaxNodeType::EQ);

	std::list<SyntaxNodePtr> results;
	for (auto rule_pair_and_idx : 
		sub_info.rule_exprs | boost::adaptors::indexed())
	{
		const auto& rule_pair = rule_pair_and_idx.value();
		const auto idx = rule_pair_and_idx.index();

		auto lhs_match = try_build_map(rule_pair.first,
			node);
		auto rhs_match = try_build_map(rule_pair.second,
			node);

		if (lhs_match)
		{
			results.splice(results.end(),
				substitute_tree(rule_pair.second, sub_info,
					lhs_match.get(), idx));
		}
		if (rhs_match)
		{
			results.splice(results.end(),
				substitute_tree(rule_pair.first, sub_info,
					rhs_match.get(), idx));
		}
	}

	return results;
}


std::list<SyntaxNodePtr> substitute_tree(
	SyntaxNodePtr node,
	const SubstitutionInfo& sub_info,
	const std::map<size_t, SyntaxNodePtr>& free_var_map,
	size_t rule_idx)
{
	// rebuild the free variable mapping but include what to do for
	// all the non-mapped free variables
	std::map<size_t, std::list<SyntaxNodePtr>> all_var_map;
	for (const auto& sub : free_var_map)
		all_var_map[sub.first] = { sub.second };

	// note that we are iterating over the RULE's free IDs, and
	// adding mappings to the OTHER STATEMENT's free IDs (this
	// is because `node` should be the other side of the rule that
	// was substituted.)
	for (const auto id : sub_info.rule_free_vars[rule_idx])
	{
		auto map_iter = all_var_map.find(id);
		if (map_iter == all_var_map.end())
		{
			// if a new free variable is being introduced in this
			// substitution, which doesn't have any value it is
			// being substituted for, then we should just substitute
			// it for ANY free variable which was already existing
			// in `node`:
			auto& sublist = all_var_map[id];

			// create free vars
			std::transform(sub_info.free_var_ids.begin(),
				sub_info.free_var_ids.end(),
				std::back_inserter(sublist),
				[](size_t id)
				{ return std::make_shared<FreeSyntaxNode>(id); });

			// create constants
			std::transform(sub_info.const_symbol_ids.begin(),
				sub_info.const_symbol_ids.end(),
				std::back_inserter(sublist),
				[](size_t id)
				{ return std::make_shared<ConstantSyntaxNode>(id); });

			// this is to ensure that we can actually return
			// something (we must be able to replace this un-subbed
			// free variable with something, after all).
			ATP_LOGIC_ASSERT(!sublist.empty());
		}
		// else this free variable has already been covered
	}

	typedef std::list<SyntaxNodePtr> ResultT;

	auto eq_constructor = [](ResultT lhs, ResultT rhs)
		-> ResultT
	{
		ResultT result;

		// cartesian product
		for (auto l : lhs)
			for (auto r : rhs)
				result.emplace_back(std::make_shared<EqSyntaxNode>(
					l, r));
		return result;
	};

	auto free_constructor = [&all_var_map](size_t id)
		-> ResultT
	{
		auto iter = all_var_map.find(id);

		// we should've assigned every free variable a sub:
		ATP_LOGIC_ASSERT(iter != all_var_map.end());

		return iter->second;
	};

	auto const_constructor = [](size_t id)
		-> ResultT
	{
		return { std::make_shared<ConstantSyntaxNode>(id) };
	};

	// unfortunately this constructor is really long because we have
	// to take a cartesian product of the list of lists given to us.
	auto func_constructor = [](size_t id,
		std::list<ResultT>::iterator begin,
		std::list<ResultT>::iterator end)
		-> ResultT
	{
		// none of them should be empty
		ATP_LOGIC_ASSERT(std::none_of(begin, end,
			boost::bind(&ResultT::empty, _1)));

		// need to take the cartesian product of the elements
		
		// stores the tuple (current-iterator, begin-iterator,
		// end-iterator)
		std::list<boost::tuple<ResultT::iterator,
			const ResultT::iterator,
			const ResultT::iterator>> iter_begin_end;
		ResultT result;

		for (auto iter = begin; iter != end; ++iter)
		{
			// we start at the begin, hence the first element
			// is always begin()
			iter_begin_end.push_back(boost::make_tuple(
				iter->begin(), iter->begin(), iter->end()));
		}

		while (true)
		{
			// get the next element which is not an end iterator,
			// and in the process, reset any elements which are
			// already end iterators
			// if they are all end iterators then we are done
			
			// none of these should point to and end-iterator
			ATP_LOGIC_ASSERT(std::none_of(iter_begin_end.begin(),
				iter_begin_end.end(), [](auto tup)
				{ return tup.get<0>() == tup.get<2>(); }));

			// now add the result under the current list of
			// iterators:

			ResultT current_child_list;
			for (auto iter : iter_begin_end)
				current_child_list.push_back(*iter.get<0>());

			// and finally construct the function:
			result.emplace_back(
				std::make_shared<FuncSyntaxNode>(id,
				current_child_list.begin(),
				current_child_list.end()));


			// increment front iterator:
			++iter_begin_end.front().get<0>();

			auto ibe_iter = iter_begin_end.begin();
			while (ibe_iter != iter_begin_end.end() &&
				ibe_iter->get<0>() == ibe_iter->get<2>())
			{
				// reset back to begin
				ibe_iter->get<0>() = ibe_iter->get<1>();

				// move to next digit
				++ibe_iter;

				// try to carry forward the bit, if we've not already
				// reached the end!
				if (ibe_iter == iter_begin_end.end())
				{
					// we have exhausted all possibilities and now we
					// are done!
					return result;
				}
				else
				{
					// carry forward:
					++ibe_iter->get<0>();
				}
			}

		}
	};

	return fold_syntax_tree<ResultT>(eq_constructor,
		free_constructor, const_constructor, func_constructor,
		node);
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


std::vector<std::pair<SyntaxNodePtr, SyntaxNodePtr>>
get_statement_sides(const std::vector<Statement>& stmts)
{
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
	std::vector<PairSyntaxTree> exprs;
	exprs.reserve(stmts.size());

	std::transform(stmts.begin(), stmts.end(),
		std::back_inserter(exprs),
		[&eq_constructor, &const_constructor,
		&free_constructor, &func_constructor](const Statement& stmt)
		{
			return stmt.fold<PairSyntaxTree>(eq_constructor,
				free_constructor, const_constructor,
				func_constructor);
		});

	return exprs;
}


std::set<size_t> get_free_var_ids(SyntaxNodePtr p_node)
{
	std::list<SyntaxNodePtr> stack;
	std::set<size_t> var_ids;

	stack.push_back(p_node);

	while (!stack.empty())
	{
		p_node = stack.back();
		stack.pop_back();

		apply_to_syntax_node<void>(
			[&stack](EqSyntaxNode& node)
			{
				stack.push_back(node.left());
				stack.push_back(node.right());
			},
			[&var_ids](FreeSyntaxNode& node)
			{
				var_ids.insert(node.get_free_id());
			},
			[](ConstantSyntaxNode&) {},
			[&stack](FuncSyntaxNode& node)
			{
				stack.insert(stack.end(), node.begin(), node.end());
			},
		*p_node);
	}

	return var_ids;
}


}  // namespace semantics
}  // namespace equational
}  // namespace logic
}  // namespace atp

