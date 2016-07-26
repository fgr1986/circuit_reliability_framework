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

#ifndef MONTECARLO_CRITICAL_PARAMETER_VALUE_SIMULATION_H
#define MONTECARLO_CRITICAL_PARAMETER_VALUE_SIMULATION_H

// Radiation simulator
#include "spectre_simulation.hpp"
#include "spectre_simulations_vector.hpp"
#include "critical_parameter_value_simulation.hpp"
#include "../simulation_results/montecarlo_simulation_results.hpp"

class MontecarloCriticalParameterValueSimulation : public SpectreSimulation {
public:
	MontecarloCriticalParameterValueSimulation();
	virtual ~MontecarloCriticalParameterValueSimulation();

	/// virtual spectre simulation
	virtual void RunSimulation() override;

	void set_montecarlo_iterations( const unsigned int montecarlo_iterations ){ this->montecarlo_iterations = montecarlo_iterations; };
	void set_max_parallel_montecarlo_instances( const unsigned int max_parallel_montecarlo_instances ){
		this->max_parallel_montecarlo_instances = max_parallel_montecarlo_instances; };

	unsigned int get_correctly_simulated_count(){ return correctly_simulated_count; }
	/**
	 * @brief gets simulation_results
	 * @details
	 * @return simulation_results
	 */
	MontecarloSimulationResults* get_montecarlo_simulation_results(){ return &montecarlo_simulation_results; }
	SpectreSimulationsVector* get_critical_parameter_value_simulations_vector(){
		return &critical_parameter_value_simulations_vector; }

	void set_plot_last_transients(const  bool plot_last_transients) { this->plot_last_transients = plot_last_transients; }
	void set_plot_critical_parameter_value_evolution( const bool plot_critical_parameter_value_evolution) { this->plot_critical_parameter_value_evolution = plot_critical_parameter_value_evolution; }

private:

	unsigned int montecarlo_iterations;
	unsigned int correctly_simulated_count = 0;

	unsigned int max_parallel_montecarlo_instances;

	bool plot_last_transients;
	bool plot_critical_parameter_value_evolution;
	MontecarloSimulationResults montecarlo_simulation_results;
	SpectreSimulationsVector critical_parameter_value_simulations_vector;

	CriticalParameterValueSimulation* CreateMonteCarloIteration( unsigned int threadNumber );

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

	bool AnalyzeMontecarloResults();

	bool PlotScatters();

	bool PlotMetricScatters(
		const std::string& gnuplotScriptFolder, const std::string& imagesFolder );

};

#endif /* MONTECARLO_CRITICAL_PARAMETER_VALUE_SIMULATION_H */
