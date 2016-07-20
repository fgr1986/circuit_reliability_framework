/**
 * @file results_processor.cpp
 *
 * @date Created on: April 2, 2016
 *
 * @author Author: Fernando García <fgarcia@die.upm.es> <fernando.garca@gmail.com>
 *
 * @section DESCRIPTION
 *
 * Process partial results, computes the mean and export to a file
 *
 */

// c++ std required libraries includes
// #include <iostream>
// Boost

#include <boost/algorithm/string.hpp>
// radiation simulator includes
#include "results_processor.hpp"
#include "../global_functions_and_constants/global_constants.hpp"
#include "../global_functions_and_constants/global_template_functions.hpp"
#include "../global_functions_and_constants/files_folders_io_constants.hpp"

ResultsProcessor::ResultsProcessor() {
}

ResultsProcessor::~ResultsProcessor() {
	#ifdef DESTRUCTORS_VERBOSE
		std::cout<< "ResultsProcessor destructor\n";
	#endif
}

bool ResultsProcessor::PreProcessResultsFiles( const std::string&& path,
	unsigned int& totalRows, unsigned int& totalColumns ){
	totalRows = 0;
	totalColumns = 0;
	bool correctlyProcessed = true;
	std::ifstream file( path );
	try {
		if ( file.is_open() && file.good() ) {
			// header
			bool firstDataLine = false;
			std::string currentReadLine;
			std::vector<std::string> lineTockensSpaces;
			while( !firstDataLine && getline(file, currentReadLine) ) {
				if( !currentReadLine.empty() && !boost::starts_with(currentReadLine, "#") ){
					lineTockensSpaces.clear();
					boost::trim(currentReadLine); // prevent errors comming from final spaces
					boost::split(lineTockensSpaces, currentReadLine,
						boost::is_any_of(kDelimiter), boost::token_compress_on);
					totalColumns = lineTockensSpaces.size();
					firstDataLine = true;
					++totalRows;
				}
			}
			// File parsing, wait till magnitude values appear
			while( getline(file, currentReadLine) ) {
				if( !currentReadLine.empty() && !boost::starts_with(currentReadLine, "#") ){
					++totalRows;
				}
			}
		} // is open and good
	}catch (std::exception const& ex) {
		std::string ex_what = ex.what();
		log_io->ReportError2AllLogs( "Exception while parsing the file: ex-> " + ex_what );
		correctlyProcessed = false;
	}
	file.close();
	// log_io->ReportRedStandard("[debug] PreProcessResultsFiles, totalColumns:" + number2String(totalColumns));
	return correctlyProcessed;
}

bool ResultsProcessor::MeanProcessResultsFiles( const std::map<std::string, std::string>* paths,
	const std::string outputPath, const std::vector<unsigned int>&& columnIndexes ){
	#ifdef RESULTS_POST_PROCESSING_VERBOSE
		 log_io->ReportBlueStandard( "Processing:" + outputPath );
	#endif
	unsigned int totalColumns = 0;
	unsigned int matrixColumns = columnIndexes.size();
	unsigned int totalRows = 0;
	unsigned int totalFiles = paths->size();
	std::string firstPath = (*paths->begin()).first;
	bool result = PreProcessResultsFiles( std::move(firstPath), totalRows, totalColumns );
	if( !result ){
		log_io->ReportError2AllLogs( "[Error] Error preprocessing " + firstPath );
		return false;
	}
	bool* computedColumns = new bool[totalColumns];
	for(unsigned int j=0; j<totalColumns;++j){
		computedColumns[j] = vectorContains( columnIndexes, j );
	}
	// create data structure
	Matrix matrix = new double*[totalRows];
	for(unsigned int i = 0; i < totalRows; ++i){
		matrix[i] = new double[matrixColumns];
		// initialization
		for(unsigned int j=0; j<matrixColumns;++j){
			matrix[i][j] = 0;
		}
	}
	// process all files
	for( auto const &path: *paths ){
		result = result && MeanProcessResultFile( std::move(path.first), matrix, computedColumns );
		if( !result ){
			log_io->ReportError2AllLogs( "[Error] Error processing " + path.first );
			return false;
		}
	}
	// export to file
	std::ofstream gnuplotMapFile;
	gnuplotMapFile.open( outputPath.c_str() );
	gnuplotMapFile.setf(std::ios::scientific);
	std::ifstream exampleFile( firstPath );
	try {
		if ( exampleFile.is_open() && exampleFile.good() ) {
			std::string currentReadLine;
			std::vector<std::string> lineTockensSpaces;
			for( unsigned int row = 0; row<totalRows; ++row ){
				// ommit empty and comments
				while( getline(exampleFile, currentReadLine) ) {
					if( currentReadLine.empty() || boost::starts_with(currentReadLine, "#") ){
						gnuplotMapFile << currentReadLine << "\n";
					}else{
						break; // break while
					}
				}
				lineTockensSpaces.clear();
				boost::split(lineTockensSpaces, currentReadLine, boost::is_any_of(kDelimiter), boost::token_compress_on);
				unsigned int columnCounter = 0;
				unsigned int matrixColumnCounter = 0;
				for( auto const &s : lineTockensSpaces ){
					if( computedColumns[columnCounter++] ){
						gnuplotMapFile << matrix[row][matrixColumnCounter++]/totalFiles;
					}else{
						gnuplotMapFile << s << " ";
					}
					if( columnCounter<totalColumns ){
						gnuplotMapFile << " ";
					}
				}
				gnuplotMapFile << "\n";
			}
		} // is open and good
	}catch (std::exception const& ex) {
		std::string ex_what = ex.what();
		log_io->ReportError2AllLogs( "[PROCESSING ERROR] Exception while parsing the file: ex-> " + ex_what );
		result = false;
	}
	// close files
	gnuplotMapFile.close();
	exampleFile.close();
	// clear memory
	delete[] computedColumns;
	for(unsigned int i = 0; i < totalRows ; ++i){
		delete [] matrix[i];
	}
	return result;
}

bool ResultsProcessor::StatisticProcessResultsFiles( const std::map<std::string, std::string>* paths,
	const std::string outputPath, const std::vector<unsigned int>&& columnIndexes ){
	#ifdef RESULTS_POST_PROCESSING_VERBOSE
		 log_io->ReportBlueStandard( "Processing:" + outputPath );
	#endif
	unsigned int originalColumns = 0;
	unsigned int totalRows = 0;
	unsigned int totalFiles = paths->size();
	std::string firstPath = (*paths->begin()).first;
	bool result = PreProcessResultsFiles( std::move(firstPath), totalRows, originalColumns );
	if( !result ){
		log_io->ReportError2AllLogs( "[Error] Error preprocessing " + firstPath );
		return false;
	}
	bool* computedColumns = new bool[ originalColumns ];
	for(unsigned int j=0; j<originalColumns; ++j){
		computedColumns[j] = vectorContains( columnIndexes, j );
	}
	// originalColumns //  Max Min Mean
	unsigned int matrixColumns = 3*columnIndexes.size();
	// create data structure
	Matrix matrix = new double*[totalRows];
	for(unsigned int i = 0; i < totalRows; ++i){
		matrix[i] = new double[matrixColumns];
		// initialization
		for(unsigned int j=0; j<matrixColumns;){
			matrix[i][j++] = 0; // max
			matrix[i][j++] = std::numeric_limits<double>::max();	// min
			matrix[i][j++] = 0; // mean
		}
	}
	// process all files
	for( auto const &path: *paths ){
		result = result && StatisticProcessResultFile( std::move(path.first), matrix, computedColumns );
		if( !result ){
			log_io->ReportError2AllLogs( "[Error] Error processing " + path.first );
			return false;
		}
	}
	// export to file
	std::ofstream gnuplotMapFile;
	gnuplotMapFile.open( outputPath.c_str() );
	gnuplotMapFile.setf(std::ios::scientific);
	gnuplotMapFile << "# note that each computed column will have max, min and mean values\n";
	std::ifstream exampleFile( firstPath );
	try {
		if ( exampleFile.is_open() && exampleFile.good() ) {
				std::string currentReadLine;
				std::vector<std::string> lineTockensSpaces;
				for( unsigned int row = 0; row<totalRows; ++row ){
					// ommit empty and comments
					while( getline(exampleFile, currentReadLine) ) {
						if( currentReadLine.empty() || boost::starts_with(currentReadLine, "#") ){
							gnuplotMapFile << currentReadLine << "\n";
						}else{
							break; // break while
						}
					}
					lineTockensSpaces.clear();
					boost::split(lineTockensSpaces, currentReadLine, boost::is_any_of(kDelimiter), boost::token_compress_on);
					unsigned int columnCounter = 0;
					unsigned int matrixColumnCounter = 0;
					for( auto const &s : lineTockensSpaces ){
						if( computedColumns[ columnCounter++ ] ){
							// mean max min
							gnuplotMapFile << matrix[row][matrixColumnCounter++] << " ";
							gnuplotMapFile << matrix[row][matrixColumnCounter++] << " ";
							gnuplotMapFile << matrix[row][matrixColumnCounter++]/totalFiles;
						}else{
							gnuplotMapFile << s;
						}
						if( columnCounter<originalColumns ){
							gnuplotMapFile << " ";
						}
					}
					gnuplotMapFile << "\n";
				}
		} // is open and good
	}catch (std::exception const& ex) {
		std::string ex_what = ex.what();
		log_io->ReportError2AllLogs( "[PROCESSING ERROR] Exception while parsing the file: ex-> " + ex_what );
		result = false;
	}
	// close files
	gnuplotMapFile.close();
	exampleFile.close();
	// clear memory
	delete[] computedColumns;
	for( unsigned int i = 0; i < totalRows ; ++i){
		delete [] matrix[i];
	}
	return result;
}

bool ResultsProcessor::StatisticProcessStatisticsFiles( const std::map<std::string, std::string>* paths,
		const std::string outputPath, const std::vector<unsigned int>&& maxFieldsColumnIndexes,
		const std::vector<unsigned int>&& minFieldsColumnIndexes, const std::vector<unsigned int>&& meanFieldsColumnIndexes ){
	#ifdef RESULTS_POST_PROCESSING_VERBOSE
		 log_io->ReportBlueStandard( "StatisticProcessStatisticsFiles Processing:" + outputPath );
	#endif
	unsigned int originalColumns = 0;
	unsigned int totalRows = 0;
	unsigned int totalFiles = paths->size();
	std::string firstPath = (*paths->begin()).first;
	bool result = PreProcessResultsFiles( std::move(firstPath), totalRows, originalColumns );
	if( !result ){
		log_io->ReportError2AllLogs( "[Error] Error preprocessing " + firstPath );
		return false;
	}
	// create data structures
	Matrix matrixMax = new double*[totalRows];
	Matrix matrixMin = new double*[totalRows];
	Matrix matrixMean = new double*[totalRows];
	for(unsigned int i = 0; i < totalRows; ++i){
		matrixMax[i] = new double[maxFieldsColumnIndexes.size()];
		matrixMin[i] = new double[minFieldsColumnIndexes.size()];
		matrixMean[i] = new double[meanFieldsColumnIndexes.size()];
	}
	// to reduce computational load on std::find
	// 1) iterate over columns
	// 2), use of computedColumns
	bool* computedColumnsMax = new bool[originalColumns];
	bool* computedColumnsMin = new bool[originalColumns];
	bool* computedColumnsMean = new bool[originalColumns];
	for( unsigned int j=0; j<originalColumns; ++j ){
		computedColumnsMax[j] = vectorContains( maxFieldsColumnIndexes, j );
		computedColumnsMin[j] = vectorContains( minFieldsColumnIndexes, j );
		computedColumnsMean[j] = vectorContains( meanFieldsColumnIndexes, j );
	}
	// data structure init
	for(unsigned int i = 0; i < totalRows; ++i){
		for(unsigned int j=0; j<maxFieldsColumnIndexes.size(); ++j){
			matrixMax[i][j] = 0;	// for min
		}
		for(unsigned int j=0; j<maxFieldsColumnIndexes.size(); ++j){
			matrixMin[i][j] = std::numeric_limits<double>::max();	// for min
		}
		for(unsigned int j=0; j<meanFieldsColumnIndexes.size(); ++j){
			matrixMean[i][j] = 0;	// for mean
		}
	}
	// process all files
	for( auto const &path: *paths ){
		result = result && StatisticProcessStatisticsFile(
			std::move(path.first), matrixMax, matrixMin, matrixMean,
			computedColumnsMax, computedColumnsMin, computedColumnsMean );
		if( !result ){
			log_io->ReportError2AllLogs( "[Error] Error processing " + path.first );
			return false;
		}
	}
	// export to file
	std::ofstream gnuplotMapFile;
	gnuplotMapFile.open( outputPath.c_str() );
	gnuplotMapFile.setf(std::ios::scientific);
	gnuplotMapFile << "# note that each computed column will have max, min and mean values\n";
	std::ifstream exampleFile( firstPath );
	try {
		if ( exampleFile.is_open() && exampleFile.good() ) {
				std::string currentReadLine;
				std::vector<std::string> lineTockensSpaces;
				for( unsigned int row = 0; row<totalRows; ++row ){
					// ommit empty and comments
					while( getline(exampleFile, currentReadLine) ) {
						if( currentReadLine.empty() || boost::starts_with(currentReadLine, "#") ){
							gnuplotMapFile << currentReadLine << "\n";
						}else{
							break; // break while
						}
					} // end of comment
					lineTockensSpaces.clear();
					boost::split(lineTockensSpaces, currentReadLine, boost::is_any_of(kDelimiter), boost::token_compress_on);
					unsigned int columnCounter = 0;
					unsigned int maxCounter = 0;
					unsigned int minCounter = 0;
					unsigned int meanCounter = 0;
					for( auto const &s : lineTockensSpaces ){
						if( computedColumnsMax[columnCounter] ){
							gnuplotMapFile << matrixMax[row][maxCounter++];
						}else if( computedColumnsMean[columnCounter] ){
							gnuplotMapFile << matrixMin[row][minCounter++];
						}else if( computedColumnsMean[columnCounter] ){
							gnuplotMapFile << matrixMean[row][meanCounter++]/totalFiles;
						}else{
							gnuplotMapFile << s;
						}
						++columnCounter;
						if( columnCounter<originalColumns ){
							gnuplotMapFile << " ";
						}else{
							log_io->ReportRedStandard("[debug] columnCounter:" + number2String(columnCounter) + ", originalColumns:" + number2String(originalColumns));
						}
					} // end tockens
					gnuplotMapFile << "\n";
				}
		} // is open and good
	}catch (std::exception const& ex) {
		std::string ex_what = ex.what();
		log_io->ReportError2AllLogs( "[PROCESSING ERROR] Exception while parsing the file: ex-> " + ex_what );
		result = false;
	}
	// close files
	gnuplotMapFile.close();
	exampleFile.close();
	// clear memory
	delete[] computedColumnsMean;
	delete[] computedColumnsMax;
	delete[] computedColumnsMin;
	for( unsigned int i = 0; i < totalRows ; ++i ){
		delete [] matrixMax[i];
		delete [] matrixMin[i];
		delete [] matrixMean[i];
	}
	return result;
}

bool ResultsProcessor::StatisticProcessStatisticsFile( const std::string&& path,
	Matrix& matrixMax, Matrix& matrixMin, Matrix& matrixMean,
	bool* computedColumnsMax, bool* computedColumnsMin, bool* computedColumnsMean ){
	#ifdef RESULTS_POST_PROCESSING_VERBOSE
		 log_io->ReportBlueStandard( "StatisticProcessStatisticsFile Processing:" + path );
	#endif
	unsigned int currentRow = 0;
	bool correctlyExported = true;
	std::ifstream file( path );
	try {
		if ( file.is_open() && file.good() ) {
			// header
			std::string currentReadLine;
			std::vector<std::string> lineTockensSpaces;
			while( getline(file, currentReadLine) ) {
				if( !currentReadLine.empty() && !boost::starts_with(currentReadLine, "#") ){
					lineTockensSpaces.clear();
					boost::split(lineTockensSpaces, currentReadLine, boost::is_any_of(kDelimiter), boost::token_compress_on);
					unsigned int currentColumn = 0;
					unsigned int maxCounter = 0;
					unsigned int minCounter = 0;
					unsigned int meanCounter = 0;
					for( auto const &st : lineTockensSpaces ){
						double parsedValue = std::atof( st.c_str() );
						if( computedColumnsMax[currentColumn] && matrixMax[currentRow][maxCounter]<parsedValue ){
							matrixMax[currentRow][maxCounter++] = parsedValue;
						}else if( computedColumnsMin[currentColumn] && matrixMin[currentRow][minCounter]>parsedValue ){
							matrixMin[currentRow][minCounter++] = parsedValue;
						}else if( computedColumnsMean[currentColumn] ){
							matrixMean[currentRow][meanCounter++] += parsedValue;
						}
						// else do nothing
						++currentColumn;
					} // ends token for
					++currentRow;
				} // not a comment or empty
			} // ends while getline
		} // ends is_open and good
	}catch (std::exception const& ex) {
		std::string ex_what = ex.what();
		log_io->ReportError2AllLogs( "[PROCESSING ERROR] Exception while parsing the file: ex-> " + ex_what );
		correctlyExported = false;
	}
	file.close();
	return correctlyExported;
}

bool ResultsProcessor::MeanProcessResultFile( const std::string&& path,
	Matrix& matrix, bool* computedColumns ){
	#ifdef RESULTS_POST_PROCESSING_VERBOSE
		 log_io->ReportBlueStandard( "Processing:" + path );
	#endif
	unsigned int currentRow = 0;
	bool correctlyExported = true;
	std::ifstream file( path );
	try {
		if ( file.is_open() && file.good() ) {
			// header
			std::string currentReadLine;
			std::vector<std::string> lineTockensSpaces;
			while( getline(file, currentReadLine) ) {
				if( !currentReadLine.empty() && !boost::starts_with(currentReadLine, "#") ){
					lineTockensSpaces.clear();
					boost::split(lineTockensSpaces, currentReadLine,
						boost::is_any_of(kDelimiter), boost::token_compress_on);
					unsigned int currentColumn = 0;
					unsigned int matrixColumnCounter = 0;
					for( auto const &st : lineTockensSpaces ){
						if( computedColumns[currentColumn++] ){
							matrix[currentRow][matrixColumnCounter++] += std::atof( st.c_str() );
						}
						// else do nothing
					} // ends token for
					++currentRow;
				} // not a comment or empty
			} // ends while getline
		} // ends is_open and good
	}catch (std::exception const& ex) {
		std::string ex_what = ex.what();
		log_io->ReportError2AllLogs( "[PROCESSING ERROR] Exception while parsing the file: ex-> " + ex_what );
		correctlyExported = false;
	}
	file.close();
	return correctlyExported;
}

bool ResultsProcessor::StatisticProcessResultFile( const std::string&& path,
	Matrix& matrix, bool* computedColumns ){
	#ifdef RESULTS_POST_PROCESSING_VERBOSE
		 log_io->ReportBlueStandard( "Processing:" + path );
	#endif
	unsigned int currentRow = 0;
	bool correctlyExported = true;
	std::ifstream file( path );
	try {
		if ( file.is_open() && file.good() ) {
			// header
			std::string currentReadLine;
			std::vector<std::string> lineTockensSpaces;
			while( getline(file, currentReadLine) ) {
				if( !currentReadLine.empty() && !boost::starts_with(currentReadLine, "#") ){
					std::vector<std::string> lineTockensSpaces;
					boost::split(lineTockensSpaces, currentReadLine,
						boost::is_any_of(kDelimiter), boost::token_compress_on);
					unsigned int currentColumn = 0;
					unsigned int matrixColumnCounter = 0;
					for( auto const &st : lineTockensSpaces ){
						if( computedColumns[currentColumn++] ){
							// mean max min
							double value = std::atof( st.c_str() );
							if ( matrix[currentRow][matrixColumnCounter] < value ){
								matrix[currentRow][matrixColumnCounter] = value;
							}
							if ( matrix[currentRow][matrixColumnCounter+1] > value ){
								matrix[currentRow][matrixColumnCounter+1] = value;
							}
							matrix[currentRow][matrixColumnCounter+2] += value;
							// update counter
							matrixColumnCounter +=3 ;
						} // column to be processed
					} // ends tocken for
					++currentRow;
				} // not a comment or empty
			} // ends while getline
		} // ends is_open and good
	}catch (std::exception const& ex) {
		std::string ex_what = ex.what();
		log_io->ReportError2AllLogs( "[PROCESSING ERROR] Exception while parsing the file: ex-> " + ex_what );
		correctlyExported = false;
	}
	file.close();
	return correctlyExported;
}

bool ResultsProcessor::ExportScenariosList(
	const std::string& topFolder, const std::vector<AlteredScenarioSummary*>& scenariosList ){
	// Export output
	bool correctlyExported = true;
	std::ofstream outputFile;
	try{
		outputFile.open( topFolder + "/" + kResultsFolder + "/" + kScenariosSummaryFile.c_str() );
		if (!outputFile){
			log_io->ReportError2AllLogs( kTab + "error writing " + kScenariosSummaryFile);
			return false;
		}
		outputFile << "# folder_path altered_element_path\n";
		for( auto const &s : scenariosList ){
			outputFile << s->get_altered_scenario_folder_path() << " "
				<< s->get_altered_scenario_altered_element_path() << " " << "\n";
		}
	}catch (std::exception const& ex) {
		std::string ex_what = ex.what();
		log_io->ReportError2AllLogs( "Exception while parsing the file: ex-> " + ex_what );
		correctlyExported = false;
	}
	outputFile.close();
	return correctlyExported;
}

bool ResultsProcessor::ExportProfilesList(
	const std::string& topFolder, const std::vector<SimulationParameter*>& parameterList ){
	log_io->ReportPlainStandard( "Exporting profiles list with "
		+ number2String(parameterList.size()) + " parameters" );
	// Export output
	bool correctlyExported = true;
	// params to be sweeped
	std::vector<SimulationParameter*> parameters2sweep;
	// Threads Creation
	for( auto const &p : parameterList ){
		if( p->get_allow_sweep() ){
			// Init parameter sweep (increments etc)
			p->InitSweep();
			parameters2sweep.push_back( p );
		}
	}
	 // go through every possible parameter combination
	unsigned int totalProfiles = 1;
	for( auto const &p : parameters2sweep ){
		totalProfiles = totalProfiles*p->get_sweep_steps_number();
	}

	std::ofstream outputFile;
	try{
		outputFile.open( topFolder + "/" + kResultsFolder + "/" + kProfilesSummaryFile.c_str() );
		if (!outputFile){
			log_io->ReportError2AllLogs( kTab + "error writing " + kProfilesSummaryFile);
			return false;
		}
		outputFile << " # Profile number, Profile id -> Parameter List \n";
		// create list
		std::vector<unsigned int> parameterCountIndexes(parameters2sweep.size(), 0);
		for(unsigned int i=0; i<totalProfiles; ++i){
			// update parameter values
			unsigned int sweepedParamIndex = 0;
			std::string auxIndexes = "P";
			for( auto const &j : parameterCountIndexes ){ auxIndexes += number2String(j); }
			outputFile << i << " | " << auxIndexes << " -> | ";
			for( auto const &p : parameters2sweep ){
				outputFile << p->get_name() << " :" << p->GetSweepValue( parameterCountIndexes.at(sweepedParamIndex)) << " | ";
				sweepedParamIndex++;
			}
			outputFile << "\n";
			UpdateParameterSweepIndexes( parameterCountIndexes, parameters2sweep);
		} // end profiles
	}catch (std::exception const& ex) {
		std::string ex_what = ex.what();
		log_io->ReportError2AllLogs( "Exception while parsing the file: ex-> " + ex_what );
		correctlyExported = false;
	}
	outputFile.close();
	return correctlyExported;
}
