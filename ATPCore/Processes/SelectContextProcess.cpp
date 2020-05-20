/**
\file

\author Samuel Barrett

*/


#include "SelectContextProcess.h"
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
class SelectContextProcess :
	public QueryProcess
{
public:
	SelectContextProcess(
		proc_data::LogicEssentials& data) :
		QueryProcess(data.db), m_data(data)
	{
		ATP_CORE_LOG(trace) <<
			"Created context selection process...";

		m_data.lang = logic::create_language(
			logic::LangType::EQUATIONAL_LOGIC);

		// check this beforehand, because we're going to use the
		// presence/absence of a non-null CTX pointer as a way of
		// checking success at the end
		ATP_CORE_ASSERT(m_data.ctx == nullptr);
	}

protected:
	db::QueryBuilderPtr create_query() override
	{
		return m_data.db->create_query_builder(
			db::QueryBuilderType::SELECT_CTX);
	}

	void on_load_values(db::IQueryTransaction& query) override
	{
		ATP_CORE_ASSERT(query.arity() == 2);
		ATP_CORE_ASSERT(m_data.ctx == nullptr);

		db::DValue ctx_filename, ctx_id;

		if (!query.try_get(0, db::DType::STR, &ctx_filename))
		{
			ATP_CORE_LOG(error) << "Failed to get context for "
				"proof; database query couldn't convert model "
				"filename to string.";

			force_fail();
			return;
		}

		if (!query.try_get(1, db::DType::INT, &ctx_id))
		{
			ATP_CORE_LOG(error) << "Failed to get context for "
				"proof; database query couldn't convert model "
				"context ID to integer.";

			force_fail();
			return;
		}

		if (!boost::filesystem::is_regular_file(
			db::get_str(ctx_filename)))
		{
			ATP_CORE_LOG(error) << "Could not find model context at "
				"path: \"" << db::get_str(ctx_filename) << "\". "
				"Check the working directory?";

			force_fail();
			return;
		}

		if (db::get_int(ctx_id) < 0)
		{
			ATP_CORE_LOG(error) << "Model context ID was negative.";

			force_fail();
			return;
		}

		std::ifstream fin(db::get_str(ctx_filename));

		if (!fin)
		{
			ATP_CORE_LOG(error) << "Failed to open model context "
				"file at \"" << db::get_str(ctx_filename) << "\""
				", perhaps it's being used by another process?";

			force_fail();
			return;
		}

		m_data.ctx_id = db::get_int(ctx_id);
		m_data.ctx = m_data.lang->try_create_context(fin);

		if (m_data.ctx == nullptr)
		{
			ATP_CORE_LOG(error) << "Failed to create model context "
				"because the JSON file was bad.";

			force_fail();
			return;
		}
	}

	void on_finished() override
	{
		if (m_data.ctx == nullptr)
		{
			ATP_CORE_LOG(error) << "Failed to select a model context"
				" from the database because (probably) no rows were "
				"returned from the query.";
			force_fail();
			return;
		}

		ATP_CORE_LOG(info) << "Successfully loaded context \""
			<< m_data.ctx->context_name() << "\"!";
	}

	void on_failed() override
	{
		ATP_CORE_LOG(error) << "Select context process failed.";
	}

private:
	proc_data::LogicEssentials& m_data;
};


ProcessPtr create_select_ctx_process(
	proc_data::DatabaseEssentials& db_data,
	proc_data::LogicEssentials& logic_data)
{
	ATP_CORE_ASSERT(db_data.db != nullptr);
	logic_data.db = std::move(db_data.db);

	return std::make_shared<SelectContextProcess>(
		logic_data);
}


}  // namespace core
}  // namespace atp


