#pragma once


/**
\file

\author Samuel Barrett

\brief Contains a helper class for writing processes which are
	centred around running a query.
*/


#include <ATPDatabase.h>
#include "../ATPCoreAPI.h"
#include "IProcess.h"


namespace atp
{
namespace core
{


/**
\brief This process implements the IProcess virtual functions in
	terms of other virtual functions which are centred around the
	actual database operations that are ongoing, making it easier
	to implement such a process.
*/
class ATP_CORE_API QueryProcess :
	public IProcess
{
public:
	QueryProcess(const db::DatabasePtr& p_db);

	inline bool done() const override
	{
		return m_done;
	}
	inline bool has_failed() const override
	{
		return m_failed;
	}
	inline bool waiting() const override
	{
		return m_db_op != nullptr &&
			m_db_op->waiting();
	}
	void run_step() override;

protected:  // implement these functions!

	virtual std::unique_ptr<db::IQueryBuilder> create_query() = 0;
	virtual void on_load_values(db::IQueryTransaction& query) { }
	virtual void on_finished() { }
	virtual void on_failed() { }

private:
	db::DatabasePtr m_db;
	db::TransactionPtr m_db_op;
	bool m_done, m_failed;
};


}  // namespace core
}  // namespace atp


