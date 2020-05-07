#pragma once


/**
\file

\author Samuel Barrett

\brief Contains a class for converting a statement into a list of
	integer observations for immediate use in a HMM.
*/


#include <ATPLogic.h>
#include "../ATPCoreAPI.h"


namespace atp
{
namespace core
{


/**
\brief This class is for converting arrays of statements into
	matrices of sequences of integer observation values.

\details Since efficiency is very important for training, it is worth
	the design cost to handle each kind of logic separately and to
	get an efficient conversion to an observation sequence.

\note Symbol IDs are represented in the range 0..N-1, and free
	variables are represented by N, where N=symb_ids.size() is the
	size of the array of symbol IDs given.
*/
class ATP_CORE_API HMMStmtToObs
{
public:
	HMMStmtToObs(const logic::LanguagePtr& p_lang,
		const logic::ModelContextPtr& p_ctx,
		std::vector<size_t> symb_ids);

	std::vector<std::vector<size_t>> convert(
		const logic::StatementArrayPtr& p_stmts) const;

private:
	logic::LanguagePtr m_lang;
	logic::ModelContextPtr m_ctx;
	std::vector<size_t> m_symb_ids;
};


}  // namespace core
}  // namespace atp


