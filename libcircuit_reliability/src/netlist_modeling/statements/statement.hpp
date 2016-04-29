/*
 * instance.h
 *
 *  Created on: March 11, 2013
 *      Author: fernando
 */

#ifndef STATEMENT_H
#define STATEMENT_H

#include <string>
#include <vector>

#include "../../io_handling/log_io.hpp"

#include "../parameter.hpp"
#include "../node.hpp"
#include "../scope.hpp"

class Statement {
public:
	virtual ~Statement();

	// Export Statement 2 scs
	virtual std::string ExportCircuitStatement(std::string indentation);
	// Get a deep copy of the statement
	virtual Statement* GetCopy() = 0;

	// Getter/Setter methods
	Statement* get_belonging_circuit() const{ return belonging_circuit; }
	int get_id() const {return id;}
	int get_statement_type() const {return statement_type; }
	std::string get_statement_type_description() const {return statement_type_description; }
	std::string get_parallel_statements() const {return parallel_statements; }
	bool get_has_parallel_statements() const {return has_parallel_statements; }
	bool get_has_brackets() const {return has_brackets; }
	bool get_has_parameters() const {return has_parameters; }
	bool get_has_nodes() const {return has_nodes; }
	bool get_has_children() const {return has_children; }
	bool get_has_raw_content() const {return has_raw_content; }
	// bool get_injected() const {return injected; }
	bool get_altered() const {return altered; }
	bool get_unalterable() const{ return unalterable; }
	bool get_can_be_injected() const {return can_be_injected; }
	bool get_can_be_substituted() const{return can_be_substituted; }
	std::string get_substitute_master_name() const{return substitute_master_name;}
	bool get_correctly_parsed() const {return correctly_parsed; }
	std::string get_master_name() const {return master_name; }
	std::string get_name() const {return name; }
	std::string get_description() const {return description; }
	std::string get_raw_content() const {return raw_content; }
	std::vector<Node*>* get_nodes() {return &nodes;}
	std::vector<Statement*>* get_children() {return &children; }
	std::vector<Statement*>* get_dependency_children() {return &dependency_children; }
	std::vector<Parameter*>* get_parameters() {return &parameters; }
	void set_id(int id) { this->id = id; }
	void set_statement_type(int statement_type) { this->statement_type = statement_type; }
	void set_statement_type_description(std::string statement_type_description) {
		this->statement_type_description = statement_type_description; }
	void set_parallel_statements(std::string parallel_statements) { this->parallel_statements = parallel_statements; }
	void set_has_parallel_statements(bool has_parallel_statements) { this->has_parallel_statements = has_parallel_statements; }
	void set_has_brackets(bool has_brackets) { this->has_brackets = has_brackets; }
	void set_has_nodes(bool has_nodes) { this->has_nodes = has_nodes; }
	void set_has_children(bool has_children) { this->has_children = has_children; }
	void set_has_parameters(bool has_parameters) { this->has_parameters = has_parameters; }
	void set_has_raw_content(bool has_raw_content) { this->has_raw_content = has_raw_content; }
	void set_name(std::string name) { this->name = name; }
	void set_description(std::string description) { this->description = description; }
	void set_master_name(std::string master_name) { this->master_name = master_name; }
	void set_raw_content(std::string raw_content) { this->raw_content = raw_content; }
	void set_altered(bool altered) { this->altered = altered; }
	void set_unalterable(bool unalterable){ this->unalterable = unalterable;}
	void set_can_be_injected(bool can_be_injected) { this->can_be_injected = can_be_injected; }
	void set_can_be_substituted( bool can_be_substituted){
		this->can_be_substituted = can_be_substituted;	}
	void set_substitute_master_name( std::string substitute_master_name){
		this->substitute_master_name = substitute_master_name;	}
	void set_correctly_parsed(bool correctly_parsed) { this->correctly_parsed = correctly_parsed; }
	void set_log_io( LogIO* log_io ){ this->log_io = log_io; }
	LogIO* get_log_io() const { return log_io; }
	void set_parent( Statement* parent ){ this->parent = parent; }
	void set_global_scope_parent( Statement* global_scope_parent ){ this->global_scope_parent = global_scope_parent;}
	Statement* get_parent() const{ return parent; }
	Statement* get_global_scope_parent() const{ return global_scope_parent; }
	void set_belonging_scope( Scope* belonging_scope ){ this->belonging_scope = belonging_scope; }
	Scope* get_belonging_scope() const { return belonging_scope; }
	Scope* get_own_scope() const { return own_scope; }
	bool get_has_own_scope() const { return has_own_scope; }
	bool get_consider_instances_dependency() const{ return consider_instances_dependency; }
	void set_consider_instances_dependency( bool consider_instances_dependency ){
		this->consider_instances_dependency = consider_instances_dependency; }
	bool get_scanned_for_instances_dependency() const{ return scanned_for_instances_dependency; }
	void set_scanned_for_instances_dependency( bool scanned_for_instances_dependency ){
		this->scanned_for_instances_dependency = scanned_for_instances_dependency; }
	Statement* get_dependency_parent() const{ return dependency_parent; }
	void set_dependency_parent( Statement* dependency_parent ){
		this->dependency_parent = dependency_parent; }
	Statement* get_global_dependency_parent() const{ return global_dependency_parent; }
	void set_global_dependency_parent( Statement* global_dependency_parent ){
		this->global_dependency_parent = global_dependency_parent; }

	std::string get_path_to_belonging_circuit() const{return path_to_belonging_circuit;}
	void set_path_to_belonging_circuit( std::string path_to_belonging_circuit ){
		this->path_to_belonging_circuit = path_to_belonging_circuit; }

	std::string get_formal_path_to_belonging_circuit() const{return formal_path_to_belonging_circuit;}
	void set_formal_path_to_belonging_circuit( std::string formal_path_to_belonging_circuit ){
		this->formal_path_to_belonging_circuit = formal_path_to_belonging_circuit; }

	/**
	 * @brief sets if it should be exported
	 *
	 * @param mute_exportation
	 */
	void set_mute_exportation( bool mute_exportation) {this->mute_exportation = mute_exportation;}

	// Add or Find and retrieve attributes
	void AddStatement( Statement* instance );
	void RemoveStatement( Statement* instance );
	void AddDescendant( Statement* descendant );
	void AddStatementAtBeginning( Statement* instance);
	void AddNode( std::string node_name );
	void AddNode( std::string node_name, bool pin_in_subcircuit );
	void AddLocalNode( Node* node );
	void AddParameter( Parameter* parameter );
	bool GetChildById( int search_id, Statement*& child );
	Parameter* GetParameter( int position );
	Parameter* GetParameter( std::string name );

	/**
	 * @brief gets a parameter present in the statement.
	 * @details ets a parameter present in the statement.
	 * If the paramameter is not present, it searches recursively in the children.
	 *
	 * @param name Parameter name
	 * @param parameter parameter found
	 *
	 * @return true if the parameter has been found
	 */
	bool GetParameterHierarchical( std::string name, Parameter*& parameter );
	Node* GetNode( int position );
	Node* GetNode( std::string name );

	// Radiation properties
	virtual bool TestUnalterableStatement(const std::string master_name);
	virtual bool TestCanBeInjected(const std::string master_name);
	virtual bool TestCanBeSubstituted(const std::string master_name,
		std::string& new_master_name );

	/**
	 * @brief Mutes the exportation of all non essential (radiation_simulator) analysis
	 * @details Mute the exportation of all non essential (radiation_simulator) analysis
	 * Is overriden in analysis_statement.hpp class
	 */
	virtual void MuteAllNonEssentialAnalysis();


	/**
	 * @brief Mutes the exportation of all analysis which are not the main transient
	 * Is overriden in analysis_statement.hpp class and circuit_statement.hpp class
	 */
	virtual void MuteNonMainTransientAnalysis();

protected:
	// Logger
	LogIO* log_io;

	// Each statement 'S'

	//////////////////////////
	// Statement Properties //
	//////////////////////////

	// Circuit in which the statement 'S' is enclosed
	Statement* belonging_circuit;
	// Id
	int id;
	// Statement type id
	int statement_type;
	/// wheter or not the statement is exported
	bool mute_exportation;
	// Allow/has/number of parallel instances
	bool has_parallel_statements;
	std::string parallel_statements;
	// Correctly parsed from netlist to our program
	bool correctly_parsed;
	// Properties as Class (instance, subcircuit, model...), not as a specific instance,
	// that determine if the element can have the related property.
	bool has_brackets;
	bool has_nodes;
	bool has_children;
	bool has_parameters;
	bool has_raw_content;
	// Refers the master ('DP') that 'S' instantiates/represent
	std::string master_name;
	// Description of the statement Class
	std::string statement_type_description;
	// Name identifier
	std::string name;
	// Description (if any)
	std::string description;
	// Value(s) not considered in this simulator
	std::string raw_content;
	// List of pointers to the nodes to which
	// the statement is connected
	std::vector<Node*> nodes;
	// Parameters of the statement
	std::vector<Parameter*> parameters;
	// Statements embedded inside the 'parent'
	// Ex: the resistor and capacitor in a subcircuit named 'RC'
	// containing a resistor in series with a capacitor
	std::vector<Statement*> children;
	// Pointer to the parent 'P' statements (which embeds the statement 'S').
	Statement* parent;
	/// Path to the belonging circuit: statemnet->parent->...
	std::string path_to_belonging_circuit;
	// Human readable
	std::string formal_path_to_belonging_circuit;

	/////////////////////////
	// Dependency Managing //
	/////////////////////////

	// Flag: statement has been scanned considering dependency
	bool scanned_for_instances_dependency;
	// Flag: statement should be scanned considering dependency
	bool consider_instances_dependency;
	// Statements that instantiates the statement as the 'dependency_parent'
	// Ex: the instance myRC witch instantiates a subcircuit named 'RC'
	// containing a resistor in series with a capacitor
	std::vector<Statement*> dependency_children;
	// Pointer to the statement 'DP' that is instantiate by 'S'.
	// Therefore, 'DP' is identified by the 'master_name' of 'S'
	Statement* dependency_parent;
	// Pointer to the statement 'GDP' that is the final parent
	// in a chain instantiations.
	// Ex: 'S'->'DP1'->'DP2'->...->'GDP'.
	Statement* global_dependency_parent;

	////////////////////
	// Scope Managing //
	////////////////////
	// Pointer to the global scope parent: circuit or subcircuit
	Statement* global_scope_parent;
	Scope* belonging_scope;
	Scope* own_scope;
	bool has_own_scope;

	//////////////////////////
	// Radiation Properties //
	//////////////////////////
	bool unalterable;
	// Altered by injection/substitution
	// If it has children, all the perceptible children
	// have to be altered.
	bool altered;
	// Specific Statement, not as Class, can be injected
	bool can_be_injected;
	// Specific Statement, not as Class, can be substituted
	bool can_be_substituted;
	// Master name which replace the original in case of radiation
	std::string substitute_master_name;


	/////////////////////
	// Parsing Methods //
	/////////////////////
	void RemoveExcessiveEmptyLines( std::string& s);
	bool ProcessLine( std::string& statementLine,
		std::string& currentReadLine, Statement& parent,
		int& statementCount, bool& parsingSpectreCode );
	// Parse parameters
	/**
	 * @brief Parse a parameters sentence
	 * @details Parses a parameters sentence as the following if non-valued parameters are allowed
	 * '[parameters] p1 p2 p3=v2 +(1+3) +1 + v22    p4 p5=v4+4 p6 p7  p8 p9 p10=1 p11=11 p12=v12+v13+ v14 +v15 p13=11'
	 * or as the following if non-valued parameters are NOT allowed
	 * '[parameters] p3=v2 +(1+3) +1 + v22    p5=v4+4 p10=1 p11=11 p12=v12+v13+ v14 +v15 p13=11'
	 * Note that [parameters] at the beging of the sentence is optional
	 *
	 * @param parameterLine string to be parsed
	 * @param allowUnvaluedParameters
	 *
	 * @return true if correctly parsed
	 */
	bool ParseParameters( std::string& parameterLine, bool allowUnvaluedParameters );
	// Parse Statement
	bool ParseStatement( std::ifstream* file, std::string& statementLine,
		Statement& parent, Statement& global_scope_parent,
		std::string& currentReadLine, int& statementCount,
		bool& endOfFile, bool& parsingSpectreCode, const bool permissiveParsingMode );

	////////////////////////////////////
	// Experiment Environment Methods //
	////////////////////////////////////

	virtual void AddGlobalNode( Node* node );
	virtual bool TestReservedStatement(const std::string start);
	virtual bool TestAnalysisStatement(const std::string statementCode);
	virtual bool TestIsControlStatementAdvanced(const std::string control_master_name);
	virtual int TestConditionalStatement(const std::string statementCode);
	virtual bool TestIsAnalysisStatementAdvanced(const std::string analysis_master_name);
	virtual bool TestIsControlStatementSpecialSintax(const std::string control_master_name);
	virtual int TestIncludeStatement(const std::string start);
	virtual bool TestControlStatement(
		const std::string statementCode, bool& special_syntax_control_statement);
	virtual bool TestTransistorStatement( const std::string statementCode );
	virtual bool TestUnalterableNode( const std::string node_name );
	virtual bool TestIsInstanceOfPrimitive(const std::string m_name);
	virtual bool TestIsNonSpectreLanguage( const std::string statementCode );
	virtual bool TestIsSpectreLanguage( const std::string statementCode );

	// Create a deep copy of the list of children
	void deepCopyOfChildren(const std::vector<Statement*>& source );

};

#endif /* STATEMENT_H_ */
