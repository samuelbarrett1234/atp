#pragma once


/*

SyntaxNodeToStr.h

A helper function for converting a syntax tree into a string
representation, done using a fold.

*/


#include <string>
#include <Internal/Equational/KnowledgeKernel.h>
#include <Internal/Equational/SyntaxNodes.h>


// convert a syntax tree to string using a fold
std::string syntax_tree_to_str(
	const atp::logic::equational::KnowledgeKernel& ker,
	atp::logic::equational::SyntaxNodePtr p_tree);


// return true if and only if there exists a permutation of the free
// variables which makes the string-conversion of the syntax tree
// p_tree equal to the given test_stmt (test statement). also pass in
// the number of free variables, to help.
// IMPORTANT: it assumes the free variables in test_stmt are of the
// form: x0, x1, x2, ...
bool exists_free_var_assignment(
	const atp::logic::equational::KnowledgeKernel& ker,
	atp::logic::equational::SyntaxNodePtr p_tree,
	std::string test_stmt,
	size_t num_free_vars
	);


