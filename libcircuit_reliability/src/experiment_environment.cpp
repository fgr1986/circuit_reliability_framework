 /**
 * @file experiment_environment.cpp
 *
 * @date Created on: Jan 2, 2014
 *
 * @author Author: Fernando García <fgarcia@die.upm.es> <fernando.garca@gmail.com>
 *
 * @section DESCRIPTION
 *
 * Main code of HTMLIO Class (see experiment_environment.hpp)
 *
 */

#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string/regex.hpp>

#include "experiment_environment.hpp"

#include "global_functions_and_constants/global_template_functions.hpp"
#include "global_functions_and_constants/regex_constants.hpp"
#include "global_functions_and_constants/global_constants.hpp"
#include "global_functions_and_constants/statements_constants.hpp"
#include "global_functions_and_constants/include_statement_constants.hpp"
#include "global_functions_and_constants/conditional_statement_constants.hpp"

ExperimentEnvironment::ExperimentEnvironment() {
	this->technology_models_file_path = "not_defined";
  this->technology_considering_sections = false;
	/// technology golden section
	this->technology_golden_section = "not_defined";
	/// technology experiment section
	this->technology_experiment_section = "not_defined";
}

ExperimentEnvironment::~ExperimentEnvironment() {
	#ifdef DESTRUCTORS_VERBOSE
		log_io->ReportPlainStandard( "ExperimentEnvironment destructor. direction:" + number2String(this));
	#endif
	deleteContentsOfVectorOfPointers(analysis_types);
	deleteContentsOfVectorOfPointers(control_statement_types);
	deleteContentsOfVectorOfPointers(global_nodes);
}

void ExperimentEnvironment::TestEnvironment(){
	if( unalterable_statements.size() < 0){
		log_io->ReportError2AllLogs( "[Warning] No unalterable_statements in environment.");
	}
	if( unalterable_nodes.size() < 0){
		log_io->ReportError2AllLogs( "[Warning] No unalterable_nodes in environment.");
	}
	if( primitive_names.size() < 0){
		log_io->ReportError2AllLogs( "[Warning] No primitives in environment.");
	}
	if( reserved_words.size() < 0){
		log_io->ReportError2AllLogs( "[Warning] No reserved_words in environment.");
	}
	if( analysis_types.size() < 0){
		log_io->ReportError2AllLogs( "[Warning] No analysis_types in environment.");
	}
	if( control_statement_types.size() < 0){
		log_io->ReportError2AllLogs( "[Warning] No control_statement_types in environment.");
	}
	if( transistor_statement_types.size() < 0){
		log_io->ReportError2AllLogs( "[Warning] No transistor_statement_types in environment.");
	}
	simulation_mode->get_alteration_mode()->TestAlterationMode();
}



void ExperimentEnvironment::AddUnalterableStatement( std::string unalterableStatement){
	unalterable_statements.push_back(unalterableStatement);
}

void ExperimentEnvironment::AddExcludedCanonicalFolder( std::string excluded_canonical_folder ){
	excluded_canonical_folders.push_back( excluded_canonical_folder );
}

void ExperimentEnvironment::AddPrimitiveName( std::string primitive_name ){
	primitive_names.push_back( primitive_name );
}

void ExperimentEnvironment::AddReservedWord( std::string reserved_word ) {
	this->reserved_words.push_back( reserved_word );
}

void ExperimentEnvironment::AddGlobalNode( Node* node ) {
	bool alreadyInserted = false;
	for( auto const &n : global_nodes ){
		if( n->get_name().compare(node->get_name()) == 0 ){
			alreadyInserted = true;
		}
	}
	if( !alreadyInserted ){
		this->global_nodes.push_back( new Node( node->get_name(),
			!TestUnalterableNode( node->get_name()) ) );
		log_io->ReportPlainStandard( k3Tab
			+ "* New global node found: '" + node->get_name() + "'" );
	}
}

void ExperimentEnvironment::AddAnalysisType( AnalysisStatement* analysis_type ) {
	this->analysis_types.push_back( analysis_type );
}


void ExperimentEnvironment::AddTransistorStatementType( TransistorStatement* transistor_statement_type ) {
	this->transistor_statement_types.push_back( transistor_statement_type );
}

void ExperimentEnvironment::AddControlStatementType( ControlStatement* control_statement_type) {
	this->control_statement_types.push_back( control_statement_type );
}

void ExperimentEnvironment::AddUnalterableNode( std::string unalterable_node ){
	this->unalterable_nodes.push_back( unalterable_node );
}

bool ExperimentEnvironment::TestUnalterableStatement( const std::string& masterName){
	if( unalterable_statements.size()==0 ){
		return false;
	}
	for(auto const & us : unalterable_statements ) {
		if( us.compare( masterName )==0 ) {
			return true;
		}
	}
	return false;
}

bool ExperimentEnvironment::TestCanBeInjected(const std::string& master_name){

	if(! simulation_mode->get_alteration_mode()->get_perform_alteration() ){
		return false;
	}
	if( simulation_mode->get_alteration_mode()->get_inject_all_mode() ){
		return true;
	}
	// The statements that are going to be injected are specified in the experiment_conf.xml file
	for( auto const & as : *(simulation_mode->get_alteration_mode()->get_alterable_statements()) ) {
		if( (as.first).compare( master_name )==0 ) {
			return true;
		}
	}
	return false;
}

bool ExperimentEnvironment::TestCanBeSubstituted(const std::string& master_name,
	std::string& new_master_name ){

	if(! simulation_mode->get_alteration_mode()->get_perform_alteration() ){
		return false;
	}
	for( auto const & as : (*simulation_mode->get_alteration_mode()->get_alterable_statements()) ) {
		if( (as.first).compare( master_name )==0 ) {
			new_master_name = as.second;
			return true;
		}
	}
	return false;
}

bool ExperimentEnvironment::TestIsCanonicalFolderExcluded( const std::string& excluded_canonical_folder ){
	if( excluded_canonical_folders.size() ==0 ){
		return false;
	}
	for( auto const & ecf : excluded_canonical_folders ) {
		// fgarcia debug
		log_io->ReportPlain2Log( kTab + "comparing '" + ecf + "' with '" + excluded_canonical_folder + "'" );
		if( boost::starts_with( excluded_canonical_folder, ecf) ) {
			return true;
		}
	}
	return false;
}

bool ExperimentEnvironment::TestIsInstanceOfPrimitive( const std::string& master_name ){
	if( primitive_names.size() ==0 ){
		return false;
	}
	for( auto const &pn : primitive_names ) {
		if( pn.compare( master_name )== 0 ) {
			return true;
		}
	}
	return false;
}

bool ExperimentEnvironment::TestUnalterableNode(const std::string& node_name){
	if( unalterable_nodes.size() ==0 ){
		return false;
	}
	for( auto const &un : unalterable_nodes ){
		if( un.compare( node_name ) == 0 ){
			return true;
		}
	}
	return false;
}

bool ExperimentEnvironment::TestReservedStatement(const std::string& start){
	if( reserved_words.size() ==0 ){
		return false;
	}
	for( auto const &rw : reserved_words ){
		if( rw.compare(start) == 0 ){
			return true;
		}
	}
	return false;
}

// fgarcia
int ExperimentEnvironment::TestIncludeStatement(const std::string& start){
	for(int i=0; i<kIncludeWordsSize; ++i){
		if(start.compare(kIncludeWords[i]) == 0){
			return i;
		}
		++i;
	}
	return -1;
}

// fgarcia
int ExperimentEnvironment::TestConditionalStatement(const std::string& start){
	for(int i=0; i<kConditionalWordsSize; ++i){
		if(boost::starts_with(start, kConditionalWords[i])){
			return  ((int)i/2);
		}
		++i;
	}
	return -1;
}

void ExperimentEnvironment::UpdatekAnalysisStatementRegEx(){
	// 1 //name (([) nodes(])) Mastername ( params ) bla bla bla { OR
	// 2 //name (([) nodes(])) Mastername ( params ) bla bla bla {fasfasdf} OR
	// 3 //name (([) nodes(])) Mastername ( params ) bla bla bla

	#ifdef VERBOSE_REGEX
		log_io->ReportPlain2Log( kEmptyLine + kEmptyLine + kTab
			+ "kParameterRegEx:'" + kParameterRegEx + "'"
			+ kEmptyLine );
	#endif

	kAnalysisStatementRegEx = "";
	std::string analysis_names = "";
	for( auto const &at : analysis_types ){
		analysis_names += "\\b" + at->get_master_name() + "\\b|";
		//analysis_names += "\\s+" + (*it_a)->get_master_name() +"\\s+|";
	}
	boost::replace_last( analysis_names, "|", kEmptyWord);

	kAnalysisStatementRegEx =
		"(?:\\b" + kSuportedNamesCharsRegEx
		+ "\\b\\s+\\(?(?:\\s*\\b" + kSuportedNamesCharsRegEx + "\\b\\s+)*\\)?\\s*)("
		+ analysis_names + ")(?=(\\s+\\b" + kSuportedNamesCharsRegEx
		+ "\\b\\s*=\\s*" + kSuportedValuesCharsRegEx +")|$)"
		+ "|" // 3
		+ "(?:\\b" + kSuportedNamesCharsRegEx
		+ "\\b\\s+\\(?(?:\\s*\\b" + kSuportedNamesCharsRegEx + "\\b\\s+)*\\)?\\s*)("
		+ analysis_names + ")(?=(\\s*\\{)|$)"; // 1, 2
	#ifdef VERBOSE_REGEX
		log_io->ReportPlain2Log( kEmptyLine + kEmptyLine + kTab
			+ "kAnalysisStatementRegEx:'" + kAnalysisStatementRegEx + "'"
			+ kEmptyLine );
	#endif
}

bool ExperimentEnvironment::TestAnalysisStatement( const std::string& statementCode ){
	boost::regex ip_regex(kAnalysisStatementRegEx);
	boost::smatch what;
	#ifdef VERBOSE_REGEX
		log_io->ReportPlain2Log( "analysis statement regex");
	#endif
	return boost::regex_search(statementCode, what, ip_regex);
}

bool ExperimentEnvironment::TestIsNonSpectreLanguage( const std::string& statementCode ){
	boost::regex ip_regex(kNonSpectreLanguageRegEx);
	boost::smatch what;
	#ifdef VERBOSE_REGEX
		log_io->ReportPlain2Log( "non spectre language regex");
	#endif
	return boost::regex_search(statementCode, what, ip_regex);
}

bool ExperimentEnvironment::TestIsSpectreLanguage( const std::string& statementCode ){
	boost::regex ip_regex(kSpectreLanguageRegEx);
	boost::smatch what;
	#ifdef VERBOSE_REGEX
		log_io->ReportPlain2Log( "spectre language regex");
	#endif
	return boost::regex_search(statementCode, what, ip_regex);
}

bool ExperimentEnvironment::TestIsAnalysisStatementAdvanced(const std::string& analysis_master_name){
	for( auto const & at : analysis_types ){
		if( ( at->get_master_name()).compare(analysis_master_name) == 0 ){
			return at->get_advanced_analysis();
		}
	}
	return false;
}

void ExperimentEnvironment::UpdatekTransistorStatementRegEx(){
	// 1 //name (([) nodes(])) Mastername ( params )

	kTransistorStatementRegEx = "";
	std::string transistor_master_names = "";
	for( auto const & tst : transistor_statement_types ){
		transistor_master_names += "\\b" + tst->get_master_name() + "\\b|";
	}
	boost::replace_last( transistor_master_names, "|", kEmptyWord);

	kTransistorStatementRegEx =
		"(?:" + kSuportedNamesCharsRegEx
		+ "\\s+\\(?(?:\\s*" + kSuportedNamesCharsRegEx + "\\s+)*\\)?\\s*)("
		+ transistor_master_names + ")(?=(\\s*)|$)"; // 1
	#ifdef VERBOSE_REGEX
		log_io->ReportPlain2Log( kEmptyLine + kEmptyLine + kTab
			+ "kTransistorStatementRegEx:'" + kTransistorStatementRegEx + "'"
			+ kEmptyLine );
	#endif
}

bool ExperimentEnvironment::TestTransistorStatement(
		const std::string& statementCode){
	boost::regex ip_regex(kTransistorStatementRegEx);
	boost::smatch what;
	#ifdef VERBOSE_REGEX
		log_io->ReportPlain2Log( "transistor regex");
	#endif
	return boost::regex_search(statementCode, what, ip_regex);
}

void ExperimentEnvironment::UpdatekControlStatementRegEx(){
	// 1 // name (([) nodes(])) Mastername ( params ) bla bla bla { OR
	// 2 // name (([) nodes(])) Mastername ( params ) bla bla bla {fasfasdf} OR
	// 3 // name (([) nodes(])) Mastername ( params ) bla bla bla
	// 4 // save stuff
	// 5 // mismatch
		// {
	// 6 // mismatch {

	std::string control_statement_names = "";
	std::string special_control_statement_names = "";
	for( auto const & cst : control_statement_types ){
		if(cst->get_special_syntax_control_statement()){
			special_control_statement_names += "\\b" + cst->get_master_name() + "\\b|";
		}else{
			control_statement_names += "\\b" + cst->get_master_name() + "\\b|";
		}
	}
	boost::replace_last( special_control_statement_names, "|", kEmptyWord);
	boost::replace_last( control_statement_names, "|", kEmptyWord);

	// kControlStatementRegEx =
	// 	"(?:" + kSuportedNamesCharsRegEx
	// 	+ "\\s+\\(?(?:\\s*" + kSuportedNamesCharsRegEx + "\\s+)*\\)?\\s*)("
	// 	+ control_statement_names + ")(?=\\s+" + kSuportedNamesCharsRegEx
	// 	+ "\\s*=\\s*" + kSuportedValuesCharsRegEx +")" //3
	// 	+ "|" // 3
	// 	+ "(?:" + kSuportedNamesCharsRegEx
	// 	+ "\\s+\\(?(?:\\s*" + kSuportedNamesCharsRegEx + "\\s+)*\\)?\\s*)("
	// 	+ control_statement_names + ")(?=\\s*\\{)"; // 1, 2

	// kControlStatementRegExSpecial = "(\\s+|^)("
	// 	//+ "|" //4
	// 	+ special_control_statement_names
	// 	//+ ")\\s+"; //4
	// 	+ ")(?=\\s+)" //4
	// 	+ "|" //5
	// 	+ "(\\s+|^)(" + special_control_statement_names + ")(?=$)" //5
	// 	+ "|" //6
	// 	+ "(^\\s*)(" + special_control_statement_names + ")(?=\\s*\\{)"; //6


	kControlStatementRegEx =
		"(?:\\b" + kSuportedNamesCharsRegEx
		+ "\\b\\s+\\(?(?:\\s*\\b" + kSuportedNamesCharsRegEx + "\\b\\s+)*\\)?\\s*)("
		+ control_statement_names + ")(?=\\s+\\b" + kSuportedNamesCharsRegEx
		+ "\\b\\s*=\\s*" + kSuportedValuesCharsRegEx +")" //3
		+ "|" // 3
		+ "(?:\\b" + kSuportedNamesCharsRegEx
		+ "\\b\\s+\\(?(?:\\s*\\b" + kSuportedNamesCharsRegEx + "\\b\\s+)*\\)?\\s*)("
		+ control_statement_names + ")(?=\\s*\\{)"; // 1, 2

	kControlStatementRegExSpecial = "(\\s+|^)("
		//+ "|" //4
		+ special_control_statement_names
		//+ ")\\s+"; //4
		+ ")(?=\\s+)" //4
		+ "|" //5
		+ "(\\s+|^)(" + special_control_statement_names + ")(?=$)" //5
		+ "|" //6
		+ "(^\\s*)(" + special_control_statement_names + ")(?=\\s*\\{)"; //6
	#ifdef VERBOSE_REGEX
		log_io->ReportPlain2Log( kEmptyLine + kEmptyLine + kTab
			+ "kControlStatementRegEx:'" + kControlStatementRegEx + "'"
			);
		log_io->ReportPlain2Log( kEmptyLine + kTab
			+ "kControlStatementRegExSpecial:'" + kControlStatementRegExSpecial + "'"
			+ kEmptyLine );
	#endif
}

bool ExperimentEnvironment::TestControlStatement( const std::string& statementCode,
		bool& special_syntax_control_statement){

	boost::regex ip_regex(kControlStatementRegEx);
	boost::regex ip_regex_special(kControlStatementRegExSpecial);
	boost::smatch what;
	#ifdef VERBOSE_REGEX
		log_io->ReportPlain2Log( "ControlStatement regex");
	#endif
	bool standard_syntax_control_statement = boost::regex_search(statementCode, what, ip_regex);
	#ifdef VERBOSE_REGEX
		log_io->ReportPlain2Log( "ControlStatement Special regex");
	#endif
	special_syntax_control_statement = boost::regex_search(statementCode, what, ip_regex_special);
	return standard_syntax_control_statement || special_syntax_control_statement;


	// bool isMatchFound;
	// if ( (isMatchFound =
	// 	boost::regex_search(statementCode, what, ip_regex)) ) {
	// 	special_syntax_control_statement = false;

	// 	log_io->ReportPlainStandard( "1 " );
	// 	log_io->ReportPlainStandard( statementCode );
	// 	//log_io->ReportPlainStandard( kControlStatementRegEx );
	// 	for (unsigned int i=0; i < what.size(); i++)
	// 	{
	// 		log_io->ReportPlainStandard( "WHAT " + i + " '" + what[i] + "'" );
	// 	}

	// }else if((isMatchFound =
	// 	boost::regex_search( statementCode, what, ip_regex_special))){
	// 	special_syntax_control_statement = false;

	// 	log_io->ReportPlainStandard( "2 " );
	// 	log_io->ReportPlainStandard( statementCode );
	// 	//log_io->ReportPlainStandard( kControlStatementRegEx );
	// 	for (unsigned int i=0; i < what.size(); i++)
	// 	{
	// 		log_io->ReportPlainStandard( "WHAT " + i + " '" + what[i] + "'" );
	// 	}
	// }else{
	// 	log_io->ReportPlainStandard( "NOT MATCHED " );
	// }
	// return isMatchFound;

}

bool ExperimentEnvironment::TestIsControlStatementAdvanced(const std::string& control_master_name){
	for( auto const &cst : control_statement_types ){
		if( ( cst->get_master_name()).compare(control_master_name) == 0 ){
			return cst->get_advanced_control_statement();
		}
	}
	return false;
}

bool ExperimentEnvironment::TestIsControlStatementSpecialSintax(const std::string& control_master_name){
	for( auto const &cst : control_statement_types ){
		if( ( cst->get_master_name()).compare(control_master_name) == 0 ){
			return cst->get_special_syntax_control_statement();
		}
	}
	return false;
}
