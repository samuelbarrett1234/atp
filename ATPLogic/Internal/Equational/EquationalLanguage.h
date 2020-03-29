#pragma once


/*

EquationalLanguage.h

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


class ATP_LOGIC_API EquationalLanguage: public ILanguage
{
public:
	EquationalLanguage();

	virtual bool load_kernel(IKnowledgeKernel& ker,
		std::istream& in) const override;

	virtual KnowledgeKernelPtr create_empty_kernel() const override;

	virtual StatementArrayPtr create_stmts(std::istream& in,
		StmtFormat input_format) const override;

	virtual void save_stmts(std::ostream& out,
		StatementArrayPtr p_stmts,
		StmtFormat output_format) const override;

private:

};


}  // namespace logic
}  // namespace atp


