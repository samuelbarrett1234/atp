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
\brief Different flags for successor generation for `ProofState
	objects.

\see iter_settings
*/
typedef size_t IterSettings;


/**
\namespace iter_settings

\brief Contains different settings that can be passed to the
	construction of ProofStates in the Knowledge Kernel.

\details There are many different ways in which one might want to
	iterate over ProofState successors. The most natural place to
	specify these wants is at construction, in the knowledge kernel.
	Use the below as a set of flags to pass to `begin_proof_of`
	in the knowledge kernel implementation.
*/
namespace iter_settings
{


/**
\brief Use the implementation's selected default settings.

\details The implementation will decide which settings it considers
	"default".
*/
static const constexpr IterSettings DEFAULT = 0;

/**
\brief Don't repeat successors where possible (performs extra
	checks to ensure this doesn't happen).

\details This prevents consideration of successor states which
	loop back to states already seen - the precise meaning of this
	flag, and its execution, however, is left to the logic
	implementation. There is certainly a speed / state space size
	tradeoff here.
*/
static const constexpr IterSettings NO_REPEATS = 1 << 0;

/**
\brief Try to enumerate the successors in a random order.

\details It is not always possible to randomise, however sometimes it
	is useful for models which expect successors to be IID
	(independent and identically distributed).
*/
static const constexpr IterSettings RANDOMISED = 1 << 1;
}


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

	\param stmt The statement to target proving

	\param flags Flags indicating how the returned proof state should
		iterate over successors.

	\pre `stmt` is a valid statement within the context of this
		knowledge kernel.

	\pre iter_settings_supported(flags)
	*/
	virtual ProofStatePtr begin_proof_of(
		const IStatement& stmt,
		IterSettings flags = iter_settings::DEFAULT) const = 0;

	/**
	\brief Determine if the given flags are supported by this kind of
		knowledge kernel.

	\returns True if supported, false if not.
	*/
	virtual bool iter_settings_supported(
		IterSettings flags) const = 0;

	/**
	\brief A "trivial" statement is either reflexive or implied
		directly by one of the axioms.

	\pre `stmt` is a valid statement within the context of this
		knowledge kernel.
	*/
	virtual bool is_trivial(
		const IStatement& stmt) const = 0;

	/**
	\brief Load the given set of theorems into the knowledge kernel
		so it can use them in proofs.

	\returns A reference ID in case you wish to remove these theorems
		from the kernel in the future (e.g. for streaming).

	\pre The statements are valid within the context of this
		knowledge kernel (e.g. type-correct.)

	\pre p_thms->size() > 0
	*/
	virtual size_t add_theorems(
		StatementArrayPtr p_thms) = 0;

	/**
	\brief Unload the set of previously added theorems.

	\param ref_id The return value of the corresponding
		`add_theorems` call made earlier.

	\pre `ref_id` must have been returned from an earlier call to
		`add_theorems`, and must not have already been removed.
	*/
	virtual void remove_theorems(size_t ref_id) = 0;

	/**
	\brief Set the random number generation seed (overwrites last
		value if any)

	\note Random number generation may be used by components of this
		library created by the knowledge kernel, for example to
		iterate over proof state successors randomly.
	*/
	virtual void set_seed(size_t seed) = 0;

	/**
	\brief Generate a random number (think of this as just std::rand)

	\note Random number generation may be used by components of this
		library created by the knowledge kernel, for example to
		iterate over proof state successors randomly.
	*/
	virtual size_t generate_rand() const = 0;
};


typedef std::shared_ptr<IKnowledgeKernel> KnowledgeKernelPtr;


}  // namespace logic
}  // namespace atp


