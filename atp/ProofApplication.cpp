/**

\file

\author Samuel Barrett

*/


#include "ProofApplication.h"
#include <sstream>
#include <fstream>
#include <boost/filesystem.hpp>
#include <ATPSearch.h>
#include "ATP.h"


ProofApplication::ProofApplication(std::ostream& out) :
	m_out(out),
	m_max_steps(10),
	m_step_size(1000)
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
	m_lang = atp::logic::create_language(
		atp::logic::LangType::EQUATIONAL_LOGIC);

	if (m_lang == nullptr)
	{
		m_out << "Failed to create language object." << std::endl;
		return false;
	}

	m_ctx = m_lang->try_create_context(ctx_in);

	// if we fail here, it's because the JSON parsing failed
	if (!m_ctx)
	{
		m_out << "There was a problem loading the file \"" <<
			path << "\". Check for JSON syntax mistakes." << std::endl;

		return false;
	}

	m_ker = m_lang->try_create_kernel(*m_ctx);

	// if we fail here, it's because the logical syntax checking
	// returned false - note that the axioms aren't checked for
	// syntax errors until we try to create the kernel.
	if (!m_ker)
	{
		m_out << "There was a problem loading the file \"" <<
			path << "\". Check for Statement syntax mistakes." << std::endl;

		return false;
	}

	m_out << "Successfully loaded the context file \"" << 
		m_ctx->context_name() << "\"." << std::endl;
	return true;  // success
}


bool ProofApplication::set_db(std::string path)
{
	ATP_PRECOND(m_solver == nullptr);

	std::ifstream in(path);

	if (!in)
	{
		m_out << "Could not open the database configuration file "
			<< '"' << path << '"' << std::endl;
		return false;
	}

	m_db = atp::db::load_from_config_file(in);

	if (m_db == nullptr)
	{
		m_out << "Failed to create the database! Check the database "
			<< "configuration file at \"" << path << "\" is correct "
			<< "and that none of the files it references have been "
			<< "deleted or moved." << std::endl;
		return false;
	}

	return true;
}


bool ProofApplication::set_search_file(std::string path)
{
	ATP_PRECOND(m_ker != nullptr);

	if (!boost::filesystem::is_regular_file(path))
	{
		m_out << "Error: search settings file \"" << path <<
			"\" does not exist (as a file)." << std::endl;
		return false;
	}

	std::ifstream in(path);

	if (!in)
	{
		m_out << "There was a problem opening the file \"" <<
			path << "\"." << std::endl;

		return false;
	}

	atp::search::SearchSettings settings;

	if (!atp::search::load_search_settings(m_ker,
		in, &settings))
	{
		m_out << "There was a problem parsing the file \"" <<
			path << "\"." << std::endl;
		return false;
	}

	// now extract information from the `settings` object:

	if (settings.p_solver == nullptr)
	{
		// use the default solver

		m_solver = atp::search::create_default_solver(
			m_ker);
	}
	else
	{
		m_solver = settings.p_solver;
	}

	m_max_steps = settings.max_steps;
	m_step_size = settings.step_size;

	m_ker->set_seed(settings.seed);

	m_out << "Successfully loaded search settings \"" <<
		settings.name << "\"" << std::endl;

	return true;  // success
}


bool ProofApplication::add_proof_task(std::string path_or_stmt)
{
	if (!m_lang || !m_ker)
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

		p_stmts = m_lang->deserialise_stmts(in,
			atp::logic::StmtFormat::TEXT,
			*m_ctx);
	}
	else
	{
		std::stringstream s(path_or_stmt);

		p_stmts = m_lang->deserialise_stmts(s,
			atp::logic::StmtFormat::TEXT,
			*m_ctx);
	}

	if (!p_stmts)
	{
		m_out << "Failed to load statements: \"" <<
			path_or_stmt << "\". Check syntax and types."
			<< std::endl;

		return false;
	}

	m_tasks.push_back(p_stmts);

	return true;
}


void ProofApplication::run()
{
	// concatenate all the tasks together into one big array
	const auto tasks = atp::logic::concat(m_tasks);
	m_solver->set_targets(tasks);

	m_out << "Solver initialised; starting proofs..." << std::endl;

	for (size_t i = 0; i < m_max_steps
		&& m_solver->any_proof_not_done(); ++i)
	{
		m_solver->step(m_step_size);

		const auto states = m_solver->get_states();
		m_out << (i + 1) << '/'
			<< m_max_steps << " : " <<
			std::count(states.begin(), states.end(),
				atp::logic::ProofCompletionState::UNFINISHED) <<
			" proof(s) remaining." << std::endl;
	}

	// count the number successful / failed / unfinished:

	const auto states = m_solver->get_states();
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

	auto proofs = m_solver->get_proofs();
	auto times = m_solver->get_agg_time();
	auto mems = m_solver->get_max_mem();
	auto exps = m_solver->get_num_expansions();

	for (size_t i = 0; i < proofs.size(); i++)
	{
		switch (states[i])
		{
		case atp::logic::ProofCompletionState::PROVEN:
			m_out << "Proof of \"" << tasks->at(i).to_str()
				<< "\" was successful; the statement is true."
				<< std::endl << "Proof:" << std::endl
				<< proofs[i]->to_str() << std::endl << std::endl;
			break;
		case atp::logic::ProofCompletionState::NO_PROOF:
			m_out << "Proof of \"" << tasks->at(i).to_str()
				<< "\" was unsuccessful; it was impossible to prove "
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

		m_out << std::endl;
	}

	// reset its state
	m_solver->clear();
}


