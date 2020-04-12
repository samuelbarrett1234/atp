#pragma once


/**

\file

\author Samuel Barrett

\brief Contains the IKnowledgeKernel interface, which represents a
    structure for performing logical deduction based on all known
	information (axioms AND already proven theorems).

*/


#include <memory>
#include <vector>
#include "../ATPLogicAPI.h"
#include "IStatement.h"
#include "IStatementArray.h"
#include "IProofState.h"


namespace atp
{
namespace logic
{


/**

\interface IKnowledgeKernel

\brief Represents a collection of definitions and axioms, associated
    with a particular ILanguage, and performs logical deduction.

\details The knowledge kernel contains all logical rules,
	axioms, and available theorems. Contrast this system with
	IModelContext - the latter object is for storing definitions and
	axioms in text form, whereas the knowledge kernel, which is
	created using the model context, condenses the text form into
	something more efficient and also allows you to load and unload
	already proven theorems.
	Note that every kernel state is associated with a kind of
	hash-code; this is useful because the code is designed such that
	if two kernels agree on their code, they agree on the return
	results for all of their functions (assuming no hash collisions).
	

*/
class ATP_LOGIC_API IKnowledgeKernel
{
public:
	virtual ~IKnowledgeKernel() = default;

	/**

	\brief Characterises the information provided to this knowledge
	    kernel in a single integer code.

	\details If two knowledge kernels agree on their code, then
	    they agree on the return results of all the functions
		below. (This code is computed similarly to a hash code of all
		the user definitions, axioms, and available theorems.)

	*/
	virtual size_t get_integrity_code() const = 0;

	/**
	\brief Create a new proof state which, initially empty, is set to
		try and prove `stmt`.

	\pre `stmt` is a valid statement within the context of this
		knowledge kernel.
	*/
	virtual ProofStatePtr begin_proof_of(
		const IStatement& stmt) const = 0;

	/**
	\brief A "trivial" statement is either reflexive or implied
		directly by one of the axioms.
	*/
	virtual bool is_trivial(
		const IStatement& stmt) const = 0;
};


typedef std::shared_ptr<IKnowledgeKernel> KnowledgeKernelPtr;


}  // namespace logic
}  // namespace atp


