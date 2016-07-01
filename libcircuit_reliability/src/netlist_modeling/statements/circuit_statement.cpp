/*
 * circuit_statement.cpp
 *
 *  Created on: April 1, 2013
 *  Author: fernando
 */

#include "boost/filesystem.hpp"   // includes all needed Boost.Filesystem declarations
#include <boost/algorithm/string.hpp>

#include "circuit_statement.hpp"

#include "../../global_functions_and_constants/global_template_functions.hpp"
#include "../../global_functions_and_constants/global_constants.hpp"
#include "../../global_functions_and_constants/statements_constants.hpp"


CircuitStatement::CircuitStatement( ) {
	this->id = kNotDefinedInt;
	this->circuit_id = kNotDefinedInt;
	this->statement_type = kCircuitStatement;
	this->parallel_statements = kDefaultParallelStatements;
	this->has_parallel_statements = false;
	this->correctly_parsed = false;
	this->name = kNotDefinedString;
	this->master_name = kNotDefinedString;
	this->description = kNotDefinedString;
	this->has_brackets = true;
	this->has_nodes = true;
	this->has_children = true;
	this->has_parameters = true;
	this->has_raw_content = false;
	this->raw_content = kNotDefinedString;
	this->mute_exportation = false;
	// Radiation Properties
	this->unalterable = false;
	this->altered = false;
	this->can_be_injected = false;
	this->can_be_substituted = false;
	this->substitute_master_name = kNotDefinedString;
	//belonging_circuit and parent is this
	this->belonging_circuit = this;
	this->parent = this;
	this->statement_type_description = kCircuitStatementDesc;
	// scope
	this->has_own_scope = true;
	this->own_scope = new Scope( "circuit_" + number2String(circuit_id)
		+ "_"+ number2String(id) + "_scope", name , false );
	this->belonging_scope = own_scope;
	this->considerate_dependencies = false;
	// Dependency
	this->scanned_for_instances_dependency = true;
	this->consider_instances_dependency = false;
}

CircuitStatement::CircuitStatement( const bool considerate_dependencies ) {
	this->id = kNotDefinedInt;
	this->circuit_id = kNotDefinedInt;
	this->statement_type = kCircuitStatement;
	this->parallel_statements = kDefaultParallelStatements;
	this->has_parallel_statements = false;
	this->correctly_parsed = false;
	this->name = kNotDefinedString;
	this->master_name = kNotDefinedString;
	this->description = kNotDefinedString;
	this->has_brackets = true;
	this->has_nodes = true;
	this->has_children = true;
	this->has_parameters = true;
	this->has_raw_content = false;
	this->raw_content = kNotDefinedString;
	this->mute_exportation = false;
	// Radiation Properties
	this->unalterable = false;
	this->altered = false;
	this->can_be_injected = false;
	this->can_be_substituted = false;
	this->substitute_master_name = kNotDefinedString;
	//belonging_circuit and parent is this
	this->belonging_circuit = this;
	this->parent = this;
	this->statement_type_description = kCircuitStatementDesc;
	// scope
	this->global_scope_parent = this;
	this->has_own_scope = true;
	this->own_scope = new Scope( "circuit_" + number2String(circuit_id)
		+ "_"+ number2String(id) + "_scope", name , false );
	this->belonging_scope = own_scope;
	this->considerate_dependencies = considerate_dependencies;
	// Dependency
	this->scanned_for_instances_dependency = true;
	this->consider_instances_dependency = false;
}

CircuitStatement::CircuitStatement(const CircuitStatement& orig) {
	this->id = orig.id;
	this->circuit_id = orig.circuit_id;
	this->statement_type = orig.statement_type;
	this->path_to_belonging_circuit = orig.path_to_belonging_circuit;
	this->parallel_statements = orig.parallel_statements;
	this->has_parallel_statements = orig.has_parallel_statements;
	this->correctly_parsed = orig.correctly_parsed;
	this->name = orig.name;
	this->master_name = orig.master_name;
	this->description = orig.description;
	this->has_brackets = orig.has_brackets;
	this->has_nodes = orig.has_nodes;
	this->has_children = orig.has_children;
	this->has_parameters = orig.has_parameters;
	this->has_raw_content = orig.has_raw_content;
	this->raw_content = orig.raw_content;
	this->mute_exportation = orig.mute_exportation;
	this->unalterable = orig.unalterable;
	this->can_be_substituted = orig.can_be_substituted;
	this->substitute_master_name = orig.substitute_master_name;
	this->altered = orig.altered;
	this->can_be_injected = orig.can_be_injected;
	//belonging_circuit and parent is this
	this->belonging_circuit = this;
	this->parent = this;
	this->statement_type_description = orig.statement_type_description;
	deepCopyVectorOfPointers(orig.nodes, nodes);
	deepCopyOfCircuitChildren( orig.children );
	deepCopyOfCircuitSpecialChildren( orig.simulation_special_children );
	deepCopyVectorOfPointers(orig.parameters, parameters);
	this->experiment_environment = orig.experiment_environment;
	this->referenced_include_statements = orig.referenced_include_statements;
	this->referenced_circuit_statements = orig.referenced_circuit_statements;
	this-> included_analysis_statements = orig.included_analysis_statements;
	this->included_control_statements = orig.included_control_statements;
	// scope
	this->global_scope_parent = this;
	this->has_own_scope = true;
	this->own_scope = new Scope( "circuit_" + number2String(orig.circuit_id)
		+ "_"+ number2String(orig.id) + "_scope", orig.name , false );
	this->belonging_scope = own_scope;
	// post_parsing_statement_handler
	this->post_parsing_statement_handler = orig.post_parsing_statement_handler;
	this->considerate_dependencies = orig.considerate_dependencies;
	// Dependency
	this->scanned_for_instances_dependency = true;
	this->consider_instances_dependency = false;
}

CircuitStatement* CircuitStatement::GetCopy() {
	return new CircuitStatement(*this);
}

void CircuitStatement::deepCopyOfCircuitChildren( const std::vector<Statement*>& source ) {
	// delete previous elements
	deleteContentsOfVectorOfPointers(children);
	// reserve memory
	children.reserve( source.size() );
	for( auto const &s : source ){
		children.push_back( s->GetCopy() ) ;
		children.back()->set_belonging_scope( belonging_scope );
		children.back()->set_parent( this );
		if( s->get_statement_type()==kIncludeStatement ){
			IncludeStatement* pInclude = dynamic_cast<IncludeStatement*>( s );
			referenced_include_statements.push_back( pInclude );
		}else if( s->get_statement_type()==kGlobalStatement ){
			GlobalStatement* pGlobal = dynamic_cast<GlobalStatement*>( s );
			included_global_statements.push_back( pGlobal );
		}
	}
}

void CircuitStatement::deepCopyOfCircuitSpecialChildren(const std::vector<Statement*>& source ) {
	// delete previous elements
	deleteContentsOfVectorOfPointers(simulation_special_children);
	// reserve memory
	simulation_special_children.reserve( source.size() );
	for( auto const &s : source ){
		simulation_special_children.push_back( s->GetCopy() ) ;
		simulation_special_children.back()->set_belonging_scope( belonging_scope );
		simulation_special_children.back()->set_parent( this );
	}
}

// Used for copy Only the cildren
void CircuitStatement::basicCopy( CircuitStatement* orig ){
	deepCopyOfCircuitChildren( *orig->get_children() );
	deepCopyOfCircuitSpecialChildren( *orig->get_simulation_special_children() );
	deepCopyVectorOfPointers( *orig->get_parameters(), parameters);
	deepCopyVectorOfPointers( *orig->get_referenced_circuit_statements(), referenced_circuit_statements );
	// copy global/ include statements
}

CircuitStatement::~CircuitStatement() {
	#ifdef DESTRUCTORS_VERBOSE
		log_io->ReportPlainStandard( "CircuitStatement destructor. Name: " + name + ". Direction:" + number2String(this));
	#endif
	deleteContentsOfVectorOfPointers(simulation_special_children);
}

void CircuitStatement::MuteNonMainTransientAnalysis(){
	// Regular analysis
	if( included_analysis_statements.size()>0 ){
		for( auto const &as : included_analysis_statements ){
			as->MuteNonMainTransientAnalysis();
		}
	}
	if( simulation_special_children.size()>0 ){
		// Analysis included in special children
		for( auto const& ss : simulation_special_children ){
			ss->MuteNonMainTransientAnalysis();
		}
	}
}

std::string CircuitStatement::ExportCircuitStatement( const std::string& indentation ){
	//circuit statements do not indent the childs
	std::string cs = kCommentWord1 + name  + ", circuit_id: "
		+ number2String(circuit_id) + kEmptyLine + kEmptyLine;
	// languaje (spectre language)
	cs += kLanguageSentence + kEmptyLine;
	// global nodes
	if(included_global_statements.size() > 0){
		for( auto const &gs :  included_global_statements ){
			cs += kEmptyLine + gs->ExportCircuitStatement("");
		}
	}
	// Parameters
	if( parameters.size() > 0 ){
		cs += kEmptyLine + kParametersWord + kDelimiter;
		//export parameters
		for( auto const &p : parameters ){
			cs +=  kDelimiter + p->ExportParameter();
		}
		cs += kEmptyLine;
	}
	// Include statements
	if(referenced_include_statements.size() > 0){
		for(  auto const &ris : referenced_include_statements ){
			cs += kEmptyLine + ris->ExportCircuitStatement("");
		}
	}
	// export children
	if(children.size() > 0){
		for( auto const & c : children ){
			if( c->get_statement_type()!= kGlobalStatement
				&& c->get_statement_type()!= kIncludeStatement){
				cs += kEmptyLine + c->ExportCircuitStatement("");
			}
		}
	}
	// export special children (simulation mode related) at the end of netlist file
	if(simulation_special_children.size() > 0){
		for( auto const & sc : simulation_special_children ){
			cs += kEmptyLine + sc->ExportCircuitStatement("");
		}
	}
	return cs;
}

bool CircuitStatement::ExportCircuit2SCS( const std::string& outputFilePath ){
	std::ofstream outputFile;
	outputFile.open(outputFilePath.c_str());
	if (!outputFile){
		log_io->ReportError2AllLogs( kTab + "error writing " + outputFilePath);
		return false;
	}
	outputFile << ExportCircuitStatement( "" );
	outputFile.close();
  return true;
}

void CircuitStatement::UpdateOwnScope(){
	own_scope->set_name( "circuit_" + number2String(circuit_id)
		+ "_"+ number2String(id) + "_scope" );
	own_scope->set_main_statement_name( name );
}

// Process the specified netlist
// @param parent Circuit to be created
bool CircuitStatement::ParseNetlist( const std::string& netlist, int& statementCount,
	bool& parsingSpectreCode, const bool permissiveParsingMode ){
	log_io->ReportPlainStandard( k2Tab + "Parsing netlist " );
	bool correctly_parsed = false;
	// update scope name
	UpdateOwnScope();
	// Check availability of file
	if( !boost::filesystem::exists(netlist) ){
		log_io->ReportError2AllLogs( k2Tab + "Netlist " + netlist + " does not exists!!" );
		// throw new std::exception();
		return false;
	}else{
		#ifdef PARSING_VERBOSE_MIN
			log_io->ReportPlain2Log( k2Tab + "Netlist " + netlist + " exists" );
		#endif
	}
	std::string currentReadLine;
	std::string statementCode;
	bool endOfFile = false;
	bool completedStatement = true;
	try {
		experiment_environment->TestEnvironment();
		// Parsing spectre language if the file is
		parsingSpectreCode =  boost::ends_with( netlist, kDot + kSCS) ;
		std::ifstream file(netlist.c_str());
		#ifdef PARSING_VERBOSE
			log_io->ReportPlain2Log( k2Tab + "Netlist " + netlist + " opened" );
		#endif
		if (file.is_open()) {
			if (file.good()) {
				// Buffers initialization
				statementCode = "";
				// File parsing
				while(!endOfFile && getline(file, currentReadLine)) {
					// ProcessLine
					if( ProcessLine(statementCode, currentReadLine, *this, statementCount, parsingSpectreCode) ){
						continue;
					}else{
						// parse childs
						if(statementCode.compare(kEmptyWord) != 0 &&
							statementCode.compare(kEmptyLine) !=0 ){
							completedStatement = completedStatement
							&& ParseStatement(&file, statementCode, *this, *this, currentReadLine,
							statementCount, endOfFile, parsingSpectreCode, permissiveParsingMode );
						}
						// reset line Buffers
						statementCode = currentReadLine;
					}
				} //ends while
				// proccess final statement if not previously parsed
				if(!endOfFile && statementCode.compare(kEmptyWord) != 0 &&
					statementCode.compare(kEmptyLine) !=0 ){
					completedStatement = completedStatement
						&& ParseStatement(&file, statementCode, *this, *this, currentReadLine,
							statementCount, endOfFile, parsingSpectreCode, permissiveParsingMode );
				}
				// reset line Buffers
				statementCode = currentReadLine;
			}
		}
		file.close();
		correctly_parsed = true;
	} catch (std::exception const&  ex) {
		correctly_parsed = false;
		std::string ex_what = ex.what();
		//log_io->ReportError2AllLogs( "Exception while parsing the netlist: ex-> " + ex.what() );
		log_io->ReportError2AllLogs( "Exception while parsing the netlist: ex-> " + ex_what );
	}
	log_io->ReportPlain2Log(  kLongDelimiter );
	if(completedStatement && correctly_parsed){
		log_io->ReportPlain2Log(  "The netlist was correctly parsed. " + number2String(statementCount) + " statements added." );
	}else{
		log_io->ReportPlain2Log(  "The netlist was not parsed as espected. " );
	}
	log_io->ReportPlain2Log(  kLongDelimiter );
	return completedStatement;
}


void CircuitStatement::AddIncludeStatementAndRegister( IncludeStatement* statement ){
	statement->set_parent( this );
	children.insert(children.begin(), statement);
	referenced_include_statements.push_back( statement );
}

void CircuitStatement::AddSimulationSpecialStatement( Statement* statement ){
	simulation_special_children.push_back( statement );
}

// Calls to Experiment environment

void CircuitStatement::AddGlobalNode( Node* node ){
	experiment_environment->AddGlobalNode( node );
}

bool CircuitStatement::TestUnalterableStatement(const std::string master_name){
	return experiment_environment->TestUnalterableStatement( master_name );
}

bool CircuitStatement::TestCanBeInjected(const std::string master_name){
	return experiment_environment->TestCanBeInjected( master_name );
}

bool CircuitStatement::TestCanBeSubstituted(const std::string master_name,
	std::string& new_master_name ){
	return experiment_environment->TestCanBeSubstituted( master_name, new_master_name );
}

bool CircuitStatement::TestIsNonSpectreLanguage( const std::string statementCode ){
	return experiment_environment->TestIsNonSpectreLanguage( statementCode );
}

bool CircuitStatement::TestIsSpectreLanguage( const std::string statementCode ){
	return experiment_environment->TestIsSpectreLanguage( statementCode );
}

bool CircuitStatement::TestUnalterableNode( const std::string node_name ){
	return experiment_environment->TestUnalterableNode( node_name );
}

bool CircuitStatement::TestReservedStatement(const std::string start){
	return ( experiment_environment->TestReservedStatement( start ) );
}

int CircuitStatement::TestIncludeStatement(const std::string start){
	return ( experiment_environment->TestIncludeStatement( start ) );
}

int CircuitStatement::TestConditionalStatement(const std::string start){
	return ( experiment_environment->TestConditionalStatement( start ) );
}

bool CircuitStatement::TestAnalysisStatement(const std::string statementCode){
	return ( experiment_environment->TestAnalysisStatement( statementCode ) );
}

bool CircuitStatement::TestIsAnalysisStatementAdvanced(const std::string analysis_master_name){
	return ( experiment_environment->TestIsAnalysisStatementAdvanced( analysis_master_name ) );
}

bool CircuitStatement::TestControlStatement(
	const std::string statementCode, bool& special_syntax_control_statement){
	return ( experiment_environment->TestControlStatement(
		statementCode, special_syntax_control_statement) );
}

bool CircuitStatement::TestIsControlStatementAdvanced(const std::string control_master_name){
	return ( experiment_environment->TestIsControlStatementAdvanced( control_master_name ) );
}

bool CircuitStatement::TestIsControlStatementSpecialSintax(const std::string control_master_name){
	return ( experiment_environment->TestIsControlStatementSpecialSintax( control_master_name ) );
}

bool CircuitStatement::TestTransistorStatement( const std::string statementCode ){
	return ( experiment_environment->TestTransistorStatement( statementCode ) );
}

bool CircuitStatement::TestIsInstanceOfPrimitive( const std::string master_name ){
	return ( experiment_environment->TestIsInstanceOfPrimitive( master_name ) );
}

// Instances listings

void CircuitStatement::AddReferencedIncludeStatement( IncludeStatement* referenced_include_statement ){
	referenced_include_statements.push_back( referenced_include_statement );
}

void CircuitStatement::AddReferencedCircuitStatement( CircuitStatement* referenced_circuit_statement ){
	referenced_circuit_statements.push_back( referenced_circuit_statement );
}

void CircuitStatement::AddIncludedAnalysisStatement( AnalysisStatement* included_analysis_statement ){
	included_analysis_statements.push_back( included_analysis_statement );
	post_parsing_statement_handler->AddAnalysisStatementFound( included_analysis_statement );
}

void CircuitStatement::AddIncludedConditionalStatement( ConditionalStatement* included_conditional_statement ){
	included_conditional_statements.push_back( included_conditional_statement );
}

void CircuitStatement::AddIncludedControlStatement( ControlStatement* included_control_statement ){
	included_control_statements.push_back( included_control_statement );
}

void CircuitStatement::AddIncludedGlobalStatement( GlobalStatement* included_global_statement ){
	included_global_statements.push_back( included_global_statement );
}


void CircuitStatement::AddIncludedLibraryStatement( LibraryStatement* included_library_statement ){
	included_library_statements.push_back( included_library_statement );
}

void CircuitStatement::AddIncludedParamTestStatement( ParamTestStatement* included_param_test_statement ){
	included_param_test_statements.push_back( included_param_test_statement );
}

void CircuitStatement::AddIncludedSectionStatement( SectionStatement* included_section_statement ){
	included_section_statements.push_back( included_section_statement );
}

void CircuitStatement::AddIncludedSimpleStatement( SimpleStatement* included_simple_statement ){
	included_simple_statements.push_back( included_simple_statement );
}

void CircuitStatement::AddIncludedUserDefinedFunctionStatement( UserDefinedFunctionStatement* included_user_defined_function_statement ){
	included_user_defined_function_statements.push_back( included_user_defined_function_statement );
}

// Considered for dependency
void CircuitStatement::AddIncludedModelStatement( ModelStatement* included_model_statement ){
	included_model_statements.push_back( included_model_statement );
	if( considerate_dependencies ){
		post_parsing_statement_handler->AddModelStatementFound( included_model_statement );
	}
}


void CircuitStatement::AddIncludedAnalogModelStatement( AnalogModelStatement* included_analog_model_statement ){
	included_analog_model_statements.push_back( included_analog_model_statement );
	if( considerate_dependencies ){
		post_parsing_statement_handler->AddAnalogModelStatementFound( included_analog_model_statement );
	}
}

void CircuitStatement::AddIncludedInstanceStatement( InstanceStatement* included_instance_statement ){
	included_instance_statements.push_back( included_instance_statement );
	if( considerate_dependencies ){
		post_parsing_statement_handler->AddInstanceStatementFound( included_instance_statement );
	}
}

void CircuitStatement::AddIncludedSubcircuitStatement( SubcircuitStatement* included_subcircuit_statement ){
	included_subcircuit_statements.push_back( included_subcircuit_statement );
	if( considerate_dependencies ){
		post_parsing_statement_handler->AddSubcircuitStatementFound( included_subcircuit_statement );
	}
}

void CircuitStatement::AddIncludedTransistorStatement( TransistorStatement* included_transistor_statement ){
	included_transistor_statements.push_back( included_transistor_statement );
	if( considerate_dependencies ){
		post_parsing_statement_handler->AddTransistorStatementFound( included_transistor_statement );
	}
}
