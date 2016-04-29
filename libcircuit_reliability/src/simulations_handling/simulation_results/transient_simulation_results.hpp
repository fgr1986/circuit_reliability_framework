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

#ifndef TRANSIENT_SIMULATION_RESULTS_H
#define TRANSIENT_SIMULATION_RESULTS_H

// c++ std libraries
#include <string>
#include <vector>
#include <map>
// Radiation simulator
#include "../magnitude_errors.hpp"
#include "../../netlist_modeling/simulation_parameter.hpp"

class TransientSimulationResults {
public:
	TransientSimulationResults();
	virtual ~TransientSimulationResults();

	/**
	* @brief Add a simulation parameter
	*
	* @param parameter
	*/
	// void AddSimulationParameter( SimulationParameter* parameter );

	/**
	* @brief Add a simulation parameter
	*
	* @param parameter
	*/
	void RegisterSimulationParameters( std::vector<SimulationParameter*>* simulationParameters );

	/**
	* @brief Adds a new Magnitude-error pair
	*
	* @param magnitude
	* @param error
	*/
	void AddMagnitudeErrors( MagnitudeErrors* magnitudeErrors );

	/**
	* @brief set spectre_result
	*
	* @param spectre_result
	*/
	void set_spectre_result( int spectre_result ){ this->spectre_result = spectre_result; }

	/**
	* @brief set reliability_result
	*
	* @param reliability_result
	*/
	void set_reliability_result( int reliability_result ){ this->reliability_result = reliability_result; }

	/**
	* @brief gets spectre_result
	* @return spectre_result
	*/
	int get_spectre_result() const { return spectre_result; }

	/**
	* @brief gets reliability_result
	* @return reliability_result
	*/
	unsigned int get_reliability_result() const { return reliability_result; }

	/**
	* @brief gets reliability_result
	* @return reliability_result
	*/
	std::string get_s_reliability_result();

	/**
	* @brief gets the lists of errors in the magnitudes
	* @return magnitudes_errors
	*/
	std::vector<MagnitudeErrors*>* get_magnitudes_errors(){ return &magnitudes_errors; }

	/**
	* @brief get simulation_parameters list
	* @return simulation_parameters
	*/
	std::map<std::string,std::string>* get_simulation_parameters(){ return &simulation_parameters; }

	/**
	* @brief sets full_id
	*
	* @param full_id
	*/
	void set_full_id( std::string full_id ){
		this->full_id = full_id;
	}

	/**
	* @brief gets full_id
	* @return full_id
	*/
	std::string get_full_id() const{return full_id;}

	/**
	* @brief and an image
	*
	* @param itransientIageTitle
	* @param transientImagePath
	*/
	void AddTransientImage(
		const std::string& transientImagePath, const std::string&  transientImageTitle  );

	/**
	* @brief get image path
	* @return return image_path
	*/
	std::map<std::string,std::string>* get_transient_image_paths(){ return &transient_image_paths; }

	/**
	* @brief sets original_file_path
	*
	* @param original_file_path
	*/
	void set_original_file_path( const std::string original_file_path ){ this->original_file_path = original_file_path; }

	/**
	* @brief get original_file_path
	* @return return original_file_path
	*/
	std::string get_original_file_path() const{ return original_file_path; }

	/**
	* @brief sets processed_file_path
	*
	* @param image_path
	*/
	void set_processed_file_path( std::string processed_file_path ){ this->processed_file_path = processed_file_path; }

	/**
	* @brief get processed_file_path
	* @return return processed_file_path
	*/
	std::string get_processed_file_path() const{ return processed_file_path; }


	/**
	* @brief sets title
	*
	* @param image_path
	*/
	void set_title( std::string title ){ this->title = title; }

	/**
	* @brief get title
	* @return return title
	*/
	std::string get_title() const{ return title; }

	/**
	* @brief returns true if there has been any error in any magnitude
	* @return has_magnitudes_errors
	*/
	bool get_has_magnitudes_errors() const{return has_magnitudes_errors;}

	void set_has_magnitudes_errors( bool has_magnitudes_errors ){ this->has_magnitudes_errors = has_magnitudes_errors; }

private:
	/// simulated parameters: name-value
	std::map<std::string,std::string> simulation_parameters;
	/// magnitude errors:
	/// one per magnitude
	std::vector<MagnitudeErrors*> magnitudes_errors;
	/// has magnitude errors
	bool has_magnitudes_errors;
	/// spectre result
	int spectre_result;
	/// reliability result
	unsigned int reliability_result;
	/// simulation result full_id (also related to the folder)
	std::string full_id;
	/// simulation result title
	std::string title;

	/// paths to the exported transient images (paths_title)
	std::map<std::string,std::string> transient_image_paths;

	/// filepath from title
	std::string original_file_path;
	/// filepath from title
	std::string processed_file_path;
};

#endif /* TRANSIENT_SIMULATION_RESULTS_H */
