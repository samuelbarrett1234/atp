#pragma once


/**

\file

\author Samuel Barrett

\brief Contains an iterator which blocks repeated states

*/


#include "../../ATPLogicAPI.h"
#include "../../Interfaces/IProofState.h"


namespace atp
{
namespace logic
{
namespace equational
{


class ProofState;  // forward declaration
class Statement;  // foward declaration


/**
\brief The NoRepeatIterator blocks states that have already been
	seen on the path from the root to the current node. It does so by
	wrapping around another given iterator.

\note It doesn't necessarily block states that have been seen before
	on a different branch - only ones that have occurred from the
	root to the current proof state node.
*/
class ATP_LOGIC_API NoRepeatIterator :
	public IPfStateSuccIter
{
public:
	static PfStateSuccIterPtr construct(
		const ProofState& parent,
		PfStateSuccIterPtr iter);

	NoRepeatIterator(const ProofState& parent,
		PfStateSuccIterPtr iter);

	bool valid() const override;
	ProofStatePtr get() const override;
	void advance() override;
	size_t size() const override;

private:
	// bring the iterator forward until it has reached a point where
	// it is not being blocked
	/**
	\brief Bring the iterator forward until it has reached a point
		where it is valid and not blocked
	*/
	void forward();

	/**
	\brief Determine if a statement equivalent to this one has
		occurred on the path from the current node to the root node
	*/
	bool blocked() const;

private:
	const ProofState& m_parent;
	PfStateSuccIterPtr m_iter;

	// invariant: either we are invalid, or iter is valid and the
	// proof state pointed to is not blocked
};


}  // namespace equational
}  // namespace logic
}  // namespace atp


