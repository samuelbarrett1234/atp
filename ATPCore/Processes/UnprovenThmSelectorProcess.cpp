/**
\file

\author Samuel Barrett

*/


#include <sstream>
#include "UnprovenThmSelectorProcess.h"
#include "QueryProcess.h"


namespace atp
{
namespace core
{


class UnprovenThmSelectProc :
	public QueryProcess
{
public:
	UnprovenThmSelectProc(size_t N,
		proc_data::ProofSetupEssentials& data) :
		m_num_to_load(N), m_data(data),
		QueryProcess(data.db)
	{
		ATP_CORE_LOG(trace) << "Creating process to load " << N <<
			" unproven theorems from the database...";
	}

private:
	db::QueryBuilderPtr create_query() override
	{
		ATP_CORE_LOG(trace) << "Setting up unproven theorem loader "
			"query...";

		auto _p_bder = m_data.db->create_query_builder(
			atp::db::QueryBuilderType::RANDOM_THM_SELECTION);

		auto p_bder = dynamic_cast<
			atp::db::IRndThmSelectQryBder*>(_p_bder.get());

		ATP_CORE_ASSERT(p_bder != nullptr);

		p_bder->set_limit(m_num_to_load)
			->set_context(m_data.ctx_id, m_data.ctx)
			->set_proven(false);  // we want unproven statements

		return _p_bder;
	}

	void on_load_values(db::IQueryTransaction& query) override
	{
		ATP_CORE_ASSERT(query.arity() == 1);

		atp::db::DValue stmt_str;

		if (!query.try_get(0, atp::db::DType::STR,
			&stmt_str))
		{
			ATP_CORE_LOG(warning) << "Encountered non-string "
				<< "statement value in database. Type "
				<< "could be null?";
		}
		else
		{
			ATP_CORE_LOG(trace) << "Adding '"
				<< atp::db::get_str(stmt_str)
				<< "' as a target to try and prove.";

			m_loaded_thms << atp::db::get_str(stmt_str)
				<< std::endl;
		}
	}

	void on_finished() override
	{
		ATP_CORE_LOG(trace) << "Finishing loading of proof data...";

		// construct theorems...
		m_data.target_thms =
			m_data.lang->deserialise_stmts(
				m_loaded_thms, logic::StmtFormat::TEXT,
				*m_data.ctx);

		if (m_data.target_thms == nullptr)
		{
			force_fail();
			return;
		}
		else if (m_data.target_thms->size() > 0)
		{
			ATP_CORE_LOG(info) << "Proof Process "
				"selected " << m_data.target_thms->size()
				<< " theorems from the theorem database to try and "
				"prove.";
		}
		else
		{
			ATP_CORE_LOG(error) <<
				"Proof process could not find any "
				"theorems to load from the database as targets.";

			force_fail();
		}
	}

	void on_failed() override
	{
		ATP_CORE_LOG(error) << "Query to get target theorems "
			"failed. Bailing...";
	}

private:
	std::stringstream m_loaded_thms;
	size_t m_num_to_load;
	proc_data::ProofSetupEssentials& m_data;
};


ATP_CORE_API ProcessPtr create_unproven_thm_select_proc(
	size_t num_to_load, proc_data::ProofSetupEssentials& input,
	proc_data::ProofSetupEssentials& output)
{
	// check all the setup data is valid:
	ATP_CORE_PRECOND(input.db != nullptr);
	ATP_CORE_PRECOND(input.lang != nullptr);
	ATP_CORE_PRECOND(input.ctx != nullptr);
	ATP_CORE_PRECOND(input.target_thms == nullptr);

	// start by moving all the data across
	output = std::move(input);

	return std::make_shared<UnprovenThmSelectProc>(num_to_load, output);
}


}  // namespace core
}  // namespace atp


