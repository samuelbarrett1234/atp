/**

\file

\author Samuel Barrett

*/

#include "ProofApplication.h"
#include <sstream>
#include <fstream>
#include <boost/filesystem.hpp>
#include <ATPSearch.h>


ProofApplication::ProofApplication(std::ostream& out) :
	m_out(out)
{ }


bool ProofApplication::set_context_file(std::string path)
{
	if (!boost::filesystem::is_regular_file(path))
	{
		m_out << "Error: context file \"" << path <<
			"\" does not exist (as a file)." << std::endl;
		return false;
	}

	std::ifstream ctx_in(path);

	if (!ctx_in)
	{
		m_out << "There was a problem opening the file \"" <<
			path << "\"." << std::endl;

		return false;
	}

	// todo: allow the user to specify this
	m_pLang = atp::logic::create_language(
		atp::logic::LangType::EQUATIONAL_LOGIC);

	if (m_pLang == nullptr)
	{
		m_out << "Failed to create language object." << std::endl;
		return false;
	}

	m_pContext = m_pLang->try_create_context(ctx_in);

	// if we fail here, it's because the JSON parsing failed
	if (!m_pContext)
	{
		m_out << "There was a problem loading the file \"" <<
			path << "\". Check for JSON syntax mistakes." << std::endl;

		return false;
	}

	m_pKnowledgeKernel = m_pLang->try_create_kernel(*m_pContext);

	// if we fail here, it's because the logical syntax checking
	// returned false - note that the axioms aren't checked for
	// syntax errors until we try to create the kernel.
	if (!m_pContext)
	{
		m_out << "There was a problem loading the file \"" <<
			path << "\". Check for Statement syntax mistakes." << std::endl;

		return false;
	}

	// we are finally done!

	return true;
}


bool ProofApplication::add_proof_task(std::string path_or_stmt)
{
	if (!m_pLang || !m_pKnowledgeKernel)
		return false;  // context file not set

	atp::logic::StatementArrayPtr p_stmts;

	if (boost::filesystem::is_regular_file(path_or_stmt))
	{
		std::ifstream in(path_or_stmt);

		if (!in)
		{
			m_out << "Could not open the file \""
				<< path_or_stmt << std::endl;
			return false;
		}

		p_stmts = m_pLang->deserialise_stmts(in,
			atp::logic::StmtFormat::TEXT,
			*m_pContext);
	}
	else
	{
		std::stringstream s(path_or_stmt);

		p_stmts = m_pLang->deserialise_stmts(s,
			atp::logic::StmtFormat::TEXT,
			*m_pContext);
	}

	if (!p_stmts)
	{
		m_out << "Failed to load statements: \"" <<
			path_or_stmt << "\". Check syntax and types."
			<< std::endl;

		return false;
	}

	m_pTasks.push_back(p_stmts);

	return true;
}


void ProofApplication::run()
{
	/**
	\todo Make the solver configurable
	*/
	auto p_solver = atp::search::create_solver(
		m_pKnowledgeKernel,
		atp::search::SolverType::ITERATIVE_DEEPENING_UNINFORMED,
		atp::search::HeuristicCollection());
	const size_t num_steps_per_update = 10000;
	const size_t max_num_updates = 10;

	// concatenate all the tasks together into one big array
	const auto tasks = atp::logic::concat(m_pTasks);
	p_solver->set_targets(tasks);

	m_out << "Solver initialised; starting proofs..." << std::endl;

	for (size_t i = 0; i < max_num_updates
		&& p_solver->any_proof_not_done(); ++i)
	{
		p_solver->step(num_steps_per_update);

		const auto states = p_solver->get_states();
		m_out << (i + 1) << '/'
			<< max_num_updates << " : " <<
			std::count(states.begin(), states.end(),
				atp::logic::ProofCompletionState::UNFINISHED) <<
			" proof(s) remaining." << std::endl;
	}

	// count the number successful / failed / unfinished:

	const auto states = p_solver->get_states();
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


	m_out << "Done! Results:" << std::endl
		<< '\t' << num_true << " theorem(s) were proven true,"
		<< std::endl
		<< '\t' << num_failed << " theorem(s) have no proof,"
		<< std::endl
		<< '\t' << num_unfinished <<
		" theorem(s) did not finish in the allotted time."
		<< std::endl;

	m_out << "More details:" << std::endl;

	auto proofs = p_solver->get_proofs();
	auto times = p_solver->get_agg_time();
	auto mems = p_solver->get_max_mem();
	auto exps = p_solver->get_num_expansions();

	for (size_t i = 0; i < proofs.size(); i++)
	{
		switch (states[i])
		{
		case atp::logic::ProofCompletionState::PROVEN:
			m_out << "Proof of \"" << tasks->at(i).to_str()
				<< "\" was successful; the statement is true."
				<< std::endl;
			break;
		case atp::logic::ProofCompletionState::NO_PROOF:
			m_out << "Proof of \"" << tasks->at(i).to_str()
				<< "\" was unsuccessful; it was impossible to prove"
				<< "using the given solver and the current settings."
				<< std::endl;
			break;
		case atp::logic::ProofCompletionState::UNFINISHED:
			m_out << "Proof of \"" << tasks->at(i).to_str()
				<< "\" was unsuccessful; not enough time allocated."
				<< std::endl;
			break;
		}

		m_out << "Total time taken: " << times[i] << "s"
			<< std::endl;
		m_out << "Max nodes in memory: " << mems[i]
			<< std::endl;
		m_out << "Total node expansions: " << exps[i]
			<< std::endl;
	}
}


