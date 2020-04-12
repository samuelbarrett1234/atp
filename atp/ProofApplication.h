#pragma once


/**

\file

\author Samuel Barrett

\brief Contains the simple command line application for doing a proof

*/


#include <string>
#include <vector>
#include <ATPLogic.h>


/**

\brief The simple command-line application for proving a small set of
    statements.

\details This class encapsulates the following use case: "given a
    logical language, and a context, try to prove a given set of
	statements all in one go." It does not store the results in the
	database; this application is intended to be quick and simple.
*/
class ProofApplication
{
public:
	/**
	\param out The stream to write all application text output.
	*/
	ProofApplication(std::ostream& out);

	/**
	\brief Loads the language and knowledge kernel from a given
	    context file
		
	\details A context file specifies the language, the
		location of the definition file, and the location of the
		axiom file.

	\returns True iff success.
	*/
	bool set_context_file(std::string path);

	// add a new set of target statements to the collection of proof
	// tasks. this can either be a path to a file containing a list
	// of statements, or it can be a statement itself.
	/**
	\brief Add a new one/many target statements, to attempt to prove
	    when `run` is called.

	\param path_or_stmt Can either be a path to a file of statements
	    or can just be a list of statements represented as a string
		(in both cases, multiple statements should be line-separated)

	\returns True iff success.

	\pre Must've already set the context file successfully.

	*/
	bool add_proof_task(std::string path_or_stmt);

	/**
	\brief Run all proofs that have been set.
	*/
	void run();

private:
	std::ostream& m_out;
	atp::logic::LanguagePtr m_pLang;
	atp::logic::ModelContextPtr m_pContext;
	atp::logic::KnowledgeKernelPtr m_pKnowledgeKernel;
	std::vector<atp::logic::StatementArrayPtr> m_pTasks;
};


