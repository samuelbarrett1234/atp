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


class ATP_DATABASE_API SQLiteCheckAxInDbQryBder :
	public ICheckAxInDbQryBder
{
public:
	std::string build() override;
	inline ICheckAxInDbQryBder* set_ctx(
		size_t ctx_id,
		const logic::ModelContextPtr& p_ctx) override
	{
		ATP_DATABASE_PRECOND(p_ctx != nullptr);
		m_ctx_id = ctx_id;
		m_ctx = p_ctx;
		return this;
	}
	inline ICheckAxInDbQryBder* set_lang(
		const logic::LanguagePtr& p_lang) override
	{
		ATP_DATABASE_PRECOND(p_lang != nullptr);
		m_lang = p_lang;
		return this;
	}
	inline ICheckAxInDbQryBder* reset() override
	{
		m_ctx_id = boost::none;
		m_ctx = boost::none;
		m_lang = boost::none;
		return this;
	}

private:
	boost::optional<size_t> m_ctx_id;
	boost::optional<logic::ModelContextPtr> m_ctx;
	boost::optional<logic::LanguagePtr> m_lang;
};


}
}