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
	virtual void RunSimulation() override;

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
	 * @brief Retrieve Processed Metrics
	 * @details Retrieve Processed Metrics
	 * @return metrics vector
	 */
	std::vector<Metric*>* get_processed_metrics() { return processed_metrics; }

	/**
	 * @brief Retrieve Processed Metrics
	 * @details Retrieve Processed Metrics
	 * @return metrics vector
	 */
	void set_processed_metrics( std::vector<Metric*>* processed_metrics ) { this->processed_metrics = processed_metrics; }

	/**
	 * @brief Sets metrics_2be_found
	 * @parammetrics_2be_found
	 */
	void set_metrics_2be_found( std::vector<Metric*>* metrics_2be_found ){
		this->metrics_2be_found = metrics_2be_found; }

	/**
	 * @brief Retrieve Processed Metrics
	 * @details Retrieve Processed Metrics
	 * @return processed NDMetricsStructure
	 */
	NDMetricsStructure* GetGoldenMetrics();

	/**
	 * @brief Sets similarComputedGS
	 * @param similarComputedGS
	 */
	void set_similarComputedGS( GoldenSimulation* similarComputedGS ){
		this->similarComputedGS = similarComputedGS; }


	/**
	 * @brief copies processed_metrics
	 * @param processed_metrics
	 */
	bool CopyResultsFromGoldenSimulation( GoldenSimulation* similarSimulatedGS );

	/**
	 * @brief Sets similarComputedGS
	 * @param similarComputedGS
	 */
	GoldenSimulation* get_similarComputedGS(){ return similarComputedGS; }

private:
	/// Simulation Results
	TransientSimulationResults transient_simulation_results;
	/// Singular results path
	std::string singular_results_path;
	/// Golden Simulation saves the metrics here
	std::vector<Metric*>* processed_metrics;
	/// Metrics to be found
	std::vector<Metric*>* metrics_2be_found;

	/// new in 3.2.0
	GoldenSimulation* similarComputedGS;

	std::vector<Metric*>* CreateGoldenMetricsVector();

	/**
	 * @brief Run the previously configured spectre simulation
	 *
	 * @return spectre output
	 */
	int RunSpectre();

	/**
	 * @brief Virtual overrides TestSetup
	 *
	 * @return true if the set up is correct.
	 */
	virtual bool TestSetUp() override;

	int CreateGoldenGnuplotTransientImages();

};

#endif /* GOLDEN_SIMULATION_H */
