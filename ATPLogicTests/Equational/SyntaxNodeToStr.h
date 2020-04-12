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
	const atp::logic::equational::ModelContext& ctx,
	atp::logic::equational::SyntaxNodePtr p_tree);


