/**

\file

\author Samuel Barrett

*/

#include "SyntaxNodeToStr.h"
#include "../Test.h"
#include <list>
#include <map>
#include <algorithm>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/lexical_cast.hpp>
#include <Internal/Equational/SyntaxTreeFold.h>


using atp::logic::equational::KnowledgeKernel;
using atp::logic::equational::SyntaxNodePtr;
using atp::logic::equational::fold_syntax_tree;


// a version of syntax_tree_to_str which allows you to specify a
// map `free_map` such that a variable with free ID `i` will appear
// as though to have ID `free_map[i]` when converting to string, if
// `i` is in the mapping (if not, it is left as-is).
std::string syntax_tree_to_str(const KnowledgeKernel& ker,
	SyntaxNodePtr p_tree, const std::map<size_t, size_t>& free_map);


// function to be used as the EqSyntaxNode fold constructor
static std::string eq_node_to_str(std::string lhs, std::string rhs);


// function to be used as the FreeSyntaxNode fold constructor
static std::string free_node_to_str(
	const std::map<size_t, size_t>& free_id_map, size_t free_id);


// function to be used as the ConstantSyntaxNode fold constructor
static std::string const_node_to_str(const KnowledgeKernel& ker,
	size_t symbol_id);


// function to be used as the FuncSyntaxNode fold constructor
static std::string func_node_to_str(const KnowledgeKernel& ker,
	size_t symbol_id, std::list<std::string>::iterator begin,
	std::list<std::string>::iterator end);


/////////////////////////////////////////////////////////////////////


std::string syntax_tree_to_str(const KnowledgeKernel& ker,
	SyntaxNodePtr p_tree)
{
	// just forward the call directly on to the version with
	// the free_id_map
	return syntax_tree_to_str(ker, p_tree,
		std::map<size_t, size_t>());
}


std::string syntax_tree_to_str(const KnowledgeKernel& ker,
	SyntaxNodePtr p_tree,
	const std::map<size_t, size_t>& free_id_map)
{
	return fold_syntax_tree<std::string>(
		&eq_node_to_str, boost::bind(&free_node_to_str,
			boost::ref(free_id_map), _1),
		boost::bind(&const_node_to_str, boost::ref(ker),
			_1),
		boost::bind(&func_node_to_str, boost::ref(ker),
			_1, _2, _3),
		p_tree);
}


std::string eq_node_to_str(std::string lhs, std::string rhs)
{
	return lhs + " = " + rhs;
}


std::string free_node_to_str(
	const std::map<size_t, size_t>& free_id_map, size_t free_id)
{
	auto iter = free_id_map.find(free_id);
	if (iter != free_id_map.end())
	{
		free_id = iter->second;
	}
	return "x" + boost::lexical_cast<std::string>(free_id);
}


std::string const_node_to_str(const KnowledgeKernel& ker,
	size_t symbol_id)
{
	// might as well test the arity here:
	BOOST_TEST(ker.symbol_arity_from_id(symbol_id) == 0);

	return ker.symbol_name(symbol_id);
}


std::string func_node_to_str(const KnowledgeKernel& ker,
	size_t symbol_id, std::list<std::string>::iterator begin,
	std::list<std::string>::iterator end)
{
	// might as well test the arity here:
	BOOST_TEST(std::distance(begin, end)
		== ker.symbol_arity_from_id(symbol_id));

	return ker.symbol_name(symbol_id) + '(' +
		boost::algorithm::join(
			boost::make_iterator_range(begin, end),
			", ") + ')';
}


