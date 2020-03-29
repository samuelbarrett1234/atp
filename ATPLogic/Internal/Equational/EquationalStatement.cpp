#include "EquationalStatement.h"
#include "EquationalMatching.h"


namespace atp
{
namespace logic
{


StmtForm EquationalStatement::form() const
{
	if (eq_matching::trivially_true(m_root))
	{
		return StmtForm::CANONICAL_TRUE;
	}
	else return StmtForm::NOT_CANONICAL;

	// equational statements cannot be canonically false
}


}  // namespace logic
}  // namespace atp


