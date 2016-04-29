/*
 * circuit_radiator.hpp
 *
 *  Created on: Jan 23, 2014
 *  Author: fernando
 */


#ifndef CIRCUIT_RADIATOR_H
#define CIRCUIT_RADIATOR_H

///CircuitRadiator Class
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
#include "simulations_handling/spectre_handlers/radiation_spectre_handler.hpp"
// Netlist modeling
#include "netlist_modeling/statements/statement.hpp"
#include "netlist_modeling/statements/circuit_statement.hpp"
 
class CircuitRadiator {

public:
	/// Default constructor
	CircuitRadiator();
	/// Default destructor
	virtual ~CircuitRadiator();

	/**
 	* @details Exports golden netlist as well as
	* the radiation_subcircuit_ahdl netlist
 	* @param goldenFolder <std::string> Golden netlist folder
 	* @param radiationSubcircuitAHDLFolder <std::string> AHDL netlist folder
 	* @return true if the method correctly ends.
 	*/
	bool SetUpSingularScenarios( std::string goldenFolder,
		std::string goldenAhdlFolder, std::string radiationSubcircuitAHDLFolder );
	
	/**
 	* @details Analyzes and radiates the main circuit
 	* @return true if the method correctly ends.
 	*/
	bool RadiateMainCircuit( );
	
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
	void set_radiation_spectre_handler( RadiationSpectreHandler* radiation_spectre_handler ) {
		this->radiation_spectre_handler = radiation_spectre_handler;}
	
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
	RadiationSpectreHandler* radiation_spectre_handler;
	/// Mode selector mode
	SimulationMode*  simulation_mode;
	/// Experiment top_folder
	std::string top_folder;
	/// Path to the altered scenarios netlist
	std::string altered_scenarios_folder;
	/// Main circuit
	CircuitStatement* main_circuit;
	/// radiation subcircuits circuit
	CircuitStatement radiationSubcircuitsCircuit;
	/// ahdl radiation subcircuits circuit
	CircuitStatement radiationSubcircuitsCircuitAHDL;

	/**
 	* @details Radiates a circuit
 	* @param circuit <CircuitStatement&> Golden netlist folder
 	* @param modificationCounter <int> Modification(injections/substitutions) counter. 
 	* @return true if the method correctly ends.
 	*/
	bool RadiateCircuit( CircuitStatement& circuit, int& modificationCounter );

	/**
 	* @details Analyze if a statement is perceptible of being altered by INJECTION
 	* @param statement <Statement&> Golden netlist folder
 	* @param modificationCounter <int> Modification(injections/substitutions) counter. 
 	* @return true if the method correctly ends.
 	*/
	bool AnalyzeStatementForInjection( Statement& , int& modificationCounter );
	
	/**
 	* @details Analyze if a statement is perceptible of being altered by SUBSTITUTION
 	* @param statement <Statement&> Golden netlist folder
 	* @param modificationCounter <int> Modification(injections/substitutions) counter. 
 	* @return true if the method correctly ends.
 	*/
	bool AnalyzeStatementForReplacement( Statement& statement, int& modificationCounter );
	
	/**
 	* @details Injects a statement
 	* @param statement <InstanceStatement&> Golden netlist folder
 	* @param modificationCounter <int> Modification(injections/substitutions) counter. 
 	* @return true if the method correctly ends.
 	*/
	bool InjectStatement( InstanceStatement& statement, int& modificationCounter );
	
	/**
 	* @details Injects a particular node of a statement which is NOT
	* a child of an (inline) subcircuit 
 	* @param node <Node&> Injected node
 	* @param statement <InstanceStatement&> Golden netlist folder
 	* @param modificationCounter <int> Modification(injections/substitutions) counter. 
 	* @return true if the method correctly ends.
 	*/
	bool InjectSimpleNode(  Node& node, InstanceStatement& statement, int& modificationCounter );

	/**
 	* @details Injects a particular node of an statement which 
	* is child of an (inline) subcircuit
	* Therefore, any instance of the P should be altered and substituted
	* Here, P is the group of 'p' where p are the instances of the 
	*     * (inline) subcircuit
	*     * (analog) models of the (inline) subcircuit
 	* @param node <Node&> Injected node
 	* @param statement <InstanceStatement&> Golden netlist folder
 	* @param modificationCounter <int> Modification(injections/substitutions) counter. 
 	* @return true if the method correctly ends.
 	*/
	bool InjectNodeOfSubcircuitChild( Node& node, InstanceStatement& statement, int& modificationCounter );
	
	/**
 	* @details Replaces a subcircuit child ('s') for a altered instance
 	* 
 	* @param childOfSubcircuitStatement <Statement&> Substituted child of subcircuit statement.
 	* @param modificationCounter <int> Modification(injections/substitutions) counter. 
 	* @return true if the method correctly ends.
 	*/
	bool SubstituteSubcircuitChild( Statement& childOfSubcircuitStatement,
		int& modificationCounter );

	/**
 	* @details Replaces a statement which is not a subcircuit child.
 	* 
 	* @param statement <Statement&> Replaced statement.
 	* @param modificationCounter <int> Modification(injections/substitutions) counter. 
 	* @return true if the method correctly ends.
 	*/
	bool SubstituteSimpleStatement( Statement& statement,
		int& modificationCounter );

	/**
 	* @details Replaces all the instances of an altered subcircuit, including:
 	* 	* Direct instances of the subcircuit.
 	* 	* Instance of an (analog) model of (...an (analog) model of a...) the subcircuit.
 	* @param deepLevel <std::string&> Register of the deep of the replacement.
 	* @param alteredParameterName <std::string> Parameter referring the substitute master name (in substitution mode)
 	*        or the injected node name (injection mode). [Needed for report]
 	* @param alteredStatementsCircuit <CircuitStatement&> Altered statements circuit.
 	* @param originalSubcircuit <SubcircuitStatement&> Original subcircuit whose instances are going to be replaced.
 	* @param alteredSubcircuit <SubcircuitStatement&> Altered subcircuit. The replaced instances will instantiate it.
 	* @param modificationCounter <int> Modification(injections/substitutions) counter. 
 	* @param alteredStatementName <std::string> Altered statement name  [Needed for report]
 	* @param alteredStatementMasterName <std::string> Altered statement Master Name name  [Needed for report]
 	* @param alteredStatementPathToBelongingCircuit <std::string> Altered path to circuit [Needed for report]
 	* @param alteredScopeName <std::string> Altered scope Master Name name  [Needed for report]
 	* @return true if the method correctly ends.
 	*/
	bool ReplaceAlteredSubcircuitInstances( std::string& deepLevel, std::string alteredParameterName,
		CircuitStatement& alteredStatementsCircuit,
		SubcircuitStatement& originalSubcircuit, SubcircuitStatement& alteredSubcircuit, int& modificationCounter,
		std::string alteredStatementName, std::string alteredStatementMasterName,
		std::string& alteredStatementPathToBelongingCircuit, std::string alteredScopeName );

	/**
 	* @details Replaces an instance of an altered subcircuit.
 	* @param deepLevel <std::string&> Register of the deep of the replacement.
 	* @param alteredParameterName <std::string> Parameter referring the substitute master name (in substitution mode)
 	*        or the injected node name (injection mode). [Needed for report]
 	* @param alteredStatementsCircuit <CircuitStatement&> Altered statements circuit.
 	* @param instanceOfSubcircuit <InstanceStatement&> InstanceStatement to be replaced.
 	* @param originalSubcircuit <SubcircuitStatement&> Original subcircuit whose instances are going to be replaced.
 	* @param alteredSubcircuit <SubcircuitStatement&> Altered subcircuit. The replaced instances will instantiate it.
 	* @param modificationCounter <int> Modification(injections/substitutions) counter. 
 	* @param alteredStatementName <std::string> Altered statement name [Needed for report]
 	* @param alteredStatementMasterName <std::string> Altered statement Master Name name [Needed for report]
 	* @param alteredStatementPathToBelongingCircuit <std::string> Altered path to circuit [Needed for report]
 	* @param alteredScopeName <std::string> Altered scope Master Name name [Needed for report]
 	* @return true if the method correctly ends.
 	*/
	bool ReplaceAlteredSubcircuitInstance(std::string& deepLevel,
		std::string alteredParameterName,
		CircuitStatement& alteredStatementsCircuit, InstanceStatement& instanceOfSubcircuit,
		SubcircuitStatement& originalSubcircuit, SubcircuitStatement& alteredSubcircuit,
		int& modificationCounter, std::string alteredStatementName, std::string alteredStatementMasterName,
		std::string alteredStatementPathToBelongingCircuit, std::string alteredScopeName );

	/**
 	* @details Searchs the parent of an statement and proceed to inject it.
 	* If the statement is a circuit, it itself gets injected.
 	* @param statement <Statement&> Statement whose parent is going to be injected.
 	* @param modifiedCircuit <CircuitStatement&> Altered circuit.
 	* @param injectorSource <InstanceStatement&> Instance of the injector radiation source.
 	* @return true if the method correctly ends.
 	*/
	bool AddRadiationSourceInstance2Parent(Statement& statement,
		CircuitStatement& modifiedCircuit, InstanceStatement& injectorSource);

	/**
 	* @details Creates a new Circuit Statement which will includes the altered subcircuit and 
 	* intermediate (analog) models in found during ReplaceAlteredSubcircuitInstances.
 	* @return CircuitStatement* alteredStatementsCircuit.
 	*/
	CircuitStatement* GetAlteredStatementsCircuit();

	/**
 	* @details Creates the Circuit Statement which includes the radiation source subcircuit
 	* ready to be instantiated.
 	* Creates an auxiliary circuit for the AHDL compiled sources
 	*/
	void CreateRadiationAndAHDLCircuits();

	/**
 	* @details Creates a new instance of the altered source.
 	* @param node <Node&> injected node.
 	* @param radiationSourceName <std::string> radiation source name.
 	* @return InstanceStatement* injector source instance.
 	*/
	InstanceStatement* GetNewInjectorInstance(  Node& node,
		std::string radiationSourceName );
	/**
 	* @details Creates a new instance of the altered subcircuit source.
 	* @param rsss <RadiationSourceSubcircuitStatement*> radiation source.
 	* @return InstanceStatement* radiation subcircuit source instance.
 	*/
	InstanceStatement* GetNewReplacementSourceInstance(
		RadiationSourceSubcircuitStatement* rsss );

	/**
 	* @details Creates a new control statement to monitor the radiation injector
 	* @param radiationSourceName <std::string> radiation source name.
 	* @return ControlStatement* injector source monitor.
 	*/
	ControlStatement* GetNewSaveInjectionStatement(  std::string path2BelongingCircuit );
	
	/**
 	* @details Creates a new instance an auxiliary resistor
 	* @param rsss <RadiationSourceSubcircuitStatement*> radiation source connected to the resistor.
 	* @return InstanceStatement* resistor instance.
 	*/
	InstanceStatement* GetNewRauxInstance( RadiationSourceSubcircuitStatement* rsss );

	/**
 	* @details Creates a new instance an auxiliary resistor
 	* @param nodeName <std::string> Node to which the resistor is connected.
 	* @return InstanceStatement* resistor instance.
 	*/
	InstanceStatement* GetNewRauxInstance( std::string nodeName );

	/**
 	* @details Creates a new Include statement referring the radiation circuit including the subcircuit source
 	* @return IncludeStatement* include statement.
 	*/
	IncludeStatement* GetNewIncludeStatementOfRadiationCircuit();

	/**
 	* @details Creates a new Include statement referring the circuit including the altered statements
 	* @return IncludeStatement* include statement.
 	*/
	IncludeStatement* GetNewIncludeStatementOfAlteredStatementsCircuit();

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

#endif /* CIRCUIT_RADIATOR_H */
