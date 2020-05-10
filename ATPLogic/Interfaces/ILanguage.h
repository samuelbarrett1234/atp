#pragma once


/**

\file

\author Samuel Barrett

\brief Contains the ILanguage interface which acts as a factory class
    for each type of logic in the library.

*/


#include <istream>
#include <ostream>
#include <memory>
#include "../ATPLogicAPI.h"
#include "IStatement.h"
#include "IKnowledgeKernel.h"
#include "IModelContext.h"


namespace atp
{
namespace logic
{


/**
Different statement file formats
*/
enum class StmtFormat
{
	BINARY,
	TEXT
};


/**

\interface ILanguage

\brief Manages model contexts and knowledge kernels within a logical 
    language framework

\details This file represents the syntax of the logic and axiom set being
    used in a particular context. In particular, it constructs, builds
    and several classes.
*/
class ATP_LOGIC_API ILanguage
{
public:
	virtual ~ILanguage() = default;

	/**
	\brief Try to create a model context from the given input

	\param in An input stream to the model context file in JSON

	\returns Nullptr on failure, otherwise returns a model context
	    object.

	\warning Model context objects don't check for syntax errors,
	    this will occur when a knowledge kernel is constructed.
	*/
	virtual ModelContextPtr try_create_context(
		std::istream& in) const = 0;

	/**
	\brief Create a knowledge kernel corresponding to a given model
	    context.

	\returns Nullptr if the axioms or definitions in the model
	    context contain errors, otherwise returns a KK initialised
		only with the axioms.
	*/
	virtual KnowledgeKernelPtr try_create_kernel(
		const IModelContext& ctx) const = 0;

	/**

	\brief Load an array of statements from an input stream, in the
	    given context.

	\note Returns nullptr if this operation failed, for example if
	    the statements were syntactically incorrect or ill-typed. In
		this case, the stream can still be modified.

	*/
	virtual StatementArrayPtr deserialise_stmts(std::istream& in,
		StmtFormat input_format,
		const IModelContext& ctx) const = 0;

	/**
	\brief Save an array of statements

	\pre p_stmts != nullptr
	*/
	virtual void serialise_stmts(std::ostream& out,
		const StatementArrayPtr& p_stmts,
		StmtFormat output_format) const = 0;

	/**
	\brief Reduce an array of statements to some "normal form".

	\details This, more specifically, means: reducing free variables,
		forcing an ordering about the equals sign, and reducing to
		equivalence classes under the statement equivalence relation.
		To summarise: normalising an array of statements doesn't
		reduce the amount of "information" you have, but makes it
		more efficient.

	\returns A new array of statements with the above effects applied
		(n.b. the returned array might not be the same length!)
	*/
	virtual StatementArrayPtr normalise(
		const StatementArrayPtr& p_stmts) = 0;
};


typedef std::shared_ptr<ILanguage> LanguagePtr;


}  // namespace logic
}  // namespace atp


