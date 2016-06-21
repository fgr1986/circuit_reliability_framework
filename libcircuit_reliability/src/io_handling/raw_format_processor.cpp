/*
 * psfascii_processor.cpp
 *
 * Created on: March 11, 2013
 *	 Author: fernando
 */

// c++ std libraries
#include <fstream>
// # for sleep
#include <chrono>
#include <thread>
// Boost
#include <boost/algorithm/string.hpp>
#include "boost/filesystem.hpp"  // includes all needed Boost.Filesystem declarations
// Radiation simulator
#include "raw_format_processor.hpp"
// constants
#include "../global_functions_and_constants/global_template_functions.hpp"
#include "../global_functions_and_constants/global_constants.hpp"
#include "../global_functions_and_constants/files_folders_io_constants.hpp"

RAWFormatProcessor::RAWFormatProcessor() {
	this->transient_file_path = kNotDefinedString;
	this->processed_file_path = kNotDefinedString;
	this->log_file_path = kNotDefinedString;
	this->format = kMatlab;
	this->export_processed_magnitudes = false;
	this->is_golden = false;
}

RAWFormatProcessor::RAWFormatProcessor( std::vector<Magnitude*>* magnitudes,
	const std::string& transient_file_path, const std::string& processed_file_path, const std::string& log_file_path ) {
	this->magnitudes = magnitudes;
	this->transient_file_path = transient_file_path;
	this->processed_file_path = processed_file_path;
	this->log_file_path = log_file_path;
	this->format = kMatlab;
	this->export_processed_magnitudes = false;
}

RAWFormatProcessor::~RAWFormatProcessor() {
}

bool RAWFormatProcessor::CheckRequirements(){
	if( log_file_path.compare(kEmptyWord)==0 || transient_file_path.compare(kEmptyWord)==0
		|| transient_file_path.compare(kNotDefinedString)== 0){
		log_io->ReportError2AllLogs( k2Tab + "ProcessPSFASCII: some file path is not defined." );
		// correctly_processed = false;
		// return correctly_processed;
		return false;
	}
	if( magnitudes== nullptr || magnitudes->size() <= 0){
		log_io->ReportError2AllLogs( k2Tab + "ProcessPSFASCII: magnitudes not defined." );
		return false;
	}
	// wait for spectre buffers to end write operation
	if( !boost::filesystem::exists(transient_file_path) ){
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	} // last chance
	if( !boost::filesystem::exists(transient_file_path) ){
		log_io->ReportError2AllLogs( k2Tab + "File " + transient_file_path + " does not exists!!" );
		std::string auxCommand = "ls -lah " + transient_file_path +  " >> myDebug.log";
		std::string auxCommandFull = "echo '" + auxCommand +  "'; " + auxCommand;
		int auxCommandResult = std::system( auxCommandFull.c_str()  ) ;
		log_io->ReportError2AllLogs( k2Tab + "[debug] Result of '" + auxCommand + "': " + number2String(auxCommandResult) );
		return false;
	}
	if( !boost::filesystem::exists(log_file_path) ){
		log_io->ReportError2AllLogs( k2Tab + "File " + log_file_path + " does not exists!!" );
		return false;
	}
	// first mag is time
	// if(!magnitudes->at(magnitudeIndex.front())->is_transient_magnitude()){
	// 	log_io->ReportError2AllLogs( k2Tab + "First mag is not a transient magnitude" );
	// 	return false;
	// }
	return true;
}

bool RAWFormatProcessor::ProcessPSFASCII( ){
	if( !CheckRequirements() ){
 		log_io->ReportError2AllLogs( k2Tab + "ProcessPSFASCII: abort." );
 		correctly_processed = false;
		return correctly_processed;
	}
	std::string currentReadLine;
	bool valueReady = false;
	bool completedStatement = true;
	try {
		std::ifstream file( transient_file_path );
		#ifdef PSFASCII_VERBOSE
			log_io->ReportPlain2Log( k2Tab + "File " + transient_file_path + " opened" );
		#endif
		if ( file.is_open() ) {
			if ( file.good() ) {
				// File parsing, wait till magnitude values appear
				while( !valueReady && getline(file, currentReadLine) ) {
					valueReady = kPSFAsciiValueWord.compare(currentReadLine)== 0;
				}
				// pre-cache
				// std::vector<Magnitude*>::iterator
				auto it_m = magnitudes->begin();
				auto it_begin = magnitudes->begin();
				auto it_end = magnitudes->end();
				std::vector<std::string> lineTockensSpaces;
				while( getline(file, currentReadLine) ) {
					if( currentReadLine.compare(kPSFAsciiEndWord)==0 ){
						break; // end of tran.tran
					}
					boost::split(lineTockensSpaces, currentReadLine, boost::is_any_of(kDelimiter), boost::token_compress_on);
					if( (*it_m)->is_transient_magnitude() ){
						(*(it_m)++)->AddValue( atof( lineTockensSpaces.at(1).c_str() ) );
					} // ++it_m;
					if(it_m == it_end){
						it_m = it_begin;
					}
				}
			}
		}
		file.close();
	}catch (std::exception const& ex) {
		std::string ex_what = ex.what();
		log_io->ReportError2AllLogs( "Exception while parsing the file: ex-> " + ex_what );
 		correctly_processed = false;
		return correctly_processed;
	}

	// Test
	#ifdef PSFASCII_VERBOSE
		log_io->ReportPlain2Log( kLongDelimiter );
		log_io->ReportPlain2Log( k2Tab + "Magnitude Lengths:" );
		log_io->ReportPlain2Log( k2Tab + "Transient file: " + transient_file_path );
		for( auto const& m : *magnitudes ){
			log_io->ReportPlain2Log( k2Tab + m->get_name() + " " + number2String( m->get_values()->size() ) );
		}
		log_io->ReportPlain2Log( kLongDelimiter );
	#endif

	if(completedStatement){
		#ifdef PSFASCII_VERBOSE
			log_io->ReportPlain2Log( "The file was correctly processed. " );
		#endif
		if( export_processed_magnitudes ){
			if(!ExportMagnitudes2File()){
				log_io->ReportError2AllLogs( "Exception while exporting results" );
				correctly_processed = false;
				return correctly_processed;
			}
		}
	}else{
		log_io->ReportPlain2Log( "The file was not correctly processed. " );
	}
 	correctly_processed = completedStatement;
	return correctly_processed;
}

bool RAWFormatProcessor::PrepProcessTransientMagnitudes( std::vector<Magnitude*>* unsortedMags,
	std::vector<Magnitude*>* sortedMags, const std::string& spectreResultTrans ){
	if( !boost::filesystem::exists(spectreResultTrans) ){
		log_io->ReportError2AllLogs( k2Tab + "File " + spectreResultTrans + " does not exists!!" );
		return false;
	}
	sortedMags->clear();
	sortedMags->reserve( unsortedMags->size() );
	if( unsortedMags->front()->get_name().compare("time")!=0 ){
		log_io->ReportError2AllLogs( k2Tab + "First magnitude in xml file should be 'time'" );
		return false;
	}
	// add time
	sortedMags->push_back( new Magnitude( * unsortedMags->front() ));
	sortedMags->back()->set_found_in_results( true );
	try {
		bool valueReady = false;
		std::string currentReadLine;
		std::ifstream file( spectreResultTrans );
		if ( file.is_open() ) {
			if ( file.good() ) {
				// File parsing, wait till magnitude values appear
				while( !valueReady && getline(file, currentReadLine) ) {
					valueReady = kPSFAsciiTraceWord.compare(currentReadLine)== 0;
				}
				std::vector<std::string> lineTockensSpaces;
				while( getline(file, currentReadLine) ) {
					if( currentReadLine.compare(kPSFAsciiValueWord)==0 ){
						break; // start mag values
					}
					boost::split(lineTockensSpaces, currentReadLine, boost::is_any_of(kDelimiter), boost::token_compress_on);
					for( auto const &m : *unsortedMags ){
						if( m->is_transient_magnitude() && m->get_enclosed_name().compare(lineTockensSpaces.at(0))==0 ) {
							sortedMags->push_back( new Magnitude(*m) );
							sortedMags->back()->set_found_in_results( true );
						}
					}
				} // end of file or start of values
			} // file is good
		} // file is open
	}catch (std::exception const& ex) {
		std::string ex_what = ex.what();
		log_io->ReportError2AllLogs( "Exception while parsing the file: ex-> " + ex_what );
 		correctly_processed = false;
		return correctly_processed;
	}
	// add non transient mags
	for( auto const &m : *unsortedMags ){
		if( !m->is_transient_magnitude() ) {
			sortedMags->push_back( new Magnitude(*m) );
		}
	}
	return true;
}

// deprecated
// bool RAWFormatProcessor::ProcessPSFASCIIUnSorted(){
// 	if( !CheckRequirements() ){
// 		log_io->ReportError2AllLogs( k2Tab + "ProcessPSFASCII: abort." );
// 		correctly_processed = false;
// 		return correctly_processed;
// 	}
// 	std::string currentReadLine;
// 	bool valueReady = false;
// 	bool completedStatement = true;
// 	try {
// 		std::ifstream file( transient_file_path );
// 		#ifdef PSFASCII_VERBOSE
// 			log_io->ReportPlain2Log( k2Tab + "File " + transient_file_path + " opened" );
// 		#endif
// 		if ( file.is_open() ) {
// 			if ( file.good() ) {
// 				// File parsing, wait till magnitude values appear
// 				while( !valueReady && getline(file, currentReadLine) ) {
// 					if( kPSFAsciiValueWord.compare(currentReadLine)== 0 ){
// 						valueReady = true;
// 					}
// 				}
// 				if(!valueReady){
// 					log_io->ReportError2AllLogs( "Exception. kPSFAsciiValueWord not found.");
// 					correctly_processed = false;
// 					return correctly_processed;
// 				}
// 				#ifdef PSFASCII_VERBOSE
// 					log_io->ReportPlain2Log( k2Tab + "VALUE" );
// 				#endif
// 				std::vector<int> magnitudeIndex;
// 				std::vector<bool> magnitudeSaved;
// 				int magCount = 0;
// 				std::vector<std::string> lineTockensSpaces;
// 				bool isFirstMag = true;
// 				bool isFirstLoop = true;
// 				bool isSaved = false;
// 				std::string firstMag = "";
// 				//set magnitudes magnitudeIndex
// 				while( isFirstLoop && getline(file, currentReadLine) ) {
// 					// Test if we have reached the end of file
// 					if( currentReadLine.compare(kPSFAsciiEndWord)==0 ){
// 						break;
// 					}
// 					boost::split(lineTockensSpaces, currentReadLine,
// 						boost::is_any_of(kDelimiter), boost::token_compress_on);
// 					// test end of first loop
// 					if( firstMag.compare(lineTockensSpaces.front())==0 ){
// 						isFirstLoop = false;
// 						if( magnitudeSaved.front() ){
// 							magnitudes->at( magnitudeIndex.front() )->AddValue( atof( lineTockensSpaces.back().c_str() ) );
// 						}
// 					}else{
// 						// catch first magnitude
// 						if( isFirstMag ){
// 							firstMag = lineTockensSpaces.front();
// 							isFirstMag = false;
// 						}
// 						//create magnitudeIndex vector
// 						magCount = 0;
// 						isSaved = false;
// 						for( auto const& m : *magnitudes ){
// 							if( lineTockensSpaces.front().compare(m->get_enclosed_name())==0 ){
// 								isSaved = true;
// 								magnitudeIndex.push_back( magCount );
// 								m->set_found_in_results( true );
// 								m->AddValue( atof( lineTockensSpaces.back().c_str() ) );
// 								break;
// 							}
// 							++magCount;
// 						}
// 						if(!isSaved){
// 							magnitudeIndex.push_back( kNotDefinedInt );
// 						}
// 						magnitudeSaved.push_back( isSaved );
// 					}
// 				}
// 				if ( isFirstLoop ){
// 					log_io->ReportError2AllLogs( "Exception. At least one magnitude has been not found.");
// 					log_io->ReportError2AllLogs( "Magnitudes Found:");
// 					for( auto const& mi : magnitudeIndex ){
// 						log_io->ReportError2AllLogs( magnitudes->at(mi)->get_enclosed_name() );
// 					}
// 					correctly_processed = false;
// 					return correctly_processed;
// 				}else{
// 					for( auto const& m : *magnitudes ){
// 						if( !m->get_found_in_results()
// 							&& ((is_golden && m->get_plottable_in_golden()) || !is_golden) ){
// 							log_io->ReportError2AllLogs( "Exception. At least one magnitude has been not found: " + m->get_name() );
// 							if(is_golden){
// 								log_io->ReportError2AllLogs( "Is golden");
// 							}
// 							if(m->get_plottable_in_golden()){
// 								log_io->ReportError2AllLogs( "m->get_plottable_in_golden()");
// 							}
// 							correctly_processed = false;
// 							return correctly_processed;
// 						}
// 					}
// 				}
//
// 				// the first magnitude of the second loop has been read,
// 				// therefore, magnitudeIndex.begin() + 1;
// 				std::vector<int>::iterator it_i = magnitudeIndex.begin() + 1;
// 				std::vector<bool>::iterator it_b = magnitudeSaved.begin() + 1;
// 				//magnitudeIndex already placed
// 				while( getline(file, currentReadLine) ) {
// 					if( currentReadLine.compare(kPSFAsciiEndWord)==0 ){
// 						break;
// 					}
// 					boost::split(lineTockensSpaces, currentReadLine,
// 						boost::is_any_of(kDelimiter), boost::token_compress_on);
// 					if(*it_b){
// 						magnitudes->at( *it_i )->AddValue( atof( lineTockensSpaces.back().c_str() ) );
// 					}
// 					it_b++;
// 					it_i++;
// 					if(it_b == magnitudeSaved.end()){
// 						it_i = magnitudeIndex.begin();
// 						it_b = magnitudeSaved.begin();
// 					}
// 				}
// 			} // file is good
// 		} // file.is_open
// 		file.close();
// 	}catch (std::exception const& ex) {
// 		std::string ex_what = ex.what();
// 		log_io->ReportError2AllLogs( "Exception while processing the file: ex-> " + ex_what );
// 		correctly_processed = false;
// 		return correctly_processed;
// 	}
// 	// Test
// 	#ifdef PSFASCII_VERBOSE
// 		log_io->ReportPlain2Log( kLongDelimiter );
// 		log_io->ReportPlain2Log( k2Tab + "Magnitude Lengths:" );
// 		log_io->ReportPlain2Log( k2Tab + "Transient file: " + transient_file_path );
// 		for( auto const& m : *magnitudes ){
// 			log_io->ReportPlain2Log( k2Tab + m->get_name() + " " + number2String( m->get_values()->size() ) );
// 			if( !m->get_found_in_results() ){
// 				log_io->ReportError2AllLogs( k2Tab + m->get_name()
// 					+ " has not been found in results PSFASCII_VERBOSE," ) ;
// 				log_io->ReportError2AllLogs( k2Tab + "but it is likely to be found on altered scenarios results." ) ;
// 				log_io->ReportError2AllLogs( k2Tab + "m->set_found_in_results(true) MANUALLY" ) ;
// 				m->set_found_in_results( true );
// 			}
// 		}
// 		log_io->ReportPlain2Log( kLongDelimiter );
// 	#endif
//
// 	if(completedStatement){
// 		#ifdef PSFASCII_VERBOSE
// 			log_io->ReportPlain2Log( "The file was correctly processed. " );
// 		#endif
// 		if( export_processed_magnitudes ){
// 			if(!ExportMagnitudes2File()){
// 				log_io->ReportError2AllLogs( "Exception while exporting results" );
// 				correctly_processed = false;
// 				return correctly_processed;
// 			}
// 		}
// 	}else{
// 		log_io->ReportError2AllLogs( "The file was not correctly processed. " );
// 	}
// 	correctly_processed = completedStatement;
// 	return correctly_processed;
// }

bool RAWFormatProcessor::ExportMagnitudes2File(){
	if( processed_file_path.compare(kEmptyWord)==0
		|| processed_file_path.compare(kNotDefinedString)== 0){
 		log_io->ReportError2AllLogs( k2Tab + "ProcessPSFASCII: processed_file_path not defined." );
		correctly_processed = false;
		return false;
	}
	// Export output
	std::ofstream outputFile;
	outputFile.open(processed_file_path.c_str());
	if (!outputFile){
		log_io->ReportError2AllLogs( kTab + "error writing " + processed_file_path);
		correctly_processed = false;
		return false;
	}
	std::string m_names;
	std::string mDelimiter;
	switch ( format ){
		case kMatlab:{
			mDelimiter = " ";
			outputFile << "%Magnitude Lengths:" << kEmptyLine;
			m_names ="%";
			for( auto const& m : *magnitudes ){
				outputFile << "%" << m->get_name() << mDelimiter << m->get_values()->size() << kEmptyLine;
				m_names += m->get_name() + mDelimiter;
			}
			outputFile << m_names << kEmptyLine;
		}
		break;
		case kGnuPlot:{
			mDelimiter = " ";
			m_names ="#";
			for( auto const& m : *magnitudes ){
				outputFile << "#" << mDelimiter << m->get_name() <<  mDelimiter << m->get_values()->size() << kEmptyLine;
				m_names += m->get_name() + mDelimiter;
			}
			outputFile << m_names << kEmptyLine;
		}
		break;
		case kCSV:{
			mDelimiter = ";";
			outputFile << "#Magnitude Lengths:" << kEmptyLine;
			m_names ="";
			for( auto const& m : *magnitudes ){
				outputFile << "#" << m->get_name() << mDelimiter << m->get_values()->size() << kEmptyLine;
				m_names += m->get_name() + mDelimiter;
			}
			outputFile << m_names << kEmptyLine;
		}
		break;
		default:{
			log_io->ReportError2AllLogs( "The file format (matlab, gnuplot or csv) was not defined. " );
			return false;
		}
		break;
	}
	// fgarcia: magnitudes report only visible with PSFASCII_VERBOSE
	#ifdef PSFASCII_VERBOSE
 	log_io->ReportPlain2Log( k2Tab + "-> Exporting magnitudes values to file: " + processed_file_path );
	#endif
	for (unsigned int i=0; i< magnitudes->at(0)->get_values()->size(); ++i){
		for( auto const& m : *magnitudes ){
			if( m->get_found_in_results() && m->get_values()->size()>0){
				outputFile << m->GetValue(i) << mDelimiter;
			}else{
				outputFile << "notFound" << mDelimiter;
			}
		}
		outputFile << "\n";
	}
	outputFile.close();
	#ifdef PSFASCII_VERBOSE
 	log_io->ReportPlain2Log( k2Tab + "-> Magnitudes exported. FilePath: " + transient_file_path );
	#endif
	return true;
}
