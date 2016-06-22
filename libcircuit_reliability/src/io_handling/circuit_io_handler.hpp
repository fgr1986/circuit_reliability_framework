/**
 * @file circuit_io_handler.hpp
 *
 * @date Created on:Jan 22, 2014
 *
 * @author Author: Fernando García <fgarcia@die.upm.es> <fernando.garca@gmail.com>
 *
 * @section DESCRIPTION
 *
 * This Class performs two key tasks:
 *     + Parses the different circuits
 *     + Handles the netlist circuits exportation.
 * Additionally, it has the main folder creation method.
 *
 */

#ifndef CIRCUIT_IO_HANDLER_H
#define CIRCUIT_IO_HANDLER_H

///CircuitIOHandler Class
// c++ std libraries includes
#include <string>
// radiation simulator io includes
#include "log_io.hpp"
// radiation simulator includes
#include "../experiment_environment.hpp"
#include "../post_parsing_statement_handler.hpp"
// simulations handling
#include "../simulations_handling/simulation_mode.hpp"
// #include "../simulations_handling/spectre_handler.hpp"
// netlist modeling includes
#include "../metric_modeling/metric.hpp"
#include "../netlist_modeling/statements/statement.hpp"
#include "../netlist_modeling/statements/circuit_statement.hpp"

class CircuitIOHandler {

public:
	/// Default constructor
	CircuitIOHandler();
	/// Default destructor
	virtual ~CircuitIOHandler();

	/**
 	* @details Reads (parses) the injection radiation sources from a netlist.
 	* @param permissiveParsingMode <const bool> Permisive parsing mode.
 	* @return true if the sources have been correctly parsed.
 	*/
	bool ReadInjectionRadiationSourcesNetlist( const bool permissiveParsingMode );

	/**
 	* Reads (parses) the replacement radiation sources from a netlist.
 	* @param permissiveParsingMode <const bool> Permisive parsing mode.
 	* @return true if the sources have been correctly parsed.
 	*/
	bool ReadReplacementRadiationSourcesNetlist( const bool permissiveParsingMode );

	/**
 	* @details Reads (parses) the main circuit (as well as its referenced circuits) from a netlist.
 	* @param stateNumber <int> Program stage
 	* @param netlistFile <std::string> Main netlist file
 	* @param mainCircuit <CircuitStatement&> Main Circuit
 	* @param permissiveParsingMode <const bool> Permisive parsing mode.
 	* @return true if the method correctly ends.
 	*/
	bool ReadMainNetlist( int stateNumber, std::string netlistFile,
		CircuitStatement& mainCircuit, const bool permissiveParsingMode );

	/**
 	* @details Exports a non-altered circuit
 	* @param circuit <CircuitStatement&> Circuit to be exported.
 	* @param isMain <bool> The circuit is (or not) the main one.
 	* @param folder <std::string> Path to where export the circuit.
 	* @return true if the method correctly ends.
 	*/
	bool ExportNonAlteredNetlist( CircuitStatement& circuit, bool isMain, std::string folder );

	/**
 	* @details Exports an altered circuit
 	* @param circuit <CircuitStatement&> Circuit to be exported.
 	* @param avoidCircuitId <int> Circuit id of the altered circuit.
 	* @param isMain <bool> The circuit is (or not) the main one.
 	* @param folder <std::string> Path to where export the circuit.
 	* @return true if the method correctly ends.
 	*/
	bool ExportAlteredNetlist( CircuitStatement& circuit, int avoidCircuitId,
		bool isMain, std::string folder );

	/**
 	* @details Exports a circuit with a simple alteration:
 	* either the injection of a simple node or the substitution of a simple statement.
 	*
 	* @param radiationCircuit <CircuitStatement&> Radiation circuit
 	* @param alteredCircuit <CircuitStatement&> Circuit altered.
 	* @param mainCircuit <CircuitStatement&> Main circuit.
 	* @param avoidCircuitId <int> Circuit id of the altered circuit.
 	* @param folder <std::string> Path to where export the circuit.
 	* @return true if the method correctly ends.
 	*/
	bool ExportSimpleAlteredScenario( CircuitStatement& radiationCircuit, CircuitStatement& alteredCircuit,
		CircuitStatement& mainCircuit, std::string folder );

	/**
 	* @details Exports a circuit with a complex alteration:
 	* either the injection/substitution of a subcircuit child.
 	*
 	* @param radiationCircuit <CircuitStatement&> Radiation circuit.
 	* @param alteredCircuit <CircuitStatement&> Circuit altered.
 	* @param alteredStatementsCircuit <CircuitStatement&> Circuit including the chain of altered statements.
 	* @param mainCircuit <CircuitStatement&> Main circuit.
 	* @param avoidCircuitId <int> Circuit id of the altered circuit.
 	* @param folder <std::string> Path to where export the circuit.
 	* @return true if the method correctly ends.
 	*/
	bool ExportAlteredScenario( CircuitStatement& radiationCircuit, CircuitStatement& alteredCircuit,
		CircuitStatement& alteredStatementsCircuit, CircuitStatement& mainCircuit,
		std::string folder );

	/**
 	* @details Exports a singluar circuit.
 	*
 	* @param singularFolder <std::string> Path to where export the circuit.
 	* @param singularCircuit <CircuitStatement&> Singular circuit to be exported.
 	* @return true if the method correctly ends.
 	*/
	bool ExportSingularNetlist( std::string singularFolder, CircuitStatement& singularCircuit );

	/// Sets the mode selector
	void set_simulation_mode( SimulationMode* simulation_mode) { this->simulation_mode = simulation_mode; }
	/// Sets the log manager
	void set_log_io( LogIO* log_io ){ this->log_io = log_io; }
	/// Sets the experiment environment
	void set_experiment_environment( ExperimentEnvironment* experiment_environment ) {
		this->experiment_environment = experiment_environment;}
	/// Sets the Dependence trees handler
	void set_post_parsing_statement_handler( PostParsingStatementHandler* post_parsing_statement_handler ) {
		this->post_parsing_statement_handler = post_parsing_statement_handler;}

	/**
	 * @brief Adds a metric to the vector of metrics
	 *
	 * @param metric
	 */
	void AddMetric( Metric* metric );


private:
	/// Simulation mode manager
	SimulationMode* simulation_mode;
	/// Log manager.
	LogIO* log_io;
	/// Experiment environment.
	ExperimentEnvironment* experiment_environment;
	/// Dependence trees handler.
	PostParsingStatementHandler* post_parsing_statement_handler;
	/// List of metrics, needed to create the 'monitors'
	/// and save the metrics transients
	std::vector<Metric*> metrics;
	/// List of parsed circuits.
	std::vector<std::string> parsed_circuits;

	/**
 	* @details Reads (parses) a specified circuit (as well as its referenced circuits) from a netlist.
 	* @param netlistAbsolutePath <std::string> Circuit netlist file (absolute path).
 	* @param mainCircuitFolder <std::string> Main circuit folder path.
 	* @param circuit <CircuitStatement&> Circuit.
 	* @param parentCircuit <CircuitStatement&> Parent circuit.
 	* @param isMain <bool> Circuit is/is not the main circuit.
 	* @param circuitCount <int&> Circuit count.
 	* @param statementCount <int&> Statement count.
 	* @param parsingSpectreCode <bool&> Currently parsing spectre code.
 	* @param permissiveParsingMode <const bool> Permisive parsing mode.
 	* @return true if the method correctly ends.
 	*/
	bool ReadNetlist( std::string netlistAbsolutePath, std::string mainCircuitFolder,
		CircuitStatement& circuit, CircuitStatement& parentCircuit,
		bool isMain, int& circuitCount, int& statementCount,
		bool& parsingSpectreCode, const bool permissiveParsingMode );

	/**
 	* @details Tests if a netlist has been already parsed
 	* @param path <std::string> Circuit netlist file.
 	*
 	* @return true if the netlist has already been parsed
 	*/
	bool NetlistAlreadyParsed( std::string path );

	/**
	 * @brief Adds required simulation statements
	 * @details Adds the corresponding analysis statements and related control statements
	 *
	 * @param circuit CircuitStatement& target circuit
	 */
	void AddSimulationSpecialStatements( CircuitStatement& circuit );
};

#endif /* CIRCUIT_IO_HANDLER_H */
