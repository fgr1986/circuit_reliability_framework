/**
 * @file log_io.cpp
 *
 * @date Created on: April 20, 2013
 * 
 * @author Author: Fernando García <fgarcia@die.upm.es> <fernando.garca@gmail.com>
 * 
 * @section DESCRIPTION
 * 
 * Logger Class.
 * Exports default and error logs.
 * Handles standard output.
 * 
 */

// c++ std required libraries includes
#include <iostream>
// radiation simulator includes
#include "log_io.hpp"
// constants includes
#include "../global_functions_and_constants/terminal_color_constants.hpp"
#include "../global_functions_and_constants/global_constants.hpp"
#include "../global_functions_and_constants/files_folders_io_constants.hpp"

/// Global VERBOSE control
#define VERBOSE ;

LogIO::LogIO() {	
	// Redirecting standard streams
	pcerr_stream = new std::ofstream( kErrorFile.c_str() );
	prev_cerr_buf = std::cerr.rdbuf( pcerr_stream->rdbuf() );
	#ifdef VERBOSE
		pclog_stream = new std::ofstream( kLogFile.c_str() );
		prev_log_buf = std::clog.rdbuf( pclog_stream->rdbuf() );
	#endif
}

LogIO::~LogIO() {
	#ifdef DESTRUCTORS_VERBOSE
		ReportPlain2Log("LogIO destructor");
	#endif
}

void LogIO::CloseStreams(){
	// Restore the previous streambuf (used in log and error files)
	#ifdef VERBOSE
		std::clog.rdbuf(prev_log_buf);
		std::cerr.rdbuf(prev_cerr_buf);
	#endif
	//close streams
	pclog_stream->close();
	pcerr_stream->close();
	// delete &analysis_stream;
}

void LogIO::ReportStage( std::string text, bool red ){	
	if(red){
		ReportRedStandard( kLongDelimiter );
		ReportRedStandard( kTab + text );
		ReportRedStandard( kLongDelimiter );		
	}else{
		ReportGreenStandard( kLongDelimiter );
		ReportGreenStandard( kTab + text );
		ReportGreenStandard( kLongDelimiter );
	}
}

void LogIO::ReportNodeInjection( bool childOfSubcircuit, std::string alteredNodeName,
		std::string statement_name, std::string alteredStatementScopeName, std::string parent_name,
		std::string parent_id, std::string modificationCounter ){
	if( childOfSubcircuit ){
		ReportCyanStandard( k2Tab + kShortDelimiter + kShortDelimiter);
		ReportCyanStandard( k2Tab + " Injection of child of subcircuit.");
		ReportCyanStandard( k2Tab + kShortDelimiter + kShortDelimiter );
		ReportPlainStandard( k3Tab + "|| Injected Statement: '" + statement_name + "', in '" + parent_name + "'" );
		ReportPlainStandard( k3Tab + "|| Scope: '" + alteredStatementScopeName + "'" );
		ReportPlainStandard( k3Tab + "|| Node: '" + alteredNodeName + "'" );
	}else{
		ReportPurpleStandard( k2Tab + kShortDelimiter + kShortDelimiter);
		ReportPurpleStandard( k2Tab + " Alteration #" + modificationCounter
			+ ", standard injection, NOT a subcircuit child.");
		ReportPurpleStandard( k2Tab + kShortDelimiter + kShortDelimiter);
		ReportPlainStandard( k3Tab + "|| Statement: '" + statement_name + "'" );
		ReportPlainStandard( k3Tab + "|| Parent: parent_id: "
			+ parent_id + ", parent_name: " + parent_name );
		ReportPlainStandard( k3Tab + "|| Scope: '" + alteredStatementScopeName + "'" );
		ReportPlainStandard( k3Tab + "|| Node: '" + alteredNodeName + "'" );
	}
}

void LogIO::ReportSubstitution( bool childOfSubcircuit, std::string new_master_name,
		std::string statement_name, std::string alteredStatementScopeName, std::string parent_name,
		std::string parent_id, std::string modificationCounter ){
	if( childOfSubcircuit ){
		ReportCyanStandard( k2Tab + kShortDelimiter + kShortDelimiter);
		ReportCyanStandard( k2Tab + " Replacement of child of subcircuit.");
		ReportCyanStandard( k2Tab + kShortDelimiter + kShortDelimiter );
		ReportPlainStandard( k3Tab + "|| Replaced Statement: '" + statement_name + "', in '" + parent_name + "'" );
		ReportPlainStandard( k3Tab + "|| Scope: '" + alteredStatementScopeName + "'" );
	}else{
		ReportPurpleStandard( k2Tab + kShortDelimiter + kShortDelimiter);
		ReportPurpleStandard( k2Tab + " Alteration #" + modificationCounter
			+ ", standard injection, NOT a subcircuit child.");
		ReportPurpleStandard( k2Tab + kShortDelimiter + kShortDelimiter);
		ReportPlainStandard( k3Tab + "|| Statement: '" + statement_name + "'" );
		ReportPlainStandard( k3Tab + "|| New master Name: '" + new_master_name + "'" );
		ReportPlainStandard( k3Tab + "|| Parent: parent_id: "
			+ parent_id + ", parent_name: " + parent_name );
		ReportPlainStandard( k3Tab + "|| Scope: '" + alteredStatementScopeName + "'" );
	}
}

void LogIO::ReportStatementNotRadiated( std::string statement_name, std::string parent_id, 
		std::string parent_name, std::string alteredStatementScopeName ){	
	ReportPlain2Log( k3Tab + kShortDelimiter );
	ReportPlain2Log( k3Tab + "|| Statement '" + statement_name+ "'" );
	ReportPlain2Log( k3Tab + "|| parent_id: " + parent_id );
	ReportPlain2Log( k3Tab + "|| parent_name: " + parent_name );
	ReportPlain2Log( k3Tab + "|| Scope: '" + alteredStatementScopeName + "'" );
	ReportPlain2Log( k3Tab + "|| Not injected" );
	ReportPlain2Log( k3Tab + kShortDelimiter );
}

void LogIO::ReportConfigurationOptions( std::string xml_cadence,
		std::string xml_tech, std::string xml_file_experiment,
		std::string folder, std::string threads_number ){	
	ReportPlainStandard( kTab + kShortDelimiter );
	ReportPlainStandard( kTab + "|| ExperimentEnvironment Configuration: ||" );
	ReportPlainStandard( kTab + kShortDelimiter ); 
	ReportPlainStandard( kTab + "-> Date '" + GetCurrentDateTime() );
	ReportPlainStandard( kTab + "-> Cadence xml conf file '" + xml_cadence + "'" );
	ReportPlainStandard( kTab + "-> Technology xml conf file '" + xml_tech + "'" );
	ReportPlainStandard( kTab + "-> Experiment xml conf file '" + xml_file_experiment + "'" );
	ReportPlainStandard( kTab + "-> Experiment folder '" + folder + "'" );
	ReportPlainStandard( kTab + "-> Using " +threads_number + " threads" );
	ReportPlainStandard( kTab + kShortDelimiter ); 
}


// Get current date/time, format is DD-MM-YYYY.HH:mm:ss
std::string LogIO::GetCurrentDateTime() {
	time_t	 now = time(0);
	struct tm tstruct;
	char	 buf[80];
	tstruct = *localtime(&now);
	// Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
	// for more information about date/time format
	strftime(buf, sizeof(buf), "%d-%m-%Y.%X", &tstruct);
	return buf;
}

bool LogIO::ExportReadmeStandardInjection( std::string path2Alteration,
	std::string alteredStatementMasterName,
	std::string alteredStatementName, std::string alteredStatementScopeName,
	std::string alteredNodeName, std::string folder ){
	ReportPlain2Log( k2Tab + "Exporting readme..." );
	bool success = true;
	std::string oReadmeFile;
	oReadmeFile = folder + kFolderSeparator + kReadMeFile;
	std::ofstream outputFile;
	outputFile.open( oReadmeFile.c_str() );

	outputFile << kLongDelimiter << kEmptyLine 
		 << kTab << kSpectreHandlerLibTitle << " " << kSpectreHandlerLibVersion << kEmptyLine << kLongDelimiter << kEmptyLine 
		 << kEmptyLine << "Exported at: " << GetCurrentDateTime() << kEmptyLine
		 << kEmptyLine << "Altered Statement. Name: '" 
		 << alteredStatementName << "', Master Name: '" << alteredStatementMasterName << "'"
		 << kEmptyLine << "At node '" << alteredNodeName << "'."
		 << kEmptyLine << "Scope: " << alteredStatementScopeName << kEmptyLine
		 << "Full path: " << path2Alteration << kEmptyLine;

	outputFile.close();
	return success;
}

bool LogIO::ExportReadmeStandardSubstitution(  std::string path2Alteration,
	std::string alteredStatementMasterName,
	std::string alteredStatementName, std::string newStatementMasterName, std::string folder ){
	ReportPlain2Log( k2Tab + "Exporting readme..." );
	bool success = true;
	std::string oReadmeFile;
	oReadmeFile = folder + kFolderSeparator + kReadMeFile;
	std::ofstream outputFile;
	outputFile.open( oReadmeFile.c_str() );

	outputFile << kLongDelimiter << kEmptyLine 
		 << kTab << kSpectreHandlerLibTitle << " " << kSpectreHandlerLibVersion << kEmptyLine << kLongDelimiter << kEmptyLine 
		 << kEmptyLine << "Exported at: " << GetCurrentDateTime() << kEmptyLine
		 << kEmptyLine << "Altered Statement. Name: '" 
		 << alteredStatementName << "', Master Name: '" << alteredStatementMasterName << "'"
		 << kEmptyLine << "New master_name: '" << newStatementMasterName << "'." << kEmptyLine
		 << "Full path: " << path2Alteration << kEmptyLine;

	outputFile.close();
	return success;
}


void LogIO::ReportInstanceOfSubcircuitReplacement(
		bool injectionMethod, std::string deepLevel, std::string alteredParamName,
		std::string statement_name,
		std::string modificationCounter, std::string oldSubcircuit,
		std::string newSubcircuit, std::string circuitName, std::string folder,
		std::string alteredStatementName, std::string alteredStatementMasterName,
		std::string alteredScopeName ){

	ReportBlueStandard( k3Tab + kShortDelimiter + kShortDelimiter );
	ReportBlueStandard( k3Tab + "Alteration #" + modificationCounter  );
	ReportBlueStandard( k3Tab + kLongDelimiter  );
	ReportBlueStandard( k3Tab + deepLevel  );
	ReportBlueStandard( k3Tab + kLongDelimiter  );
	ReportPlainStandard( k3Tab + "Substitution:"  );
	ReportPlainStandard( k3Tab + "Instance statement: '" + statement_name + "'" );
	ReportPlainStandard( k3Tab + "Belonging circuit: '" + circuitName + "'" );
	ReportPlainStandard( k3Tab + "Instance of (Old subcircuit): '" + oldSubcircuit + "'" );
	ReportPlainStandard( k3Tab + "Instance of (New subcircuit): '" + newSubcircuit + "'"  );		
	ReportPlainStandard( k3Tab + kLongDelimiter  );
	if( injectionMethod){
		ReportPlainStandard( k3Tab  + "Original injection:"  );
		ReportPlainStandard( k3Tab  + "Altered Statement. Name: '"  );
		ReportPlainStandard( k3Tab + alteredStatementName + "', Master Name: '" + alteredStatementMasterName + "'" );
		ReportPlainStandard( k3Tab  + "At node '" + alteredParamName + "'." );
		ReportPlainStandard( k3Tab  + "Scope: " + alteredScopeName );
	}else{
		ReportPlainStandard( k3Tab  + "Original substitution:" );
		ReportPlainStandard( k3Tab  + "Altered Statement. Name: '"
		+ alteredStatementName + "', Old Master Name: '" + alteredStatementMasterName + "'");
		ReportPlainStandard( k3Tab  + "New Master Name '" + alteredParamName + "'.");
		ReportPlainStandard( k3Tab  + "Scope: " + alteredScopeName );
	}
}

bool LogIO::ExportReadmeInstanceOfSubcircuitReplacement(  std::string path2Alteration,
		bool injectionMethod, std::string deepLevel, std::string alteredParamName,
		std::string statement_name,
		std::string modificationCounter, std::string oldSubcircuit,
		std::string newSubcircuit, std::string circuitName, std::string folder,
		std::string alteredStatementName, std::string alteredStatementMasterName,
		std::string alteredScopeName ){
	bool success = true;
	std::string oReadmeFile;
	oReadmeFile = folder + kFolderSeparator + kReadMeFile;
	std::ofstream outputFile;
	outputFile.open( oReadmeFile.c_str() );

	outputFile << kLongDelimiter << kEmptyLine 
		<< kTab << kSpectreHandlerLibTitle << " " << kSpectreHandlerLibVersion 
		<< kEmptyLine << kTab << "Exported at: " << GetCurrentDateTime() << kEmptyLine
		<< kEmptyLine << kLongDelimiter << kEmptyLine
		<< kEmptyLine << "Alteration #" << modificationCounter << kEmptyLine
		<< kEmptyLine << kLongDelimiter << kEmptyLine
		<< kEmptyLine << deepLevel << kEmptyLine
		<< kEmptyLine << kLongDelimiter << kEmptyLine
		<< kEmptyLine << "Substitution:" << kEmptyLine 
		<< kEmptyLine << "Instance statement: '" << statement_name << "'"
		<< kEmptyLine << "Belonging circuit: '" << circuitName << "'"
		<< kEmptyLine << "Instance of (Old subcircuit): '" << oldSubcircuit << "'"
		<< kEmptyLine << "Instance of (New subcircuit): '" << newSubcircuit << "'"		
		<< kEmptyLine << kLongDelimiter << kEmptyLine;
	if( injectionMethod){
		outputFile
			 << kEmptyLine << "Original injection:" << kEmptyLine 
			 << kEmptyLine << "Altered Statement. Name: '" 
			 << alteredStatementName << "', Master Name: '" << alteredStatementMasterName << "'"
			 << kEmptyLine << "At node '" << alteredParamName << "'."
			 << kEmptyLine << "Scope: " << alteredScopeName << kEmptyLine;
	}else{
		outputFile
			 << kEmptyLine << "Original substitution:" << kEmptyLine 
			 << kEmptyLine << "Altered Statement. Name: '" 
			 << alteredStatementName << "', Old Master Name: '" << alteredStatementMasterName << "'"
			 << kEmptyLine << "New Master Name '" << alteredParamName << "'."
			 << kEmptyLine << "Scope: " << alteredScopeName << kEmptyLine;	
	}
	outputFile << "Full path: " << path2Alteration << kEmptyLine;
	outputFile.close();
	ReportPlain2Log( k2Tab + "Exported readme " + oReadmeFile );
	return success;
}

void LogIO::ReportPlain2Log( std::string text ){
	#ifdef VERBOSE
		std::clog << kTab << text << std::endl;
	#endif
}

void LogIO::ReportPlainStandard( std::string text ){
	std::cout << kTab << text << std::endl;
	#ifdef VERBOSE
		std::clog << kTab << text << std::endl;
	#endif
}

void LogIO::ReportPlain2AllLogs( std::string text ){
	std::cout << kTab << text << std::endl;
	#ifdef VERBOSE
		std::clog << kTab << text << std::endl;
	#endif
	std::cerr << kTab << text << std::endl;
}

void LogIO::ReportYellowStandard( std::string text ){
	std::cout << kTab << kYellow << text << kEndColor << std::endl;
	#ifdef VERBOSE
		std::clog << kTab << text << std::endl;
	#endif
}

void LogIO::ReportGreenStandard( std::string text ){
	std::cout << kTab << kGreen << text << kEndColor << std::endl;
	#ifdef VERBOSE
		std::clog << kTab << text << std::endl;
	#endif
}

void LogIO::ReportGrayStandard( std::string text ){
	std::cout << kTab << kGray << text << kEndColor << std::endl;
	#ifdef VERBOSE
		std::clog << kTab << text << std::endl;
	#endif
}

void LogIO::ReportRedStandard( std::string text ){
	std::cout << kTab << kRed << text << kEndColor << std::endl;
	#ifdef VERBOSE
		std::clog << kTab << text << std::endl;
	#endif
}

void LogIO::ReportBlueStandard( std::string text ){
	std::cout << kTab << kBlue << text << kEndColor << std::endl;
	#ifdef VERBOSE
		std::clog << kTab << text << std::endl;
	#endif
}

void LogIO::ReportPurpleStandard( std::string text ){
	std::cout << kTab << kPurple << text << kEndColor << std::endl;
	#ifdef VERBOSE
		std::clog << kTab << text << std::endl;
	#endif
}

void LogIO::ReportCyanStandard( std::string text ){
	std::cout << kTab << kCyan << text << kEndColor << std::endl;
	#ifdef VERBOSE
		std::clog << kTab << text << std::endl;
	#endif
}

void LogIO::ReportInfo2AllLogs( std::string text ){
	std::cout << kTab << kGreen << text << kEndColor << std::endl;
	#ifdef VERBOSE
		std::clog << kTab << text << std::endl;
	#endif
	std::cerr << kTab << text << std::endl;
}

void LogIO::ReportUserInformationNeeded( std::string text ){
	std::cout << kTab << kYellow << text << kEndColor << std::endl;
	#ifdef VERBOSE
		std::clog << kTab << text << std::endl;
	#endif
	std::cerr << kTab << text << std::endl;
}

void LogIO::ReportError2AllLogs( std::string text ){
	std::cout << kTab << kRed << text << kEndColor << std::endl;
	#ifdef VERBOSE
		std::clog << kTab << text << std::endl;
	#endif
	std::cerr << kTab << text << std::endl;
}

void LogIO::ReportThread( const std::string threadInfo, const int level ){
	std::string tabulation = level>1 ? k2Tab : kTab;
	tabulation = ( level >= 3 ? kTab : ""  ) + tabulation;
	std::cout << tabulation << kYellow << "[Thread] [Level " << level << "] " << kEndColor << threadInfo << std::endl;
	#ifdef VERBOSE
		std::clog << kTab << "[Thread] " << threadInfo << std::endl;
	#endif
}