/**
\file

\author Samuel Barrett

*/


#include "FixedSelectionStrategy.h"
#include "../ATPSearchLog.h"


namespace atp
{
namespace search
{


FixedSelectionStrategy::FixedSelectionStrategy(
	const logic::LanguagePtr& p_lang,
	const logic::ModelContextPtr& p_ctx, size_t ctx_id,
	size_t n) :
	m_lang(p_lang), m_ctx(p_ctx), m_ctx_id(ctx_id), m_num_thms(n)
{
	ATP_SEARCH_PRECOND(m_lang != nullptr);
	ATP_SEARCH_PRECOND(m_ctx != nullptr);
}


void FixedSelectionStrategy::set_targets(const logic::StatementArrayPtr& p_targets)
{ /* do nothing */ }


db::QueryBuilderPtr FixedSelectionStrategy::create_getter_query(const db::DatabasePtr& p_db)
{
	ATP_SEARCH_PRECOND(p_db != nullptr);

	auto _p_bder = p_db->create_query_builder(
		atp::db::QueryBuilderType::RANDOM_THM_SELECTION);

	auto p_bder = dynamic_cast<
		atp::db::IRndThmSelectQryBder*>(_p_bder.get());

	ATP_SEARCH_ASSERT(p_bder != nullptr);

	p_bder->set_limit(m_num_thms)
		->set_context(m_ctx_id, m_ctx)
		->set_proven(true);  // DEFINITELY load proven statements!

	return _p_bder;
}


void FixedSelectionStrategy::load_values(const db::IQueryTransaction& query)
{
	ATP_SEARCH_PRECOND(query.has_values());

	if (query.arity() != 1)
	{
		ATP_SEARCH_LOG(error) << "Failed to initialise proof."
			" Kernel initialisation query returned an arity "
			"of " << query.arity() << ", which "
			"differed from the expected result of 1.";
	}
	else
	{
		db::DValue stmt_str;

		if (!query.try_get(0, atp::db::DType::STR,
			&stmt_str))
		{
			ATP_SEARCH_LOG(warning) << "Encountered non-string "
				<< "statement value in database. Type "
				<< "could be null?";
		}
		else
		{
			ATP_SEARCH_LOG(trace) << "Adding '"
				<< atp::db::get_str(stmt_str)
				<< "' to helper theorems.";

			m_cur_thms << atp::db::get_str(stmt_str)
				<< std::endl;
		}
	}
}


logic::StatementArrayPtr FixedSelectionStrategy::done()
{
	return m_lang->deserialise_stmts(m_cur_thms,
		logic::StmtFormat::TEXT, *m_ctx);
}


}  // namespace search
}  // namespace atp


