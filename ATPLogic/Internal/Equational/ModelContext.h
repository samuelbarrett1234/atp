#pragma once


/**

\file

\author Samuel Barrett

\brief Contains the equational logic ModelContext which implements
    the IModelContext interface.

*/


#include <map>
#include <vector>
#include <boost/bimap.hpp>
#include "../../ATPLogicAPI.h"
#include "../../Interfaces/IModelContext.h"
#include "Language.h"


namespace atp
{
namespace logic
{
namespace equational
{


/**
\brief Basic implementation of the model context, for equational logic.

\note Construct one of these objects using the builder function!
*/
class ATP_LOGIC_API ModelContext :
    public IModelContext
{
private:
    // construct using the builder function!
    ModelContext() = default;

public:
    /**
    \brief A builder function for this model context implementation.

    \returns `nullptr` on failure, otherwise returns a valid object.

    \todo Document input format!
    */
    static ModelContextPtr try_construct(const Language& parent,
        std::istream& in);
    

    // interface functions


    inline std::string context_name() const override
    {
        return m_name;
    }

    inline std::string symbol_name(size_t symbol_id) const override
    {
        ATP_LOGIC_PRECOND(is_defined(symbol_id));
        return m_name_to_id.right.at(symbol_id);
    }
    inline size_t symbol_id(std::string symbol_name) const override
    {
        ATP_LOGIC_PRECOND(is_defined(symbol_name));
        return m_name_to_id.left.at(symbol_name);
    }

    inline size_t symbol_arity(std::string symbol_name) const override
    {
        ATP_LOGIC_PRECOND(is_defined(symbol_name));
        const auto id = m_name_to_id.left.at(symbol_name);
        return m_id_to_arity.at(id);
    }
    inline size_t symbol_arity(size_t symbol_id) const override
    {
        ATP_LOGIC_PRECOND(is_defined(symbol_id));
        return m_id_to_arity.at(symbol_id);
    }

    inline bool is_defined(std::string symbol_name) const override
    {
        auto iter = m_name_to_id.left.find(symbol_name);
        return (iter != m_name_to_id.left.end());
    }
    inline bool is_defined(size_t symbol_id) const override
    {
        auto iter = m_name_to_id.right.find(symbol_id);
        return (iter != m_name_to_id.right.end());
    }

    inline std::vector<size_t> all_constant_symbol_ids() const override
    {
        return m_const_ids;
    }

    inline std::vector<size_t> all_function_symbol_ids() const override
    {
        return m_func_ids;
    }

    inline size_t num_axioms() const override
    {
        return m_axioms.size();
    }
    inline std::string axiom_at(size_t index) const override
    {
        ATP_LOGIC_PRECOND(index < m_axioms.size());
        return m_axioms.at(index);
    }

    // NOT part of the interface IModelContext:

    inline const std::map<size_t, size_t>& id_to_arity_map() const
    {
        return m_id_to_arity;
    }

private:
    std::string m_name;
    boost::bimap<std::string, size_t> m_name_to_id;
    std::map<size_t, size_t> m_id_to_arity;
    std::vector<std::string> m_axioms;
    std::vector<size_t> m_const_ids, m_func_ids;
};


}  // namespace equational
}  // namespace logic
}  // namespace atp


