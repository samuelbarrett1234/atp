/**
\file

\author Samuel Barrett

*/


#include <Internal/Equational/Statement.h>
#include <Internal/Equational/StatementArray.h>
#include "HMMStmtToObs.h"


namespace atp
{
namespace core
{


// function specialised to equational logic
std::vector<std::vector<size_t>> convert_equational_logic(
	const logic::equational::StatementArray& arr,
	const std::vector<size_t>& symb_ids)
{
	std::vector<std::vector<size_t>> result(arr.size());
	const size_t FREE_VAR = symb_ids.size();

	for (size_t i = 0; i < arr.size(); ++i)
	{
		const auto& stmt = arr.my_at(i);

		for (auto subexpr : stmt)
		{
			switch (subexpr.root_type())
			{
			case logic::equational::SyntaxNodeType::FREE:
				result[i].push_back(FREE_VAR);
				break;
			case logic::equational::SyntaxNodeType::CONSTANT:
			case logic::equational::SyntaxNodeType::FUNC:
			{
				auto iter = std::find(symb_ids.begin(),
					symb_ids.end(), subexpr.root_id());

				ATP_CORE_PRECOND(iter != symb_ids.end());

				const size_t idx = std::distance(symb_ids.begin(),
					iter);

				result[i].push_back(idx);
			}
			break;
			// do nothing for EQ nodes
			}
		}
	}

	return result;
}


HMMStmtToObs::HMMStmtToObs(
	const logic::ModelContextPtr& p_ctx,
	std::vector<size_t> symb_ids) :
	m_ctx(p_ctx), m_symb_ids(std::move(symb_ids))
{ }


std::vector<std::vector<size_t>> HMMStmtToObs::convert(
	const logic::StatementArrayPtr& p_stmts) const
{
	ATP_CORE_PRECOND(p_stmts != nullptr);

	if (auto p_arr = dynamic_cast<
		const logic::equational::StatementArray*>(p_stmts.get()))
	{
		return convert_equational_logic(*p_arr, m_symb_ids);
	}
	else
	{
		ATP_CORE_LOG(fatal) << "Bad logic type! (Perhaps the logic "
			"library was updated and the other libraries were not "
			"updated?";
		ATP_CORE_ASSERT(false && "Bad logic type!");
		throw std::exception();
	}
}


}  // namespace core
}  // namespace atp


