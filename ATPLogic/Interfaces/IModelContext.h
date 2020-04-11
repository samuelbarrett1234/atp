#pragma once


/**

\file

\author Samuel Barrett

\brief Contains the IModelContext interface for managing definitions
    and axioms.

*/


#include <memory>
#include <vector>
#include <string>
#include "../ATPLogicAPI.h"
#include "IStatement.h"


namespace atp
{
namespace logic
{


/**
\interface IModelContext

\brief A model context is a collection of definitions and axioms

\details Model contexts are loaded from a file, and become immutable
    after that. They are created via the ILanguage interface. They
    produce knowledge kernels, which are related to model contexts.

\note Every symbol has a name, an ID, and an arity. The ID is pretty
    much just a hash of the name, and is only there for efficiency
    reasons (so that Statement objects don't need to store strings
    for names).
*/
class ATP_LOGIC_API IModelContext
{
public:
    virtual ~IModelContext() = default;

    /**
    \brief Get the name given to this context (e.g. "group theory")
        or return "" if no name was given.
    */
    virtual std::string context_name() const = 0;

    virtual bool is_defined(std::string symbol_name) const = 0;
    virtual bool is_defined(size_t symbol_id) const = 0;

    /**
    \pre is_defined(symbol_id)
    */
    virtual std::string symbol_name(size_t symbol_id) const = 0;

    /**
    \pre is_defined(symbol_name)
    */
    virtual size_t symbol_id(std::string symbol_name) const = 0;

    /**
    \pre is_defined(symbol_id)
    */
    virtual size_t symbol_arity(size_t symbol_id) const = 0;

    /**
    \pre is_defined(symbol_name)
    */
    virtual size_t symbol_arity(std::string symbol_name) const = 0;

    virtual size_t num_axioms() const = 0;

    /**
    \brief Returns a list of all symbol IDs of user-defined constants
    */
    virtual std::vector<size_t> all_constant_symbol_ids() const = 0;

    /**
    \brief Returns a list of all symbol IDs of user-defined functions
    */
    virtual std::vector<size_t> all_function_symbol_ids() const = 0;

    /**
    \pre index < num_axioms()

    \returns The axiom at the given index as a string.
    */
    virtual std::string axiom_at(size_t index) const = 0;
};
typedef std::shared_ptr<IModelContext> ModelContextPtr;


}  // namespace logic
}  // namespace atp


