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

	std::stringstream query_builder;

	for (size_t i = 0; i < (*m_ctx)->num_axioms(); ++i)
	{
		// WARNING: the axiom may be in a form which uses bad
		// variable names (like x and y vs x0 and x1), and a
		// quick and dirty solution is to serialise it and then
		// deserialise it, putting it into a more standardised
		// form!

		std::stringstream s((*m_ctx)->axiom_at(i));
		auto p_stmts = (*m_lang)->deserialise_stmts(s,
			atp::logic::StmtFormat::TEXT, *(*m_ctx));
		ATP_DATABASE_ASSERT(p_stmts != nullptr);
		const std::string ax_str = p_stmts->at(0).to_str();

		query_builder << "INSERT OR IGNORE INTO theorems(stmt, ctx) "
			<< "VALUES ( '" << ax_str << "', " << *m_ctx_id << ");\n\n";

		query_builder << "INSERT OR IGNORE INTO proofs(thm_id, "
			<< "is_axiom) VALUES ((SELECT id FROM theorems WHERE stmt='"
			<< ax_str << "'), 1);\n\n";
	}

	return query_builder.str();
}


}  // namespace db
}  // namespace atp


