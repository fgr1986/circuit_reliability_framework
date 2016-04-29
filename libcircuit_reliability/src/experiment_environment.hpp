 /**
 * @file experiment_environment.hpp
 *
 * @date Created on: Jan 2, 2014
 *
 * @author Author: Fernando García <fgarcia@die.upm.es> <fernando.garca@gmail.com>
 *
 * @section DESCRIPTION
 *
 * This Class contain the environment regarding the experiment:
 * 		+ Simulation configuration
 * 		+ Keywords list and relating testing methods
 * 		+ Radiation Sources list and relating testing methods
 * 		+ Unalterable nodes list and relating testing methods
 * 		+ Technology Transistors list and relating testing methods
 * 		+ Injectable Statements list and relating testing methods
 * 		+ Replaceable Statements list and relating testing methods
 * 		+ Unalterable Statements list and relating testing methods
 * 		+ Excluded Folders list and relating testing methods
 * 		+ Control Statements list and relating testing methods
 * 		+ Analysis Statements list and relating testing methods
 * 		+ Bsources Statements list and relating testing methods
 * 		+ Global Nodes Statements list and relating testing methods
 *
 */


#ifndef EXPERIMENT_ENVIRONMENT_H
#define EXPERIMENT_ENVIRONMENT_H

///ExperimentEnvironment Class
// c++ std libraries includes
#include <string>
#include <vector>
// radiation io simulator includes
#include "io_handling/log_io.hpp"
// Radiation Simulator includes
#include "simulations_handling/simulation_mode.hpp"
// Netlist modeling includes
#include "netlist_modeling/statements/analysis_statement.hpp"
#include "netlist_modeling/statements/control_statement.hpp"
#include "netlist_modeling/statements/statement.hpp"
#include "netlist_modeling/statements/transistor_statement.hpp"
#include "netlist_modeling/statements/radiation_source_subcircuit_statement.hpp"
#include "netlist_modeling/node.hpp"

class ExperimentEnvironment {
public:
	/// Default constructor
	ExperimentEnvironment();
	/// Default destructor
	virtual ~ExperimentEnvironment();
	/**
 	* @details get_excluded_canonical_folders method
 	* @return excluded_canonical_folders <std::vector<std::string>*>
 	*/
	std::vector<std::string>* get_excluded_canonical_folders() {return &excluded_canonical_folders;}

	/**
 	* @details get_primitive_names method
 	* @return primitive_names <std::vector<std::string>*>
 	*/
	std::vector<std::string>* get_primitive_names() {return &primitive_names;}

	/**
 	* @details get_unalterable_nodes method
 	* @return unalterable_nodes <std::vector<std::string>*>
 	*/
	std::vector<std::string>* get_unalterable_nodes() {return &unalterable_nodes;}

	/**
 	* @details get_reserved_words method
 	* @return reserved_words <std::vector<std::string>*>
 	*/
	std::vector<std::string>* get_reserved_words() {return &reserved_words;}

	/**
 	* @details get analysis_types method
 	* @return analysis_types <std::vector<AnalysisStatement*>*>
 	*/
	std::vector<AnalysisStatement*>* get_analysis_types() {return &analysis_types;}

	/**
 	* @details get global_nodes method
 	* @return global_nodes <std::vector<Node*>*>
 	*/
	std::vector<Node*>* get_global_nodes() {return &global_nodes;}

	/**
 	* @details get control_statement_types method
 	* @return control_statement_types <std::vector<ControlStatement*>*>
 	*/
	std::vector<ControlStatement*>* get_control_statement_types() {return &control_statement_types;}

	/**
 	* @details get transistor_statement_types method
 	* @return transistor_statement_types <std::vector<TransistorStatement*>*>
 	*/
	std::vector<TransistorStatement*>* get_transistor_statement_types() {return &transistor_statement_types;}

	/**
 	* @details get technology_name method
 	* @return technology_name <std::string>
 	*/
	std::string get_technology_name() const{
		return technology_name; }

	/**
 	* @details get technology_models_file_path method
 	* @return technology_models_file_path <std::string>
 	*/
	std::string get_technology_models_file_path() const{
		return technology_models_file_path; }

	/**
 	* @details get technology_considering_section method
 	* @return technology_considering_section <std::string>
 	*/
	bool get_technology_considering_sections() const{
		return technology_considering_sections; }

	/**
 	* @details get technology_golden_section method
 	* @return technology_golden_section <std::string>
 	*/
	std::string get_technology_golden_section() const{
		return technology_golden_section; }

	/**
 	* @details get technology_experiment_section method
 	* @return technology_experiment_section <std::string>
 	*/
	std::string get_technology_experiment_section() const{
		return technology_experiment_section; }

	/**
	 * @brief get unalterable_statements list
	 * @return unalterable_statements
	 */
	std::vector<std::string>* get_unalterable_statements(){ return &unalterable_statements; }



	/**
 	* @details sets simulation_mode and updates selected_radiation_source
 	* @param simulation_mode <SimulationModesHandler*>
 	*/
	void set_simulation_mode( SimulationMode* simulation_mode) { this->simulation_mode = simulation_mode; }

	/**
 	* @details sets technology_name
 	* @param technology_name <std::string>
 	*/
	void set_technology_name( std::string technology_name ) {
		this->technology_name = technology_name; }

	/**
 	* @details sets technology_models_file_path
 	* @param technology_models_file_path <std::string>
 	*/
	void set_technology_models_file_path( std::string technology_models_file_path ) {
		this->technology_models_file_path = technology_models_file_path; }

	/**
 	* @details sets technology_experiment_section
 	* @param technology_experiment_section <std::string>
 	*/
	void set_technology_experiment_section( std::string technology_experiment_section ) {
		this->technology_experiment_section = technology_experiment_section; }

	/**
 	* @details sets technology_considering_sections
 	* @param technology_considering_sections <std::string>
 	*/
	void set_technology_considering_sections( bool technology_considering_sections ) {
		this->technology_considering_sections = technology_considering_sections; }

	/**
 	* @details sets technology_golden_section
 	* @param technology_golden_section <std::string>
 	*/
	void set_technology_golden_section( std::string technology_golden_section ) {
		this->technology_golden_section = technology_golden_section; }

	/**
 	* @details sets log_io
 	* @param log_io <LogIO*>
 	*/
	void set_log_io( LogIO* log_io ){ this->log_io = log_io; }

	/// Updates analysis statements regex sentence
 	void UpdatekAnalysisStatementRegEx();

	/// Updates control statements regex sentence
	void UpdatekControlStatementRegEx();

	/// Updates transistors regex sentence
	void UpdatekTransistorStatementRegEx();

	/// Adds an injectable statement to the list
	void AddInjectableStatement( std::string injectable_statement );

	/// Adds a replaceable statement to the list
	void AddReplaceableStatement( std::string replaceable_statement,
		std::string new_statement );

	/// Adds a global node
	void AddGlobalNode( Node* node );

	/// Adds an excluded_canonical_folder
	void AddExcludedCanonicalFolder( std::string excluded_canonical_folder );

	/// Adds a primitive name
	void AddPrimitiveName( std::string primitive_name );

	/// Adds a reserved word
	void AddReservedWord( std::string reserved_word );

	/// Adds an unalterable node
	void AddUnalterableNode( std::string unalterable_node );

	/// Adds an unalterable statement
	void AddUnalterableStatement( std::string unalterable_statement );

	/// Adds an analysis type
	void AddAnalysisType( AnalysisStatement* analysis_type );

	/// Adds an injection source type
	void AddInjectionRadiationSourceSubcircuitStatementType(
		RadiationSourceSubcircuitStatement* radiation_source_statement_type );

	/// Adds a replacement radiation source type
	void AddReplacementRadiationSourceSubcircuitStatementType(
		RadiationSourceSubcircuitStatement* radiation_source_statement_type );

	/// Adds a control statement type
	void AddControlStatementType( ControlStatement* control_statement_type );

	/// Adds a transistor type
	void AddTransistorStatementType( TransistorStatement* transistor_statement_type );

	/// Test if the language sentence is not a spectre one
	bool TestIsNonSpectreLanguage( const std::string& statementCode );

	/// if the language sentence is a spectre one
	bool TestIsSpectreLanguage( const std::string& statementCode );

	/// Test if the parameter is a reserved statement
	bool TestReservedStatement(const std::string& start);

	/// Test if the parameter is an analysis statement
	bool TestAnalysisStatement( const std::string& statementCode );

	/// Test if the parameter is an advanced analysis statement
	bool TestIsAnalysisStatementAdvanced(const std::string& analysis_master_name);

	/// Test if the parameter is an advanced control statement
	bool TestIsControlStatementAdvanced(const std::string& control_master_name);

	/// Test if the parameter is a control statement with special sintax
	bool TestIsControlStatementSpecialSintax( const std::string& control_master_name);

	/// Test if the parameter is a control statement
	bool TestControlStatement( const std::string& statementCode,
		bool& special_syntax_control_statement );

	/// Test which kind of include statement the parameter is
	int TestIncludeStatement( const std::string& start );

	/// Test which kind of conditional statement the parameter is
	int TestConditionalStatement( const std::string& start );

	/// Test if the parameter is an unalterable node
	bool TestUnalterableNode( const std::string& node_name );

	/// Test if the parameter is a transistor
	bool TestTransistorStatement( const std::string& statementCode);

	/// Test if the parameter is an instance of a primitive statement
	bool TestIsInstanceOfPrimitive( const std::string& master_name );

	/// Test if the parameter is an unalterable statement
	bool TestUnalterableStatement( const std::string& masterName);

	/// Test if the parameter can be injected
	bool TestCanBeInjected(const std::string& master_name);

	/// Test if the parameter can be substituted
	bool TestCanBeSubstituted(const std::string& master_name,
		std::string& new_master_name );

	/// Test if the parameter file is in an excluded folder
	bool TestIsCanonicalFolderExcluded( const std::string& canonical_path );

	/// Test the environment
	void TestEnvironment();

private:
	/// Mode
	SimulationMode* simulation_mode;
	/// Loger
	LogIO* log_io;
	/// File Parsing Excluded Folders list
	std::vector<std::string> excluded_canonical_folders;
	/// Unalterable statements
	std::vector<std::string> unalterable_statements;
	/// Unalterable nodes list
	std::vector<std::string> unalterable_nodes;
	/// Primitive names list
	std::vector<std::string> primitive_names;
	/// Reserved words list
	std::vector<std::string> reserved_words;
	/// Analysis list
	std::vector<AnalysisStatement*> analysis_types;
	/// Control statements list
	std::vector<ControlStatement*> control_statement_types;
	/// Transistor list
	std::vector<TransistorStatement*> transistor_statement_types;
	/// Global Nodes list
	std::vector<Node*> global_nodes;
	/// Technology name
	std::string technology_name;
	/// technology model files
	std::string technology_models_file_path;
	/// whether technology considers sections or not
	bool technology_considering_sections;
	/// technology golden section
	std::string technology_golden_section;
	/// technology experiment section
	std::string technology_experiment_section;

	/// Analysis regex
	std::string kAnalysisStatementRegEx;
	/// transistors regex
	std::string kTransistorStatementRegEx;
	/// Control statements regex
	std::string kControlStatementRegEx;
	/// Control statements regex with special sintax
	std::string kControlStatementRegExSpecial;
	/// Spectre lang regex
	std::string kSpectreLanguageRegEx = "\\s*\\bsimulator\\b\\s+\\blang\\b\\s*=\\s*\\bspectre\\b";
	/// Non spectre lang regex
	std::string kNonSpectreLanguageRegEx = "\\s*\\bsimulator\\b\\s+\\blang\\b\\s*=\\s*";

};

#endif /* EXPERIMENT_ENVIRONMENT_H */
