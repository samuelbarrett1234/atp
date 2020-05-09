/**
\file

\author Samuel Barrett

*/


#include <chrono>
#include <sstream>
#include <boost/optional.hpp>
#include <boost/optional/optional_io.hpp>
#include "../Models/HMMConjectureModelBuilder.h"
#include "../Models/HMMConjectureModel.h"
#include "HMMModelLoaderProcess.h"
#include "QueryProcess.h"
#include "ProcessSequence.h"


namespace atp
{
namespace core
{


struct MyHMMBuildingData :
	public proc_data::HMMConjBuildingEssentials
{
	std::unique_ptr<HMMConjectureModelBuilder> model_builder;
};


/**
\brief This process is for loading the main model parameters (like
	the ID, the number of hidden states, etc).
*/
class HMMModelLoaderProcess :
	public QueryProcess
{
public:
	HMMModelLoaderProcess(
		proc_data::HMMConjBuildingEssentials& building_data,
		MyHMMBuildingData& data) :
		QueryProcess(building_data.db),
		m_building_data(building_data),
		m_model_builder(*data.model_builder)
	{
		ATP_CORE_LOG(trace) << "Creating HMM model loader "
			"process...";

		// copy most of the data over
		static_cast<proc_data::HMMConjBuildingEssentials&>(
			data) = building_data;
	}

protected:
	db::QueryBuilderPtr create_query() override
	{
		ATP_CORE_LOG(trace) << "Creating HMM model loading query...";

		auto _p_bder = m_building_data.db->create_query_builder(
			db::QueryBuilderType::FIND_HMM_CONJ_MODEL);

		auto p_bder = dynamic_cast<db::IFindHmmConjModel*>(
			_p_bder.get());

		ATP_CORE_ASSERT(p_bder != nullptr);

		p_bder->set_ctx(m_building_data.ctx_id,
			m_building_data.ctx);

		if (m_building_data.model_id.has_value())
			p_bder->set_model_id(*m_building_data.model_id);

		return _p_bder;
	}

	void on_load_values(db::IQueryTransaction& query) override
	{
		// this should be an assert, because if the database did not
		// contain the three columns we are looking for, the query
		// would not have been built in the first place and we would
		// have caught the error.
		ATP_CORE_ASSERT(query.arity() == 3);

		db::DValue id,
			num_states,
			free_q;

		if (!query.try_get(0, db::DType::INT, &id))
		{
			ATP_CORE_LOG(error) << "Query to obtain HMM conjecturer "
				"model data failed - query did not return an ID.";
			
			force_fail();
		}
		m_building_data.model_id = (size_t)db::get_int(id);

		if (query.try_get(1, db::DType::INT, &num_states))
		{
			m_model_builder.set_num_hidden_states(
				(size_t)db::get_int(num_states));
		}

		if (query.try_get(2, db::DType::FLOAT, &free_q))
		{
			m_model_builder.set_free_geometric_q(
				db::get_float(free_q));
		}
	}

	void on_finished() override
	{
		ATP_CORE_LOG(trace) << "Finished loading first portion of "
			"HMM model.";
	}

	void on_failed() override
	{
		ATP_CORE_LOG(error) << "Failed to load HMM conjecturer model"
			". Model ID was " << m_building_data.model_id << ", "
			"context ID was " << m_building_data.ctx_id << "("
			<< m_building_data.ctx->context_name() << ").";
	}

private:
	HMMConjectureModelBuilder& m_model_builder;
	proc_data::HMMConjBuildingEssentials& m_building_data;
};


/**
\brief This process is for loading the state transition parameters
	of the HMM model.
*/
class HMMStateTransLoaderProcess :
	public QueryProcess
{
public:
	HMMStateTransLoaderProcess(
		MyHMMBuildingData& building_data,
		MyHMMBuildingData& data) :
		QueryProcess(building_data.db),
		m_building_data(data),
		m_model_builder(*building_data.model_builder)
	{
		// this is important!
		ATP_CORE_PRECOND(m_building_data.model_id.has_value());

		ATP_CORE_LOG(trace) << "Creating HMM state transition"
			" parameter loader process...";

		// move the data over
		data = std::move(building_data);
	}

protected:
	db::QueryBuilderPtr create_query() override
	{
		ATP_CORE_LOG(trace) << "Creating HMM model"
			"state transition parameter loading query...";

		auto _p_bder = m_building_data.db->create_query_builder(
			db::QueryBuilderType::GET_HMM_CONJ_ST_TRANS_PARAMS);

		auto p_bder = dynamic_cast<db::IGetHmmConjectureModelParams*>(
			_p_bder.get());

		ATP_CORE_ASSERT(p_bder != nullptr);

		p_bder->set_ctx(m_building_data.ctx_id, m_building_data.ctx)
			->set_model_id(*m_building_data.model_id);

		return _p_bder;
	}

	void on_load_values(db::IQueryTransaction& query) override
	{
		// this should be an assert, because if the database did not
		// contain the three columns we are looking for, the query
		// would not have been built in the first place and we would
		// have caught the error.
		ATP_CORE_ASSERT(query.arity() == 3);

		db::DValue pre_state, post_state, prob;

		if (query.try_get(0, db::DType::INT, &pre_state)
			&& query.try_get(1, db::DType::INT, &post_state)
			&& query.try_get(2, db::DType::FLOAT, &prob))
		{
			m_model_builder.add_state_transition(
				(size_t)db::get_int(pre_state),
				(size_t)db::get_int(post_state),
				db::get_float(prob));
		}
		else
		{
			ATP_CORE_LOG(warning) << "HMM model loading: "
				"State transition parameters from database didn't"
				" have the right type. Check the tables in the "
				"database for nulls or other type errors.";
			force_fail();
		}
	}

	void on_finished() override
	{
		ATP_CORE_LOG(trace) << "Finished loading state transitions "
			"for HMM model.";
	}

	void on_failed() override
	{
		ATP_CORE_LOG(error) << "Failed to load HMM conjecturer model"
			". Model ID was " << m_building_data.model_id << ", "
			"context ID was " << m_building_data.ctx_id << "("
			<< m_building_data.ctx->context_name() << ").";
	}

private:
	HMMConjectureModelBuilder& m_model_builder;
	proc_data::HMMConjBuildingEssentials& m_building_data;
};


/**
\brief This process is for loading the observation parameters
	of the HMM model.
*/
class HMMObsLoaderProcess :
	public QueryProcess
{
public:
	HMMObsLoaderProcess(
		MyHMMBuildingData& building_data,
		MyHMMBuildingData& data) :
		QueryProcess(building_data.db),
		m_building_data(data),
		m_model_builder(*building_data.model_builder)
	{
		// this is important!
		ATP_CORE_PRECOND(m_building_data.model_id.has_value());

		ATP_CORE_LOG(trace) << "Creating HMM observation"
			" parameter loader process...";

		// move the data over
		data = std::move(building_data);
	}

protected:
	db::QueryBuilderPtr create_query() override
	{
		ATP_CORE_LOG(trace) << "Creating HMM model"
			"observation parameter loading query...";

		auto _p_bder = m_building_data.db->create_query_builder(
			db::QueryBuilderType::GET_HMM_CONJ_OBS_PARAMS);

		auto p_bder = dynamic_cast<db::IGetHmmConjectureModelParams*>(
			_p_bder.get());

		ATP_CORE_ASSERT(p_bder != nullptr);

		p_bder->set_ctx(m_building_data.ctx_id, m_building_data.ctx)
			->set_model_id(*m_building_data.model_id);

		return _p_bder;
	}

	void on_load_values(db::IQueryTransaction& query) override
	{
		// this should be an assert, because if the database did not
		// contain the three columns we are looking for, the query
		// would not have been built in the first place and we would
		// have caught the error.
		ATP_CORE_ASSERT(query.arity() == 3);

		db::DValue state, obs, prob;

		if (query.try_get(0, db::DType::INT, &state)
			&& query.try_get(1, db::DType::STR, &obs)
			&& query.try_get(2, db::DType::FLOAT, &prob))
		{
			m_model_builder.add_symbol_observation(
				(size_t)db::get_int(state),
				db::get_str(obs),
				db::get_float(prob));
		}
		else
		{
			ATP_CORE_LOG(warning) << "HMM model loading: "
				"Observation parameters from database didn't"
				" have the right type. Check the tables in the "
				"database for nulls or other type errors.";
			force_fail();
		}
	}

	void on_finished() override
	{
		ATP_CORE_LOG(trace) << "Finished loading observations "
			"for HMM model.";
	}

	void on_failed() override
	{
		ATP_CORE_LOG(error) << "Failed to load HMM conjecturer model"
			". Model ID was " << m_building_data.model_id << ", "
			"context ID was " << m_building_data.ctx_id << "("
			<< m_building_data.ctx->context_name() << ").";
	}

private:
	HMMConjectureModelBuilder& m_model_builder;
	proc_data::HMMConjBuildingEssentials& m_building_data;
};


/**
\brief This process is responsible for constructing the final model
	object from the model builder, after the other processes have
	ran.
*/
class BuildHMMModelProcess :
	public IProcess
{
public:
	BuildHMMModelProcess(MyHMMBuildingData& building_data,
		proc_data::HMMConjModelEssentials& model_data) :
		m_failed(false)
	{
		ATP_CORE_PRECOND(model_data.model == nullptr);
		ATP_CORE_PRECOND(building_data.model_builder != nullptr);

		ATP_CORE_LOG(trace) << "Building HMM model...";

		if (building_data.model_builder->can_build())
		{
			// copy over most of the stuff
			static_cast<proc_data::LogicEssentials&>(model_data)
				= (proc_data::LogicEssentials&)building_data;

			// this should've been set by the first process, if it
			// wasn't given already
			ATP_CORE_ASSERT(building_data.model_id.has_value());
			model_data.model_id = *building_data.model_id;

			// finally!!
			model_data.model = building_data.model_builder->build();
		}
		else  // oops!
		{
			ATP_CORE_LOG(error) << "Failed to build HMM model. "
				"Insufficient parameters were successfully loaded. "
				"Check the HMM tables in the database to ensure "
				"they are complete (i.e. there is a state transition"
				" between every pair of states, and there is an "
				"observation entry for each state and for each "
				"symbol in the model context.";
		}

		m_failed = (model_data.model == nullptr);
	}

	inline bool done() const override
	{
		return true;
	}
	inline bool waiting() const override
	{
		return false;
	}
	inline bool has_failed() const override
	{
		return m_failed;
	}
	inline void run_step() override { }

private:
	bool m_failed;
};


ProcessPtr create_hmm_model_loader_process(
	proc_data::HMMConjBuildingEssentials& building_data,
	proc_data::HMMConjModelEssentials& model_data)
{
	// check all the setup data is valid:
	ATP_CORE_PRECOND(building_data.db != nullptr);
	ATP_CORE_PRECOND(building_data.lang != nullptr);
	ATP_CORE_PRECOND(building_data.ctx != nullptr);

	HMMConjectureModelBuilder builder(building_data.ctx);

	// seed the conjecture generator with the current time
	const size_t seed = (size_t)
		std::chrono::high_resolution_clock
		::now().time_since_epoch().count();
	ATP_CORE_LOG(debug) << "Seeded the HMM model with " << seed;
	builder.set_random_seed(seed);

	// these are the processes we will include:

	auto make_proc_1 = boost::bind(&std::make_shared<
		HMMModelLoaderProcess, proc_data::HMMConjBuildingEssentials&,
		MyHMMBuildingData&>, _1, _2);

	auto make_proc_2 = boost::bind(&std::make_shared<
		HMMStateTransLoaderProcess, MyHMMBuildingData&,
		MyHMMBuildingData&>, _1, _2);

	auto make_proc_3 = boost::bind(&std::make_shared<
		HMMObsLoaderProcess, MyHMMBuildingData&,
		MyHMMBuildingData&>, _1, _2);

	auto make_proc_4 = boost::bind(&std::make_shared<
		BuildHMMModelProcess, MyHMMBuildingData&,
		proc_data::HMMConjModelEssentials&>, _1,
		boost::ref(model_data)  /* n.b. */);

	// now chain them together:

	auto p_proc = make_sequence<
		proc_data::HMMConjBuildingEssentials,
		MyHMMBuildingData, MyHMMBuildingData,
		MyHMMBuildingData>(
			boost::make_tuple(make_proc_1, make_proc_2,
				make_proc_3, make_proc_4));

	// copy over initial data
	p_proc->get_data<0>() = building_data;

	// init this
	p_proc->get_data<1>().model_builder = std::make_unique<
		HMMConjectureModelBuilder>(building_data.ctx);
	
	return p_proc;
}


}  // namespace core
}  // namespace atp


