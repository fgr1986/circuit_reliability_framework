/**
 * @file standard_simulation.hpp
 *
 * @date Created on: April 5, 2016
 *
 * @author Author: Fernando García <fgarcia@die.upm.es> <fernando.garca@gmail.com>
 *
 * @section DESCRIPTION
 *
 * This Class is derived from SpectreSimulation
 * and is responsible for the standard simulation mode.
 *
 */

#ifndef MONTECARLO_STANDARD_SIMULATION_H
#define MONTECARLO_STANDARD_SIMULATION_H

// Radiation simulator
#include "spectre_simulation.hpp"
#include "../simulation_results/montecarlo_simulation_results.hpp"

class MontecarloStandardSimulation : public SpectreSimulation {
public:
	MontecarloStandardSimulation();
	virtual ~MontecarloStandardSimulation();

	/// virtual spectre simulation
	virtual void RunSpectreSimulation();

	void set_montecarlo_iterations( const unsigned int montecarlo_iterations ){ this->montecarlo_iterations = montecarlo_iterations; };

	/**
	 * @brief gets simulation_results
	 * @details
	 * @return simulation_results
	 */
	MontecarloSimulationResults* get_montecarlo_simulation_results(){ return &montecarlo_simulation_results; }
	std::vector<TransientSimulationResults*>* get_montecarlo_transient_simulation_results(){ return &montecarlo_transient_simulation_results; }

	bool ProcessAndAnalyzeMontecarloTransients();

	bool PlotProfileResults();

private:

	unsigned int montecarlo_iterations;
	MontecarloSimulationResults montecarlo_simulation_results;
	std::vector<TransientSimulationResults*> montecarlo_transient_simulation_results;

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

	TransientSimulationResults* CreateTransientSimulationResults(
		const unsigned int montecarloCount );

	bool ProcessInterpolateAndAnalyzeIndividualSpectreTransientResults(
		TransientSimulationResults& individualTransientResults, std::vector<Magnitude*>& analyzedMagnitudes );

};

#endif /* MONTECARLO_STANDARD_SIMULATION_H */
