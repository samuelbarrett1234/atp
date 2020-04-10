/**

\file

\author Samuel Barrett

*/

#include "ProofApplication.h"
#include <sstream>
#include <fstream>
#include <boost/filesystem.hpp>
#include <ATPSearch.h>
#include "ParseContextFile.h"


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

	// get the folder that the context file is in
	const auto working_folder = boost::filesystem::path(
		path).parent_path();

	auto maybe_ctx_file = parse_context_file(ctx_in);

	if (!maybe_ctx_file)
	{
		m_out << "There was a problem parsing the file \"" <<
			path << "\". Check for syntax mistakes." << std::endl;

		return false;
	}

	// else proceed loading the language and knowledge kernel etc

	// get definition file and axiom file paths relative to the
	// context file directory
	auto def_file_path = working_folder
		/ maybe_ctx_file->definition_file_path;
	auto ax_file_path = working_folder
		/ maybe_ctx_file->axiom_file_path;
	
	if (!boost::filesystem::is_regular_file(
		def_file_path))
	{
		m_out << "Error: definition file \"" <<
			def_file_path.string() <<
			"\" does not exist (as a file)." << std::endl;
		return false;
	}

	if (!boost::filesystem::is_regular_file(
		ax_file_path))
	{
		m_out << "Error: axiom file \"" <<
			ax_file_path.string() <<
			"\" does not exist (as a file)." << std::endl;
		return false;
	}

	// open the definition and axiom files
	std::ifstream def_in(def_file_path.string()),
		ax_in(ax_file_path.string());

	if (!def_in)
	{
		m_out << "There was a problem opening the file \"" <<
			maybe_ctx_file->definition_file_path <<
			"\"." << std::endl;

		return false;
	}

	if (!ax_in)
	{
		m_out << "There was a problem opening the file \"" <<
			maybe_ctx_file->axiom_file_path <<
			"\"." << std::endl;

		return false;
	}

	m_pLang = atp::logic::create_language(
		maybe_ctx_file->lang_type);

	if (m_pLang == nullptr)
	{
		m_out << "Failed to create language object." << std::endl;
		return false;
	}

	m_pKnowledgeKernel = m_pLang->create_empty_kernel();

	if (!m_pLang->load_kernel_definitions(*m_pKnowledgeKernel,
		def_in))
	{
		m_out << "There was a problem parsing the file \"" <<
			maybe_ctx_file->definition_file_path <<
			"\"." << std::endl;

		return false;
	}

	if (!m_pLang->load_kernel_axioms(*m_pKnowledgeKernel,
		ax_in))
	{
		m_out << "There was a problem parsing the file \"" <<
			maybe_ctx_file->axiom_file_path <<
			"\"." << std::endl;

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
			*m_pKnowledgeKernel);
	}
	else
	{
		std::stringstream s(path_or_stmt);

		p_stmts = m_pLang->deserialise_stmts(s,
			atp::logic::StmtFormat::TEXT,
			*m_pKnowledgeKernel);
	}

	if (!p_stmts)
	{
		m_out << "Failed to parse statements: \"" <<
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

	// concatenate all the tasks together into one big array
	const auto tasks = atp::logic::concat(m_pTasks);
	p_solver->set_targets(tasks);

	m_out << "Solver initialised; starting proofs..." << std::endl;

	// run for at most 100 iterations (temp)
	for (size_t i = 0; i < 100
		&& p_solver->any_proof_not_done(); ++i)
	{
		// 10 proof steps per update
		p_solver->step(10);

		const auto states = p_solver->get_states();
		m_out << 10 * (i + 1) << '/' << 100 * 10 << " : " <<
			std::count(states.begin(), states.end(),
				atp::search::ProofState::UNFINISHED) <<
			" proof(s) remaining." << std::endl;
	}

	// count the number successful / failed / unfinished:

	const auto states = p_solver->get_states();
	size_t num_true = 0, num_failed = 0,
		num_unfinished = 0;
	for (auto st : states)
		switch (st)
		{
		case atp::search::ProofState::DONE_TRUE:
			++num_true;
			break;
		case atp::search::ProofState::NO_PROOF:
			++num_failed;
			break;
		case atp::search::ProofState::UNFINISHED:
			++num_unfinished;
			break;
		}


	m_out << "Done! Results:" << std::endl
		<< '\t' << num_true << " theorems were proven true,"
		<< std::endl
		<< '\t' << num_failed << " theorems have no proof,"
		<< std::endl
		<< '\t' << num_unfinished <<
		" theorems did not finish in the allotted time."
		<< std::endl;

	m_out << "More details:" << std::endl;

	auto proofs = p_solver->get_proofs();
	auto times = p_solver->get_agg_time();
	auto mems = p_solver->get_max_mem();
	auto exps = p_solver->get_num_expansions();

	for (size_t i = 0; i < proofs.size(); i++)
	{
		if (states[i] == atp::search::ProofState::DONE_TRUE)
		{
			m_out << "Proof of \"" << tasks->at(i).to_str()
				<< "\" was successful." << std::endl;

			m_out << "Total time taken: " << times[i] << "ns"
				<< std::endl;
			m_out << "Max nodes in memory: " << mems[i] << "ns"
				<< std::endl;
			m_out << "Total node expansions: " << exps[i] << "ns"
				<< std::endl;
				
			m_out << "Proof:" << std::endl;

			auto pf = *proofs[i];
			for (size_t j = 0; j < pf->size(); ++j)
			{
				m_out << pf->at(j).to_str() << std::endl;
			}
			m_out << std::endl;
		}
	}
}


