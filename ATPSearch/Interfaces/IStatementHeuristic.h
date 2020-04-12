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


#include <vector>
#include <memory>
#include <map>
#include <ATPLogic.h>
#include "../ATPSearchAPI.h"


namespace atp
{
namespace search
{


/**
\interface IStatementHeuristic

\brief Basically represents a function from statements to floats

\note This function has been vectorised (it operates on several
    statements, and returns a float for each of them).

\note This is an object, not just a function, because the model
    might need to store state e.g. access to GPU.
*/
class ATP_SEARCH_API IStatementHeuristic
{
public:
	virtual ~IStatementHeuristic() = default;

	/**
	\brief Evaluate the function on this array of statements

	\pre Some heuristic implementations may be tied to a particular
	    model contexts, in which case `p_stmts` needs to be valid
		in that context.
	*/
	virtual std::vector<float> predict(
		logic::StatementArrayPtr p_stmts) = 0;
};


typedef std::shared_ptr<IStatementHeuristic> StatementHeuristicPtr;
typedef std::map<std::string, StatementHeuristicPtr> HeuristicCollection;


}  // namespace search
}  // namespace atp


