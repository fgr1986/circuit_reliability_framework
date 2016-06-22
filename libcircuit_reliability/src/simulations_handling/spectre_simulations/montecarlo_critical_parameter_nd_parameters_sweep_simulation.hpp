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

#ifndef MONTECARLO_CRITICAL_PARAMETER_ND_PARAMETERS_SWEEP_SIMULATION_H
#define MONTECARLO_CRITICAL_PARAMETER_ND_PARAMETERS_SWEEP_SIMULATION_H

// Radiation simulator
// #include "spectre_simulation.hpp"
#include "spectre_simulations_vector.hpp"
#include "montecarlo_critical_parameter_value_simulation.hpp"
// netlist modeling
#include "../../netlist_modeling/simulation_parameter.hpp"
#include "../../netlist_modeling/statements/control_statement.hpp"

class MontecarloCriticalParameterNDParametersSweepSimulation : public SpectreSimulation {
public:
	MontecarloCriticalParameterNDParametersSweepSimulation();
	virtual ~MontecarloCriticalParameterNDParametersSweepSimulation();

	/// virtual spectre simulation
	virtual void RunSimulation() override;

	/**
	 * @brief gets the list of spectre_simulations_vector
	 * @return vector<SpectreSimulationVector*>
	 */
	SpectreSimulationsVector* get_montecarlo_critical_parameter_value_simulations_vector(){ return &montecarlo_critical_parameter_value_simulations_vector ;}

	void set_max_parallel_montecarlo_instances( const unsigned int max_parallel_montecarlo_instances ){
		this->max_parallel_montecarlo_instances = max_parallel_montecarlo_instances; };
	/**
	 * @brief Sets max_parallel_profile_instances
	 *
	 * @param max_parallel_profile_instances
	 */
	void set_max_parallel_profile_instances( int max_parallel_profile_instances ){
		this->max_parallel_profile_instances = max_parallel_profile_instances; }

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

	void set_montecarlo_iterations( unsigned int montecarlo_iterations) { this->montecarlo_iterations = montecarlo_iterations; }

	std::vector<unsigned int>* get_metric_column_indexes(){ return &metric_column_indexes; }

private:
	/// Montecarlo Iterations
	unsigned int montecarlo_iterations;
	/// Simulation results, organizated by parameter (first vector)
	SpectreSimulationsVector montecarlo_critical_parameter_value_simulations_vector;
	/// Max parallel instances per sweep
	unsigned int max_parallel_profile_instances;
	/// Max parallel instances per Montecarlo
	unsigned int max_parallel_montecarlo_instances;
	/// plot last transient
	bool plot_last_transients;
	/// data per metric per line for gnuplot maps
	const int data_per_metric_per_line = 8;
	std::vector<unsigned int> metric_column_indexes;

	bool InitMetricColumnIndexes( const std::vector<Metric*>& auxMetricsauxMetrics );

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
	 * @param ndProfileIndex
	 * @return The new critical_parameter_value_simulation pointer
	 */
	MontecarloCriticalParameterValueSimulation* CreateMontecarloCriticalParameterValueSimulation(
		const std::string& currentFolder, const std::vector<unsigned int> & parameterCountIndexes,
		std::vector<SimulationParameter*>& parameters2sweep, const int ndProfileIndex  );
		/**
	 * @brief Creates profile simulation
	 * @details Creates sweep simulation
	 *
	 * @param parameterCountIndexes
	 * @param parameters2sweep
	 * @param ndProfileIndex
	 */
	MontecarloCriticalParameterValueSimulation* CreateProfile(
		const std::vector<unsigned int> & parameterCountIndexes,
		std::vector<SimulationParameter*>& parameters2sweep, const unsigned int ndProfileIndex );

	bool GenerateAndPlotResults(
		const std::vector< SimulationParameter* > & parameters2sweep  );

	bool ProcessInterpolateAndAnalyzeSpectreResults(
			MontecarloCriticalParameterValueSimulation& mcStandardSim );

	bool GenerateAndPlotGeneralResults(
			const std::vector<Metric*>& auxMetrics,
			const std::vector< SimulationParameter*>& parameters2sweep,
			const std::string& mapsFolder, const std::string& gnuplotScriptFolder, const std::string& imagesFolder );

	int GnuplotGeneralResults(
		const std::string& gnuplotSpectreErrorMapFilePath,
		const std::string& gnuplotScriptFolder, const std::string& imagesFolder );

	int GnuplotGeneralMetricMetricResults( const std::vector<Metric*>& analyzedMetrics,
		double& maxUpsetRatio, const std::string& mapsFolder, const std::string& gnuplotScriptFolder, const std::string& imagesFolder );

	bool GenerateAndPlotParameterPairResults(
		const std::vector<Metric*>& auxMetrics, const unsigned int& totalAnalizableMetrics,
		const unsigned int& p1Index, const unsigned int& p2Index,
		const std::vector<SimulationParameter*>& parameters2sweep,
		const std::string& mapsFolder, const std::string& gnuplotScriptFolder, const std::string& imagesFolder );

	bool GenerateAndPlotItemizedPlane(
		const std::vector<Metric*>& auxMetrics,
		const unsigned int& p1Index, const unsigned int& p2Index, const unsigned int& profCount,
		const std::vector< SimulationParameter*>& parameters2sweep,
		const std::string& mapsFolder, const std::string& gnuplotScriptFolder, const std::string& imagesFolder,
		const std::vector<unsigned int>& profileIndexesInPlane, PlaneResultsStructure& plane );

	int GnuplotPlane( PlaneResultsStructure& plane, const bool isPartialPlane,
		const SimulationParameter& p1, const SimulationParameter& p2,
		const std::string& partialPlaneId, const std::string& gnuplotDataFile,
		const std::string& gnuplotScriptFolder, const std::string& imagesFolder );

	int GnuplotPlaneMetricResults(
		const std::vector<Metric*>& analyzedMetrics,
		PlaneResultsStructure& plane, const bool isPartialPlane,
		const SimulationParameter& p1, const SimulationParameter& p2,
		const unsigned int& partialPlaneCount,
		const std::string& partialPlaneId, const std::string& gnuplotDataFile,
		const std::string& gnuplotScriptFolder, const std::string& imagesFolder );
};

#endif /* MONTECARLO_CRITICAL_PARAMETER_ND_PARAMETERS_SWEEP_SIMULATION_H */
