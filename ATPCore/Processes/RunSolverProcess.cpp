/**
\file

\author Samuel Barrett

*/


#include "RunSolverProcess.h"


namespace atp
{
namespace core
{


class RunSolverProcess :
	public IProcess
{
public:
	RunSolverProcess(proc_data::ProofEssentials& pf_data) :
		m_pf_data(pf_data)
	{
		ATP_CORE_LOG(trace) << "Creating Run Solver process...";
	}

	inline bool done() const override
	{
		return !m_pf_data.solver->any_proof_not_done() ||
			m_cur_step == m_pf_data.max_steps;
	}
	inline bool waiting() const override
	{
		return false;
	}
	inline bool has_failed() const override
	{
		return false;
	}

	void run_step() override
	{
		ATP_CORE_PRECOND(!done());

		m_pf_data.solver->step(m_pf_data.step_size);
		++m_cur_step;

		// output

		const auto states = m_pf_data.solver->get_states();
		ATP_CORE_LOG(info) << "Proof Process: "
			<< "Step " << m_cur_step << '/'
			<< m_pf_data.max_steps << " : " <<
			std::count(states.begin(), states.end(),
				atp::logic::ProofCompletionState::UNFINISHED) <<
			" proof(s) remaining.";

		if (done())
		{
			handle_done();
		}
	}
	
private:
	void handle_done()
	{
		ATP_CORE_LOG(trace) << "Finished running solver, for "
			<< m_cur_step << "/" << m_pf_data.max_steps << " steps.";

		// output:

		// count the number successful / failed / unfinished:
		const auto states = m_pf_data.solver->get_states();
		size_t num_true = 0, num_failed = 0,
			num_unfinished = 0;
		for (auto st : states)
			switch (st)
			{
			case atp::logic::ProofCompletionState::PROVEN:
				++num_true;
				break;
			case atp::logic::ProofCompletionState::NO_PROOF:
				++num_failed;
				break;
			case atp::logic::ProofCompletionState::UNFINISHED:
				++num_unfinished;
				break;
			}

		ATP_CORE_LOG(info)
			<< "Proof process finished proving! Results:"
			<< std::endl
			<< '\t' << num_true << " theorem(s) were proven true,"
			<< std::endl
			<< '\t' << num_failed << " theorem(s) have no proof,"
			<< std::endl
			<< '\t' << num_unfinished <<
			" theorem(s) did not finish in the allotted time.";

		auto proofs = m_pf_data.solver->get_proofs();
		auto times = m_pf_data.solver->get_agg_time();
		auto mems = m_pf_data.solver->get_max_mem();
		auto exps = m_pf_data.solver->get_num_expansions();

		for (size_t i = 0; i < proofs.size(); i++)
		{
			switch (states[i])
			{
			case atp::logic::ProofCompletionState::PROVEN:
				ATP_CORE_LOG(trace) << "Proof of \""
					<< m_pf_data.target_thms->at(i).to_str()
					<< "\" was successful; the statement is true.";
				break;
			case atp::logic::ProofCompletionState::NO_PROOF:
				ATP_CORE_LOG(trace) << "Proof of \""
					<< m_pf_data.target_thms->at(i).to_str()
					<< "\" was unsuccessful; it was impossible to "
					"prove using the given solver and the current "
					"settings.";
				break;
			case atp::logic::ProofCompletionState::UNFINISHED:
				ATP_CORE_LOG(trace) << "Proof of \""
					<< m_pf_data.target_thms->at(i).to_str()
					<< "\" was unsuccessful; not enough time "
					"allocated.";
				break;
			}

			ATP_CORE_LOG(trace) << "\tTotal time taken: " <<
				times[i] << "s";
			ATP_CORE_LOG(trace) << "\tMax nodes in memory: " <<
				mems[i];
			ATP_CORE_LOG(trace) << "\tTotal node expansions: " <<
				exps[i];
		}
	}

private:
	size_t m_cur_step;
	proc_data::ProofEssentials& m_pf_data;
};


ProcessPtr create_run_solver_process(
	proc_data::ProofEssentials& proof_data_before,
	proc_data::ProofEssentials& proof_data_after)
{
	// check all the data is valid:
	ATP_CORE_PRECOND(proof_data_before.db != nullptr);
	ATP_CORE_PRECOND(proof_data_before.lang != nullptr);
	ATP_CORE_PRECOND(proof_data_before.ctx != nullptr);
	ATP_CORE_PRECOND(proof_data_before.target_thms != nullptr);
	ATP_CORE_PRECOND(proof_data_before.ker != nullptr);
	ATP_CORE_PRECOND(proof_data_before.solver != nullptr);
	ATP_CORE_PRECOND(proof_data_before.helper_thms != nullptr);
	ATP_CORE_PRECOND(proof_data_before.max_steps > 0);
	ATP_CORE_PRECOND(proof_data_before.step_size > 0);

	// copy data over, it stays the same
	if (&proof_data_before != &proof_data_after)
		proof_data_after = proof_data_before;

	return std::make_shared<RunSolverProcess>(
		proof_data_after);
}


}  // namespace core
}  // namespace atp


