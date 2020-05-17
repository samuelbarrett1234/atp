/**
\file

\author Samuel Barrett

*/


#include <queue>
#include <numeric>
#include <sstream>
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

		auto _p_bder = m_setup_data.db->create_query_builder(
			atp::db::QueryBuilderType::RANDOM_THM_SELECTION);

		auto p_bder = dynamic_cast<
			atp::db::IRndThmSelectQryBder*>(_p_bder.get());

		ATP_CORE_ASSERT(p_bder != nullptr);

		p_bder->set_limit(m_setup_data.settings.num_helper_thms
			* m_setup_data.settings.helper_thms_factor  /* get extra */)
			->set_context(m_setup_data.ctx_id, m_setup_data.ctx)
			->set_proven(true);  // DEFINITELY load proven statements!

		return _p_bder;
	}

	void on_load_values(db::IQueryTransaction& query) override
	{
		if (query.arity() != 1)
		{
			ATP_CORE_LOG(error) << "Failed to initialise proof."
				" Kernel initialisation query returned an arity "
				"of " << query.arity() << ", which "
				"differed from the expected result of 1. "
				"Bailing...";
			force_fail();
		}
		else
		{
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
					<< "' to helper theorems.";

				m_helper_thms_stream << atp::db::get_str(stmt_str)
					<< std::endl;
			}
		}
	}

	void on_finished() override
	{
		ATP_CORE_LOG(trace) << "Finishing loading of proof data...";

		// construct theorems (more than we will use)...
		auto loaded_thms =
			m_setup_data.lang->deserialise_stmts(
				m_helper_thms_stream, logic::StmtFormat::TEXT,
				*m_setup_data.ctx);

		ATP_CORE_LOG(info) << "Proof initialisation process "
			"loaded " << ((loaded_thms != nullptr) ?
				loaded_thms->size() : 0) << " theorems from the "
			"database. Now picking the best of them...";

		// get the best of them according to edit distance heuristic
		m_proof_data.helper_thms =
			get_best_stmts(std::move(loaded_thms));

		if (m_proof_data.helper_thms == nullptr)
		{
			force_fail();
			return;
		}
		else if (m_proof_data.helper_thms->size() > 0)
		{
			ATP_CORE_LOG(info) << "Proof Process "
				"loaded and selected " <<
				m_proof_data.helper_thms->size() <<
				" theorems from the database to aid in the proof.";

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
	/**
	\brief Get the best theorems (according to the edit distance
		heuristic) from the given array
	*/
	logic::StatementArrayPtr get_best_stmts(
		logic::StatementArrayPtr stmts) const
	{
		// in these cases we don't want to do any trimming
		if (stmts == nullptr || stmts->size() <=
			m_setup_data.settings.num_helper_thms)
			return stmts;

		auto p_ed = create_edit_distance_obj();

		// compute edit distance between all the pairs
		const auto distance_matrix =
			p_ed->edit_distance(*stmts,
				*m_setup_data.target_thms);

		// pick best members:
		std::vector<float> utilities;
		utilities.resize(stmts->size(), 0.0f);
		for (size_t i = 0; i < utilities.size(); ++i)
		{
			// sum up inverses of utilities (use inverse so that
			// bigger distance means smaller utility)
			for (float x : distance_matrix[i])
			{
				// higher distance means worse, so use negative
				// (note that x may be negative, as edit distance
				// isn't a distance metric, rather just a distance-
				// inspired heuristic)
				utilities[i] += -x;
			}
		}

		// obtain top m_setup_data.settings.num_helper_thms elements
		// using a priority queue
		std::priority_queue<std::pair<float, size_t>> q;
		for (size_t i = 0; i < utilities.size(); ++i)
		{
			q.emplace(utilities[i], i);
		}

		// extract out the best elements
		std::vector<logic::StatementArrayPtr> singletons;
		singletons.reserve(m_setup_data.settings.num_helper_thms);
		for (size_t i = 0; i < m_setup_data.settings.num_helper_thms;
			++i)
		{
			auto [util, k] = q.top();
			q.pop();

			ATP_CORE_LOG(trace) << "Selected \"" <<
				stmts->at(k).to_str() << "\" as a good statement to "
				"help prove the target theorems (utility was "
				<< util << ").";

			singletons.emplace_back(stmts->slice(k, k + 1));
		}
		return logic::concat(singletons);
	}

	stats::EditDistancePtr create_edit_distance_obj() const
	{
		// TEMP (todo: don't specialise to equational logic)
		return stats::create_edit_dist(
			logic::LangType::EQUATIONAL_LOGIC,
			0.1f * m_setup_data.settings.ed_symb_mismatch_cost,
			m_setup_data.settings.ed_symb_mismatch_cost);
	}

private:
	std::stringstream m_helper_thms_stream;

	proc_data::ProofSetupEssentials& m_setup_data;
	proc_data::ProofEssentials& m_proof_data;
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


