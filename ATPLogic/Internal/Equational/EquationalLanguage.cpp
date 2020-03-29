#include "EquationalLanguage.h"


namespace atp
{
namespace logic
{


EquationalLanguage::EquationalLanguage()
{
}


bool EquationalLanguage::load_kernel(IKnowledgeKernel& ker,
	std::istream& in) const
{
	return false;
}


KnowledgeKernelPtr EquationalLanguage::create_empty_kernel() const
{
	return KnowledgeKernelPtr();
}


StatementArrayPtr EquationalLanguage::create_stmts(std::istream& in,
	StmtFormat input_format) const
{
	return StatementArrayPtr();
}


void EquationalLanguage::save_stmts(std::ostream& out,
	StatementArrayPtr p_stmts, StmtFormat output_format) const
{
}


}  // namespace logic
}  // namespace atp


