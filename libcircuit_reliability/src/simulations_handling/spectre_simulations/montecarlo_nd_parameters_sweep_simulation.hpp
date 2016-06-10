/**
 * @file critical_parameter_ND_parameter_sweep_simulation.hpp
 *
 * @date Created on: March 27, 2014
 *
 * @author Author: Fernando García <fgarcia@die.upm.es> <fernando.garca@gmail.com>
 *
 * @section DESCRIPTION
 *
 * This Class is derived from SpectreSimulation
 * and is responsible for the simple sweep simulation mode.
 *
 */

#ifndef MONTECARLO_ND_PARAMETERS_SWEEP_SIMULATION_H
#define MONTECARLO_ND_PARAMETERS_SWEEP_SIMULATION_H

// Radiation simulator
// #include "spectre_simulation.hpp"
#include "spectre_simulations_vector.hpp"
#include "montecarlo_simulation.hpp"
// netlist modeling
#include "../../netlist_modeling/simulation_parameter.hpp"
#include "../../netlist_modeling/statements/control_statement.hpp"

class MontecarloNDParametersSweepSimulation : public SpectreSimulation {
public:
	MontecarloNDParametersSweepSimulation();
	virtual ~MontecarloNDParametersSweepSimulation();

	/// virtual spectre simulation
	virtual void RunSpectreSimulation();

	/**
	 * @brief gets the list of spectre_simulations_vector
	 * @return vector<SpectreSimulationVector*>
	 */
	SpectreSimulationsVector* get_montecarlo_standard_simulations_vector(){ return &montecarlo_standard_simulations_vector ;}

	/**
	 * @brief Sets max_parallel_profile_instances
	 *
	 * @param max_parallel_profile_instances
	 */
	void set_max_parallel_profile_instances( unsigned int max_parallel_profile_instances ){
		this->max_parallel_profile_instances = max_parallel_profile_instances; }

	/**
	 * @brief Sets max_parallel_montecarlo_instances
	 *
	 * @param max_parallel_montecarlo_instances
	 */
	void set_max_parallel_montecarlo_instances( unsigned int max_parallel_montecarlo_instances ){
		this->max_parallel_montecarlo_instances = max_parallel_montecarlo_instances; }

	/**
	 * @brief Sets montecarlo_iterations
	 *
	 * @param montecarlo_iterations
	 */
	void set_montecarlo_iterations( unsigned int montecarlo_iterations ){
		this->montecarlo_iterations = montecarlo_iterations; }

	/**
	 * @brief gets the plot_last_transients
	 * @return boolean plot_last_transients
	 */
	bool get_plot_last_transients(){ return plot_last_transients; }

	/**
	 * @brief Sets plot_last_transients
	 *
	 * @param plot_last_transients
	 */
	void set_plot_last_transients( bool plot_last_transients) { this->plot_last_transients = plot_last_transients; }

private:
	/// Montecarlo Iterations
	unsigned int montecarlo_iterations;
	/// Simulation results, organizated by parameter (first vector)
	SpectreSimulationsVector montecarlo_standard_simulations_vector;
	/// Max parallel instances per sweep
	unsigned int max_parallel_profile_instances;
	/// Max parallel instances per montecarlo
	unsigned int max_parallel_montecarlo_instances;
	/// plot last transient
	bool plot_last_transients;
	/// data per magnitude per line for gnuplot maps
	const int data_per_magnitude_per_line = 8;

	/**
	 * @brief Virtual overrides TestSetup
	 *
	 * @return true if the set up is correct.
	 */
	virtual bool TestSetUp() override;

	/**
	 * @brief Creates a new critical_parameter_value_simulation instance
	 *
	 * @param currentFolder
	 * @param parameterCountIndexes
	 * @param parameters2sweep
	 * @param threadNumber
	 * @return The new critical_parameter_value_simulation pointer
	 */
	MontecarloSimulation* CreateMontecarloSimulation(
		const std::string currentFolder, const std::vector<unsigned int> & parameterCountIndexes,
		std::vector<SimulationParameter*>& parameters2sweep, const int threadNumber  );
		/**
	 * @brief Creates profile simulation
	 * @details Creates sweep simulation
	 *
	 * @param parameterCountIndexes
	 * @param parameters2sweep
	 * @param threadNumber
	 */
	MontecarloSimulation* CreateProfile(
		const std::vector<unsigned int> & parameterCountIndexes,
		std::vector<SimulationParameter*>& parameters2sweep, const unsigned int threadNumber );

	/**
	 * @brief Run profile simulation
	 * @details Run sweep simulation
	 *
	 * @param cpv simulation
	 */
	void RunProfile( MontecarloSimulation* pCPVS );

	bool GenerateAndPlotResults(
		const std::vector< SimulationParameter* > & parameters2sweep  );

	bool ProcessInterpolateAndAnalyzeSpectreResults(
			MontecarloSimulation& mcStandardSim );

	bool GenerateAndPlotGeneralResults(
			const std::vector<Magnitude*>& auxMagnitudes,
			const std::vector< SimulationParameter*>& parameters2sweep,
			const std::string& mapsFolder, const std::string& gnuplotScriptFolder, const std::string& imagesFolder );

	int GnuplotGeneralResults(
		const std::string& gnuplotSpectreErrorMapFilePath,
		const std::string& gnuplotScriptFolder, const std::string& imagesFolder );

	int GnuplotGeneralMetricMagnitudeResults( const std::vector<Magnitude*>& analyzedMagnitudes,
		double& maxUpsetRatio, const std::string& mapsFolder, const std::string& gnuplotScriptFolder, const std::string& imagesFolder );

	bool GenerateAndPlotParameterPairResults(
		const std::vector<Magnitude*>& auxMagnitudes, const unsigned int& totalAnalizableMagnitudes,
		const unsigned int& p1Index, const unsigned int& p2Index,
		const std::vector<SimulationParameter*>& parameters2sweep,
		const std::string& mapsFolder, const std::string& gnuplotScriptFolder, const std::string& imagesFolder );

	bool GenerateAndPlotItemizedPlane(
		const std::vector<Magnitude*>& auxMagnitudes,
		const unsigned int& p1Index, const unsigned int& p2Index, const unsigned int& profCount,
		const std::vector< SimulationParameter*>& parameters2sweep,
		const std::string& mapsFolder, const std::string& gnuplotScriptFolder, const std::string& imagesFolder,
		const std::vector<unsigned int>& profileIndexesInPlane, PlaneResultsStructure& plane );

	int GnuplotPlane( PlaneResultsStructure& plane, const bool isPartialPlane,
		const SimulationParameter& p1, const SimulationParameter& p2,
		const std::string& partialPlaneId, const std::string& gnuplotDataFile,
		const std::string& gnuplotScriptFolder, const std::string& imagesFolder );

	int GnuplotPlaneMagnitudeResults(
		const std::vector<Magnitude*>& analyzedMagnitudes,
		PlaneResultsStructure& plane, const bool isPartialPlane,
		const SimulationParameter& p1, const SimulationParameter& p2,
		const unsigned int& partialPlaneCount,
		const std::string& partialPlaneId, const std::string& gnuplotDataFile,
		const std::string& gnuplotScriptFolder, const std::string& imagesFolder );
};

#endif /* MONTECARLO_ND_PARAMETERS_SWEEP_SIMULATION_H */
