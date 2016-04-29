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

#ifndef LOG_IO
#define LOG_IO
///LogIO Class
// c++ std required libraries includes
#include <fstream>
#include <string>
 
class LogIO {
public:
	/// Default constructor
	LogIO();
	/// Default destructor
	virtual ~LogIO();

	/**
 	* @details Writes the text without coloring in all logs.
 	* @param text <std::string> text to be written.
 	*/
	void ReportPlain2AllLogs( std::string text );

	/**
 	*@details  Writes the text formated in all logs.
 	* @param text <std::string> text to be written.
 	*/
	void ReportInfo2AllLogs( std::string text );

	/**
 	* @details Writes and report the error text in all logs
 	* Standard output gets colored in red.
 	* @param text <std::string> text to be written.
 	*/
	void ReportError2AllLogs( std::string text );

	/**
 	* @details Writes plain in the log file.
 	* @param text <std::string> text to be written.
 	*/
	void ReportPlain2Log( std::string text );

	/**
 	* @details Writes the colored information in screen/log.
 	* @param text <std::string> text to be written.
 	*/
	void ReportUserInformationNeeded( std::string text );

	/**
 	* @details Reports a new stage in the program.
 	* @param text <std::string> text to be written.
 	* @param red <bool> text written in red.
 	*/
	void ReportStage( std::string text, bool red );

	/// Reports a new node injection
	void ReportNodeInjection( bool childOfSubcircuit, std::string node_name,
		std::string statement_name, std::string scope_name, std::string parent_name,
		std::string parent_id, std::string modification_counter );

	/// Reports a new replacement of an instance of a circuit.
	void ReportInstanceOfSubcircuitReplacement(
		bool injectionMethod, std::string deepLevel, std::string alteredParamName,
		std::string statement_name,
		std::string modificationCounter, std::string oldSubcircuit,
		std::string newSubcircuit, std::string circuitName, std::string folder,
		std::string alteredStatementName, std::string alteredStatementMasterName,
		std::string alteredScopeName );
	
	
	/// Reports a substitution.
	void ReportSubstitution( bool childOfSubcircuit, std::string new_master_name,
		std::string statement_name, std::string scope_name, std::string parent_name,
		std::string parent_id, std::string modification_counter );

	/// Report an ommision of injection
	void ReportStatementNotRadiated( std::string statement_name,  std::string parent_id, 
		std::string parent_name, std::string scope_name );

	/// Report Experiment conf files
	void ReportConfigurationOptions( std::string xml_cadence,
		std::string xml_tech,
		std::string i_xml_file_experiment,
		std::string folder, std::string threads_number );

	/// Export Standard Substitution
	bool ExportReadmeStandardSubstitution(  std::string path2Alteration,
		std::string alteredStatementMasterName,
		std::string alteredStatementName, std::string newStatementMasterName, std::string folder );

	/// Report Standard Injection Readme
	bool ExportReadmeStandardInjection(  std::string path2Alteration,
		std::string master_name, std::string name, 
		std::string scope_name, std::string node_name, std::string folder );
	
	/// Export the readme file of the substitution of an instance of subcircuit
	bool ExportReadmeInstanceOfSubcircuitReplacement( std::string path2Alteration,
		bool injectionMethod, std::string deepLevel, std::string alteredParamName,
		std::string statement_name,
		std::string modificationCounter, std::string oldSubcircuit,
		std::string newSubcircuit, std::string circuitName, std::string folder,
		std::string alteredStatementName, std::string alteredStatementMasterName,
		std::string alteredScopeName );

	/// Reports plain to the logfile and standard output
	void ReportPlainStandard( std::string text );

	/// Reports in gray to the logfile and standard output
	void ReportGrayStandard( std::string text );

	/// Reports in yellow to the logfile and standard output
	void ReportYellowStandard( std::string text );

	/// Reports in green to the logfile and standard output
	void ReportGreenStandard( std::string text );

	/// Reports in red to the logfile and standard output
	void ReportRedStandard( std::string text );

	/// Reports in blue to the logfile and standard output
	void ReportBlueStandard( std::string text );

	/// Reports in purple to the logfile and standard output
	void ReportPurpleStandard( std::string text );

	/// Reports in cyan to the logfile and standard output
	void ReportCyanStandard( std::string text );

	/// Close the standard/log/cerr streams
	void CloseStreams();

	/// Gets Current date
	/// @return current date <std::string>
	std::string GetCurrentDateTime();

	/// Reports in red to the logfile and standard output
	void ReportThread( const std::string threadInfo, const int level );

private:
	/// standard error stream
	std::ofstream* pcerr_stream;
	/// standard log stream
	std::ofstream* pclog_stream;
	/// standard log buffer
	std::streambuf* prev_log_buf;	
	/// standard error buffer
	std::streambuf* prev_cerr_buf;
};

#endif /* LOG_IO */
