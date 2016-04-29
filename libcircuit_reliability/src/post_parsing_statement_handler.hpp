 /**
 * @file post_parsing_statement_handler.cpp
 *
 * @date Created on: Jan 20, 2014
 * 
 * @author Author: Fernando García <fgarcia@die.upm.es> <fernando.garca@gmail.com>
 * 
 * @section DESCRIPTION
 * 
 * Main code of HTMLIO Class (see html_io.hpp)
 * 
 */

#ifndef DEPENDENCE_TREES_HANDLER_H
#define DEPENDENCE_TREES_HANDLER_H

///PostParsingStatementHandler Class
// c++ std required libraries includes
#include <string>
// radiation simulator includes
#include "simulations_handling/simulation_modes_handler.hpp" 
// radiation io simulator includes
// netlist modeling includes
#include "netlist_modeling/statements/statement.hpp"
#include "netlist_modeling/statements/analysis_statement.hpp"
#include "netlist_modeling/statements/analog_model_statement.hpp"
#include "netlist_modeling/statements/model_statement.hpp"
#include "netlist_modeling/statements/subcircuit_statement.hpp"
#include "netlist_modeling/statements/instance_statement.hpp"
#include "netlist_modeling/statements/transistor_statement.hpp"
 
class PostParsingStatementHandler {

public:
	/// Default constructor
	PostParsingStatementHandler();
	/// Default destructor
	virtual ~PostParsingStatementHandler();

	/**
	 * @brief analyzes statement dependency
	 * @details analyzes statement dependency: direct and global dependencies 
	 * depending on what master instantiates each statement
	 * @return true if the method correctly ends
	 */
	bool AnalyzeStatementDependency();

	/**
	 * @brief Sets ups analysis exportation
	 * @details Sets ups analysis exportation
	 * (whether preserve or not additional analysis present in the netlist).
	 * * @return true if the method correctly ends
	 */
	bool SetUpAnalysisExportation();

	/**
	 * @brief sets log_io
	 * @details sets log_io
	 * 
	 * @param log_io Pointer to the mode log_io
	 */
	void set_log_io( LogIO* log_io ){ this->log_io = log_io; }

	/**
	 * @brief sets mode selector
	 * @details sets mode selector
	 * 
	 * @param simulation_modes_handler Pointer to the mode selector
	 */
	void set_simulation_modes_handler( SimulationModesHandler* simulation_modes_handler ){this->simulation_modes_handler = simulation_modes_handler;}
	/**
	 * @brief gets primitive statements found
	 * @return vector of primitive statements found
	 */
	std::vector<Statement*>* get_primitive_statements_found(){ return &primitive_statements_found; }
	
	/**
	 * @brief gets subcircuits statements found
	 * @return vector of subcircuits statements found
	 */
	std::vector<SubcircuitStatement*>* get_subcircuit_statements_found(){ return &subcircuit_statements_found; }
	
	/**
 	* @details Adds an analog model statement to the list of analog model statements found
 	* @param analog_model_statement_found <AnalogModelStatement*> model statement to be added.
 	*/
	void AddAnalogModelStatementFound( AnalogModelStatement* analogModelStatementFound );

	/**
 	* @details Adds a model statement to the list of model statements found
 	* @param model_statement_found <ModelStatement*> model statement to be added.
 	*/
	void AddModelStatementFound( ModelStatement* modelStatementFound );

	/**
 	* @details Adds an instance statement to the list of instance statements found
 	* @param instance_statement_found <InstanceStatement*> instance statement to be added.
 	*/
	void AddInstanceStatementFound( InstanceStatement* instanceStatementFound );

	/**
 	* @details Adds an subcircuit statement to the list of subcircuit statements found
 	* @param subcircuit_statement_found <SubcircuitStatement*> instance statement to be added.
 	*/
	void AddSubcircuitStatementFound( SubcircuitStatement* subcircuitStatementFound );

	/**
 	* @details Adds an transistor statement to the list of transistor statements found
 	* @param transistor_statement_found <TransistorStatement*> instance statement to be added.
 	*/
	void AddTransistorStatementFound( TransistorStatement* transistorStatementFound );
	
	/**
 	* @details Adds an analysis statement to the list of transistor statements found
 	* @param analysis_statement_found <AnalysisStatement*> instance statement to be added.
 	*/
	void AddAnalysisStatementFound( AnalysisStatement* analysisStatementGound );

	/**
	 * @brief finds the path to the circuit to which the statement belongs
	 * 
	 * 
	 * @param instance statement
	 */
	void FindPath2BelongingCircuit( InstanceStatement* instance );

private:
	/// Logger
	LogIO* log_io;
	/// mode selector
	SimulationModesHandler* simulation_modes_handler;
	/// Instance or (inline) Subcircuit Statements acting as primitives:
	/// Instanciated by (analog) models (if (inline) subcircuit or instance)
	/// Instanciated by instances (if (inline) subcircuit)
	std::vector<Statement*> primitive_statements_found;

	/// analog model statements found
	std::vector<AnalogModelStatement*> analog_model_statements_found;
	/// model statements found
	std::vector<ModelStatement*> model_statements_found;
	/// primitive statements found
	std::vector<InstanceStatement*> instance_statements_found;
	/// transistor statements found
	std::vector<TransistorStatement*> transistor_statements_found;
	/// subcircuit statements found
	std::vector<SubcircuitStatement*> subcircuit_statements_found;
	/// analysis statements found
	std::vector<AnalysisStatement*> analysis_statements_found;

	/**
 	* @details Finds and set up the dependency of a statement
 	* @param statement <Statement*> instance statement to be added.
 	* @return true if dependency parent is found
 	*/
	bool FindDependency( Statement* statement );

	/**
 	* @details Finds and set up the global dependency of an instance statement
 	* This means the final statement which the instance_statement is instancing.
 	* @param instance_statement <InstanceStatement*> instance statement to be added.
 	* @return true if global dependency parent is found
 	*/
	bool FindGlobalDependencyParent( InstanceStatement* instanceStatement );

};

#endif /* DEPENDENCE_TREES_HANDLER_H */
