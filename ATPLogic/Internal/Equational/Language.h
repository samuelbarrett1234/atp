#pragma once


/**

\file

\author Samuel Barrett

\brief Implementation of the ILanguage interface for equational logic

*/


#include "../../ATPLogicAPI.h"
#include "../../Interfaces/ILanguage.h"


namespace atp
{
namespace logic
{
namespace equational
{


class ATP_LOGIC_API Language : public ILanguage
{
public:
	ModelContextPtr try_create_context(
		std::istream& in) const override;

	KnowledgeKernelPtr try_create_kernel(
		const IModelContext& ctx) const override;

	StatementArrayPtr deserialise_stmts(std::istream& in,
		StmtFormat input_format,
		const IModelContext& ctx) const override;

	void serialise_stmts(std::ostream& out,
		const StatementArrayPtr& p_stmts,
		StmtFormat output_format) const override;

	/**
	\brief Reduce an array of statements to some "normal form".
	
	\details This, more specifically, means: reducing free variables,
		forcing an ordering about the equals sign, and reducing to
		equivalence classes under the statement equivalence relation.

	\returns A new array of statements with the above effects applied
		(n.b. the returned array might not be the same length!)
	*/
	StatementArrayPtr normalise(
		const StatementArrayPtr& p_stmts) override;
};


}  // namespace equational
}  // namespace logic
}  // namespace atp


