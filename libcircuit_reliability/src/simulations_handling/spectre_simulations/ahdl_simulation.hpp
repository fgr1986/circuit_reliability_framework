/**
 * @file golden_simulation.hpp
 *
 * @date Created on: March 27, 2014
 *
 * @author Author: Fernando García <fgarcia@die.upm.es> <fernando.garca@gmail.com>
 *
 * @section DESCRIPTION
 *
 * This Class is derived from SpectreSimulation
 * and is responsible for the singular simulation mode.
 *
 */

#ifndef AHDL_SIMULATION_H
#define AHDL_SIMULATION_H

// Radiation simulator
#include "spectre_simulation.hpp"

class AHDLSimulation : public SpectreSimulation {
public:
	AHDLSimulation();
	virtual ~AHDLSimulation();

	/// virtual spectre simulation
	virtual void RunSimulation() override;

	/**
	 * @brief gets basic_simulation_results
	 * @details
	 * @return basic_simulation_results
	 */
	TransientSimulationResults* get_simulation_results(){ return &basic_simulation_results; }

	/**
	 * @brief sets main_simulation_mode
	 *
	 * @param main_simulation_mode
	 */
	// void set_main_simulation_mode(int main_simulation_mode){this->main_simulation_mode = main_simulation_mode;}

	/**
	 * @brief get singular results path
	 * @return path to the singular results file
	 */
	std::string get_singular_results_path() const{ return singular_results_path; }

	std::string GetSpectreResultsFilePath();

	std::string GetSpectreLogFilePath();

private:
	/// Simulation Results
	TransientSimulationResults basic_simulation_results;
	bool is_golden;
	/// Singular results path
	std::string singular_results_path;

	/**
	 * @brief Run the previously configured spectre simulation
	 *
	 * @return spectre output
	 */
	int RunSpectre( );

	/**
	 * @brief Virtual overrides TestSetup
	 *
	 * @return true if the set up is correct.
	 */
	virtual bool TestSetUp() override;
};

#endif /* AHDL_SIMULATION_H */
