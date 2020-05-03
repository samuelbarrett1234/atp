#pragma once


/**

\file

\author Samuel Barrett

\brief Contains the IProofState interface, which encapsulates the
    idea of either a partial or complete proof.

*/


#include <vector>
#include <memory>
#include "../ATPLogicAPI.h"
#include "IStatement.h"
#include "IStatementArray.h"


namespace atp
{
namespace logic
{


enum class ProofCompletionState
{
    // if the target statement is true and we have a proof
    PROVEN,

    // if there does not exist a proof of the target statement
    NO_PROOF,

    // if our proof does not indicate either of the above
    UNFINISHED
};


class IPfStateSuccIter;  // forward declaration
typedef std::shared_ptr<IPfStateSuccIter> PfStateSuccIterPtr;


/**
\interface IProofState

\brief A proof state encapsulates the idea of a proof (which may be a
    WIP) and contains many different statements.

\details An object that conforms to this interface has the
    responsibility to: (i) generate successor states, (ii) detect
    when it is a complete (i.e. correct) proof. Proof states always
    have a "target statement" in mind, which they are trying to
    prove. This class is immutable (once constructed it remains
    constant).

\note These objects collaborate with the IKnowledgeKernel interface a
    lot, and they always exist with a reference to a knowledge kernel
    of some sort.

\warning Proof states should not outlive the knowledge kernels that
    create them.
*/
class ATP_LOGIC_API IProofState
{
public:
    virtual ~IProofState() = default;

    /**
    \brief Get the target statement which we are trying to prove
    */
    virtual const IStatement& target_stmt() const = 0;

    /**
    \brief Create a new iterator which allows us to enumerate the
        successors of this class.

    \note It is not guaranteed whether the work for generating these
        successors will be done lazily or during this call.

    \warning The available successors depends on what theorems the
        knowledge kernel has available. Thus if we change the
        available theorems while there are still iterators alive, it
        is up to the implementation on whether the iterators utilise
        that.
    */
    virtual PfStateSuccIterPtr succ_begin() const = 0;

    /**
    \brief Determine whether or not this proof is done
    */
    virtual ProofCompletionState completion_state() const = 0;

    /**
    \brief Return a human-readable string representation of this
        proof.

    \note The proof doesn't have to be in any particular completion
        state - it works regardless.
    */
    virtual std::string to_str() const = 0;

    /**
    \brief Get the number of times each statement is used in this
        proof state.

    \details This is a bit of a "soft" function, intended to help
        compute usefulness-heuristics. What entails a single "use"
        of a theorem is implementation-dependent. For example,
        it could be taken to mean either: (i) an equivalent statement
        was used, or (ii) this exact statement was used, etc.

    \returns An array of the same size as p_stmts of integers, where
        each integer is the number of times that statement COULD be
        used in this proof.
    */
    virtual std::vector<size_t> get_usage(
        const StatementArrayPtr& p_stmts) const = 0;
};
typedef std::shared_ptr<IProofState> ProofStatePtr;


/**
\brief This is an interface for enumerating through successors of a
    proof state.
*/
class ATP_LOGIC_API IPfStateSuccIter
{
public:
    virtual ~IPfStateSuccIter() = default;

    /**
    \returns True iff this iterator points to a successor (i.e. is
        dereferencable.)
    */
    virtual bool valid() const = 0;

    /**
    \brief Get the successor pointed to by this iterator.

    \pre valid()
    */
    virtual ProofStatePtr get() const = 0;

    /**
    \brief Increment the iterator; move on to the next successor

    \pre valid()
    */
    virtual void advance() = 0;

    /**
    \brief Get the number of proof states currently held in memory
        by this iterator

    \note If this iterator lazily evaluates nodes, this will always
        return 1.

    \details This is only for performance tracking (in the search
        library.)

    \pre valid()
    */
    virtual size_t size() const = 0;
};


}  // namespace logic
}  // namespace atp


