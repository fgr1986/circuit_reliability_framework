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

	bool GenerateAndPlotParameterPairResults(
			const unsigned int statisticMethod, const std::string& criticalParameterName,
			const unsigned int critParamOffset, const unsigned int magMetricColumnOffset, const unsigned int magGlobalColumnOffset,
			const unsigned int dataPerMetricPerLine, const std::vector<Metric*>& analyzedMetrics,
			const std::vector<SimulationParameter*>& simulationParameters,
			const std::vector<unsigned int>&& magGPCIMax, const std::vector<unsigned int>&& magGPCIMin,
			const std::vector<unsigned int>&& magGPCIMean, const std::vector<NDSimulationResults*>& simulationsNDSimulationResults );

	int GnuplotPlaneMetricResults(
		const unsigned int magMetricColumnOffset,
		const unsigned int magGlobalColumnOffset,
		const unsigned int dataPerMetricPerLine,
		const std::vector<Metric*>& analyzedMetrics,
		const SimulationParameter& p1, const SimulationParameter& p2,
		const std::string& partialPlaneId, const std::string& gnuplotDataFile,
		const std::string& gnuplotScriptFolder, const std::string& imagesFolder );

	int GnuplotCriticalParameterValuePlane(
		const unsigned int critParamOffset, const std::string& criticalParameterName,
		const SimulationParameter& p1, const SimulationParameter& p2,
		const std::string& partialPlaneId, const std::string& gnuplotDataFile,
		const std::string& gnuplotScriptFolder, const std::string& imagesFolder );

	bool ProcessStandardSimulationMode();

	bool ProcessCriticalParameterValueSimulationMode();
	bool PlotCriticalParameterValueSimulationMode(
		const std::vector<Metric*>& analyzedMetrics, const unsigned int &critParamOffset,
		const unsigned int& firstMagOffset, const unsigned int& dataPerMetricPerLine,
		const SimulationParameter& criticalParameter, const std::string& gnuplotDataFile );

	bool ProcessCriticalParameterNDParametersSweepSimulationMode();
	bool PlotCriticalParameterNDParametersSweepSimulationMode(
			const std::vector<Metric*>& analyzedMetrics, const unsigned int &critParamOffset,
			const unsigned int& profileFirstMagMetricOffset, const unsigned int& profileFirstMagGlobalOffset, const unsigned int& dataColumnsPerMetric,
			const SimulationParameter& criticalParameter, const std::string& gnuplotDataFile );

	bool ProcessMontecarloCriticalParameterNDParametersSweepMode();
	bool PlotMontecarloCriticalParameterNDParametersSweepMode(
			const std::vector<Metric*>& analyzedMetrics, const unsigned int &critParamOffset,
			const unsigned int& profileFirstMagMetricOffset, const unsigned int& profileFirstMagGlobalOffset,
			const unsigned int& dataColumnsPerMetric,
			const SimulationParameter& criticalParameter, const std::string& gnuplotDataFile );

};

#endif /* GLOBAL_RESULTS_H */
