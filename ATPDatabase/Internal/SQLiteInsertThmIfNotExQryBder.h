#pragma once


/**
\file

\author Samuel Barrett

\brief A query builder for inserting a theorem (if it doesn't
	already exist)

*/


#include <boost/optional.hpp>
#include "../ATPDatabaseAPI.h"
#include "../Interfaces/IQueryBuilders.h"


namespace atp
{
namespace db
{


class ATP_DATABASE_API SQLiteInsertThmIfNotExQryBder :
	public IInsertThmIfNotExQryBder
{
public:
	std::string build() override;
	inline IInsertThmIfNotExQryBder* set_thm(
		const std::string& thm) override
	{
		m_thm = thm;
		return this;
	}
	inline IInsertThmIfNotExQryBder* set_ctx(
		size_t ctx_id) override
	{
		m_ctx_id = ctx_id;
		return this;
	}
	inline IInsertThmIfNotExQryBder* reset() override
	{
		m_ctx_id = boost::none;
		m_thm = boost::none;
	}

private:
	boost::optional<size_t> m_ctx_id;
	boost::optional<std::string> m_thm;
};


}
}