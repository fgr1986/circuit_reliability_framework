/*
 * circuit_statement.h
 *
 *  Created on: April 1, 2013
 *  Author: fernando
 */


#ifndef CIRCUIT_STATEMENT_H
#define CIRCUIT_STATEMENT_H

#include <string>

#include "statement.hpp"
#include "include_statement.hpp"
#include "analysis_statement.hpp"
#include "section_statement.hpp"
#include "library_statement.hpp"
#include "analog_model_statement.hpp"
#include "model_statement.hpp"
#include "conditional_statement.hpp"
#include "user_defined_function_statement.hpp"
#include "subcircuit_statement.hpp"
#include "instance_statement.hpp"
#include "global_statement.hpp"
#include "simple_statement.hpp"
#include "param_test_statement.hpp"
#include "control_statement.hpp"

#include "../../experiment_environment.hpp"
#include "../../post_parsing_statement_handler.hpp"

class CircuitStatement : public Statement{
public:
	CircuitStatement( );
	CircuitStatement( const bool considerate_dependencies );
	CircuitStatement(const CircuitStatement& orig);
	virtual ~CircuitStatement();
	void basicCopy( CircuitStatement* orig );
	virtual CircuitStatement* GetCopy();
	virtual std::string ExportCircuitStatement( const std::string& indentation );

	// Experiment Environment connection methods
	// Radiation properties
	virtual bool TestUnalterableStatement(const std::string master_name);
	virtual bool TestCanBeInjected(const std::string master_name);
	virtual bool TestCanBeSubstituted(const std::string master_name,
		std::string& new_master_name );
	// Dependency
	virtual bool TestIsInstanceOfPrimitive( std::string master_name );
	virtual bool TestReservedStatement(const std::string start);
	virtual bool TestAnalysisStatement(const std::string statementCode);
	virtual bool TestIsAnalysisStatementAdvanced(const std::string analysis_master_name);
	virtual bool TestControlStatement(
		const std::string statementCode, bool& special_syntax_control_statement);
	virtual bool TestIsControlStatementAdvanced(const std::string control_master_name);
	virtual bool TestIsControlStatementSpecialSintax(const std::string control_master_name);
	virtual int TestIncludeStatement(const std::string start);
	virtual int TestConditionalStatement( const std::string statementCode );
	virtual bool TestTransistorStatement( const std::string statementCode );
	virtual bool TestUnalterableNode( const std::string node_name );

	virtual bool TestIsNonSpectreLanguage( const std::string statementCode );
	virtual bool TestIsSpectreLanguage( const std::string statementCode );

	virtual void AddGlobalNode( Node* node );

	// Circuit id
	void set_circuit_id( int circuit_id ) {
		this->circuit_id = circuit_id;}
	int get_circuit_id() const{ return circuit_id; }
	// Experiment Environment
	void set_experiment_environment( ExperimentEnvironment* experiment_environment ) {
		this->experiment_environment = experiment_environment;}
	// PostParsingStatementHandler
	void set_post_parsing_statement_handler( PostParsingStatementHandler* post_parsing_statement_handler ) {
		this->post_parsing_statement_handler = post_parsing_statement_handler;}
	PostParsingStatementHandler* get_post_parsing_statement_handler() const {
		return post_parsing_statement_handler;
	}
	void set_considerate_dependencies( bool considerate_dependencies ) {
		this->considerate_dependencies = considerate_dependencies;}
	bool get_considerate_dependencies() const {
		return considerate_dependencies;
	}

	// Instances in circuit
	std::vector<CircuitStatement*>* get_referenced_circuit_statements() {
		return &referenced_circuit_statements;
	}
	std::vector<IncludeStatement*>* get_referenced_include_statements() {
		return &referenced_include_statements;
	}
	std::vector<AnalysisStatement*>* get_included_analysis_statements() {
		return &included_analysis_statements;
	}
	std::vector<AnalogModelStatement*>* get_included_analog_model_statements() {
		return &included_analog_model_statements;
	}
	std::vector<ConditionalStatement*>* get_included_conditional_statements() {
		return &included_conditional_statements;
	}
	std::vector<ControlStatement*>* get_included_control_statements() {
		return &included_control_statements;
	}
	std::vector<GlobalStatement*>* get_included_global_statements() {
		return &included_global_statements;
	}
	std::vector<LibraryStatement*>* get_included_library_statements() {
		return &included_library_statements;
	}
	std::vector<ModelStatement*>* get_included_model_statements() {
		return &included_model_statements;
	}
	std::vector<ParamTestStatement*>* get_included_param_test_statements() {
		return &included_param_test_statements;
	}
	std::vector<InstanceStatement*>* get_included_instance_statements() {
		return &included_instance_statements;
	}
	std::vector<SectionStatement*>* get_included_section_statements() {
		return &included_section_statements;
	}
	std::vector<SimpleStatement*>* get_included_simple_statements() {
		return &included_simple_statements;
	}
	std::vector<SubcircuitStatement*>* get_included_subcircuit_statements() {
		return &included_subcircuit_statements;
	}
	std::vector<TransistorStatement*>* get_included_transistor_statements() {
		return &included_transistor_statements;
	}
	std::vector<UserDefinedFunctionStatement*>* get_included_user_defined_function_statements() {
		return &included_user_defined_function_statements;
	}

	//Update Scope
	void UpdateOwnScope();
	// Esport Statement to log
	bool ExportCircuit2SCS( const std::string& outputFilePath );

	bool ParseNetlist( const std::string& netlist, int& statementCount,
		bool& parsingSpectreCode, const bool permissiveParsingMode );

	void AddIncludeStatementAndRegister( IncludeStatement* statement );

	void AddReferencedIncludeStatement( IncludeStatement* referenced_include_statement );
	void AddReferencedCircuitStatement( CircuitStatement* referenced_circuit_statement );
	void AddIncludedAnalysisStatement( AnalysisStatement* included_analysis_statement );
	void AddIncludedAnalogModelStatement( AnalogModelStatement* included_analog_model_statement );
	void AddIncludedConditionalStatement( ConditionalStatement* included_conditional_statement );
	void AddIncludedControlStatement( ControlStatement* included_control_statement );
	void AddIncludedGlobalStatement( GlobalStatement* included_global_statement );
	void AddIncludedLibraryStatement( LibraryStatement* included_library_statement );
	void AddIncludedModelStatement( ModelStatement* included_model_statement );
	void AddIncludedParamTestStatement( ParamTestStatement* included_param_test_statement );
	void AddIncludedInstanceStatement( InstanceStatement* included_instance_statement );
	void AddIncludedSectionStatement( SectionStatement* included_section_statement );
	void AddIncludedSimpleStatement( SimpleStatement* included_simple_statement );
	void AddIncludedSubcircuitStatement( SubcircuitStatement* included_subcircuit_statement );
	void AddIncludedTransistorStatement( TransistorStatement* included_transistor_statement );
	void AddIncludedUserDefinedFunctionStatement( UserDefinedFunctionStatement* included_user_defined_function_statement );

	std::vector<Statement*>* get_simulation_special_children(){return &simulation_special_children;}
	void AddSimulationSpecialStatement( Statement* statement );

	/**
	 * @brief Mutes the exportation of all analysis which are not the main transient
	 * Is overriden in analysis_statement.hpp class and circuit_statement.hpp class
	 */
	virtual void MuteNonMainTransientAnalysis();

private:
	/// Circuit id, to be referenced
	int circuit_id;
	/// Experiment environment.
	/// Circuit statement is the only statement with direct access to the environment
	ExperimentEnvironment* experiment_environment;
	/// Handles the statements dependency
	PostParsingStatementHandler* post_parsing_statement_handler;
	/// Considerates dependencies
	bool considerate_dependencies;
	/// simulation mode children: analysis and control statements
	std::vector<Statement*> simulation_special_children;

	// Instance or (inline) Subcircuit Statements acting as primitives:
	// Instanciated by (analog) models (if (inline) subcircuit or instance)
	// Instanciated by instances (if (inline) subcircuit)
	std::vector<std::vector<Statement*>*> primitive_statements;

	// "string" statatement referencing other circuit files
	std::vector<IncludeStatement*> referenced_include_statements;
	// circuit
	std::vector<CircuitStatement*> referenced_circuit_statements;
	// analysis
	std::vector<AnalysisStatement*> included_analysis_statements;
	// analog model statement
	std::vector<AnalogModelStatement*> included_analog_model_statements;
	// conditional_statement
	std::vector<ConditionalStatement*> included_conditional_statements;
	// control statement
	std::vector<ControlStatement*> included_control_statements;
	// global_statement
	std::vector<GlobalStatement*> included_global_statements;
	// library statement
	std::vector<LibraryStatement*> included_library_statements;
	// model statement
	std::vector<ModelStatement*> included_model_statements;
	// param_test statement
	std::vector<ParamTestStatement*> included_param_test_statements;
	// primitive statement
	std::vector<InstanceStatement*> included_instance_statements;
	// radiation sources
	// No radiation sources are naturaly parsed, only added
	// section statement
	std::vector<SectionStatement*> included_section_statements;
	// simple statement
	std::vector<SimpleStatement*> included_simple_statements;
	// subcircuit statement
	std::vector<SubcircuitStatement*> included_subcircuit_statements;
	// transistor statement
	std::vector<TransistorStatement*> included_transistor_statements;
	// user_defined_function statement
	std::vector<UserDefinedFunctionStatement*> included_user_defined_function_statements;

	/// Copy of special Children
	void deepCopyOfCircuitSpecialChildren( const std::vector<Statement*>& source ) ;
	void deepCopyOfCircuitChildren( const std::vector<Statement*>& source  );
};

#endif /* CIRCUIT_STATEMENT_H */
