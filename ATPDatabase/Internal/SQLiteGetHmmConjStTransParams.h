#pragma once


/**
\file

\author Samuel Barrett

\brief Contains a query builder for getting the HMM conjecturer
	model state transition parameters.

*/


#include <boost/optional.hpp>
#include "../ATPDatabaseAPI.h"
#include "../Interfaces/IQueryBuilders.h"


namespace atp
{
namespace db
{


class SQLiteGetHmmConjStTransParams :
	public IGetHmmConjectureModelParams
{
public:
	std::string build() const;
	inline IGetHmmConjectureModelParams* reset() override
	{
		m_ctx_id.reset();
		m_model_id.reset();
		m_ctx.reset();
		return this;
	}
	inline IGetHmmConjectureModelParams* set_model_id(
		size_t mid) override
	{
		m_model_id = mid;
		return this;
	}
	inline IGetHmmConjectureModelParams* set_ctx(size_t ctx_id,
		const logic::ModelContextPtr& p_ctx) override
	{
		m_ctx_id = ctx_id;
		m_ctx = p_ctx;
	}

private:
	boost::optional<size_t> m_ctx_id,
		m_model_id;
	boost::optional<logic::ModelContextPtr> m_ctx;
};


}  // namespace db
}  // namespace atp


