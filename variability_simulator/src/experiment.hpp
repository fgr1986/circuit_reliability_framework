 /**
 * @file experiment.hpp
 *
 * @date Created on: November 15, 2013
 * 
 * @author Author: Fernando García <fgarcia@die.upm.es> <fernando.garca@gmail.com>
 * 
 * @section DESCRIPTION
 * 
 * This Class is manages the whole experiment, handling the different actuators.
 * It is the first responsible of parsing, analyzing and simulating the circuits.
 * 
 */

#ifndef EXPERIMENT_H
#define EXPERIMENT_H

///Experiment Class
// c++ std libraries includes
#include <string>
#include <vector>

// Radiation simulator includes
#include "circuit_exporter.hpp"
#include "xml_io_manager.hpp"

// Spectre Handler Library
// radiation io simulator includes
#include "io_handling/log_io.hpp"
#include "io_handling/circuit_io_handler.hpp"
#include "post_parsing_statement_handler.hpp"
// Netlist modeling includes
#include "netlist_modeling/statements/circuit_statement.hpp"
#include "netlist_modeling/statements/include_statement.hpp"
#include "netlist_modeling/statements/instance_statement.hpp"
#include "netlist_modeling/node.hpp"
// Simulation handling includes
#include "simulations_handling/simulation_modes_handler.hpp" 
#include "simulations_handling/spectre_handlers/variability_spectre_handler.hpp"

class Experiment {
public:
	/// Default constructor
	Experiment();
	
	/// Destructor
	virtual ~Experiment();
	
	/**
 	* @details set_i_netlist_file method
 	* @param i_netlist_file <std::string> set i_netlist_file
 	*/
	void set_i_netlist_file(std::string i_netlist_file) {
		this->i_netlist_file = i_netlist_file; }
	/**
 	* @details set_i_xml_file_cadence method
 	* @param i_xml_file_cadence <std::string> set i_xml_file_cadence
 	*/
	void set_i_xml_file_cadence(std::string i_xml_file_cadence) {
		this->i_xml_file_cadence = i_xml_file_cadence; }
	/**
 	* @details set_i_xml_file_technology method
 	* @param i_xml_file_technology <std::string> set i_xml_file_technology
 	*/
	void set_i_xml_file_technology(std::string i_xml_file_technology) {
		this->i_xml_file_technology = i_xml_file_technology; }
	/**
 	* @details set_i_xml_file_experiment method
 	* @param i_xml_file_experiment <std::string> set i_xml_file_experiment
 	*/
	void set_i_xml_file_experiment( std::string i_xml_file_experiment ){
		this->i_xml_file_experiment = i_xml_file_experiment; }
	/**
 	* @details get_experiment_title method
 	* @return experiment experiment_title parameter <std::string>
 	*/
	std::string get_experiment_title() const {return experiment_title;};
	/**
 	* @details set_experiment_title method
 	* @param experiment_title <std::string> set experiment experiment_title 
 	*/
	void set_experiment_title(std::string experiment_title) {
		this->experiment_title = experiment_title; }
	/**
 	* @details get_experiment_folder method
 	* @return experiment experiment_folder parameter <std::string>
 	*/
	std::string get_experiment_folder() const {return experiment_folder;};
	/**
 	* @details set_experiment_folder method
 	* @param experiment_folder <std::string> set experiment experiment_folder 
 	*/
	void set_experiment_folder(std::string experiment_folder) {
		this->experiment_folder = experiment_folder; }
	/**
 	* @details get_permissive_parsing_mode method
 	* @return experiment permissive parsing mode parameter <std::string>
 	*/
	bool get_permissive_parsing_mode() const {return permissive_parsing_mode;}; 
	/**
 	* @details set_permissive_parsing_mode method
 	* @param permissive_parsing_mode <std::string> set permissive parsing mode parameter
 	*/
	void set_permissive_parsing_mode(bool permissive_parsing_mode) {
		this->permissive_parsing_mode = permissive_parsing_mode; }
	/**
 	* @details set_export_matlab_script method
 	* @param export_matlab_script <bool> set export_matlab_script parameter
 	*/
	void set_export_matlab_script( bool export_matlab_script ){
		this->export_matlab_script = export_matlab_script;
	}
	/**
 	* @details set_log_io method
 	* @param log_io <LogIO*> set log_io parameter
 	*/
	void set_log_io( LogIO* log_io ){ this->log_io = log_io; }
	/**
 	* @details set_html_io method
 	* @param html_io <HTMLIO*> set html_io parameter
 	*/
	// void set_html_io( HTMLIO* html_io ){ this->html_io = html_io; }
	/// Configures experiment environment
	bool ConfigureEnvironment( );
	/**
 	* @details Parses main netlist
 	* @param stageNumber <int> Program stage
 	* @return method correctly executed <bool>
 	*/
	bool ReadMainNetlist( int stageNumber );
	/**
 	* @details Analyzes statement dependency
 	* @param stageNumber <int> Program stage
 	* @return method correctly executed <bool>
 	*/
	bool AnalyzeStatementDependency( int stageNumber );
	/**
 	* @details Creates and export the altered circuit scenarios
 	* @param stageNumber <int> Program stage
 	* @return method correctly executed <bool>
 	*/
	bool CreateVariationScenarios( int stageNumber );
	/**
 	* @details Simulates the altered circuit scenarios
 	* @param stageNumber <int> Program stage
 	* @return method correctly executed <bool>
 	*/
	bool SimulateVariationScenarios( int stageNumber );

private:
	/// Logger
	LogIO* log_io;
	/// Spectre simulations handler
	VariabilitySpectreHandler variability_spectre_handler;
	/// Main circuit
	CircuitStatement main_circuit;
	/// Experiment environment
	ExperimentEnvironment experiment_environment;
	/// Circuit IO handler (circuit netlist exporter)
	CircuitIOHandler circuit_io_handler;
	/// XMLIO manager: reads xml conf files and exports circuits to xml
	XMLIOManager xml_io_manager;
	/// Analyzes statements dependencies
	PostParsingStatementHandler post_parsing_statement_handler;
	/// Circuit radiator
	CircuitExporter circuit_exporter;
	/// Simulation mode selector
	SimulationModesHandler simulation_modes_handler;

	/// Input netlist file
	std::string i_netlist_file;
	/// Input cadence xml conf file
	std::string i_xml_file_cadence;
	/// Input technology xml conf file
	std::string i_xml_file_technology;
	/// Input experiment xml file
	std::string i_xml_file_experiment;
	/// Input experiment experiment_folder
	std::string experiment_folder;
	/// Input experiment (xml provided)
	std::string experiment_title;
	/// Output compiled statements folder
	std::string compiled_components_folder;
	/// Output altered scenarios folder
	std::string variation_scenario_folder;
	/// Output golden scenario folder
	std::string golden_folder ;
	std::string golden_ahdl_folder;
	/// Output variations scenario folder
	std::string variations_folder;
	/// Output variations_ahdl_folder files folder
	std::string variations_ahdl_folder;
	/// Output results scenario folder
	std::string results_folder;

	/// Permisive parsing mode
	bool permissive_parsing_mode;
	/// Matlab script exporting
	bool export_matlab_script;
	/// Creates the main folder structure for the experiment
	bool CreateFolders();
	
};

#endif /* EXPERIMENT_H */
