#pragma once


/*

EquationalStatement.h

Implementation of the IStatement interface for equational logic. In
equational logic, the main idea is to try to deduce if two things are
equal using a set of equality rules given in a definition file.

*/


#include "../../ATPLogicAPI.h"
#include "../../Interfaces/IStatement.h"


namespace atp
{
namespace logic
{


class ATP_LOGIC_API EquationalStatement : public IStatement
{
public:

	virtual StmtForm form() const override;
	virtual std::string to_str() const override;
};


}  // namespace logic
}  // namespace atp


