#pragma once


/**


\file

\author Samuel Barrett

\brief Interfaces for search heuristic functions

\details Uninformed search strategies won't get very far, thus we
    use a heuristic function (or several) to help guide the search.
    Heuristic functions are represented as objects because they might
    need persistent information, like model parameters or even just
    the knowledge kernel. Their operations are also vectorised.

*/


#include <memory>
#include <ATPLogic.h>
#include "../ATPSearchAPI.h"


namespace atp
{
namespace search
{


/**
\interface IHeuristic

\brief Basically represents a function from proof states to floats

\note This function has been vectorised (it operates on several
    statements, and returns a float for each of them).

\note This is an object, not just a function, because the model
    might need to store state e.g. access to GPU.
*/
class ATP_SEARCH_API IHeuristic
{
public:
	virtual ~IHeuristic() = default;

	/**
	\brief Evaluate the function on this proof state

	\pre Some heuristic implementations may be tied to a particular
	    model contexts, in which case `p_state` needs to be valid
		in that context.
	*/
	virtual float predict(const logic::ProofStatePtr& p_state) = 0;
};


typedef std::shared_ptr<IHeuristic> HeuristicPtr;


}  // namespace search
}  // namespace atp


