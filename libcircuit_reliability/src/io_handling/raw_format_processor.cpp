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
#include <stdexcept>
// Boost
#include <boost/algorithm/string.hpp>
#include "boost/filesystem.hpp"  // includes all needed Boost.Filesystem declarations
// Radiation simulator
#include "raw_format_processor.hpp"
#include "../metric_modeling/magnitude.hpp"
#include "../metric_modeling/ocean_eval_metric.hpp"
// constants
#include "../global_functions_and_constants/global_template_functions.hpp"
#include "../global_functions_and_constants/global_constants.hpp"
#include "../global_functions_and_constants/files_folders_io_constants.hpp"

RAWFormatProcessor::RAWFormatProcessor() {
	this->transient_file_path = kNotDefinedString;
	this->processed_file_path = kNotDefinedString;
	this->montecarlo_eval_file_path = kNotDefinedString;
	this->log_file_path = kNotDefinedString;
	this->format = kMatlab;
	this->export_processed_metrics = false;
	this->is_golden = false;
	this->is_montecarlo_nested_simulation = false;
	this->additional_save = false;
}

RAWFormatProcessor::RAWFormatProcessor( std::vector<Metric*>* metrics,
	const std::string& transient_file_path, const std::string& processed_file_path, const std::string& log_file_path ) {
	this->metrics = metrics;
	this->transient_file_path = transient_file_path;
	this->processed_file_path = processed_file_path;
	this->log_file_path = log_file_path;
	this->montecarlo_eval_file_path = kNotDefinedString;
	this->format = kMatlab;
	this->export_processed_metrics = false;
	this->is_montecarlo_nested_simulation = false;
}

RAWFormatProcessor::~RAWFormatProcessor() {
}

bool RAWFormatProcessor::CheckRequirements(){
	if( log_file_path.compare(kEmptyWord)==0 || transient_file_path.compare(kEmptyWord)==0
		|| transient_file_path.compare(kNotDefinedString)== 0){
		log_io->ReportError2AllLogs( k2Tab + "CheckRequirements: some file path is not defined." );
		return false;
	}
	if( metrics== nullptr || metrics->size() <= 0){
		log_io->ReportError2AllLogs( k2Tab + "CheckRequirements: metrics not defined." );
		return false;
	}
	if( is_montecarlo_nested_simulation && ( montecarlo_eval_file_path.compare(kEmptyWord)==0
		|| montecarlo_eval_file_path.compare(kEmptyWord)==0
		|| montecarlo_eval_file_path.compare(kNotDefinedString)== 0 )){
		log_io->ReportError2AllLogs( k2Tab + "CheckRequirements: montecarlo_eval_file_path is not defined." );
		return false;
	}
	// wait for spectre buffers to end write operation
	if( !boost::filesystem::exists(transient_file_path) ){
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	} // last chance
	if( !boost::filesystem::exists(transient_file_path) ){
		log_io->ReportError2AllLogs( k2Tab + "File " + transient_file_path + " does not exists!!" );
		return false;
	}
	if( !boost::filesystem::exists(log_file_path) ){
		log_io->ReportError2AllLogs( k2Tab + "File " + log_file_path + " does not exists!!" );
		return false;
	}
	// first mag is time
	// if(!metrics->at(magnitudeIndex.front())->is_transient_magnitude()){
	// 	log_io->ReportError2AllLogs( k2Tab + "First mag is not a transient magnitude" );
	// 	return false;
	// }
	return true;
}

bool RAWFormatProcessor::ProcessSpectreOutputs(){
	if( !CheckRequirements() ){
 		log_io->ReportError2AllLogs( k2Tab + "ProcessSpectreResults: abort." );
 		correctly_processed = false;
		return correctly_processed;
	}
	bool processSpectreLog = false;
	for( auto const& m: *metrics){
		if( !m->is_transient_magnitude() ){
			processSpectreLog = true;
			break;
		}
	}
	correctly_processed = true;
	if( processSpectreLog ){
		if( is_montecarlo_nested_simulation ){
 			correctly_processed = ProcessMontecarloEvals();
		}else{
 			correctly_processed = ProcessSpectreLogs();
		}
	}
 	correctly_processed = correctly_processed && ProcessPSFASCII();
	if(correctly_processed && export_processed_metrics ){
		if(!ExportMetrics2File()){
			log_io->ReportError2AllLogs( "Exception while exporting results" );
			correctly_processed = false;
		}
	}
	return correctly_processed;
}

bool RAWFormatProcessor::ProcessMontecarloEvals(){
	// one line per mc, (thus one line should be)
	// one column per oceanEvalMetric
	std::string currentReadLine;
	correctly_processed = true;
	std::ifstream file( montecarlo_eval_file_path );
	try {
		if ( file && file.is_open() && file.good() ) {
			#ifdef PSFASCII_VERBOSE
				log_io->ReportPlain2Log( k2Tab + "File " + montecarlo_eval_file_path + " opened" );
			#endif
			// read unique line
			if( getline(file, currentReadLine) ) {
				std::vector<std::string> lineTockensSpaces;
				boost::split(lineTockensSpaces, currentReadLine, boost::is_any_of(kDelimiter), boost::token_compress_on);
				unsigned int tockenCount = 0;
				for( auto const& m: *metrics){
					if( !m->is_transient_magnitude() ){
						if( tockenCount> lineTockensSpaces.size() ){
							log_io->ReportError2AllLogs( "tockenCount> lineTockensSpaces.size()" );
					 		correctly_processed = false;
							throw std::invalid_argument( "tockenCount> lineTockensSpaces.size()" + montecarlo_eval_file_path  );
						} // extra careful
						auto pOceanEvalMag = static_cast<OceanEvalMetric*>( m );
						pOceanEvalMag->set_value( atof( lineTockensSpaces.at(tockenCount++).c_str() ) );
					} // end OceanEvalMetric
				} // end for mag
			} // get line
		// no other line should exist
		} // end is good and open
	}catch (std::exception const& ex) {
		std::string ex_what = ex.what();
		log_io->ReportError2AllLogs( "Exception while parsing the file: ex-> " + ex_what );
 		correctly_processed = false;
	}
	// close file
	file.close();
	return correctly_processed;
}

bool RAWFormatProcessor::ProcessSpectreLogs(){
	std::string currentReadLine;
	bool valueReady = false;
	correctly_processed = true;
	std::ifstream file( log_file_path );
	try {
		if ( file && file.is_open() && file.good() ) {
			#ifdef PSFASCII_VERBOSE
				log_io->ReportPlain2Log( k2Tab + "File " + log_file_path + " opened" );
			#endif
			// File parsing, wait till kOceanEvalExportWord (export oceanEvals)
			while( !valueReady && getline(file, currentReadLine) ) {
				valueReady = kOceanEvalExportWord1.compare(currentReadLine)== 0 ||
					kOceanEvalExportWord2.compare(currentReadLine)== 0;
			}
			if( !valueReady ){
				log_io->ReportError2AllLogs( "Exception, end of file and '" + kOceanEvalExportWord1 + "' was not found in  " + log_file_path );
				correctly_processed = false;
				throw std::invalid_argument( "Exception, end of file and '" + kOceanEvalExportWord1 + "' was not found in  " + log_file_path  );
			}
			std::vector<std::string> lineTockensSpaces;
			for( auto const& m: *metrics){
				if( !m->is_transient_magnitude() ){
					if( getline(file, currentReadLine) ) {
						lineTockensSpaces.clear();
						boost::split(lineTockensSpaces, currentReadLine, boost::is_any_of("="), boost::token_compress_on);
						auto pOceanEvalMag = static_cast<OceanEvalMetric*>( m );
						// already checked in pre-process (ahdl)
						// if( is_golden ){
						// 	std::string auxString = lineTockensSpaces.at(0);
						// 	boost::algorithm::trim(auxString);
						// 	if( auxString.compare( pOceanEvalMag->get_name() )!=0 ){
						// 		log_io->ReportError2AllLogs( "Exception, mag '" + pOceanEvalMag->get_name() + "' is not '" + lineTockensSpaces.at(0) + "'" );
						// 		correctly_processed = false;
						// 		throw std::invalid_argument( "Exception, mag '" + pOceanEvalMag->get_name() + "' is not '" + lineTockensSpaces.at(0) + "'" );
						// 	}
						// } // extra careful
						pOceanEvalMag->set_value( atof( lineTockensSpaces.at(1).c_str() ) );
					}else{
						log_io->ReportError2AllLogs( "Exception, end of file" );
				 		correctly_processed = false;
						throw std::invalid_argument( "Exception, unexpected end of file" );
					}
				} // end OceanEvalMetric
			} // end of metrics
		} // end is good and open
	}catch (std::exception const& ex) {
		std::string ex_what = ex.what();
		log_io->ReportError2AllLogs( "Exception while parsing the file: ex-> " + ex_what );
 		correctly_processed = false;
	}
	// close file
	file.close();
	return correctly_processed;
}

bool RAWFormatProcessor::ProcessPSFASCII(){
	std::string currentReadLine;
	bool valueReady = false;
	std::ifstream file( transient_file_path );
	correctly_processed = true;
	try {
		if ( file && file.is_open() && file.good() ) {
			#ifdef PSFASCII_VERBOSE
				log_io->ReportPlain2Log( k2Tab + "File " + transient_file_path + " opened" );
			#endif
			// File parsing, wait till magnitude values appear
			while( !valueReady && getline(file, currentReadLine) ) {
				valueReady = kPSFAsciiValueWord.compare(currentReadLine)==0;
			}
			if( !valueReady ){
				log_io->ReportError2AllLogs( "Exception, end of file and " + kPSFAsciiValueWord + " was not found in  " + transient_file_path );
				correctly_processed = false;
				throw std::invalid_argument( "Exception, end of file and " + kPSFAsciiValueWord + " was not found in  " + transient_file_path  );
			}
			// pre-cache
			// std::vector<Metric*>::iterator
			auto it_m = metrics->begin();
			auto it_begin = metrics->begin();
			auto it_end = metrics->end();
			std::vector<std::string> lineTockensSpaces;
			while( getline(file, currentReadLine) ) {
				if( currentReadLine.compare(kPSFAsciiEndWord)==0 ){
					break; // end of tran.tran
				}
				lineTockensSpaces.clear();
				boost::split(lineTockensSpaces, currentReadLine, boost::is_any_of(kDelimiter), boost::token_compress_on);
				// if not transient mag (at least time is a magnitude)
				if( !(*it_m)->is_transient_magnitude() ){
					it_m = it_begin;
				}
				// auto pMag = static_cast<Magnitude*>(*it_m);
				// pMag->AddValue( atof( lineTockensSpaces.at(1).c_str() ) );
				// ++it_m;
				(static_cast<Magnitude*>(*(it_m)++))->AddValue( atof( lineTockensSpaces.at(1).c_str() ) );
				if(it_m == it_end){
					it_m = it_begin;
					if( additional_save ){
						// read the //save injector
						getline(file, currentReadLine);
					}
				}
			}
		} // file is open and good
		else{
			log_io->ReportError2AllLogs( k2Tab + "File " + transient_file_path + " NOT OPENED" );
	 		correctly_processed = false;
		}
	}catch (std::exception const& ex) {
		std::string ex_what = ex.what();
		log_io->ReportError2AllLogs( "Exception while parsing the file: ex-> " + ex_what );
 		correctly_processed = false;
	}
	// close file
	file.close();
	// Test
	#ifdef PSFASCII_VERBOSE
		log_io->ReportPlain2Log( kLongDelimiter );
		log_io->ReportPlain2Log( k2Tab + "Metric Lengths:" );
		log_io->ReportPlain2Log( k2Tab + "Transient file: " + transient_file_path );
		for( auto const& m : *metrics ){
			if( m->is_transient_magnitude() ){
				log_io->ReportPlain2Log( k2Tab + m->get_name() + " size: " + number2String((static_cast<Magnitude*>(m))->get_values()->size() ) + kEmptyLine );
			}else{
				log_io->ReportPlain2Log( k2Tab + m->get_name() );
			}
		}
		log_io->ReportPlain2Log( kLongDelimiter );
	#endif
	return correctly_processed;
}

bool RAWFormatProcessor::PrepProcessTransientMetrics( std::vector<Metric*>* unsortedMags,
	std::vector<Metric*>* sortedMags, const std::string& spectreResultTrans, const std::string& spectreLog ){
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
	correctly_processed = true;
	std::string currentReadLine;
	// add time
	sortedMags->push_back( new Magnitude( *(static_cast<Magnitude*>(unsortedMags->front()) ) ));
	sortedMags->back()->set_found_in_results( true );
	std::ifstream file( spectreResultTrans );
	try {
		bool valueReady = false;
		if ( file && file.is_open() && file.good() ) {
			// File parsing, wait till magnitude values appear
			while( !valueReady && getline(file, currentReadLine) ) {
				valueReady = kPSFAsciiTraceWord.compare(currentReadLine)== 0;
			}
			if( !valueReady ){
				log_io->ReportError2AllLogs( "Exception, end of file and " + kPSFAsciiTraceWord + " was not found in  " + spectreResultTrans );
				correctly_processed = false;
				throw std::invalid_argument( "Exception, end of file and " + kPSFAsciiTraceWord + " was not found in  " + spectreResultTrans  );
			}
			std::vector<std::string> lineTockensSpaces;
			while( getline(file, currentReadLine) ) {
				if( currentReadLine.compare(kPSFAsciiValueWord)==0 ){
					break; // start mag values
				}
				boost::split(lineTockensSpaces, currentReadLine, boost::is_any_of(kDelimiter), boost::token_compress_on);
				for( auto const &m : *unsortedMags ){
					if( m->is_transient_magnitude() && m->get_enclosed_name().compare(lineTockensSpaces.at(0))==0 ) {
						auto pMag = static_cast<Magnitude*>( m );
						sortedMags->push_back( new Magnitude(*pMag) );
						sortedMags->back()->set_found_in_results( true );
					}
				}
			} // end of file or start of values
		} // file is open and good
	}catch (std::exception const& ex) {
		std::string ex_what = ex.what();
		log_io->ReportError2AllLogs( "Exception while parsing the file: ex-> " + ex_what );
 		correctly_processed = false;
	}
	// close
	file.close();
	// has oceanEvalMetric?
	bool hasOceanEvalMetrics = false;
	for( auto const& m: *unsortedMags){
		if( !m->is_transient_magnitude() ){
			hasOceanEvalMetrics = true;
			break;
		}
	}
	if(!hasOceanEvalMetrics){
		log_io->ReportPlain2Log( k2Tab + "No oceanEvalMetics" );
		return correctly_processed;
	}
	// ocean eval like metrics
	std::ifstream file2( spectreLog );
	bool valueReady = false;
	try {
		if ( file2 && file2.is_open() && file2.good() ) {
			#ifdef PSFASCII_VERBOSE
				log_io->ReportPlain2Log( k2Tab + "File " + spectreLog + " opened" );
			#endif
			// File parsing, wait till kOceanEvalExportWord (export oceanEvals)
			while( !valueReady && getline(file2, currentReadLine) ) {
				valueReady = kOceanEvalExportWord1.compare(currentReadLine)== 0 ||
					kOceanEvalExportWord2.compare(currentReadLine)== 0;
			}
			if( !valueReady ){
				log_io->ReportError2AllLogs( "Exception, end of file and '" + kOceanEvalExportWord1 + "' was not found in  " + spectreLog );
				correctly_processed = false;
				throw std::invalid_argument( "Exception, end of file and '" + kOceanEvalExportWord1 + "' was not found in  " + spectreLog  );
			}
			std::vector<std::string> lineTockensSpaces;
			for( auto const& m: *unsortedMags){
				if( !m->is_transient_magnitude() ){
					if( getline(file2, currentReadLine) ) {
						lineTockensSpaces.clear();
						boost::split(lineTockensSpaces, currentReadLine, boost::is_any_of("="), boost::token_compress_on);
						auto pOceanEvalMag = static_cast<OceanEvalMetric*>( m );
						std::string auxString = lineTockensSpaces.at(0);
						boost::algorithm::trim(auxString);
						if( auxString.compare( pOceanEvalMag->get_name() )!=0 ){
							log_io->ReportError2AllLogs( "Exception, mag '" + pOceanEvalMag->get_name() + "' is not '" + lineTockensSpaces.at(0) + "'" );
							correctly_processed = false;
							throw std::invalid_argument( "Exception, mag '" + pOceanEvalMag->get_name() + "' is not '" + lineTockensSpaces.at(0) + "'" );
						}
						sortedMags->push_back( new OceanEvalMetric(*pOceanEvalMag) );
						sortedMags->back()->set_found_in_results( true );
					}else{
						log_io->ReportError2AllLogs( "Exception, end of file" );
				 		correctly_processed = false;
						throw std::invalid_argument( "Exception, unexpected end of file" );
					}
				} // end OceanEvalMetric
			} // end of metrics
		} // end is good and open
	}catch (std::exception const& ex) {
		std::string ex_what = ex.what();
		log_io->ReportError2AllLogs( "Exception while parsing the file: ex-> " + ex_what );
 		correctly_processed = false;
	}
	// close file
	file2.close();
	if( sortedMags->size()!=unsortedMags->size() ){
		log_io->ReportError2AllLogs( "Exception: not all metrics where found!!" );
		correctly_processed = false;
	}

	return correctly_processed;
}

bool RAWFormatProcessor::ExportMetrics2File(){
	if( processed_file_path.compare(kEmptyWord)==0 || processed_file_path.compare(kNotDefinedString)== 0){
 		log_io->ReportError2AllLogs( k2Tab + "ExportMetrics2File: processed_file_path not defined." );
		correctly_processed = false;
		return false;
	}
	// std::cout << "[debug]Exporting " << processed_file_path << "\n";
	// Export output
	std::ofstream outputFile;
	outputFile.open(processed_file_path.c_str());
	outputFile.setf(std::ios::scientific);
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
			outputFile << "%Metric Lengths:" << kEmptyLine;
			m_names ="%";
			for( auto const& m : *metrics ){
				if( m->is_transient_magnitude() ){
					outputFile << "%" << mDelimiter << m->get_name() <<  mDelimiter << (static_cast<Magnitude*>(m))->get_values()->size() << kEmptyLine;
				}else{
					outputFile << "%" << mDelimiter << m->get_name() << kEmptyLine;
				}
				m_names += m->get_name() + mDelimiter;
			}
			outputFile << m_names << kEmptyLine;
		}
		break;
		case kGnuPlot:{
			mDelimiter = " ";
			m_names ="#";
			for( auto const& m : *metrics ){
				if( m->is_transient_magnitude() ){
					outputFile << "#" << mDelimiter << m->get_name() <<  mDelimiter << (static_cast<Magnitude*>(m))->get_values()->size() << kEmptyLine;
				}else{
					outputFile << "#" << mDelimiter << m->get_name() << kEmptyLine;
				}
				m_names += m->get_name() + mDelimiter;
			}
			outputFile << m_names << kEmptyLine;
		}
		break;
		case kCSV:{
			mDelimiter = ";";
			outputFile << "#Metric Lengths:" << kEmptyLine;
			m_names ="";
			for( auto const& m : *metrics ){
				if( m->is_transient_magnitude() ){
					outputFile << "#" << mDelimiter << m->get_name() <<  mDelimiter << (static_cast<Magnitude*>(m))->get_values()->size() << kEmptyLine;
				}else{
					outputFile << "#" << mDelimiter << m->get_name() << kEmptyLine;
				}
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
	// fgarcia: metrics report only visible with PSFASCII_VERBOSE
	#ifdef PSFASCII_VERBOSE
 	log_io->ReportPlain2Log( k2Tab + "-> Exporting metrics values to file: " + processed_file_path );
	#endif
	unsigned int numberOfTimeValues = (static_cast<Magnitude*>( metrics->at(0) ))->get_values()->size();
	for (unsigned int i=0; i<numberOfTimeValues; ++i){
		for( auto const& m : *metrics ){
			if( m->get_found_in_results() ){
				outputFile << m->get_value_at(i) << mDelimiter;
				// if( m->is_transient_magnitude() ) {
				// 	auto pMag = static_cast<Magnitude*>( m );
				// 	outputFile << pMag->GetValue(i) << mDelimiter;
				// }else if( ! m->is_transient_magnitude() ){
				// 	auto pOceanEvalMag = static_cast<OceanEvalMetric*>( m );
				// 	outputFile << pOceanEvalMag->get_value() << mDelimiter;
				// }
			}else{
				outputFile << "notFound" << mDelimiter;
			}
		}
		outputFile << "\n";
	}
	outputFile.close();
	#ifdef PSFASCII_VERBOSE
 	log_io->ReportPlain2Log( k2Tab + "-> Metrics exported. FilePath: " + transient_file_path );
	#endif
	return true;
}
