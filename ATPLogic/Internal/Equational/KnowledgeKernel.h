#pragma once


/**

\file

\author Samuel Barrett

\brief Implementation of the IKnowledgeKernel for equational logic

*/


#include <map>
#include <string>
#include <vector>
#include <list>
#include <boost/bimap.hpp>
#include "../../ATPLogicAPI.h"
#include "../../Interfaces/IKnowledgeKernel.h"
#include "Statement.h"
#include "StatementArray.h"
#include "ModelContext.h"


namespace atp
{
namespace logic
{
namespace equational
{


class ATP_LOGIC_API KnowledgeKernel :
	public IKnowledgeKernel
{
private:
	// use builder function instead of constructing this way
	KnowledgeKernel(const ModelContext& ctx);

public:
	static KnowledgeKernelPtr try_construct(const Language& lang,
		const ModelContext& ctx);

	size_t get_integrity_code() const override;

	ProofStatePtr begin_proof_of(
		const IStatement& stmt) const override;

	bool is_trivial(
		const IStatement& stmt) const override;

	size_t add_theorems(
		StatementArrayPtr p_thms) override;

	void remove_theorems(size_t ref_id) override;

	inline const StatementArray& get_active_rules() const
	{
		return *m_active_rules;
	}

private:
	/**
	\brief Checks that all symbol IDs and arities agree with those
		defined in the model context.

	\returns true iff success.
	*/
	static bool type_check(const ModelContext& ctx,
		const Statement& stmt);

	void rebuild_active_rules();

private:
	const ModelContext& m_context;
	
	StatementArrayPtr m_axioms;

	// mapping from theorem reference ID to the theorem array
	std::map<size_t, StatementArrayPtr> m_theorems;

	// the next ID to assign when `add_theorems` is called
	size_t m_next_thm_ref_id;

	// a compilation of all axioms and added theorems (invariant:
	// this array is always the union of m_axioms and all added
	// theorems)
	// see `rebuild_active_rules`
	StatementArrayPtr _m_active_rules;
	const StatementArray* m_active_rules;
};


}  // namespace equational
}  // namespace logic
}  // namespace atp


