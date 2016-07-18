/**
 * ND_SIMULATION_RESULTS_H.hpp
 *
 *  Created on: Jun 16, 2014
 *      Author: fernando
 */

#ifndef ND_SIMULATION_RESULTS_H
#define ND_SIMULATION_RESULTS_H

// c++ std libraries
#include <string>
#include <vector>
// Radiation simulator
#include "plane_results_structure.hpp"

class NDSimulationResults  {
public:

	/**
	 * @brief Default constructor
	 */
	NDSimulationResults();
	virtual ~NDSimulationResults();

	void set_simulation_id( std::string simulation_id ){ this->simulation_id = simulation_id; }
	std::string get_simulation_id(){ return simulation_id; }

	void set_general_image_path( std::string general_image_path ){ this->general_image_path = general_image_path; }
	std::string get_general_image_path(){ return general_image_path; }

	void set_general_data_path( std::string general_data_path ){ this->general_data_path = general_data_path; }
	std::string get_general_data_path(){ return general_data_path; }

	void set_title( std::string title ){ this->title = title; }
	std::string get_title(){ return title; }

	void AddPlaneResultsStructure( PlaneResultsStructure* plane );

	std::vector<PlaneResultsStructure*>* get_plane_results_structures(){ return plane_results_structures; };
	PlaneResultsStructure* GetPlaneResultsStructure( const unsigned int planeIndex);

	void ReservePlanesInMemory( const unsigned int planesLength );

	// void AddGeneralMetricDataPath( const std::string& path, const std::string& title );
	void AddGeneralMetricImagePath( const std::string& path, const std::string& title );
	// std::map<std::string,std::string>* get_general_metrics_data_paths(){ return & general_metrics_data_paths; };
	std::map<std::string,std::string>* get_general_metrics_image_paths(){ return & general_metrics_image_paths; };

protected:
	// // Log manager
	// LogIO* log_io;

	bool n_dimensional;
	// title
	std::string simulation_id;
	std::string title;
	// general results
	std::string general_image_path;
	std::string general_data_path;
	// for each metric, a map with metric (path_title)
	std::map<std::string,std::string> general_metrics_image_paths;
	// data is the same as general_data_path
	// std::map<std::string,std::string> general_metrics_data_paths;
	// each combination of p1-p2
	std::vector<PlaneResultsStructure*>* plane_results_structures;
};

#endif /* ND_SIMULATION_RESULTS_H */
