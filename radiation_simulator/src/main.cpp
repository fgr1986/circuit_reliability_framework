 /**
 * @file main.cpp
 *
 * @date Created on: February 20, 2013
 *
 * @author Author: Fernando García <fgarcia@die.upm.es> <fernando.garca@gmail.com>
 *
 * @section DESCRIPTION
 *
 * Radiation simulator main class.
 *
 * @arg First argument needed: cadence xml conf file.
 * @arg Second argument needed: technology xml conf file.
 * @arg Third argument needed: experiment xml conf file.
 * @arg Fourth argument Spectre netlist file.
 * @arg Fifth argument needed: experiment_environment folder (will be the outputs).
 * @arg Sixth argument (optional): Permissive mode while parsing the netlist. strict/permissive. Default: strict
 *
 */

// Radiation simulator includes
#include "experiment.hpp"

// Spectre Handler Library
// Constants includes
#include "global_functions_and_constants/global_constants.hpp"
#include "global_functions_and_constants/global_template_functions.hpp" // for GetCurrentDateTime
// Radiation simulator io includes
#include "io_handling/log_io.hpp"

/// Permissive parameter
static const std::string kPermissiveParsingMode = "permissive";
/// Strict parameter
static const std::string kStrictParsingMode = "strict";

static const std::string kAppTitle = "Radiation Simulator";
static const std::string kAppVersion = "v3.1.2";
static const std::string kAppAuthor = "Fernando García, fgarcia@die.upm.es";
static const std::string kAppVersionDate = "19/07/2016";


// // Get current date/time, format is DD-MM-YYYY.HH:mm:ss
// std::string GetCurrentDateTime("%d-%m-%Y.%X") {
// 	time_t	 now = time(0);
// 	struct tm tstruct;
// 	char	 buf[80];
// 	tstruct = *localtime(&now);
// 	// Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
// 	// for more information about date/time format
// 	strftime(buf, sizeof(buf), "%d-%m-%Y.%X", &tstruct);
// 	return buf;
// }

/*
 * @details main function of radiation simulator
 *
 * @param argc Number of arguments <int>
 * @param argv Program arguments <char**>
 */
int main(int argc, char** argv) {
	LogIO log_io;
	Experiment experiment;
	std::string i_netlist_file;
	std::string i_xml_file_cadence;
	std::string i_xml_file_technology;
	std::string i_xml_file_experiment;
	std::string folder;
	bool permissive_parsing_mode;
	bool matlab_script;
	// HTMLIO html_io;

	//info
	permissive_parsing_mode = false;
	log_io.ReportInfo2AllLogs( "" );
	log_io.ReportInfo2AllLogs( k2Tab + kShortDelimiter );
	log_io.ReportInfo2AllLogs( k2Tab + "||         " + kAppTitle  +"           ||" );
	log_io.ReportInfo2AllLogs( k2Tab + "||  " + kAppAuthor  +"  ||" );
	log_io.ReportInfo2AllLogs( k2Tab + kShortDelimiter );
	log_io.ReportInfo2AllLogs( k2Tab + "|| App:     " + kAppVersion + ", " + kAppVersionDate + "          ||" );
	log_io.ReportInfo2AllLogs( k2Tab + kShortDelimiter );
	log_io.ReportInfo2AllLogs( k2Tab + "|| Lib:     " + kSpectreHandlerLibVersion + ", " + kSpectreHandlerLibVersionDate + "          ||" );
	log_io.ReportInfo2AllLogs( k2Tab + kShortDelimiter );
	log_io.ReportInfo2AllLogs( "" );
	log_io.ReportInfo2AllLogs( "" );
	//argument processing
	if (argc < 6 || argc > 7) {
		log_io.ReportPlain2AllLogs( "First argument needed: cadence xml conf file." );
		log_io.ReportPlain2AllLogs( "Second argument needed: technology xml conf file." );
		log_io.ReportPlain2AllLogs( "Third argument needed: experiment xml conf file." );
		log_io.ReportPlain2AllLogs( "Fourth argument Spectre netlist file. " );
		log_io.ReportPlain2AllLogs( "Fifth argument needed: experiment folder (will be the outputs)." );
		log_io.ReportPlain2AllLogs( "Sixth argument (optional): Permissive mode while parsing the netlist. strict/permissive. Default: strict " );
		log_io.ReportError2AllLogs( "End of program." );
		log_io.CloseStreams();
		return 4;
	}
	i_xml_file_cadence = argv[1];
	i_xml_file_technology = argv[2];
	i_xml_file_experiment = argv[3];
	i_netlist_file = argv[4];
	folder = argv[5];
	//format folders
	if( folder.at(folder.size()-1) =='/'){
		folder = folder.substr(0, folder.size()-1);
		log_io.ReportPlain2AllLogs ("Formatting folder/folders: " + folder );
	}
	if(argc == 7){
		std::string parsingMode = argv[6];
		permissive_parsing_mode = kPermissiveParsingMode.compare( parsingMode )==0;
		if(!permissive_parsing_mode){
			if( kStrictParsingMode.compare( parsingMode )!=0 ){
				log_io.ReportPlain2AllLogs( "If specified, sixth argument should be 'permisive'/'strict' model while parsing the netlist. Default: 'strict' " );
				log_io.ReportError2AllLogs( "End of program." );
				log_io.CloseStreams();
			}
		}
	}

	//set environment
	experiment.set_log_io( &log_io );
	experiment.set_permissive_parsing_mode( permissive_parsing_mode );
	experiment.set_experiment_folder( folder );
	experiment.set_i_netlist_file( i_netlist_file );
	experiment.set_i_xml_file_cadence( i_xml_file_cadence );
	experiment.set_i_xml_file_technology( i_xml_file_technology );
	experiment.set_i_xml_file_experiment( i_xml_file_experiment );
	matlab_script = true;
	experiment.set_export_matlab_script( matlab_script );
	//configure experiment environment
	if( !experiment.ConfigureEnvironment() ){
		log_io.ReportError2AllLogs("!!!! Error reading xml or radiation injectors subcircuits. Please, check xml conf files!!!" );
		log_io.CloseStreams();
		return 3;
	}

	// Read main netlist and children netlist
	if( !experiment.ReadMainNetlist( 2 ) ){
		log_io.ReportError2AllLogs( "!!!! Error reading main netlist. Please, check " + i_netlist_file + " and its included netlist." );
		log_io.CloseStreams();
		return 3;
	}

	//Scan sensitive nodes and create radiation scenarios
	if( !experiment.AnalyzeStatementDependency( 3 ) ){
		log_io.ReportError2AllLogs( "!!!! Error creating analizying netlists. Please, check log and error files." );
		log_io.CloseStreams();
		return 3;
	}

	//Scan sensitive nodes and create radiation scenarios
	if( !experiment.CreateRadiatedScenarios( 4 ) ){
		log_io.ReportError2AllLogs( "!!!! Error creating radiation scenarios. Please, check log and error files." );
		log_io.CloseStreams();
		return 3;
	}

	log_io.ReportInfo2AllLogs("Simulations started: " + GetCurrentDateTime("%d-%m-%Y.%X"));

	//Scan sensitive nodes and create radiation scenarios
	if( !experiment.SimulateRadiatedScenarios( 5 ) ){
		log_io.ReportError2AllLogs( "!!!! Error simulating the different scenarios scenarios. Please, check log and error files." );
		log_io.CloseStreams();
		return 3;
	}

	log_io.ReportInfo2AllLogs("Simulations ended: " + GetCurrentDateTime("%d-%m-%Y.%X"));

	log_io.ReportPlain2AllLogs( "End of program, correctly simulated" );
	log_io.CloseStreams();
	return 0;
}
