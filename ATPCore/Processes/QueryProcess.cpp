/**
\file

\author Samuel Barrett

*/


#include "QueryProcess.h"


namespace atp
{
namespace core
{


QueryProcess::QueryProcess(const db::DatabasePtr& p_db) :
	m_done(false), m_failed(false), m_db(p_db)
{
}


void QueryProcess::run_step()
{
	ATP_CORE_PRECOND(!m_done);
	ATP_CORE_PRECOND(!waiting());

	if (m_db_op)
	{
		switch (m_db_op->state())
		{
		case db::TransactionState::RUNNING:
		{
			auto p_query = dynamic_cast<db::IQueryTransaction*>(
				m_db_op.get());
			if (p_query != nullptr && p_query->has_values())
			{
				ATP_CORE_LOG(debug) << "Extracting values from query...";

				on_load_values(*p_query);
			}
			m_db_op->step();
		}
		break;

		case db::TransactionState::COMPLETED:
			ATP_CORE_ASSERT(!m_failed);
			ATP_CORE_LOG(debug) << "Completed database transaction.";

			// this may call force_fail()
			on_finished();

			m_done = true;
			m_db_op.reset();

			if (m_failed)
			{
				ATP_CORE_LOG(warning) << "Query process child failed"
					" to complete operation, so we're bailing...";
			}
			break;

		case db::TransactionState::FAILED:
			ATP_CORE_LOG(error) << "Database transaction failed, "
				"query process bailing...";
			on_failed();
			m_done = true;
			m_failed = true;
			m_db_op.reset();
			break;
		}
	}
	else
	{
		ATP_CORE_LOG(debug) << "Building query for query "
			"process...";

		const auto qb = create_query();

		ATP_CORE_LOG(debug) << "Obtained query builder.";

		const auto q = qb->build();

		ATP_CORE_LOG(debug) << "Built query \"" << q << "\".";

		m_db_op = m_db->begin_transaction(q);

		if (m_db_op != nullptr)
		{
			ATP_CORE_LOG(debug) << "Successfully built query!";
		}
		else
		{
			ATP_CORE_LOG(error) << "Failed to build query \""
				<< q << "\", query process terminating...";
			m_done = true;
		}
	}
}


}  // namespace core
}  // namespace atp


