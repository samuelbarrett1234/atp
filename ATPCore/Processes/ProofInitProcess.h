#pragma once


/**
\file

\author Samuel Barrett

\brief Contains the process which takes setup information about the
	proof setup, and creates all the information needed to carry
	out the proof.

*/


#include "../ATPCoreAPI.h"
#include "IProcess.h"
#include "CommonProcessData.h"


namespace atp
{
namespace core
{


/**
\brief This function creates a "proof initialisation" process, which
	creates the knowledge kernel, creates the solver, and loads the
	"helper theorems" from the database to aid in the proof.

\pre All the data in `setup_data` is valid

\returns A new process.
*/
ATP_CORE_API ProcessPtr create_proof_init_process(
	proc_data::ProofSetupEssentials& setup_data,
	proc_data::ProofEssentials& proof_data);


}  // namespace core
}  // namespace atp


