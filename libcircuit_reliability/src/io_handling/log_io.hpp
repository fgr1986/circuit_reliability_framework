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
	void ReportPlain2AllLogs( const std::string& text );

	/**
 	*@details  Writes the text formated in all logs.
 	* @param text <std::string> text to be written.
 	*/
	void ReportInfo2AllLogs( const std::string& text );

	/**
 	* @details Writes and report the error text in all logs
 	* Standard output gets colored in red.
 	* @param text <std::string> text to be written.
 	*/
	void ReportError2AllLogs( const std::string& text );

	/**
 	* @details Writes plain in the log file.
 	* @param text <std::string> text to be written.
 	*/
	void ReportPlain2Log( const std::string& text );

	/**
 	* @details Writes the colored information in screen/log.
 	* @param text <std::string> text to be written.
 	*/
	void ReportUserInformationNeeded( const std::string& text );

	/**
 	* @details Reports a new stage in the program.
 	* @param text <std::string> text to be written.
 	* @param red <bool> text written in red.
 	*/
	void ReportStage( const std::string& text, bool red );

	/// Reports a new node injection
	void ReportNodeInjection( const bool childOfSubcircuit, const std::string& node_name,
		const std::string& statement_name, const std::string& scope_name, const std::string& parent_name,
		const std::string& parent_id, const std::string& modification_counter );

	/// Reports a new replacement of an instance of a circuit.
	void ReportInstanceOfSubcircuitReplacement(
		const bool injectionMethod, const std::string& deepLevel, const std::string& alteredParamName,
		const std::string& statement_name,
		const std::string& modificationCounter, const std::string& oldSubcircuit,
		const std::string& newSubcircuit, const std::string& circuitName, const std::string& folder,
		const std::string& alteredStatementName, const std::string& alteredStatementMasterName,
		const std::string& alteredScopeName );


	/// Reports a substitution.
	void ReportSubstitution( const bool childOfSubcircuit, const std::string& new_master_name,
		const std::string& statement_name, const std::string& scope_name, const std::string& parent_name,
		const std::string& parent_id, const std::string& modification_counter );

	/// Report an ommision of injection
	void ReportStatementNotRadiated( const std::string& statement_name,  const std::string& parent_id,
		const std::string& parent_name, const std::string& scope_name );

	/// Report Experiment conf files
	void ReportConfigurationOptions( const std::string& xml_cadence,
		const std::string& xml_tech,
		const std::string& i_xml_file_experiment,
		const std::string& folder, const std::string& threads_number );

	/// Export Standard Substitution
	bool ExportReadmeStandardSubstitution( const std::string& path2Alteration,
		const std::string& alteredStatementMasterName,
		const std::string& alteredStatementName, const std::string& newStatementMasterName, const std::string& folder );

	/// Report Standard Injection Readme
	bool ExportReadmeStandardInjection( const std::string& path2Alteration,
	const std::string& alteredStatementMasterName,
	const std::string& alteredStatementName, const std::string& alteredStatementScopeName,
	const std::string& alteredNodeName, const std::string& folder  );

	/// Export the readme file of the substitution of an instance of subcircuit
	bool ExportReadmeInstanceOfSubcircuitReplacement( const std::string& path2Alteration,
		const bool injectionMethod, const std::string& deepLevel, const std::string& alteredParamName,
		const std::string& statement_name,
		const std::string& modificationCounter, const std::string& oldSubcircuit,
		const std::string& newSubcircuit, const std::string& circuitName, std::string folder,
		const std::string& alteredStatementName, const std::string& alteredStatementMasterName,
		const std::string& alteredScopeName );

	/// Reports plain to the logfile and standard output
	void ReportPlainStandard( const std::string& text );

	/// Reports in gray to the logfile and standard output
	void ReportGrayStandard( const std::string& text );

	/// Reports in yellow to the logfile and standard output
	void ReportYellowStandard( const std::string& text );

	/// Reports in green to the logfile and standard output
	void ReportGreenStandard( const std::string& text );

	/// Reports in red to the logfile and standard output
	void ReportRedStandard( const std::string& text );

	/// Reports in blue to the logfile and standard output
	void ReportBlueStandard( const std::string& text );

	/// Reports in purple to the logfile and standard output
	void ReportPurpleStandard( const std::string& text );

	/// Reports in cyan to the logfile and standard output
	void ReportCyanStandard( const std::string& text );

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
