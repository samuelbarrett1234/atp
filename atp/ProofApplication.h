#pragma once


#include <string>
#include <vector>
#include <ATPLogic.h>


class ProofApplication
{
public:
	// sets the language and knowledge kernel and returns true iff
	// success (if false then the object's state will remain
	// unchanged.)
	bool set_context_file(std::string path);

	// add a new set of target statements to the collection of proof
	// tasks. this can either be a path to a file containing a list
	// of statements, or it can be a statement itself.
	bool add_proof_task(std::string path_or_stmt);

	// returns, for each statement we were asked to prove, a pair
	// containing (target-statement, proof-if-found) as strings. If
	// we could not find a proof, we say just that (in the second
	// element of the tuple.) Writes textual output to the given
	// output_stream.
	std::vector<std::pair<std::string, std::string>> run(
		std::ostream& output_stream);

private:
	atp::logic::LanguagePtr m_pLang;
	atp::logic::KnowledgeKernelPtr m_pKnowledgeKernel;
	std::vector<atp::logic::StatementArrayPtr> m_pTasks;
};


