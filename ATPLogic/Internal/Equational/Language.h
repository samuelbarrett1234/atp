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
	bool load_kernel(IKnowledgeKernel& ker,
		std::istream& in) const override;

	KnowledgeKernelPtr create_empty_kernel() const override;

	StatementArrayPtr deserialise_stmts(std::istream& in,
		StmtFormat input_format,
		const IKnowledgeKernel& ker) const override;

	void serialise_stmts(std::ostream& out,
		StatementArrayPtr p_stmts,
		StmtFormat output_format) const override;
};


}  // namespace equational
}  // namespace logic
}  // namespace atp


