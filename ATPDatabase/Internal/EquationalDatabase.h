#pragma once


/**
\file

\author Samuel Barrett

\brief IDatabase implementation for equational logic

*/


#include <ATPLogic.h>
#include "../ATPDatabaseAPI.h"
#include "../Interfaces/IDatabase.h"


struct sqlite3;  // forward declaration


namespace atp
{
namespace db
{


/**
\brief Represents a database which handles with equational logic
	statements only.
*/
class ATP_DATABASE_API EquationalDatabase :
	public IDatabase
{
public:  // builder functions
	
	/**
	\brief Create a new database object from the given SQLite database
		filename.

	\returns A valid database object if the SQL database construction was
		successful, or nullptr if the filename was incorrect / the file
		was invalid.
	*/
	static DatabasePtr load_from_file(
		const std::string& filename);

public:
	~EquationalDatabase();

	inline std::string name() const override
	{
		return m_name;
	}
	inline std::string description() const override
	{
		return m_desc;
	}
	inline logic::LanguagePtr logic_lang() const override
	{
		return m_lang;
	}

	TransactionPtr begin_transaction(
		const std::string& query_text) override;

	boost::optional<std::string> model_context_filename(
		const std::string& model_context_name) override;

	boost::optional<size_t> model_context_id(
		const std::string& model_context_name) override;

	TransactionPtr get_theorems_for_kernel_transaction(
		size_t ctx_id,
		const logic::ModelContextPtr& p_ctx,
		const logic::StatementArrayPtr& targets) override;

	TransactionPtr finished_proof_attempt_transaction(
		size_t ctx_id,
		const logic::ModelContextPtr& p_ctx,
		const std::vector<atp::logic::ProofStatePtr>& proof_states,
		const std::vector<float>& proof_times,
		const std::vector<size_t>& max_mem_usages,
		const std::vector<size_t>& num_node_expansions) override;

private:
	std::string m_name, m_desc;
	logic::LanguagePtr m_lang;
	sqlite3* m_db;
};


}  // namespace db
}  // namespace atp


