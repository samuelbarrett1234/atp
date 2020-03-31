#pragma once


/*

Language.h

Implementation of the ILanguage interface for equational logic. Note
that equational logic does NOT support \not (thus it cannot prove
that two expressions are not equal)! Thus its knowledge kernels do
not provide the propositional interface.

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


