/*
 * circuit_radiator.hpp
 *
 *  Created on: Jan 23, 2014
 *  Author: fernando
 */


#ifndef CIRCUIT_EXPORTER_H
#define CIRCUIT_EXPORTER_H

///CircuitExporter Class
// c++ std libraries
#include <string> 

// Spectre Handler Library
#include "experiment_environment.hpp"
#include "post_parsing_statement_handler.hpp"
// io
#include "io_handling/log_io.hpp"
#include "io_handling/circuit_io_handler.hpp"
// Simulations handler
#include "simulations_handling/simulation_mode.hpp" 
#include "simulations_handling/spectre_handlers/variability_spectre_handler.hpp"
// Netlist modeling
#include "netlist_modeling/statements/statement.hpp"
#include "netlist_modeling/statements/circuit_statement.hpp"
 
class CircuitExporter {

public:
	/// Default constructor
	CircuitExporter();
	/// Default destructor
	virtual ~CircuitExporter();

	/**
 	* @details Exports golden netlist as well as
	* the radiation_subcircuit_ahdl netlist
 	* @param goldenFolder <std::string> Golden netlist folder
 	* @param radiationSubcircuitAHDLFolder <std::string> AHDL netlist folder
 	* @return true if the method correctly ends.
 	*/
	bool SetUpScenarios( std::string goldenFolder, std::string goldenAhdlFolder,
	 std::string variationsAhdlFolder, std::string variationsFolder  );
	
	/// Sets the log manager 
	void set_log_io( LogIO* log_io ){ this->log_io = log_io; }
	/// Sets the main circuit
	void set_main_circuit( CircuitStatement* main_circuit ){
		this->main_circuit = main_circuit; }

	/// Sets the simulation_mode
	void set_simulation_mode( SimulationMode* simulation_mode ){ this->simulation_mode = simulation_mode; }
	/**
	 * @brief Sets experiment top folder
	 * 
	 * @param top_folder
	 */
	void set_top_folder( std::string top_folder ){ this->top_folder = top_folder; }
	/// Sets  the experiment environment
	void set_experiment_environment( ExperimentEnvironment* experiment_environment ) {
		this->experiment_environment = experiment_environment;}
	/// Sets the dependence tress handler
	void set_post_parsing_statement_handler( PostParsingStatementHandler* post_parsing_statement_handler ) {
		this->post_parsing_statement_handler = post_parsing_statement_handler;}
	// Sets the circuit_io handler 
	void set_circuit_io_handler( CircuitIOHandler* circuit_io_handler ) {
		this->circuit_io_handler = circuit_io_handler;}
	// Sets the spectre handler 
	void set_variability_spectre_handler( VariabilitySpectreHandler* variability_spectre_handler ) {
		this->variability_spectre_handler = variability_spectre_handler;}
	
private:
	/// Log manager
	LogIO* log_io;
	/// Experiment environment
	ExperimentEnvironment* experiment_environment;
	/// Dependence trees handler
	PostParsingStatementHandler* post_parsing_statement_handler;
	/// Circuit IO Handler
	CircuitIOHandler* circuit_io_handler;
	/// Spectre handler
	VariabilitySpectreHandler* variability_spectre_handler;
	/// Mode selector mode
	SimulationMode*  simulation_mode;
	/// Experiment top_folder
	std::string top_folder;
	/// Path to the altered scenarios netlist
	std::string altered_scenarios_folder;
	/// Main circuit
	CircuitStatement* main_circuit;

	/**
 	* @details Creates a new Include statement referring the circuit including the altered parameters.
 	* @return IncludeStatement* include statement.
 	*/
	IncludeStatement* GetNewIncludeStatementOfParametersCircuit();

	/**
 	* @details Generates the name of the radiation scenario folder.
 	* @param modificationCounter <int> modification counter
 	* @return <std::string> Radiated folder name.
 	*/
	std::string GetRadiatedFolderName( int modificationCounter );

	IncludeStatement* GetGoldenModelsAndSectionIncludeStatement();
	IncludeStatement* GetExperimentModelsAndSectionIncludeStatement();
};

#endif /* CIRCUIT_EXPORTER_H */
