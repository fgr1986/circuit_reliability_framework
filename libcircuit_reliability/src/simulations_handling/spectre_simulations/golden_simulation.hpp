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

#ifndef GOLDEN_SIMULATION_H
#define GOLDEN_SIMULATION_H

// Radiation simulator
#include "spectre_simulation.hpp"

class GoldenSimulation : public SpectreSimulation {
public:
	GoldenSimulation();
	virtual ~GoldenSimulation();

	/// virtual spectre simulation
	virtual void RunSpectreSimulation();

	/**
	 * @brief gets transient_simulation_results
	 * @details
	 * @return transient_simulation_results
	 */
	TransientSimulationResults* get_simulation_results(){ return &transient_simulation_results; }

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

	/**
	 * @brief Retrieve Processed Magnitudes
	 * @details Retrieve Processed Magnitudes
	 * @return magnitudes vector
	 */
	std::vector<Magnitude*>* get_processed_magnitudes() { return processed_magnitudes; }

	/**
	 * @brief Sets magnitudes_2be_found
	 * @parammagnitudes_2be_found
	 */
	void set_magnitudes_2be_found( std::vector<Magnitude*>* magnitudes_2be_found ){
		this->magnitudes_2be_found = magnitudes_2be_found; }

	/**
	 * @brief Retrieve Processed Magnitudes
	 * @details Retrieve Processed Magnitudes
	 * @return processed NDMagnitudesStructure
	 */
	NDMagnitudesStructure* GetGoldenMagnitudes();

private:
	/// Simulation Results
	TransientSimulationResults transient_simulation_results;
	/// Singular results path
	std::string singular_results_path;
	/// Golden Simulation saves the magnitudes here
	std::vector<Magnitude*>* processed_magnitudes;
	/// Magnitudes to be found
	std::vector<Magnitude*>* magnitudes_2be_found;


	std::vector<Magnitude*>* CreateGoldenMagnitudesVector();

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

	int CreateGoldenGnuplotTransientImages();

};

#endif /* GOLDEN_SIMULATION_H */
