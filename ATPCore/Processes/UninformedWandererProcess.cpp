/**
\file

\author Samuel Barrett

*/


#include "UninformedWandererProcess.h"
#include <vector>
#include <numeric>
#include <sstream>
#include "CommonProcessData.h"
#include "QueryProcess.h"
#include "ProcessSequence.h"


namespace atp
{
namespace core
{


struct WandererData :
	public proc_data::LogicEssentials
{
	logic::KnowledgeKernelPtr ker;
	logic::StatementArrayPtr starts;
	logic::StatementArrayPtr ends;
	std::vector<boost::optional<std::string>> proofs;
};


class UninformedWandererProcess :
	public QueryProcess
{
public:
	UninformedWandererProcess(
		proc_data::LogicEssentials& logic_data,
		WandererData& data, size_t N, size_t depth) :
		m_N(N), m_depth(depth), m_data(data),
		QueryProcess(logic_data.db)
	{
		// copy over old data
		static_cast<proc_data::LogicEssentials&>(m_data)
			= std::move(logic_data);

		m_data.ker = m_data.lang->try_create_kernel(*m_data.ctx);

		if (m_data.ker == nullptr)
		{
			ATP_CORE_LOG(error) << "Failed to create kernel from "
				"context \"" << m_data.ctx->context_name() << "\"!";

			force_fail();
		}
		else
		{
			ATP_CORE_LOG(info) << "Starting uninformed wanderer...";
		}
	}

protected:
	db::QueryBuilderPtr create_query() override
	{
		ATP_CORE_LOG(trace) << "Setting up uninformed wanderer "
			"starting points query...";

		auto _p_bder = m_data.db->create_query_builder(
			atp::db::QueryBuilderType::RANDOM_THM_SELECTION);

		auto p_bder = dynamic_cast<
			atp::db::IRndThmSelectQryBder*>(_p_bder.get());

		ATP_CORE_ASSERT(p_bder != nullptr);

		p_bder->set_limit(m_N)
			->set_context(m_data.ctx_id, m_data.ctx)
			->set_proven(true);  // we want proven statements

		return _p_bder;
	}

	void on_load_values(db::IQueryTransaction& query) override
	{
		ATP_CORE_ASSERT(query.arity() == 1);

		atp::db::DValue stmt_str;

		if (!query.try_get(0, atp::db::DType::STR,
			&stmt_str))
		{
			ATP_CORE_LOG(warning) << "Encountered non-string "
				<< "statement value in database. Type "
				<< "could be null?";
		}
		else
		{
			ATP_CORE_LOG(trace) << "Adding '"
				<< atp::db::get_str(stmt_str)
				<< "' as a wanderer starting point.";

			m_loaded_thms << atp::db::get_str(stmt_str)
				<< std::endl;
		}
	}

	void on_finished() override
	{
		ATP_CORE_ASSERT(m_data.ker != nullptr);

		ATP_CORE_LOG(trace) << "Finishing loading of wanderer "
			"starting points...";

		// construct theorems...
		m_data.starts =
			m_data.lang->deserialise_stmts(
				m_loaded_thms, logic::StmtFormat::TEXT,
				*m_data.ctx);

		if (m_data.starts == nullptr)
		{
			ATP_CORE_LOG(error) << "Failed to produce wanderer "
				"starting point from theorems \"" <<
				m_loaded_thms.str() << "\", perhaps there is a "
				"syntax error here somewhere?";
			force_fail();
			return;
		}
		else if (m_data.starts->size() == 0)
		{
			ATP_CORE_LOG(warning) << "Failed to load any wanderer "
				"starting points.";
			return;
		}
		else
		{
			ATP_CORE_LOG(info) << "Successfully created wanderer "
				"starting points! Count: " <<
				m_data.starts->size();
		}

		// tip: if we add the theorems we just loaded from the
		// database to the kernel, it means we can use them to
		// generate further successors of each other!
		m_data.ker->add_theorems(m_data.starts);

		run_wanderer();
	}

	void on_failed() override
	{
		ATP_CORE_LOG(error) << "Query to get starting theorems "
			"for wanderer failed. Bailing...";
	}

private:
	void run_wanderer()
	{
		// array of singletons, we will concatenate them at the end
		std::vector<logic::StatementArrayPtr> results;

		// for each starting point
		for (size_t i = 0; i < m_data.starts->size(); ++i)
		{
			// we will build the proof string into here
			std::stringstream proof;

			// start iterating at the start node, but we will
			// update this as we go
			auto iter = m_data.ker->begin_succession_of(
				m_data.starts->at(i));

			// first line of the proof
			proof << iter->get().to_str() << "\n";

			// for each depth count, pick a successor at random, and
			// update iter
			for (size_t d = 0; d < m_depth; ++d)
			{
				// build up arrays of all successors
				std::vector<logic::StmtSuccIterPtr> nexts;
				std::vector<float> utility;
				while (iter->valid())
				{
					auto next_depth = iter->dive();

					if (next_depth->valid())
					{
						// use a little heuristic to prefer selecting
						// shorter statements
						utility.push_back(1.0f /
							(float)next_depth->get().to_str().size());

						nexts.emplace_back(std::move(next_depth));
					}

					iter->advance();
				}

				if (nexts.empty())
				{
					ATP_CORE_LOG(warning) << "Ran out of successors "
						"for wandering attempt on statement \"" <<
						m_data.starts->at(i).to_str() << "\".";
					break;
				}
				else
				{
					// pick randomly according to the utility values
					// stored for each successor
					const float sum = std::accumulate(
						utility.begin(), utility.end(), 0.0f);

					// generate random fraction of `sum`
					const float r = sum * static_cast<float>(
						m_data.ker->generate_rand() % 1000000)
						/ 1.0e6f;

					float x = utility[0];
					size_t j = 0;
					while (x < r && j < utility.size())
					{
						++j;
						x += utility[j];
					}

					// select this one!
					iter = nexts[j];

					// add this as the next line in the proof
					proof << iter->get().to_str() << "\n";
				}
			}

			if (iter->valid())
			{
				ATP_CORE_LOG(debug) << "Wanderer generated: \"" <<
					iter->get().to_str() << "\"!";
				ATP_CORE_LOG(info) << "Wanderer generation " <<
					(results.size() + 1) << " / " <<
					m_data.starts->size();

				results.emplace_back(logic::from_statement(
					iter->get()));
				m_data.proofs.push_back(proof.str());
			}
			// else we failed to generate, and we already outputted a
			// message above
		}

		// don't forget to normalise the results!
		m_data.ends = m_data.lang->normalise(
			logic::concat(results));
	}

private:
	std::stringstream m_loaded_thms;
	const size_t m_N, m_depth;
	WandererData& m_data;
};


class SaveWandererResultsProcess :
	public QueryProcess
{
public:
	SaveWandererResultsProcess(WandererData& data) :
		m_data(data),
		QueryProcess(data.db)
	{
		ATP_CORE_LOG(trace) << "Beginning process to save the "
			"wanderer results...";
	}

protected:
	db::QueryBuilderPtr create_query() override
	{
		ATP_CORE_LOG(trace) << "Setting up result-saving query...";

		auto _p_bder = m_data.db->create_query_builder(
			atp::db::QueryBuilderType::SAVE_THMS_AND_PROOFS);

		auto p_bder = dynamic_cast<
			atp::db::ISaveProofResultsQryBder*>(_p_bder.get());

		ATP_CORE_ASSERT(p_bder != nullptr);

		p_bder->set_context(m_data.ctx_id, m_data.ctx)
			->add_target_thms(m_data.ends)
			->add_proofs(m_data.proofs);

		return _p_bder;
	}

	void on_finished() override
	{
		ATP_CORE_LOG(info) << "Finished saving wanderer results!";
	}

	void on_failed() override
	{
		ATP_CORE_LOG(error) << "Query to save wanderer results "
			"failed. Bailing...";
	}

private:
	WandererData& m_data;
};


ProcessPtr create_uninformed_wanderer_process(
	logic::LanguagePtr p_lang,
	size_t ctx_id, logic::ModelContextPtr p_ctx,
	db::DatabasePtr p_db, size_t N, size_t depth)
{
	ATP_CORE_PRECOND(N > 0);
	ATP_CORE_PRECOND(depth > 0);
	ATP_CORE_PRECOND(p_lang != nullptr);
	ATP_CORE_PRECOND(p_ctx != nullptr);
	ATP_CORE_PRECOND(p_db != nullptr);

	auto create_wanderer_proc = [N, depth](
		proc_data::LogicEssentials& logic_data,
		WandererData& wander_data) -> ProcessPtr
	{
		return std::make_shared<UninformedWandererProcess>(
			logic_data, wander_data, N, depth);
	};

	auto create_saver_proc = [](WandererData& wander_data)
		-> ProcessPtr
	{
		return std::make_shared<SaveWandererResultsProcess>(
			wander_data);
	};
	
	auto p_proc = make_sequence<
		proc_data::LogicEssentials,
		WandererData>(
			boost::make_tuple(
				create_wanderer_proc, create_saver_proc));

	// set initial data block
	p_proc->get_data<0>().db = std::move(p_db);
	p_proc->get_data<0>().lang = std::move(p_lang);
	p_proc->get_data<0>().ctx = std::move(p_ctx);
	p_proc->get_data<0>().ctx_id = ctx_id;

	ATP_CORE_LOG(debug) << "Created uninformed wanderer process "
		"to generate " << N << " theorems (depth parameter was "
		<< depth << ").";

	return p_proc;
}


}  // namespace core
}  // namespace atp


