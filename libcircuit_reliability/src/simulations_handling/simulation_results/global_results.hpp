/**
 * GLOBAL_RESULTS_H_montecarlo_sweep.hpp
 *
 *  Created on: Jun 16, 2014
 *      Author: fernando
 */

#ifndef GLOBAL_RESULTS_H
#define GLOBAL_RESULTS_H

// boost threads
#include <boost/thread.hpp>
// c++ std libraries
#include <string>
#include <vector>
// Radiation simulator
// radiation io simulator includes
#include "../../io_handling/log_io.hpp"
#include "../../netlist_modeling/simulation_parameter.hpp"
#include "../spectre_simulations/spectre_simulation.hpp"
#include "../simulation_mode.hpp"

class GlobalResults  {
public:

	/**
	 * @brief Default constructor
	 */
	GlobalResults(){};

	/**
	 * @brief Default Destructor
	 */
	~GlobalResults(){};

	/**
	 * @brief Set log_io
	 *
	 * @param log_io
	 */
	void set_log_io( LogIO* log_io ){
		this->log_io = log_io; }

	/**
	 * @brief Set top_folder
	 *
	 * @param top_folder
	 */
	void set_top_folder( std::string top_folder ){
		this->top_folder = top_folder; }

	/**
 	* @details sets simulation_mode and updates selected_radiation_source
 	* @param simulation_mode <SimulationModesHandler*>
 	*/
	void set_simulation_mode( SimulationMode* simulation_mode) { this->simulation_mode = simulation_mode; }


	/**
 	* @details sets simulation_mode and updates selected_radiation_source
 	* @param simulation_mode <SimulationModesHandler*>
 	*/
	void set_simulations( std::vector<SpectreSimulation*>* simulations) { this->simulations = simulations; }

	bool ProcessScenarioStatistics();

private:

	/// Log manager
	LogIO* log_io;
	/// top folder
	std::string top_folder;
	/// simulations
	std::vector<SpectreSimulation*>* simulations;
	/// SimulationsMode
	SimulationMode* simulation_mode;
	/// aux folders
	std::string gnuplot_script_folder;
	std::string images_folder;
	std::string data_folder;

	bool ProcessStandardSimulationMode();

	bool ProcessCriticalParameterValueSimulationMode();
	bool PlotCriticalParameterValueSimulationMode(
		const std::vector<Magnitude*>& analyzedMagnitudes, const unsigned int &critParamOffset,
		const unsigned int& firstMagOffset, const unsigned int& dataPerMagnitudePerLine,
		const SimulationParameter& criticalParameter, const std::string& gnuplotDataFile );

	bool ProcessCriticalParameterNDParametersSweepSimulationMode();
};

#endif /* GLOBAL_RESULTS_H */
