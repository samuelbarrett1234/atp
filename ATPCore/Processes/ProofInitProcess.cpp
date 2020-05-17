/**
\file

\author Samuel Barrett

*/


#include <ATPSearch.h>
#include "ProofInitProcess.h"
#include "QueryProcess.h"
#include "ATPStatsEditDistance.h"


namespace atp
{
namespace core
{


/**
\brief This process transfers data from ProofSetupEssentials to
	ProofEssentials, and loads "helper theorems" from the database
	as well.

\details To summarise, it: creates the knowledge kernel, creates
	the solver, loads the helper theorems from the database, and then
	hands over the ProofEssentials data.
*/
class ProofInitProcess :
	public QueryProcess
{
public:
	ProofInitProcess(
		proc_data::ProofSetupEssentials& setup_data,
		proc_data::ProofEssentials& proof_data) :
		QueryProcess(setup_data.db),
		m_setup_data(setup_data), m_proof_data(proof_data)
	{
		ATP_CORE_LOG(trace) << "Creating proof initialisation "
			"process...";

		ATP_CORE_LOG(trace) << "Creating knowledge kernel...";
		m_proof_data.ker = m_setup_data.lang->try_create_kernel(
			*m_setup_data.ctx);

		if (m_proof_data.ker == nullptr)
		{
			ATP_CORE_LOG(error) << "Failed to create knowledge "
				"kernel from model context \"" <<
				m_setup_data.ctx->context_name() << "\". Check "
				"syntax and types of the axioms in the file.";
			force_fail();
			return;
		}

		m_proof_data.ker->set_seed(m_setup_data.settings.seed);
		m_proof_data.ss_id = m_setup_data.ss_id;
		m_proof_data.max_steps = m_setup_data.settings.max_steps;
		m_proof_data.step_size = m_setup_data.settings.step_size;
		m_proof_data.target_thms = m_setup_data.target_thms;
		
		ATP_CORE_LOG(trace) << "Creating solver...";
		m_proof_data.solver =
			m_setup_data.settings.create_solver(
			m_proof_data.ker);

		if (m_proof_data.solver == nullptr)
		{
			ATP_CORE_LOG(error) << "Failed to construct solver "
				"object from search settings \"" <<
				m_setup_data.settings.name << "\".";

			force_fail();
			return;
		}
	}

protected:
	db::QueryBuilderPtr create_query() override
	{
		ATP_CORE_LOG(trace) << "Setting up kernel initialisation query...";

		ATP_CORE_ASSERT(m_setup_data.db != nullptr);
		return m_selection_strat->create_getter_query(m_setup_data.db);
	}

	void on_load_values(db::IQueryTransaction& query) override
	{
		ATP_CORE_PRECOND(query.has_values());
		m_selection_strat->load_values(query);
	}

	void on_finished() override
	{
		ATP_CORE_LOG(trace) << "Finishing loading of proof data...";

		m_proof_data.helper_thms = m_selection_strat->done();

		if (m_proof_data.helper_thms == nullptr)
		{
			force_fail();
			on_failed();
			return;
		}
		else if (m_proof_data.helper_thms->size() > 0)
		{
			ATP_CORE_LOG(info) << "Proof initialisation process "
				"loaded " << ((m_proof_data.helper_thms != nullptr) ?
					m_proof_data.helper_thms->size() : 0) << " theorems from the "
				"database.";

			m_proof_data.ker->add_theorems(m_proof_data.helper_thms);
		}
		else ATP_CORE_LOG(warning) <<
			"Proof process could not find any "
			"theorems to load from the database.";

		// make sure to do this AFTER the theorems have been set up
		// in the knowledge kernel, because this constructs iterators
		// which rely on knowing what's in the kernel beforehand
		m_proof_data.solver->set_targets(m_setup_data.target_thms);

		// transfer everything over:
		static_cast<proc_data::LogicEssentials&>(m_proof_data)
			= (proc_data::LogicEssentials&)m_setup_data;
	}

	void on_failed() override
	{
		ATP_CORE_LOG(error) << "Query to initialise proof data "
			"failed. Bailing...";
	}

private:
	proc_data::ProofSetupEssentials& m_setup_data;
	proc_data::ProofEssentials& m_proof_data;
	search::SelectionStrategyPtr m_selection_strat;
};


ProcessPtr create_proof_init_process(
	proc_data::ProofSetupEssentials& setup_data,
	proc_data::ProofEssentials& proof_data)
{
	// check all the setup data is valid:
	ATP_CORE_PRECOND(setup_data.db != nullptr);
	ATP_CORE_PRECOND(setup_data.lang != nullptr);
	ATP_CORE_PRECOND(setup_data.ctx != nullptr);
	ATP_CORE_PRECOND(setup_data.target_thms != nullptr);

	return std::make_shared<ProofInitProcess>(
		setup_data, proof_data);
}


}  // namespace core
}  // namespace atp


