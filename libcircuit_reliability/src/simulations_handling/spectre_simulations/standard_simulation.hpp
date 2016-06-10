/**
 * @file standard_simulation.hpp
 *
 * @date Created on: March 27, 2014
 *
 * @author Author: Fernando García <fgarcia@die.upm.es> <fernando.garca@gmail.com>
 *
 * @section DESCRIPTION
 *
 * This Class is derived from SpectreSimulation
 * and is responsible for the standard simulation mode.
 *
 */

#ifndef STANDARD_SIMULATION_H
#define STANDARD_SIMULATION_H

// Radiation simulator
#include "spectre_simulation.hpp"

class StandardSimulation : public SpectreSimulation {
public:
	StandardSimulation();
	virtual ~StandardSimulation();

	/// virtual spectre simulation
	virtual void RunSpectreSimulation();

	/**
	 * @brief gets simulation_results
	 * @details
	 * @return simulation_results
	 */
	TransientSimulationResults* get_basic_simulation_results(){ return &basic_simulation_results; }


private:
	/// Pointer to the only simulation results present in the list.
	TransientSimulationResults basic_simulation_results;

	virtual std::string GetSpectreResultsFilePath(const std::string& currentFolder,
		const bool& processMainTransient) override;

	virtual std::string GetProcessedResultsFilePath(const std::string& currentFolder,
		const std::string& localSimulationId, const bool& processMainTransient) override ;

	/**
	 * @brief Run the previously configured spectre simulation
	 *
	 * @param scenario_id
	 * @return spectre output
	 */
	int RunSpectre( std::string scenario_id );

	/**
	 * @brief Virtual overrides TestSetup
	 *
	 * @return true if the set up is correct.
	 */
	virtual bool TestSetUp() override;

};

#endif /* STANDARD_SIMULATION_H */
