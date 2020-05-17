#pragma once


/**
\file

\author Samuel Barrett

\brief Contains a basic theorem selector strategy which just picks
	randomly from the database to help in a proof.

*/


#include <sstream>
#include <ATPLogic.h>
#include <ATPDatabase.h>
#include "../ATPSearchAPI.h"
#include "../Interfaces/ISelectionStrategy.h"


namespace atp
{
namespace search
{


/**
\brief A basic theorem selector strategy which just picks
	randomly from the database to help in a proof.

\note This class is used as a base class for other selection
	strategies, too.
*/
class ATP_SEARCH_API FixedSelectionStrategy :
	public ISelectionStrategy
{
public:
	/**
	\param n The number of theorems to get from the database, then
		immediately return.
	*/
	FixedSelectionStrategy(const logic::LanguagePtr& p_lang,
		const logic::ModelContextPtr& p_ctx, size_t ctx_id,
		size_t n);

	virtual void set_targets(
		const logic::StatementArrayPtr& p_targets) override;
	virtual db::QueryBuilderPtr create_getter_query(
		const db::DatabasePtr& p_db) override;
	virtual void load_values(
		db::IQueryTransaction& query) override;
	virtual logic::StatementArrayPtr done() override;

protected:
	const logic::LanguagePtr m_lang;
	const logic::ModelContextPtr m_ctx;
	const size_t m_ctx_id;
	const size_t m_num_thms;
	bool m_failed;

private:
	std::stringstream m_cur_thms;
};


}  // namespace search
}  // namespace atp


