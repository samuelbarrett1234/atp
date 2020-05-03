#pragma once


/**
\file

\author Samuel Barrett

\brief Implementation of IRndProvenThmSelectQryBuilder for SQLite

*/


#include <boost/optional.hpp>
#include "../ATPDatabaseAPI.h"
#include "../Interfaces/IQueryBuilders.h"


namespace atp
{
namespace db
{


class ATP_DATABASE_API SQLiteRndProvenThmSelectQryBder :
	public IRndProvenThmSelectQryBder
{
public:
	std::string build() override;
	inline IRndProvenThmSelectQryBder* set_limit(size_t N) override
	{
		m_limit = N;
		return this;
	}
	inline IRndProvenThmSelectQryBder* set_context(size_t ctx_id,
		const logic::ModelContextPtr& p_ctx) override
	{
		ATP_DATABASE_PRECOND(p_ctx != nullptr);
		m_ctx_id = ctx_id;
		m_ctx = p_ctx;
		return this;
	}
	inline IRndProvenThmSelectQryBder* reset() override
	{
		m_limit = boost::none;
		m_ctx_id = boost::none;
		m_ctx = boost::none;
		return this;
	}

private:
	boost::optional<size_t> m_limit, m_ctx_id;
	boost::optional<logic::ModelContextPtr> m_ctx;
};


}  // namespace db
}  // namespace atp


