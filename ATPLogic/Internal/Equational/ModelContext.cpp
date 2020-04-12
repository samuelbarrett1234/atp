/**

\file

\author Samuel Barrett

*/


#include "ModelContext.h"
#include <sstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/algorithm/string/trim.hpp>


namespace pt = boost::property_tree;


namespace atp
{
namespace logic
{
namespace equational
{


// factored-out helper function for hashing a string name
static size_t hash_name(const std::string& name)
{
    return std::hash<std::string>()(name);
}


ModelContextPtr ModelContext::try_construct(const Language& parent,
    std::istream& in)
{
    // allocate new object to fill

    auto p_ctx = std::make_shared<ModelContext>();
    pt::ptree ptree;

    try
    {
        pt::read_json(in, ptree);  // todo: what about exceptions?

        p_ctx->m_name = ptree.get<std::string>("name", "");

        // load definitions
        if (auto defs = ptree.get_child_optional("definitions"))
        {
            for (auto def : defs.get())
            {
                std::string def_name =
                    def.second.get<std::string>("name");
                boost::algorithm::trim<std::string>(def_name);

                if (p_ctx->is_defined(def_name))
                    return ModelContextPtr();  // redefinition

                const size_t def_arity =
                    def.second.get<size_t>("arity");

                const size_t id = hash_name(def_name);

                p_ctx->m_name_to_id.left.insert(
                    std::make_pair(def_name, id));

                p_ctx->m_id_to_arity[id] = def_arity;

                if (def_arity == 0)
                    p_ctx->m_const_ids.push_back(id);
                else
                    p_ctx->m_func_ids.push_back(id);
            }
        }

        // load axioms
        if (auto axs = ptree.get_child_optional("axioms"))
        {
            for (auto ax : axs.get())
            {
                p_ctx->m_axioms.push_back(
                    ax.second.get_value<std::string>());
            }
        }

        return p_ctx;
    }
    catch (pt::ptree_error&)
    {
        return ModelContextPtr();
    }
}


}  // namespace equational
}  // namespace logic
}  // namespace atp


