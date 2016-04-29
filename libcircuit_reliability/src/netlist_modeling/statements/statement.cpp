/*
 * statement.cpp
 *
 * Created on: March 11 2013
 *	 Author: fernando
 */

#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string/regex.hpp>

#include "statement.hpp"
#include "../../global_functions_and_constants/global_template_functions.hpp"
#include "../../global_functions_and_constants/global_constants.hpp"
#include "../../global_functions_and_constants/regex_constants.hpp"
#include "../../global_functions_and_constants/statements_constants.hpp"
#include "../../global_functions_and_constants/netlist_words_constants.hpp"

#include "include_statement.hpp"
#include "analysis_statement.hpp"
#include "circuit_statement.hpp"
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

void Statement::deepCopyOfChildren(const std::vector<Statement*>& source ) {
	// delete previous elements
	deleteContentsOfVectorOfPointers(children);
	// reserve memory
	children.reserve( source.size() );
	for( auto const &cs : source ){
		children.push_back( cs->GetCopy() ) ;
		children.back()->set_belonging_scope( belonging_scope );
		children.back()->set_parent( this );
	}
}

Statement::~Statement() {
	#ifdef DESTRUCTORS_VERBOSE
		std::cout<< "Statement destructor" << std::endl;
		std::cout<< "deleteContentsOfVectorOfPointers(children) destructor" << std::endl;
		std::cout<< "deleteContentsOfVectorOfPointers(parameters) destructor" << std::endl;
	#endif
	// The nodes are deleted in the scope destructor
	deleteContentsOfVectorOfPointers(children);
	deleteContentsOfVectorOfPointers(parameters);
	dependency_children.clear();
}

void Statement::AddDescendant( Statement* statement ){
	dependency_children.push_back( statement );
}

void Statement::AddStatement( Statement* statement ){
	statement->set_parent( this );
	children.push_back( statement );
}

void Statement::RemoveStatement( Statement* statement ){
	for( std::vector<Statement*>::iterator it_c = children.begin();
		it_c != children.end(); ){
		if( (*it_c)==statement ){
			#ifdef PARSING_VERBOSE
				log_io->ReportPlain2Log( "erasing: '" + (*it_c)->get_name() + "'");
			#endif
			it_c = children.erase( it_c );
		}else{
			it_c++;
		}
	}
}

void Statement::AddStatementAtBeginning( Statement* statement ){
	statement->set_parent( this );
	children.insert(children.begin(), statement);
}

void Statement::AddNode( std::string node_name ){
	#ifdef PARSING_VERBOSE
		log_io->ReportPlain2Log( "AddNode: '" + node_name + "'" );
	#endif
	if( node_name.compare(kEmptyWord)==0 ){
		return;
	}
	Node* node = belonging_scope->GetNode( node_name ) ;
	if( nullptr== node ){
		node = new Node( node_name, !TestUnalterableNode( node_name ) );
		belonging_scope->AddNode( node );
		#ifdef PARSING_VERBOSE
			log_io->ReportPlain2Log( "node_name: '" + node_name + "'" + " not in scope "+ belonging_scope->get_name() );
		#endif
	}else{
		#ifdef PARSING_VERBOSE
			log_io->ReportPlain2Log( "node_name: '" + node_name + "'" + " alreadyInScope "+ belonging_scope->get_name() );
		#endif
	}
	nodes.push_back( node );
}

void Statement::AddNode( std::string node_name, bool pin_in_subcircuit ){
	#ifdef PARSING_VERBOSE
		log_io->ReportPlain2Log( "AddNode, pin_in_subcircuit: '" + node_name + "'" );
	#endif
	if( node_name.compare(kEmptyWord)==0 ){
		return;
	}
	Node* node = belonging_scope->GetNode( node_name ) ;
	if( nullptr== node ){
		node = new Node( node_name, !TestUnalterableNode( node_name ), pin_in_subcircuit );
		belonging_scope->AddNode( node );
		#ifdef PARSING_VERBOSE
			log_io->ReportPlain2Log( "node_name: '" + node_name + "'" + " not in scope "+ belonging_scope->get_name() );
		#endif
	}else{
		#ifdef PARSING_VERBOSE
			log_io->ReportPlain2Log( "node_name: '" + node_name + "'" + " alreadyInScope "+ belonging_scope->get_name() );
		#endif
	}
	nodes.push_back( node );
}

void Statement::AddLocalNode( Node* node ){
	// belonging_scope->AddNode( node );
	nodes.push_back( node );
}

void Statement::AddParameter(Parameter* parameter){
	parameters.push_back(parameter);
}

Parameter* Statement::GetParameter(int position){
	if(has_parameters) {
		return parameters.at(position);
	}else{
		return nullptr;
	}
}

Parameter* Statement::GetParameter(std::string name){
	if(has_parameters) {
		for( auto const & p : parameters ){
			if( p->get_name().compare(name) == 0 ){
				return p;
			}
		}
	}
	return nullptr;
}

bool Statement::GetParameterHierarchical( std::string paramName, Parameter*& parameter ){
	if( has_parameters ) {
		for( auto const & p : parameters ){
			if( p->get_name().compare(paramName) == 0 ){
				parameter = p;
				return true;
			}
		}
	}
	if( has_children && children.size()>0 ){
		for( auto const & c : children ){
			if( c->GetParameterHierarchical( paramName, parameter ) ){
				return true;
			}
		}
	}
	return false;
}

/// Overriden in analysis statement
void Statement::MuteAllNonEssentialAnalysis(){
	if( has_children && children.size()>0 ){
		for( auto const & c : children ){
			c->MuteAllNonEssentialAnalysis();
		}
	}
}

/// Overriden in analysis statement and circuit
void Statement::MuteNonMainTransientAnalysis(){
	if( has_children && children.size()>0 ){
		for( auto const & c : children ){
			c->MuteNonMainTransientAnalysis();
		}
	}
}

Node* Statement::GetNode(int position){
	if(has_nodes) {
		return nodes.at(position);
	}else{
		return nullptr;
	}
}

Node* Statement::GetNode(std::string name){
	if(has_nodes) {
		for( auto const & n : nodes ){
			if(n->get_name().compare(name) == 0){
				return n;
			}
		}
	}
	return nullptr;

}

bool Statement::GetChildById(int search_id, Statement*& child){
	if( this->id == search_id){
		child = this;
		return true;
	}
	else if(has_children) {
		for( auto const & c : children ){
			if( c->GetChildById(search_id, child) ){
				return true;
			}
		}
	}
	// child = nullptr;
	return false;
}

std::string Statement::ExportCircuitStatement(std::string indentation){
	return indentation + " export circuit as base. circuit: " + name;
}


bool Statement::ParseParameters(std::string& statementCode, bool allowUnvaluedParameters){
	bool completedStatement = false;
	#ifdef PARSING_VERBOSE
		log_io->ReportPlain2Log( "parsing parameters: statementCode= '" + statementCode + "'" );
	#endif
	std::vector<std::string> paramTokens; //p1=v1->{p1, v1}
	std::vector<std::string> statementTokens; //...p1=v1...->{..., p1=v1...}
	std::string paramStatement;
	// remove 'parameters' word from the begining
	boost::replace_first( statementCode, "parameters", kEmptyWord);
	// remove excessive empty lines
	RemoveExcessiveEmptyLines(statementCode);
	// apply regex expression
	std::string usedRegex = kParameterRegExNotUnvalued;
	if( allowUnvaluedParameters ){
		usedRegex = kParameterRegEx;
	}
	boost::regex ip_regex( usedRegex );

	boost::sregex_iterator it(statementCode.begin(), statementCode.end(), ip_regex);
	boost::sregex_iterator end;
	for (; it != end; ++it) {
		paramStatement = it->str();
		boost::split(paramTokens, paramStatement, boost::is_any_of(kEqualsWord), boost::token_compress_on);
		if( paramTokens.front().compare("")!=0
			&& paramTokens.back().compare("")!=0){
			if( paramTokens.size()==2 ){
				AddParameter( new Parameter( paramTokens.front(), paramTokens.back() ) );
			}else{
				AddParameter( new Parameter( paramTokens.front() ) );
			}
			#ifdef PARSING_VERBOSE
				log_io->ReportPlain2Log( "parameter added: '"
					+ paramTokens.front() + "', '" + paramTokens.back() + "' token size:" + number2String(paramTokens.size()) );
			#endif
		}
	}
	//remove parameters subcode in statementcode
	boost::algorithm::split_regex( statementTokens, statementCode, boost::regex( usedRegex ) ) ;
	statementCode = statementTokens.front();
	boost::algorithm::trim(statementCode);
	completedStatement = true;
	return completedStatement;
}


bool Statement::ParseStatement( std::ifstream* file, std::string& statementCode, Statement& parent,
	Statement& global_scope_parent, std::string& currentReadLine, int& statementCount,
	bool& endOfFile, bool& parsingSpectreCode, const bool permissiveParsingMode ){
	#ifdef PARSING_VERBOSE
		log_io->ReportPlain2Log( "parsing statement: '" + statementCode + "'" );
	#endif
	std::string readLine;
	bool completedStatement = false;
	try {
		int cond;
		std::vector<std::string> lineTockensSpaces;
		boost::split(lineTockensSpaces, statementCode, boost::is_any_of(kDelimiter), boost::token_compress_on);
		// Parsing spice/other languages statements
		if( TestIsNonSpectreLanguage(statementCode) ){
			parsingSpectreCode = TestIsSpectreLanguage(statementCode);
		}
		CircuitStatement* pBelongingCircuit = dynamic_cast<CircuitStatement*>(belonging_circuit);
		if( !parsingSpectreCode ){
			SimpleStatement* st = new SimpleStatement( belonging_circuit, parent.get_log_io(), belonging_scope);
			if( (completedStatement = ( (SimpleStatement*) st)->ParseSimpleStatement(
				global_scope_parent, statementCode, statementCount)) ){
				pBelongingCircuit->AddIncludedSimpleStatement( (SimpleStatement*) st );
				parent.AddStatement(st);
				st->set_parent( &parent );
				#ifdef PARSING_VERBOSE
					log_io->ReportPlain2Log( "Language Ommision: Not an spectre statement. Statement raw: '"
						+ st->get_raw_content() + "'");
				#endif
			}else{
				log_io->ReportError2AllLogs( "\n!!!!!!!!!!! The statement was not parsed as espected."
				"\nThe statement was not added !!!!!!!!!!!!!!" );
			}
		} else { // ends of !parsingSpectreCode
			// Parsing spectre code
			// Parameters
			if( lineTockensSpaces.front().compare(kParametersWord) == 0 ){
				completedStatement = parent.ParseParameters(statementCode, true);
				if(completedStatement){
					#ifdef PARSING_VERBOSE
						log_io->ReportPlain2Log( "The parameters statement was correctly parsed. " );
					#endif
				}
			} else { // Add new statement
				Statement* st;
				bool check_special_syntax_control = false;
				if( lineTockensSpaces.front().compare(kGlobal) == 0){ // parse global nodes
					st = new GlobalStatement( belonging_circuit, parent.get_log_io(), belonging_scope );
					if( (completedStatement = ( (GlobalStatement*) st )->ParseGlobalStatement(
						global_scope_parent, lineTockensSpaces, statementCode, statementCount)) ){
						pBelongingCircuit->AddIncludedGlobalStatement( (GlobalStatement*)st );
					}
				} else if( lineTockensSpaces.front().compare(kModelWord) == 0){ // parse model
					st = new ModelStatement( belonging_circuit, parent.get_log_io(), belonging_scope );
					if( (completedStatement = ( (ModelStatement*) st )->ParseModelStatement(
						global_scope_parent, lineTockensSpaces, statementCode, statementCount)) ){
						pBelongingCircuit->AddIncludedModelStatement( (ModelStatement*) st );
					}
				} else if( boost::contains(statementCode, kAnalogModelSplitWord)){ // parse analog model
					st = new AnalogModelStatement( belonging_circuit, parent.get_log_io(), belonging_scope );
					if( (completedStatement = ( (AnalogModelStatement*) st )->ParseAnalogModelStatement(
						global_scope_parent, lineTockensSpaces, statementCode, statementCount)) ){
						pBelongingCircuit->AddIncludedAnalogModelStatement( (AnalogModelStatement*)st );
					}
				} else if( lineTockensSpaces.front().compare(kSubcircuitWord) == 0 ){ //subcircuit
					st = new SubcircuitStatement( belonging_circuit, parent.get_log_io() );
					if( (completedStatement = ( (SubcircuitStatement*) st )->ParseSubcircuitStatement(
						global_scope_parent, file, lineTockensSpaces, statementCode, currentReadLine,
						statementCount, endOfFile, parsingSpectreCode, permissiveParsingMode )) ){
						pBelongingCircuit->AddIncludedSubcircuitStatement( (SubcircuitStatement*) st );
					}
				} else if( lineTockensSpaces.front().compare(kInLineSubcircuitWord) == 0 &&
					lineTockensSpaces.at(1).compare(kSubcircuitWord) == 0){ //inline subcircuit
					st = new SubcircuitStatement( belonging_circuit, parent.get_log_io() );
					if( (completedStatement = ( (SubcircuitStatement*) st )->ParseInlineSubcircuitStatement(
						global_scope_parent, file, lineTockensSpaces, statementCode, currentReadLine,
						statementCount, endOfFile, parsingSpectreCode, permissiveParsingMode )) ){
						pBelongingCircuit->AddIncludedSubcircuitStatement( (SubcircuitStatement*) st );
					}
				} else if( lineTockensSpaces.front().compare(kUserDefinedFunctionWord) == 0){ // user defined function
					st = new UserDefinedFunctionStatement( belonging_circuit, parent.get_log_io(), belonging_scope );
					if( (completedStatement = ( (UserDefinedFunctionStatement*) st)->ParseUserDefinedFunctionStatement(
						global_scope_parent, file, lineTockensSpaces, statementCode,
						currentReadLine, statementCount, endOfFile, parsingSpectreCode )) ) {
						pBelongingCircuit->AddIncludedUserDefinedFunctionStatement( (UserDefinedFunctionStatement*) st );
					}
				} else if( lineTockensSpaces.front().compare(kLibraryWord) == 0){ // library
					st = new LibraryStatement( belonging_circuit, parent.get_log_io(), belonging_scope);
					if( (completedStatement = ( (LibraryStatement*) st)->ParseLibraryStatement(
						global_scope_parent, file, lineTockensSpaces, statementCode,
						currentReadLine, statementCount, endOfFile, parsingSpectreCode, permissiveParsingMode)) ) {
						pBelongingCircuit->AddIncludedLibraryStatement( (LibraryStatement*) st );
					}
				} else if( lineTockensSpaces.front().compare(kSectionWord) == 0){ // section
					st = new SectionStatement( belonging_circuit, parent.get_log_io(), belonging_scope );
					if( (completedStatement = ( (SectionStatement*) st)->ParseSectionStatement(
						global_scope_parent, file, lineTockensSpaces, statementCode,
						currentReadLine, statementCount, endOfFile, parsingSpectreCode, permissiveParsingMode)) ) {
						pBelongingCircuit->AddIncludedSectionStatement( (SectionStatement*) st );
					}
				} else if( (cond=TestConditionalStatement(lineTockensSpaces.front())) >= 0 ){ // Conditional
					st = new ConditionalStatement( belonging_circuit, parent.get_log_io(), belonging_scope );
					if( (completedStatement = ( (ConditionalStatement*) st)->ParseConditionalStatement(
						global_scope_parent, file, lineTockensSpaces, statementCode,
						parent,	currentReadLine, cond, statementCount, endOfFile, parsingSpectreCode, permissiveParsingMode)) ) {
						pBelongingCircuit->AddIncludedConditionalStatement( (ConditionalStatement*) st );
					}
				} else if( (cond=TestIncludeStatement(lineTockensSpaces.front())) >= 0 ){ // Include
					st = new IncludeStatement( belonging_circuit, parent.get_log_io(), belonging_scope );
					if( (completedStatement = ( (IncludeStatement*) st)->ParseIncludeStatement(
						global_scope_parent, lineTockensSpaces, statementCode, cond, statementCount) ) ) {
						pBelongingCircuit->AddReferencedIncludeStatement( (IncludeStatement*) st );
					}
				// Paramtest
				} else if(lineTockensSpaces.size() > 1 && lineTockensSpaces.at(1).compare(kParamTestWord) == 0){
					st = new ParamTestStatement( belonging_circuit, parent.get_log_io(), belonging_scope );
					if( (completedStatement = ( (ParamTestStatement*) st)->ParseParamTestStatement(
						global_scope_parent, lineTockensSpaces, statementCode, statementCount) )){
						pBelongingCircuit->AddIncludedParamTestStatement( (ParamTestStatement*) st );
					}
				// Control Statement
				} else if( TestControlStatement( statementCode, check_special_syntax_control ) ){
					st = new ControlStatement( belonging_circuit, parent.get_log_io(),
						belonging_scope, check_special_syntax_control);
					if( (completedStatement = ( (ControlStatement*) st)->ParseControlStatement(
						global_scope_parent, file, lineTockensSpaces, statementCode,
						currentReadLine, statementCount, endOfFile, parsingSpectreCode, permissiveParsingMode)) ) {
						pBelongingCircuit->AddIncludedControlStatement( (ControlStatement*) st );
					}
				// Analysis
				} else if( TestAnalysisStatement( statementCode ) ){
					st = new AnalysisStatement( belonging_circuit, parent.get_log_io(), belonging_scope);
					if( (completedStatement = ( (AnalysisStatement*) st)->ParseAnalysisStatement(
						global_scope_parent, file, lineTockensSpaces, statementCode,
						currentReadLine, statementCount, endOfFile, parsingSpectreCode, permissiveParsingMode)) ) {
						pBelongingCircuit->AddIncludedAnalysisStatement( (AnalysisStatement*) st );
					}
				// Transistor Statement
				} else if( TestTransistorStatement( statementCode ) ){
					st = new TransistorStatement( belonging_circuit, parent.get_log_io(), belonging_scope);
					if( (completedStatement = ( (TransistorStatement*) st)->ParseTransistorStatement(
						global_scope_parent, lineTockensSpaces, statementCode, statementCount)) ){
						pBelongingCircuit->AddIncludedTransistorStatement( (TransistorStatement*) st );
					}
				// Simple statements (keywords)
				} else if(lineTockensSpaces.size() < 2 || TestReservedStatement(lineTockensSpaces.front()) ){
					st = new SimpleStatement( belonging_circuit, parent.get_log_io(), belonging_scope);
					if( (completedStatement = ( (SimpleStatement*) st)->ParseSimpleStatement(
						global_scope_parent, statementCode, statementCount)) ){
						pBelongingCircuit->AddIncludedSimpleStatement( (SimpleStatement*) st );
					}
				// Instance Statements
				} else{
					st = new InstanceStatement( belonging_circuit, parent.get_log_io(), belonging_scope );
					if( (completedStatement = ( (InstanceStatement*) st)->ParseInstanceStatement(
						global_scope_parent, lineTockensSpaces, statementCode, statementCount)) ){
						pBelongingCircuit->AddIncludedInstanceStatement( (InstanceStatement*) st );
					}
				}
				if(completedStatement){
					//Conditional statements have already been added
					if(st->get_statement_type()!= kConditionalStatement){
						parent.AddStatement(st);
						st->set_parent( &parent );
					}
					#ifdef PARSING_VERBOSE
						log_io->ReportPlain2Log( "The statement was correctly parsed and added. " );
					#endif
				}else{
					log_io->ReportError2AllLogs( "\n!!!!!!!!!!! The statement was not parsed as espected."
					"\nThe statement was not added !!!!!!!!!!!!!!" );
				}
			}
		}
	} catch (std::exception const& ex) {
		log_io->ReportError2AllLogs( "An exception occurred while parsing '"
			+ parent.get_master_name() + "' children:\n" + ex.what() );
	}
	return completedStatement;
}

// Process comments, empty lines,
// Binds splitted lines
// Returns true if the parser needs
// to read the following line
bool Statement::ProcessLine(std::string& statementCode,
	std::string& currentReadLine, Statement& parent, int& statementCount,
	bool& parsingSpectreCode ){
	bool continueReading = false;
 	//skip empty lines

	// remove spaces at the beginning of a sentence
	// also double , triple... spaces
	boost::algorithm::trim(statementCode);
	boost::replace_all( statementCode, kTab, kEmptyWord);

	// Remove invalid chars at the end of file
	// Windows EOL
	if( currentReadLine.compare( kWindowsEOL )==0 ){
		#ifdef PARSING_VERBOSE
			log_io->ReportPlain2Log( "Windows EOL alone char");
		#endif
		currentReadLine = "";
	}
	// Check currentReadLine length
 	if(!currentReadLine.length()) {
		continueReading = true;
	}
	// process comments
	else if ( boost::starts_with(currentReadLine, kCommentWord1)
		|| boost::starts_with(currentReadLine, kCommentWord2) ){
		/*std::string aux = 	currentReadLine;
		boost::replace_first( aux, kCommentWord1, kEmptyWord);
		boost::replace_first( aux, kCommentWord2, kEmptyWord);
		CommentStatement* comment = new CommentStatement();
		comment->set_id(statementCount++);
		parent.AddStatement(new CommentStatement(aux));*/
		continueReading = true;
	}
	// remove inline comments
	else if(boost::contains(currentReadLine, kCommentWord1)){
		std::vector<std::string> lineTockensComments;
		boost::split(lineTockensComments, currentReadLine, boost::is_any_of(kCommentWord1), boost::token_compress_on);
		currentReadLine = lineTockensComments.front();
	}
	// add different lines of a single sentence; "\".
	if(boost::ends_with(statementCode, kContinuesNextLineChar) ){
		if (currentReadLine.length() && !(boost::starts_with(currentReadLine, kCommentWord1)
		|| boost::starts_with(currentReadLine, kCommentWord2)) ){
			boost::replace_last( statementCode, kContinuesNextLineChar, kEmptyWord);
			statementCode += kDelimiter + currentReadLine;
		}
		continueReading = true;
	}

	else if(boost::starts_with(currentReadLine, kContinuationPreviousLineChar)){
		boost::replace_first( currentReadLine, kContinuationPreviousLineChar, kEmptyWord);
		statementCode += kDelimiter + currentReadLine;
		continueReading = true;
	}
	// double , triple... empty lines
	RemoveExcessiveEmptyLines(statementCode);
	// "= "
	boost::replace_all( statementCode, kEqualsDelimiterWord, kEqualsWord);
	// " ="
	boost::replace_all( statementCode, kDelimiterEqualsWord, kEqualsWord);
	// Empty lines
	if(statementCode.compare(kEmptyLine) == 0 ){
		statementCode = "";
	}

	return continueReading;
}

void Statement::RemoveExcessiveEmptyLines( std::string& s){
	// using namespace boost::lambda;
	// std::string result;
	//consider consequtive chars equal only if they are both lines.
	// unique_copy(s.begin(), s.end(), back_inserter(result), _1 == '\n' && _2 == '\n');
	// s = result;

	// Using regex_replace
	boost::regex r("\n+");
	s = boost::regex_replace(s,r,"\n");
}

// Experiment Environment methods

bool Statement::TestUnalterableStatement(const std::string master_name){
	return belonging_circuit->TestUnalterableStatement( master_name );
}

bool Statement::TestCanBeInjected(const std::string master_name){
	return belonging_circuit->TestCanBeInjected( master_name );
}

bool Statement::TestCanBeSubstituted(const std::string master_name,
	std::string& new_master_name ){
	return belonging_circuit->TestCanBeSubstituted( master_name, new_master_name );
}

bool Statement::TestReservedStatement(const std::string start){
	return belonging_circuit->TestReservedStatement( start );
}

int Statement::TestIncludeStatement(const std::string start){
	return belonging_circuit->TestIncludeStatement( start );
}

int Statement::TestConditionalStatement(const std::string start){
	return belonging_circuit->TestConditionalStatement( start );
}

bool Statement::TestAnalysisStatement(const std::string statementCode){
	return belonging_circuit->TestAnalysisStatement( statementCode );
}

bool Statement::TestIsAnalysisStatementAdvanced(const std::string analysis_master_name){
	return belonging_circuit->TestIsAnalysisStatementAdvanced( analysis_master_name );
}

bool Statement::TestTransistorStatement(const std::string statementCode){
	return belonging_circuit->TestTransistorStatement( statementCode );
}

bool Statement::TestControlStatement(
	const std::string statementCode, bool& special_syntax_control_statement){
	return belonging_circuit->TestControlStatement(
		statementCode, special_syntax_control_statement );
}

bool Statement::TestIsNonSpectreLanguage( const std::string statementCode ){
	return belonging_circuit->TestIsNonSpectreLanguage( statementCode );
}

bool Statement::TestIsSpectreLanguage( const std::string statementCode ){
	return belonging_circuit->TestIsSpectreLanguage( statementCode );
}

bool Statement::TestIsControlStatementAdvanced(const std::string control_master_name){
	return belonging_circuit->TestIsControlStatementAdvanced( control_master_name );
}

bool Statement::TestIsControlStatementSpecialSintax(const std::string control_master_name){
	return belonging_circuit->TestIsControlStatementSpecialSintax( control_master_name );
}

bool Statement::TestUnalterableNode( const std::string node_name ){
	return belonging_circuit->TestUnalterableNode( node_name );
}

bool Statement::TestIsInstanceOfPrimitive( const std::string m_name ){
	return belonging_circuit->TestIsInstanceOfPrimitive( m_name );
}

void Statement::AddGlobalNode( Node* node ){
	belonging_circuit->AddGlobalNode( node );
}
