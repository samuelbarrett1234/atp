#pragma once


/**

\file

\author Samuel Barrett

\brief Contains the ILanguage interface

*/


#include <istream>
#include <ostream>
#include <memory>
#include "../ATPLogicAPI.h"
#include "IStatement.h"
#include "IKnowledgeKernel.h"


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

\brief Manages knowledge kernels within a logical language framework

\details This file represents the syntax of the logic and axiom set being
    used in a particular context. In particular, it constructs, builds
    and loads knowledge kernels. It is also responsible for serialising
    statement arrays.
*/
class ATP_LOGIC_API ILanguage
{
public:
	virtual ~ILanguage() = default;

	/**
	\brief Load the knowledge kernel from a given stream (of e.g.
	    definitions.)

	\note Changes the position of the input stream

	\note If operation failed, the kernel is left unchanged, but the
	    stream may have been moved.

	\pre `ker` was created within this language.
	*/
	virtual bool load_kernel(IKnowledgeKernel& ker,
		std::istream& in) const = 0;

	virtual KnowledgeKernelPtr create_empty_kernel() const = 0;

	/**

	\brief Load an array of statements from an input stream, in the
	    context of a given kernel.

	\param ker The knowledge kernel is required for type checking
	    and such (also for distinguishing between variables and
		constants).

	\note Returns nullptr if this operation failed, for example if
	    the statements were syntactically incorrect or ill-typed. In
		this case, the stream can still be modified.

	*/
	virtual StatementArrayPtr deserialise_stmts(std::istream& in,
		StmtFormat input_format,
		const IKnowledgeKernel& ker) const = 0;

	/**
	\brief Save an array of statements

	\pre p_stmts != nullptr
	*/
	virtual void serialise_stmts(std::ostream& out,
		StatementArrayPtr p_stmts,
		StmtFormat output_format) const = 0;
};


typedef std::shared_ptr<ILanguage> LanguagePtr;


}  // namespace logic
}  // namespace atp


