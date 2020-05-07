/**
\file

\author Samuel Barrett

*/


#include <boost/numeric/ublas/matrix_proxy.hpp>
#include "HMMConjectureTrainer.h"
#include "HMMUtility.h"


namespace ublas = boost::numeric::ublas;
typedef ublas::matrix<float> Matrix;
typedef ublas::matrix_range<Matrix> MatrixRange;


namespace atp
{
namespace core
{


HMMConjectureTrainer::HMMConjectureTrainer(
	const logic::LanguagePtr& p_lang, size_t ctx_id,
	const logic::ModelContextPtr& p_ctx,
	Matrix st_trans_init,
	Matrix st_obs_init,
	std::vector<size_t> symbols, float smoothing) :
	m_lang(p_lang), m_ctx_id(m_ctx_id), m_ctx(m_ctx),
	m_st_trans(std::move(st_trans_init)),
	m_st_obs(std::move(st_obs_init)),
	m_smoothing(smoothing), m_symbols(std::move(symbols)),
	m_num_states(m_st_trans.size1()),
	m_stmt_to_obs(m_lang, m_ctx, m_symbols)
{
	ATP_CORE_PRECOND(m_symbols.size() > 0);
	ATP_CORE_PRECOND(m_lang != nullptr);
	ATP_CORE_PRECOND(m_ctx != nullptr);
	ATP_CORE_PRECOND(m_num_states > 0);
	ATP_CORE_PRECOND(m_st_trans.size2() == m_num_states);
	ATP_CORE_PRECOND(m_st_obs.size2() == m_num_states);
	ATP_CORE_PRECOND(m_st_obs.size1() == m_symbols.size());
	ATP_CORE_PRECOND(m_smoothing >= 0.0f);

	// adjoin an extra row of observations, for free variables:
	m_st_obs.resize(m_st_obs.size1() + 1, m_st_obs.size2());
	for (size_t i = 0; i < m_num_states; ++i)
	{
		float sum = 0.0f;
		for (size_t j = 0; j < m_symbols.size(); ++j)
		{
			sum += m_st_obs(j, i);
		}
		m_st_obs(m_symbols.size(), i) = 1.0f - sum;
	}
}


Matrix HMMConjectureTrainer::get_obs_mat()
{
	return MatrixRange(m_st_obs,
		ublas::range(0, m_symbols.size()),
		ublas::range(0, m_num_states));
}


void HMMConjectureTrainer::train(
	const logic::StatementArrayPtr& p_stmts, size_t N)
{
	const auto observations = m_stmt_to_obs.convert(p_stmts);

	hmm::baum_welch(m_st_trans, m_st_obs,
		observations, N, m_smoothing);
}



}  // namespace core
}  // namespace atp


