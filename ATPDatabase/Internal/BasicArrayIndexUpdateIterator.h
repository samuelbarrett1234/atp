#pragma once

/**
\file

\author Samuel Barrett

\brief Contains the update iterator for the BasicArrayIndex index.

*/


#include <vector>
#include "../ATPDatabaseAPI.h"
#include "../Interfaces/Data.h"
#include "../Interfaces/DBIterators.h"
#include "BasicArrayIndexSelectAllIterator.h"


namespace atp
{
namespace db
{
namespace basic_array_index_detail
{


/**
\brief This update iterator is for the basic array index.

\detais It extends the SelectAll iterator by enabling functionality
	to edit the data. Like SelectAllIterator, it does not support
	filtering, so again you would have to create a wrapper iterator
	for that.
*/
class ATP_DATABASE_API UpdateIterator :
	public SelectAllIterator,
	public IDBUpdateIterator
{
public:
	UpdateIterator(const logic::ILanguage& lang,
		const logic::IModelContext& ctx,
		const BasicArrayIndex& parent,
		std::shared_ptr<ILock> p_lock,
		std::shared_ptr<IReadableStream> p_stream,
		const std::vector<DType>& col_types,
		const std::vector<std::string>& col_names);

	inline bool is_mutable(const Column& col) const override
	{
		return true;
	}
	inline bool all_mutable() const override
	{
		return true;
	}
	void set(const Column& col, const DValue& value) override;
	void set_all(const std::vector<DValue>& values) override;

private:
	void set(size_t idx, const DValue& value);
};


}  // namespace basic_array_index_detail
}  // namespace db
}  // namespace atp


