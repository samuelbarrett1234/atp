/**
\file

\author Samuel Barrett

*/


#include <sstream>
#include "SQLiteCheckAxInDbQryBder.h"


namespace atp
{
namespace db
{


std::string SQLiteCheckAxInDbQryBder::build()
{
	ATP_DATABASE_PRECOND(m_ctx_id.has_value());
	ATP_DATABASE_PRECOND(m_ctx.has_value());
	ATP_DATABASE_PRECOND(m_lang.has_value());

	// WARNING: the axiom may be in a form which uses bad
	// variable names (like x and y vs x0 and x1), and a
	// quick and dirty solution is to serialise it and then
	// deserialise it, putting it into a more standardised
	// form!

	std::stringstream ax_builder;

	for (size_t i = 0; i < (*m_ctx)->num_axioms(); ++i)
	{
		ax_builder << (*m_ctx)->axiom_at(i) << "\n";
	}

	auto deserialised_stmts = (*m_lang)->deserialise_stmts(
		ax_builder, logic::StmtFormat::TEXT, *(*m_ctx));

	if (deserialised_stmts == nullptr)
	{
		ATP_DATABASE_LOG(error) << "There was a problem loading the "
			"axioms - perhaps there was a typo in them? The axioms "
			"were: \"" << ax_builder.str() << "\".";
		return "";
	}

	auto normed_stmts = (*m_lang)->normalise(deserialised_stmts);

	std::stringstream query_builder;
	for (size_t i = 0; i < (*m_ctx)->num_axioms(); ++i)
	{
		const std::string ax_str = normed_stmts->at(0).to_str();

		query_builder << "INSERT OR IGNORE INTO theorems(stmt, ctx_id) "
			"VALUES ( '" << ax_str << "', " << *m_ctx_id << ");\n\n";

		query_builder << "INSERT OR IGNORE INTO proofs(thm_id, "
			"is_axiom) VALUES ((SELECT thm_id FROM theorems WHERE stmt='"
			<< ax_str << "' AND ctx_id = " << *m_ctx_id <<
			"), 1);\n\n";
	}

	return query_builder.str();
}


}  // namespace db
}  // namespace atp


