/**
 * PLANE_RESULTS_STRUCTURE_H.hpp
 *
 *  Created on: April 1, 2016
 *      Author: fernando
 */

#ifndef PLANE_RESULTS_STRUCTURE_H
#define PLANE_RESULTS_STRUCTURE_H

// c++ std libraries
#include <iostream>
#include <string>
#include <vector>
#include <map>

class PlaneResultsStructure  {
public:

	/**
	 * @brief Default constructor
	 */
	PlaneResultsStructure();
	virtual ~PlaneResultsStructure();

	void set_plane_id( const std::string& plane_id ){ this->plane_id = plane_id; }
	std::string get_plane_id(){ return plane_id; }

	void set_general_data_path( const std::string& general_data_path ){ this->general_data_path = general_data_path; }
	std::string get_general_data_path(){ return general_data_path; }

	void set_general_image_path( const std::string& general_image_path ){ this->general_image_path = general_image_path; }
	std::string get_general_image_path(){ return general_image_path; }

	std::map<std::string,std::string>* get_general_metrics_image_paths(){ return & general_metrics_image_paths; };

	void AddGeneralMetricImagePath( const std::string& path, const std::string& title );

	void AddItemizedDataPath( const std::string& path, const std::string& title );
	void AddItemizedImagePath( const std::string& path, const std::string& title );

	void AddItemizedMetricImagePath(  const unsigned int& planeIndex,
		const std::string& path, const std::string& title );

	void ResizeItemizedPlanesMemory( unsigned int planesCount );

	std::map<std::string, std::string>* get_itemized_data_paths(){ return & itemized_data_paths; };
	std::map<std::string, std::string>* get_itemized_image_paths(){ return & itemized_image_paths; };
	std::vector<std::map<std::string,std::string>>* get_itemized_metrics_image_paths(){ return & itemized_metrics_image_paths; };

protected:
	// std::string simulation_id;
	std::string plane_id;
	// main data/image (qcrit or upsets)
	std::string general_data_path;
	std::string general_image_path;
	std::map<std::string,std::string> general_metrics_image_paths;
	// for each itemized plane, a map with metric (path_title)
	std::vector<std::map<std::string,std::string>> itemized_metrics_image_paths;
	// path, title (qcrit or upsets)
	std::map<std::string,std::string> itemized_data_paths;
	std::map<std::string,std::string> itemized_image_paths;
};

#endif /* PLANE_RESULTS_STRUCTURE_H */
