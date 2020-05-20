/**
\file

\author Samuel Barrett

*/


#include "SelectSearchSettingsProcess.h"
#include <fstream>
#include <boost/filesystem.hpp>
#include "QueryProcess.h"


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
class SelectSearchSettingsProcess :
	public QueryProcess
{
public:
	SelectSearchSettingsProcess(
		proc_data::ProofSetupEssentials& data) :
		QueryProcess(data.db), m_data(data),
		m_collected_data(false)
	{
		ATP_CORE_LOG(trace) <<
			"Created search settings selection process...";
	}

protected:
	db::QueryBuilderPtr create_query() override
	{
		auto _p_bder = m_data.db->create_query_builder(
			db::QueryBuilderType::SELECT_SS);

		auto p_bder = dynamic_cast<db::ISelectSearchSettings*>(
			_p_bder.get());

		p_bder->set_ctx_id(m_data.ctx_id);

		return _p_bder;
	}

	void on_load_values(db::IQueryTransaction& query) override
	{
		ATP_CORE_ASSERT(query.arity() == 2);
		ATP_CORE_ASSERT(m_data.ctx == nullptr);

		db::DValue ss_filename, ss_id;

		if (!query.try_get(0, db::DType::STR, &ss_filename))
		{
			ATP_CORE_LOG(error) <<
				"Failed to get search settings for "
				"proof; database query couldn't convert "
				"filename to string.";

			force_fail();
			return;
		}

		if (!query.try_get(1, db::DType::INT, &ss_id))
		{
			ATP_CORE_LOG(error) <<
				"Failed to get search settings for "
				"proof; database query couldn't convert "
				"search settings ID to integer.";

			force_fail();
			return;
		}

		if (!boost::filesystem::is_regular_file(
			db::get_str(ss_filename)))
		{
			ATP_CORE_LOG(error) <<
				"Could not find search settings at path: \""
				<< db::get_str(ss_filename) << "\". "
				"Check the working directory?";

			force_fail();
			return;
		}

		if (db::get_int(ss_id) < 0)
		{
			ATP_CORE_LOG(error) <<
				"Search settings ID was negative.";

			force_fail();
			return;
		}

		std::ifstream fin(db::get_str(ss_filename));

		if (!fin)
		{
			ATP_CORE_LOG(error) << "Failed to open search settings "
				"file at \"" << db::get_str(ss_filename) << "\""
				", perhaps it's being used by another process?";

			force_fail();
			return;
		}

		if (!search::load_search_settings(m_data.ctx, fin,
			&m_data.settings))
		{
			ATP_CORE_LOG(error) <<
				"Failed to create search settings "
				"because the JSON file was bad.";

			force_fail();
			return;
		}

		m_data.ss_id = db::get_int(ss_id);

		m_collected_data = true;
	}

	void on_finished() override
	{
		if (!m_collected_data)
		{
			ATP_CORE_LOG(error) << "No rows returned from the "
				"database for search settings selection query, "
				" for context \"" << m_data.ctx->context_name()
				<< "\", ID = " << m_data.ctx_id;
			force_fail();
			return;
		}

		ATP_CORE_LOG(info) <<
			"Successfully loaded search settings \""
			<< m_data.settings.name << "\"!";
	}

	void on_failed() override
	{
		ATP_CORE_LOG(error) << "Search settings selection process "
			"failed.";
	}

private:
	bool m_collected_data;  // set to true when we load the data
	proc_data::ProofSetupEssentials& m_data;
};


ProcessPtr create_select_ss_process(
	proc_data::LogicEssentials& setup_data,
	proc_data::ProofSetupEssentials& proof_data)
{
	ATP_CORE_ASSERT(setup_data.db != nullptr);
	ATP_CORE_ASSERT(setup_data.lang != nullptr);
	ATP_CORE_ASSERT(setup_data.ctx != nullptr);

	// copy data over:
	static_cast<proc_data::LogicEssentials&>(proof_data)
		= std::move(setup_data);

	return std::make_shared<SelectSearchSettingsProcess>(
		proof_data);
}


}  // namespace core
}  // namespace atp


