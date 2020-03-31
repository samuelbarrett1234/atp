#pragma once


/*

ILanguage.h

This file represents the syntax of the logic and axiom set being
used in a particular context. In particular, it constructs, builds
and loads knowledge kernels. It is also responsible for serialising
statement arrays.

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


/// <summary>
/// The formats to serialise a statement array to and from.
/// </summary>
enum class StmtFormat
{
	BINARY,
	TEXT
};


/// <summary>
/// Object for serialising statements, and constructing knowledge
/// kernels.
/// </summary>
class ATP_LOGIC_API ILanguage
{
public:
	virtual ~ILanguage() = default;

	// Precondition: 'ker' is a knowledge kernel created by this
	// particular language object.
	// Postcondition: modifies the stream by reading the input for
	// the knowledge kernel, updates the information into the kernel,
	// and returns true iff the load was successful.
	// If the load failed, the stream position may still have been
	// moved, but the knowledge kernel object will be restored to its
	// prior state.
	virtual bool load_kernel(IKnowledgeKernel& ker,
		std::istream& in) const = 0;

	// Postcondition: returns an empty kernel object to use for
	// loading (this is the only way of obtaining a knowledge
	// kernel). Empty kernels are initialised with knowledge of
	// propositional logic only.
	virtual KnowledgeKernelPtr create_empty_kernel() const = 0;

	// Load an array of statements from a text file or from binary
	// (see the StmtFormat enumeration for the different formats).
	// Requires the kernel for type checking etc.
	virtual StatementArrayPtr deserialise_stmts(std::istream& in,
		StmtFormat input_format,
		const IKnowledgeKernel& ker) const = 0;

	// Save an array of statements to a file (either in text format
	// or binary format, see the StmtFormat enumeration.)
	virtual void serialise_stmts(std::ostream& out,
		StatementArrayPtr p_stmts,
		StmtFormat output_format) const = 0;
};


typedef std::shared_ptr<ILanguage> LanguagePtr;


}  // namespace logic
}  // namespace atp


