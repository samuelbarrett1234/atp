/**
\file

\author Samuel Barrett

*/


#include <sstream>
#include "HMMLoadTrainingDataProcess.h"
#include "QueryProcess.h"
#include "../Models/HMMConjectureModel.h"


namespace atp
{
namespace core
{


class HMMLoadTrainingDataProc :
	public QueryProcess
{
public:
	HMMLoadTrainingDataProc(size_t max_dataset_size,
		proc_data::HMMConjecturerTrainingEssentials& train_data) :
		m_max_dataset_size(max_dataset_size),
		m_train_data(train_data),
		QueryProcess(train_data.db)
	{
		ATP_CORE_LOG(trace) << "Creating HMM training-data loader "
			"process...";
	}

protected:
	db::QueryBuilderPtr create_query() override
	{
		ATP_CORE_LOG(trace) << "Setting up query to get proven theorems"
			" from the database to train on.";

		auto _p_bder = m_train_data.db->create_query_builder(
			atp::db::QueryBuilderType::RANDOM_THM_SELECTION);

		auto p_bder = dynamic_cast<
			atp::db::IRndThmSelectQryBder*>(_p_bder.get());

		ATP_CORE_ASSERT(p_bder != nullptr);

		p_bder->set_limit(m_max_dataset_size)
			->set_context(m_train_data.ctx_id, m_train_data.ctx)
			->set_proven(true);  // DEFINITELY load proven statements!

		return _p_bder;
	}

	void on_load_values(db::IQueryTransaction& query) override
	{
		ATP_CORE_ASSERT(query.arity() == 1);

		db::DValue stmt_str;

		if (!query.try_get(0, atp::db::DType::STR,
			&stmt_str))
		{
			ATP_CORE_LOG(warning) << "Encountered non-string "
				<< "statement value in database. Type "
				<< "could be null?";
		}
		else
		{
			ATP_CORE_LOG(debug) << "Adding '"
				<< atp::db::get_str(stmt_str)
				<< "' to helper theorems.";

			m_stmt_strs << atp::db::get_str(stmt_str)
				<< std::endl;
		}
	}

	void on_finished() override
	{
		m_train_data.dataset = m_train_data.lang->deserialise_stmts(
			m_stmt_strs, logic::StmtFormat::TEXT, *m_train_data.ctx);

		if (m_train_data.dataset == nullptr)
		{
			ATP_CORE_LOG(error) <<
				"Failed to get dataset for training. There was"
				" a problem with the following batch of "
				"statements retrieved from the database: \""
				<< m_stmt_strs.str() << '"';
		}
		else if (m_train_data.dataset->size() > 0)
		{
			ATP_CORE_LOG(info) << "HMM Train Process: "
				"Loaded dataset of size" <<
				m_train_data.dataset->size()
				<< " from the theorem database!";
		}
		else
		{
			ATP_CORE_LOG(warning) <<
				"HMM train process could not find any "
				"theorems to load from the database."
				" Stopping process...";

			force_fail();
		}
	}

	void on_failed() override
	{
		ATP_CORE_LOG(error) << "Failed to gather statement database "
			"for HMM model with ID " << m_train_data.model_id <<
			" (however, of course, loading the dataset is "
			"independent of the HMM conjecturer model.";
	}

private:
	const size_t m_max_dataset_size;
	proc_data::HMMConjecturerTrainingEssentials& m_train_data;
	std::stringstream m_stmt_strs;
};


ATP_CORE_API ProcessPtr create_hmm_training_data_load_proc(
	size_t num_epochs, size_t max_dataset_size,
	proc_data::HMMConjModelEssentials& model_data,
	proc_data::HMMConjecturerTrainingEssentials& train_data)
{
	// check inputs
	ATP_CORE_PRECOND(model_data.db != nullptr);
	ATP_CORE_PRECOND(model_data.lang != nullptr);
	ATP_CORE_PRECOND(model_data.ctx != nullptr);
	ATP_CORE_PRECOND(model_data.model != nullptr);

	// move data over
	static_cast<proc_data::HMMConjModelEssentials&>(train_data)
		= std::move(model_data);
	train_data.num_epochs = num_epochs;  // store this

	return std::make_shared<HMMLoadTrainingDataProc>(
		max_dataset_size, train_data);
}


}  // namespace core
}  // namespace atp


