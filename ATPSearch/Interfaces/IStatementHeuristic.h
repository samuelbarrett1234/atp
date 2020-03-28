#pragma once


/*


Uninformed search strategies won't get very far, thus we use a
heuristic function (or several) to help guide the search.
Heuristic functions are represented as objects because they might
need persistent information, like model parameters or even just the
knowledge kernel. Their operations are also vectorised.


*/


#include <vector>
#include <memory>
#include <map>
#include <ATPLogic.h>
#include "ATPSearchAPI.h"


namespace atp
{
namespace search
{


// Basically a persistent function from to floats which has been
// vectorised. It is assumed that if the heuristic needs access
// to the knowledge kernel then it was given it in its constructor.
class ATP_SEARCH_API IStatementHeuristic
{
public:
	virtual ~IStatementHeuristic() = default;

	// evaluate on this array of statements
	// (if this heuristic uses a knowledge kernel, the kernel may
	// assert that p_stmts is valid. Check the implementation for
	// this.)
	virtual std::vector<float> predict(
		logic::StatementArrayPtr p_stmts) = 0;
};


typedef std::shared_ptr<IStatementHeuristic> StatementHeuristicPtr;
typedef std::map<std::string, StatementHeuristicPtr> HeuristicCollection;


}  // namespace search
}  // namespace atp


