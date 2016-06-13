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

#ifndef CRITICAL_PARAMETER_ND_PARAMETERS_SWEEP_SIMULATION
#define CRITICAL_PARAMETER_ND_PARAMETERS_SWEEP_SIMULATION

// Radiation simulator
#include "spectre_simulation.hpp"
#include "spectre_simulations_vector.hpp"
#include "critical_parameter_value_simulation.hpp"
// netlist modeling
#include "../../netlist_modeling/simulation_parameter.hpp"
#include "../../netlist_modeling/statements/control_statement.hpp"

class CriticalParameterNDParameterSweepSimulation : public SpectreSimulation {
public:
	CriticalParameterNDParameterSweepSimulation();
	virtual ~CriticalParameterNDParameterSweepSimulation();

	/// virtual spectre simulation
	virtual void RunSimulation() override;

	/**
	 * @brief get critical parameter value
	 * @return critical_parameter_value
	 */
	double get_critical_parameter_value() const {return critical_parameter_value;}

	/**
	 * @brief get not_affected_by_max_value
	 * @return not_affected_by_max_value
	 */
	bool get_not_affected_by_max_value() const{ return not_affected_by_max_value;}


	/**
	 * @brief get affected_by_min_value
	 * @return affected_by_min_value
	 */
	bool get_affected_by_min_value() const{ return affected_by_min_value; }

	/**
	 * @brief gets the list of spectre_simulations_vector
	 * @return vector<SpectreSimulationVector*>
	 */
	SpectreSimulationsVector* get_critical_parameter_value_simulations_vector(){ return &critical_parameter_value_simulations_vector ;}

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

	std::vector<unsigned int>* get_magnitude_column_indexes(){ return &magnitude_column_indexes; }

private:

	/// Critical parameter value
	double critical_parameter_value;
	/// Not affected with the max quantity
	bool not_affected_by_max_value;
	bool affected_by_min_value;
	/// Simulation results
	SpectreSimulationsVector critical_parameter_value_simulations_vector;
	/// Max parallel instances per sweep
	int max_parallel_profile_instances;
	/// plot last transient
	bool plot_last_transients;
	/// data per magnitude per line for gnuplot maps
	const int data_per_magnitude_per_line = 3;
	std::vector<unsigned int> magnitude_column_indexes;

	bool InitMagnitudeColumnIndexes( const std::vector<Magnitude*>& auxMagnitudes );
	/**
	 * @brief Creates a new critical_parameter_value_simulation instance
	 *
	 * @param currentFolder
	 * @param parameterCountIndexes
	 * @param parameters2sweep
	 * @param ndProfileIndex
	 * @return The new critical_parameter_value_simulation pointer
	 */
	CriticalParameterValueSimulation* CreateCriticalParameterValueSimulation(
		const std::string& currentFolder, const std::vector<unsigned int> & parameterCountIndexes,
		std::vector<SimulationParameter*>& parameters2sweep, const int ndProfileIndex  );

	/**
	 * @brief Virtual overrides TestSetup
	 *
	 * @return true if the set up is correct.
	 */
	virtual bool TestSetUp() override;

	/**
	 * @brief Creates profile simulation
	 * @details Creates sweep simulation
	 *
	 * @param parameterCountIndexes
	 * @param parameters2sweep
	 * @param ndProfileIndex
	 */
	CriticalParameterValueSimulation* CreateProfile(
		const std::vector<unsigned int> & parameterCountIndexes,
		std::vector<SimulationParameter*>& parameters2sweep, const unsigned int ndProfileIndex );

	/**
	 * @brief Run profile simulation
	 * @details Run sweep simulation
	 *
	 * @param cpv simulation
	 */
	void RunProfile( CriticalParameterValueSimulation* pCPVS );

	/// Results Handling

	int GnuplotGeneralResults(
		const std::string& gnuplotSpectreErrorMapFilePath,
		const std::string& gnuplotScriptFolder, const std::string& imagesFolder );

	bool GenerateAndPlotResults(
		const std::vector< SimulationParameter* > & parameters2sweep  );

	bool GenerateAndPlotGeneralResults(
		const std::vector<Magnitude*>& auxMagnitudes,
		const std::vector<SimulationParameter*>& parameters2sweep,
		const std::string& mapsFolder, const std::string& gnuplotScriptFolder, const std::string& imagesFolder );

	int GnuplotGeneralMetricMagnitudeResults(
		const std::vector<Magnitude*>& analyzedMagnitudes, double& maxCritCharge,
		const std::string& mapsFolder, const std::string& gnuplotScriptFolder, const std::string& imagesFolder );

	bool GenerateAndPlotParameterPairResults(
		const std::vector<Magnitude*>& auxMagnitudes, const unsigned int& totalAnalizableMagnitudes,
		const unsigned int& p1Index, const unsigned int& p2Index,
		const std::vector<SimulationParameter*>& parameters2sweep,
		const std::string& mapsFolder, const std::string& gnuplotScriptFolder, const std::string& imagesFolder );

	bool GenerateAndPlotItemizedPlane(
		const std::vector<Magnitude*>& auxMagnitudes,
		const unsigned int& p1Index, const unsigned int& p2Index, const unsigned int& itemizedCount,
		const std::vector<SimulationParameter*>& parameters2sweep,
		const std::string& mapsFolder, const std::string& gnuplotScriptFolder, const std::string& imagesFolder,
		const std::vector<unsigned int>& profileIndexesInPlane, PlaneResultsStructure& plane );

	int GnuplotPlane(
		PlaneResultsStructure& plane, const bool isPartialPlane,
		const SimulationParameter& p1, const SimulationParameter& p2,
		const std::string& partialPlaneId, const std::string& gnuplotDataFile,
		const std::string& gnuplotScriptFolder, const std::string& imagesFolder  );

	int GnuplotPlaneMagnitudeResults(
		const std::vector<Magnitude*>& analyzedMagnitudes,
		PlaneResultsStructure& plane, const bool isPartialPlane,
		const SimulationParameter& p1, const SimulationParameter& p2,
		const unsigned int& partialPlaneCount,
		const std::string& partialPlaneId, const std::string& gnuplotDataFile,
		const std::string& gnuplotScriptFolder, const std::string& imagesFolder );
};

#endif /* CRITICAL_PARAMETER_ND_PARAMETERS_SWEEP_SIMULATION */
