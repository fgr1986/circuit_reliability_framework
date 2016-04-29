/**
* @file transient_simulation_results.hpp
*
* @date Created on: March 27, 2014
*
* @author Author: Fernando García <fgarcia@die.upm.es> <fernando.garca@gmail.com>
*
* @section DESCRIPTION
*
* This Class is a container of the simulation results.
* It is related to a single transient.
*
*/

#ifndef MONTECARLO_SIMULATION_RESULTS_H
#define MONTECARLO_SIMULATION_RESULTS_H

// c++ std libraries
#include <string>
#include <vector>
#include <map>
// Radiation simulator
#include "../magnitude_errors.hpp"
#include "../../netlist_modeling/simulation_parameter.hpp"

// structure containing the summary results
// regarding a specific metric of several montecarlo transients
struct metric_montecarlo_results_t {
	std::string metric_magnitude_name;
	std::string mc_scatter_image_path;
	// includes punctual errors
	double max_error_global;
	// only errors considered in metric
	double max_error_metric;
	double min_error_metric;
	double mean_max_error_metric;
	double median_max_error_metric;
	double q12_max_error_metric;
	double q34_max_error_metric;
};

class MontecarloSimulationResults {
public:
	MontecarloSimulationResults();
	virtual ~MontecarloSimulationResults();

	/**
	* @brief Add a simulation parameter
	*
	* @param parameter
	*/
	void RegisterSimulationParameters( std::vector<SimulationParameter*>* simulationParameters );

	/**
	* @brief set spectre_result
	*
	* @param spectre_result
	*/
	void set_spectre_result( int spectre_result ){ this->spectre_result = spectre_result; }

	/**
	* @brief gets spectre_result
	* @return spectre_result
	*/
	int get_spectre_result() const { return spectre_result; }

	/**
	* @brief gets the lists of errors in the magnitudes
	* @return magnitudes_errors
	*/
	std::vector<metric_montecarlo_results_t*>* get_metric_montecarlo_results(){ return &metric_montecarlo_results; }

	void AddMetricMontecarloResults( metric_montecarlo_results_t* mMCResults );

	/**
	* @brief get simulation_parameters list
	* @return simulation_parameters
	*/
	std::map<std::string,std::string>* get_simulation_parameters(){ return &simulation_parameters; }

	void set_critical_parameter_value_image_path( const std::string critical_parameter_value_image_path ){
		this->critical_parameter_value_image_path = critical_parameter_value_image_path;
	}

	void set_critical_parameter_value_data_path( const std::string critical_parameter_value_data_path ){
		this->critical_parameter_value_data_path = critical_parameter_value_data_path;
	}

	std::string get_critical_parameter_value_data_path(){
		return critical_parameter_value_data_path;
	}

	/**
	* @brief sets full_id
	*
	* @param full_id
	*/
	void set_full_id( const std::string full_id ){
		this->full_id = full_id;
	}

	/**
	* @brief sets upsets_count
	*
	* @param upsets_count
	*/
	void set_upsets_count( const unsigned int upsets_count ){
		this->upsets_count = upsets_count;
	}

	/**
	* @brief gets upsets_count
	* @return upsets_count
	*/
	unsigned int get_upsets_count() const{return upsets_count;}

	/**
	* @brief sets mean_critical_parameter_value
	*
	* @param mean_critical_parameter_value
	*/
	void set_mean_critical_parameter_value( const double mean_critical_parameter_value ){
		this->mean_critical_parameter_value = mean_critical_parameter_value;
	}

	/**
	* @brief gets mean_critical_parameter_value
	* @return mean_critical_parameter_value
	*/
	double get_mean_critical_parameter_value() const{return mean_critical_parameter_value;}

	/**
	* @brief gets full_id
	* @return full_id
	*/
	std::string get_full_id() const{return full_id;}

	void ReserveMetricMontecarloResults( unsigned int metricMontecarloResultsLength );

private:
	/// simulated parameters: name-value
	std::map<std::string,std::string> simulation_parameters;
	/// critical parameter value
	double mean_critical_parameter_value;
	/// upsets count
	unsigned int upsets_count;
	/// results for montecarlo_critical_parameter_value_simulation_nd_parameter sim
	std::string critical_parameter_value_image_path;
	/// results for montecarlo_critical_parameter_value_simulation_nd_parameter sim
	std::string critical_parameter_value_data_path;
	/// magnitude errors: magnitude-has_error
	std::vector<metric_montecarlo_results_t*> metric_montecarlo_results;
	/// spectre result
	int spectre_result;
	/// simulation result full_id (also related to the folder)
	std::string full_id;
};

#endif /* MONTECARLO_SIMULATION_RESULTS_H */
