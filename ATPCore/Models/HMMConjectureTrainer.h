#pragma once


/**
\file

\author Samuel Barrett

\brief Contains a class which manages the training of a hidden Markov
	model for producing automatic conjectures.

*/


#include <vector>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <ATPLogic.h>
#include "../ATPCoreAPI.h"
#include "HMMStmtToObs.h"


namespace atp
{
namespace core
{


/**
\brief A class for iteratively improving a hidden Markov model for
	generating a set of strings in an unsupervised fashion.
*/
class ATP_CORE_API HMMConjectureTrainer
{
public:
	/**
	\brief Create a new HMM conjecture model trainer

	\param st_trans_init The initial state transition matrix

	\param st_obs_init The initial observation matrix, NOT including
		free variables.

	\param symbols The array of symbol IDs which the observation
		matrix represents. Thus, observation i would mean seeing
		the symbol at symbols[i]. Note that this does not include
		free variables; they can be thought of as observation at
		index symbols.size().

	\pre st_trans_init is of shape (N, N) and st_obs_init is of shape
		(M, N) for some integers N, M such that M=symbols.size()

	\pre The array of symbols is nonempty.

	\param smoothing The smoothing parameter (equivalent to applying
		a Dirichlet prior distribution over the probabilities). See
		also "Laplace smoothing".

	\pre smoothing >= 0.0f
	*/
	HMMConjectureTrainer(const logic::LanguagePtr& p_lang,
		size_t ctx_id, const logic::ModelContextPtr& p_ctx,
		boost::numeric::ublas::matrix<float> st_trans_init,
		boost::numeric::ublas::matrix<float> st_obs_init,
		std::vector<size_t> symbols, float smoothing);

	/**
	\brief Train on each statement N times

	\pre The statements in p_stmts are associated with the model
		context given in the constructor.
	*/
	void train(const logic::StatementArrayPtr& p_stmts, size_t N);

	inline const boost::numeric::ublas::matrix<
		float>& get_st_trans_mat() const
	{
		return m_st_trans;
	}
	boost::numeric::ublas::matrix<float> get_obs_mat();
	inline const std::vector<size_t>& get_symbols() const
	{
		return m_symbols;
	}

private:
	const logic::LanguagePtr m_lang;
	const size_t m_ctx_id;
	const logic::ModelContextPtr m_ctx;

	// the matrices we are estimating:
	boost::numeric::ublas::matrix<float> m_st_trans,
		m_st_obs;
	const float m_smoothing;

	const size_t m_num_states;
	std::vector<size_t> m_symbols;

	HMMStmtToObs m_stmt_to_obs;
};


}  // namespace core
}  // namespace atp


