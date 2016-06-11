/**
 * @file critical_parameter_value_simulation.hpp
 *
 * @date Created on: March 27, 2014
 *
 * @author Author: Fernando García <fgarcia@die.upm.es> <fernando.garca@gmail.com>
 *
 * @section DESCRIPTION
 *
 * This Class is derived from SpectreSimulation
 * and is responsible for the critical parameter simulation mode.
 *
 */

#ifndef CRITICAL_PARAMETER_VALUE_SIMULATION_H
#define CRITICAL_PARAMETER_VALUE_SIMULATION_H

// Radiation simulator
#include "spectre_simulation.hpp"
// netlist modeling
#include "../../netlist_modeling/simulation_parameter.hpp"

class CriticalParameterValueSimulation : public SpectreSimulation {
public:
	CriticalParameterValueSimulation();
	virtual ~CriticalParameterValueSimulation();

	/// virtual spectre simulation
	virtual void RunSimulation() override;

	/**
	 * @brief Set plot_last_transients
	 *
	 * @param plot_last_transients
	 */
	void set_plot_last_transients( bool plot_last_transients ){
		this->plot_last_transients = plot_last_transients;
	}

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
	 * @brief gets simulation results
	 * @details gets a vector of the different simulation results
	 * @return vector<TransientSimulationResults*>
	 */
	std::vector<TransientSimulationResults*>* get_transient_simulations_results() { return &transient_simulations_results; }

	/**
	 * @brief gets simulation results
	 * @details gets a vector of the different simulation results
	 * @return vector<TransientSimulationResults*>
	 */
	TransientSimulationResults* get_last_valid_transient_simulation_results();

private:
	// results of nested simulation
	static constexpr unsigned int NOT_SENSITIVE_2_MAX = 0;
	static constexpr unsigned int SENSITIVE_2_MIN = 1;
	static constexpr unsigned int SENSITIVE_2_OTHER = 2;

	/// plot_last_transients
	bool plot_last_transients;
	/// Critical parameter value
	double critical_parameter_value;
	/// local critical_parameter
	SimulationParameter* local_critical_parameter;
	/// Simulations results
	std::vector<TransientSimulationResults*> transient_simulations_results;

	/// critical_parameter_values_evolution
	std::vector<double> critical_parameter_values_evolution;
	std::string critical_parameter_values_evolution_image_path;
	/// Not affected with the max quantity
	bool not_affected_by_max_value;
	/// Is affected by the min quantity
	bool affected_by_min_value;

	virtual std::string GetSpectreResultsFilePath(const std::string& currentFolder,
		const bool& processMainTransient) override;

	virtual std::string GetProcessedResultsFilePath(const std::string& currentFolder,
		const std::string& localSimulationId, const bool& processMainTransient) override ;

	/**
	 * @brief Simulates a specific scenario
	 * @details Simulates a specific scenario
	 * where the radiation parameters have the default value
	 * and the critical parameter the set up value.
	 *
	 * @param simulationResults
	 * @param parameterVariationCount
	 * @param currentParameterValue
	 * @param localSimulationFolder
	 * @param localSimulationId
	 * @return true if the method correctly ends.
	 */
	bool SimulateParameterCriticalValue( TransientSimulationResults& simulationResults,
		int parameterVariationCount, double currentParameterValue,
		std::string& localSimulationFolder, std::string& localSimulationId  );

	/**
	 * @brief Plots and deletes the transient, if appropriate
	 *
	 * @param simulationResults
	 * @param localSimulationFolder
	 * @param localSimulationId
	 * @return true if the method correctly ends.
	 */
	void HandleIntermediatePlotAndRawResults(
		TransientSimulationResults& simulationResults,
		const std::string localSimulationId );

	/**
	 * @brief Run the previously configured spectre simulation
	 *
	 * @param currentFolder
	 * @param scenario_id
	 * @param parameterChangeCount
	 * @return spectre output
	 */
	int RunSpectre(  std::string currentFolder,
		std::string scenario_id, std::string parameterChangeCount );



	/**
	 * @brief Virtual overrides TestSetup
	 *
	 * @return true if the set up is correct.
	 */
	virtual bool TestSetUp() override;

	/**
	 * @brief Creates a gnuplot of the transient
	 *
	 * @param currentFolder
	 * @param transientSimulationResults
	 * @return gnuplot output
	 */
	// virtual int CreateGnuplotTransientImages(
	// 	std::string currentFolder, TransientSimulationResults& transientSimulationResults );

	/**
	 * @brief Creates a gnuplot of the transient
	 *
	 * @param currentFolder
	 * @return gnuplot output
	 */
	int CreateGnuplotCriticalParameterEvolution( );

	virtual void ReportEndOfCriticalParameterValueSimulation(
		std::string localSimulationFolder, std::string localSimulationId,
		TransientSimulationResults& lastResults, const int endType  );
};

#endif /* CRITICAL_PARAMETER_VALUE_SIMULATION_H */
