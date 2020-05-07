/**
\file

\author Samuel Barrett

*/


#include "HMMConjectureModelBuilder.h"
#include "HMMConjectureModel.h"


namespace atp
{
namespace core
{


HMMConjectureModelBuilder::HMMConjectureModelBuilder(logic::ModelContextPtr p_ctx) :
	m_ctx(std::move(p_ctx)),
	m_valid(true)
{
	ATP_CORE_PRECOND(m_ctx != nullptr);
}


bool HMMConjectureModelBuilder::can_build() const
{
	return m_valid && m_q.has_value() && m_num_states.has_value()
		&& m_rand_seed.has_value() && check_state_trans()
		&& check_obs();
}


std::unique_ptr<HMMConjectureModel> HMMConjectureModelBuilder::build() const
{
	ATP_CORE_PRECOND(can_build());

	// get all symbol IDs
	auto all_symb_ids = m_ctx->all_constant_symbol_ids();
	{
		auto temp = m_ctx->all_function_symbol_ids();
		all_symb_ids.insert(all_symb_ids.end(), temp.begin(),
			temp.end());
	}

	// build the matrices for (state transitions and observations)
	boost::numeric::ublas::matrix<float> st_trans(
		*m_num_states, *m_num_states), st_obs(
			all_symb_ids.size(), *m_num_states);
	for (size_t i = 0; i < *m_num_states; ++i)
	{
		for (size_t j = 0; j < *m_num_states; ++j)
		{
			st_trans(i, j) = m_state_trans.at(std::make_pair(i, j));
		}

		for (size_t j  = 0; j < all_symb_ids.size(); ++j)
		{
			st_obs(i, j) = m_symb_obs.at(std::make_pair(i,
				all_symb_ids[j]));
		}
	}

	return std::make_unique<HMMConjectureModel>(
		m_ctx, *m_num_states, *m_q, std::move(st_trans),
		std::move(st_obs), std::move(all_symb_ids), *m_rand_seed);
}


void HMMConjectureModelBuilder::reset()
{
	m_valid = true;
	m_state_trans.clear();
	m_symb_obs.clear();
	m_num_states = boost::none;
	m_q = boost::none;
}


void HMMConjectureModelBuilder::add_symbol_observation(
	size_t state, std::string symbol, float p)
{
	if (!m_ctx->is_defined(symbol))
	{
		// log this here, because even though this won't cause a
		// problem (as this class is designed to be robust to such
		// errors), there is nowhere else where we will have this
		// kind of information (i.e. the specific symbol text).
		ATP_CORE_LOG(warning) << "When building HMMConjectureModel, "
			"encountered bogus symbol \"" << symbol << "\".";
		m_valid = false;
		return;
	}

	const size_t symbol_id = m_ctx->symbol_id(symbol);

	m_symb_obs[std::make_pair(state, symbol_id)] = p;

	m_valid = m_valid && p >= 0.0f && p <= 1.0f;
}


bool HMMConjectureModelBuilder::check_state_trans() const
{
	ATP_CORE_PRECOND(m_num_states.has_value());
	ATP_CORE_PRECOND(m_valid);

	// we need to check here that: between every two states there is
	// a transition, and the sum of the probabilities coming out of
	// a given state is 1

	std::vector<std::vector<boost::optional<float>>> mat(
		*m_num_states, std::vector<boost::optional<float>>(
			*m_num_states, boost::none));

	for (const auto& tr : m_state_trans)
	{
		const size_t pre = tr.first.first;
		const size_t post = tr.first.second;
		const float p = tr.second;

		ATP_CORE_ASSERT(p >= 0.0f && p <= 1.0f);

		if (pre >= *m_num_states || post >= *m_num_states)
			return false;  // bad pre/post state

		// the std::map ensures this
		ATP_CORE_ASSERT(!mat[pre][post].has_value());

		mat[pre][post] = p;
	}

	for (size_t i = 0; i < *m_num_states; ++i)
	{
		float sum = 0.0f;

		for (size_t j = 0; j < *m_num_states; ++j)
		{
			// if state transition matrix hasn't been properly built
			// then we are not in a position to build the HMM model!
			if (!mat[i][j].has_value())
				return false;

			sum += *mat[i][j];
		}

		// if the state transition probabilities from state `i` don't
		// sum to 1 (or, close enough) then we cannot build the model
		if (std::abs(sum - 1.0f) > 1.0e-6f)
			return false;
	}

	// if we get here, all checks have passed and we're good to go
	return true;
}


bool HMMConjectureModelBuilder::check_obs() const
{
	ATP_CORE_PRECOND(m_num_states.has_value());
	ATP_CORE_PRECOND(m_valid);

	// only thing to check here is that: for every state, there is
	// an entry to every symbol in the model context, and the sum of
	// the probabilities for a state is <= 1 (raise a warning if
	// exactly 1). We don't need equality with 1 because any residual
	// probability is just the probability of generating a free
	// variable.

	// get all symbol IDs
	auto all_symb_ids = m_ctx->all_constant_symbol_ids();
	{
		auto temp = m_ctx->all_function_symbol_ids();
		all_symb_ids.insert(all_symb_ids.end(), temp.begin(),
			temp.end());
	}

	for (size_t i = 0; i < *m_num_states; ++i)
	{
		float sum = 0.0f;

		for (size_t id : all_symb_ids)
		{
			auto trans_iter = m_symb_obs.find(std::make_pair(i, id));

			// we haven't covered all our bases
			if (trans_iter == m_symb_obs.end())
				return false;

			sum += trans_iter->second;
		}

		if (sum > 1.0f)
			return false;  // bad probabilities

		if (sum == 1.0f)
			ATP_CORE_LOG(warning) << "Encountered a HMM model which "
				"had zero probability of generating a free variable";
	}

	// finally check that we aren't mapping any extra states
	for (const auto& tr : m_symb_obs)
	{
		if (tr.first.first >= *m_num_states)
			return false;  // bad state ID
	}

	// else all checks passed, so we're good
	return true;
}


}  // namespace core
}  // namespace atp


