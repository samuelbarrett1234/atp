#pragma once


/*

ATPLogic.h

Main header file for this library.

*/


#include "ATPLogicAPI.h"
#include "Interfaces/IStatement.h"
#include "Interfaces/IStatementArray.h"
#include "Interfaces/IKnowledgeKernel.h"
#include "Interfaces/ILanguage.h"


namespace atp
{
namespace logic
{


enum class LangType
{
	EQUATIONAL_LOGIC,
	FIRST_ORDER_LOGIC,  // not yet implemented
	SECOND_ORDER_LOGIC  // not yet implemented
};


/// <summary>
/// Construct a language object for the given supported language
/// type. Returns an empty object if the language type was invalid
/// which could happen if the LangType has not yet been implemented.
/// You will only have to create one language object for each type.
/// </summary>
ATP_LOGIC_API LanguagePtr create_language(LangType lt);


// Postcondition: returns the concatenation of the two arrays
// with 'l' placed first
ATP_LOGIC_API StatementArrayPtr concat(const IStatementArray& l,
	const IStatementArray& r);


// Postcondition: returns the concatenation of the array of arrays
// returning a single 1-dimensional array.
ATP_LOGIC_API StatementArrayPtr concat(
	const std::vector<StatementArrayPtr>& stmts);


}  // namespace logic
}  // namespace atp


