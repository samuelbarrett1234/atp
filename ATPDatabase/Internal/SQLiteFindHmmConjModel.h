#pragma once


/**
\file

\author Samuel Barrett

\brief Contains a query builder for finding HMM conjecturer models

*/


#include <boost/optional.hpp>
#include "../ATPDatabaseAPI.h"
#include "../Interfaces/IQueryBuilders.h"


namespace atp
{
namespace db
{


class SQLiteFindHmmConjModel :
	public IFindHmmConjModel
{
public:
	std::string build() const;
	inline IFindHmmConjModel* reset() override
	{
		m_ctx_id.reset();
		m_model_id.reset();
		m_ctx.reset();
		return this;
	}
	inline IFindHmmConjModel* set_model_id(size_t mid) override
	{
		m_model_id = mid;
		return this;
	}
	inline IFindHmmConjModel* set_ctx(size_t ctx_id,
		const logic::ModelContextPtr& p_ctx) override
	{
		m_ctx_id = ctx_id;
		m_ctx = p_ctx;
	}

private:
	boost::optional<size_t> m_ctx_id,
		m_model_id;  // this one is actually optional
	boost::optional<logic::ModelContextPtr> m_ctx;
};


}  // namespace db
}  // namespace atp


