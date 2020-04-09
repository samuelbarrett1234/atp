#pragma once


/**

\file

\author Samuel Barrett

\brief Main include file for the ATPLogic library

\details This is the main file for the ATPLogic library, where it
    should be possible to use all of the library from just this
	include file, unless you are wanting details of particular logics

*/


#include "ATPLogicAPI.h"
#include "Interfaces/IStatement.h"
#include "Interfaces/IStatementArray.h"
#include "Interfaces/IKnowledgeKernel.h"
#include "Interfaces/ILanguage.h"


/**

\namespace atp::logic

\brief The namespace of the ATPLogic library.
*/


namespace atp
{
namespace logic
{


/**
This is an enumeration of logical language types.
*/
enum class LangType
{
	/**
	A rudimentary implementation of equational logic, which is
	capable of making proofs by manipulating variables and constants
	subject to a given set of rules.
	*/
	EQUATIONAL_LOGIC
};


/**
\brief Allocates a new language object for a particular logical
    language.

\details Allocates a new language object for a particular logical
    language. Should only have to do this once per language type,
	because these objects hold little, if any, state.

\param lt The type of the logical language to create.
*/
ATP_LOGIC_API LanguagePtr create_language(LangType lt);


/**
\brief Create a singleton statment array from a given statement.

\param stmt The statement to create the array from.
*/
ATP_LOGIC_API StatementArrayPtr from_statement(const IStatement& stmt);


/**
\brief Concatenate two statement arrays of the same type.

\param l The left-hand statement array to be placed first
\param r The right-hand statement array to be placed second
*/
ATP_LOGIC_API StatementArrayPtr concat(const IStatementArray& l,
	const IStatementArray& r);


/**
\brief Concatenate a list of statement arrays into one statement
    array.

\param stmts The array of statement arrays, which will be flattened
    into one, in order.
*/
ATP_LOGIC_API StatementArrayPtr concat(
	const std::vector<StatementArrayPtr>& stmts);


}  // namespace logic
}  // namespace atp


