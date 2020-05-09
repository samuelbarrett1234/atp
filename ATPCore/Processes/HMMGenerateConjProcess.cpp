/**
\file

\author Samuel Barrett

*/


#include <vector>
#include <sstream>
#include <boost/optional.hpp>
#include <boost/optional/optional_io.hpp>
#include "HMMGenerateConjProcess.h"
#include "../Models/HMMConjectureModelBuilder.h"
#include "../Models/HMMConjectureModel.h"


namespace atp
{
namespace core
{


class HMMConjGenProcess :
	public IProcess
{
private:
	/**
	\brief We generate conjectures in a recursive manner, hence
		bundle up all of the data we need to put in the stack into
		one struct.
	*/
	struct ATP_CORE_API CurrentStmtStackFrame
	{
		// observation parameters given by HMM model generation
		const size_t cur_obs, cur_obs_arity;
		const bool cur_obs_is_free;

		// a string representing the current expression so far (as
		// its children are completed, they will be appended here).
		std::stringstream done_so_far;

		// the next child that needs to be converted (which will be
		// appearing immediately above this entry on the stack).
		// as soon as cur_idx == cur_obs_arity, this will be ready to
		// pop from the stack
		size_t cur_idx;
	};

public:
	HMMConjGenProcess(
		size_t num_to_generate,
		proc_data::HMMConjGenerationEssentials& gen_data) :
		m_gen_data(gen_data), m_num_to_generate(num_to_generate),
		m_num_done(0), m_saved_results(false), m_failed(false)
	{
		ATP_CORE_LOG(info) << "Starting conjecture generation...";
	}

	inline bool done() const override
	{
		return m_num_done == m_num_to_generate && m_saved_results;
	}
	inline bool waiting() const override
	{
		return false;
	}
	inline bool has_failed() const override
	{
		return m_failed;
	}
	void run_step() override
	{
		if (m_num_done < m_num_to_generate)
		{
			++m_num_done;
			generate_a_conjecture();
		}
		else
		{
			ATP_CORE_LOG(info) << "Generated " << m_num_to_generate
				<< " conjectures!";

			// convert textual statements into an object version
			m_gen_data.generated_stmts =
				m_gen_data.lang->deserialise_stmts(m_completed,
					logic::StmtFormat::TEXT, *m_gen_data.ctx);
			m_saved_results = true;

			if (m_gen_data.generated_stmts == nullptr)
			{
				ATP_CORE_LOG(error) << "Failed to convert "
					"conjectured statements to object form - "
					"perhaps the generator is capable of producing "
					"bad syntax? Statements were: \"" <<
					m_completed.str() << "\".";
				
				m_failed = true;
			}
			else
			{
				ATP_CORE_LOG(trace) << "Successfully generated the "
					"following " << m_num_to_generate <<
					" conjectures: \n" << m_completed.str();
			}
		}
	}

private:
	void generate_a_conjecture()
	{
		// reset state to default, before generating a new statement
		m_gen_data.model->reset_state();

		// it is important that they are evaluated in this order, as the
		// HMM state is passed between them.
		auto expr1 = generate_expression(),
			expr2 = generate_expression();

		if (expr1.has_value() && expr2.has_value())
		{
			const std::string stmt = *expr1 + " = " + *expr2;
			m_completed << stmt << "\n";

			ATP_CORE_LOG(debug) << "HMMConjectureProcess just "
				"generated the statement \"" <<
				stmt << "\".";
		}
		else
		{
			ATP_CORE_LOG(warning) << "Failed to generate conjecture."
				" Expressions were \"" << expr1 << "\", \"" <<
				expr2 << "\".";
		}

		// TODO: there are a few ways we could use a HMM to generate
		// these statements; in particular, in the way we let the
		// state transition. Is there a nice way of selecting between
		// them? For now we will only implement the one below.
	}

	// helper function for generating a conjecture
	// this can fail, but on rare occasions
	boost::optional<std::string> generate_expression()
	{
		// to ensure termination, create an iteration limit:
		static const size_t ITERATION_LIMIT = 10000;

		// do not reset model state here!

		std::vector<CurrentStmtStackFrame> stack;

		auto add_cur_model_obs = [&stack, this]()
		{
			stack.emplace_back(CurrentStmtStackFrame{
				m_gen_data.model->current_observation(),
				m_gen_data.model->current_observation_arity(),
				m_gen_data.model->current_observation_is_free_var(),
				std::stringstream(), 0
				});

			if (m_gen_data.model->current_observation_is_free_var())
			{
				stack.back().done_so_far << 'x' <<
					m_gen_data.model->current_observation();
			}
			else
			{
				stack.back().done_so_far <<
					m_gen_data.ctx->symbol_name(
					m_gen_data.model->current_observation());

				if (m_gen_data.model->current_observation_arity() > 0)
				{
					stack.back().done_so_far << "(";
				}
			}

			m_gen_data.model->advance();
		};

		// push first element
		add_cur_model_obs();

		size_t iters = 0;
		while (stack.front().cur_idx < stack.front().cur_obs_arity
			&& iters < ITERATION_LIMIT)
		{
			++iters;

			// handle back element (top of stack)
			if (stack.back().cur_idx < stack.back().cur_obs_arity)
			{
				add_cur_model_obs();
			}
			else if (stack.size() > 1)
			{
				auto elem = std::move(stack.back());
				stack.pop_back();

				// advance next child and add our string to it:
				++stack.back().cur_idx;
				stack.back().done_so_far << elem.done_so_far.str();

				// add comma or closing bracket
				if (stack.back().cur_idx < stack.back().cur_obs_arity)
				{
					stack.back().done_so_far << ", ";
				}
				else
				{
					stack.back().done_so_far << ")";
				}
			}
		}

		if (iters == ITERATION_LIMIT)
		{
			ATP_CORE_LOG(error) << "Iteration limit reached in generating "
				"conjecture from HMMConjectureProcess, indicating bad "
				"model parameters. Stack size was " << stack.size() <<
				".";
			return boost::none;
		}
		return stack.front().done_so_far.str();
	}

private:
	bool m_failed;

	proc_data::HMMConjGenerationEssentials& m_gen_data;
	const size_t m_num_to_generate;
	size_t m_num_done;  // number of conjectures generated

	// stream where we put completed statements
	std::stringstream m_completed;

	// false until we convert the stream of textual statements to
	// statement objects
	bool m_saved_results;
};


ProcessPtr create_hmm_conj_gen_process(
	size_t num_to_generate,
	proc_data::HMMConjModelEssentials& model_data,
	proc_data::HMMConjGenerationEssentials& gen_data)
{
	// check all the model data is valid:
	ATP_CORE_PRECOND(model_data.db != nullptr);
	ATP_CORE_PRECOND(model_data.lang != nullptr);
	ATP_CORE_PRECOND(model_data.ctx != nullptr);
	ATP_CORE_PRECOND(model_data.model != nullptr);

	// move most of the data across:
	static_cast<proc_data::HMMConjModelEssentials&>(gen_data)
		= std::move(model_data);

	return std::make_shared<HMMConjGenProcess>(num_to_generate,
		gen_data);
}


}  // namespace core
}  // namespace atp


