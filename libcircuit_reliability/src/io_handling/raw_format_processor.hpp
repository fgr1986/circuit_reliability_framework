/*
 * magnitude.h
 *
 *  Created on: December 16, 2013
 *      Author: fernando
 */

#ifndef RAWFORMATPROCESSOR_H
#define RAWFORMATPROCESSOR_H

// c++ std libraries
#include <string>
#include <vector>
// Radiation simulator
// radiation io simulator includes
#include "log_io.hpp"
// netlist modeling
#include "../netlist_modeling/magnitude.hpp"

class RAWFormatProcessor {
public:
	RAWFormatProcessor();
	RAWFormatProcessor( std::vector<Magnitude*>* magnitudes,
		const std::string& transient_file_path, const std::string& processed_file_path, const std::string& log_file_path  );
	virtual ~RAWFormatProcessor();

	bool ProcessPSFASCII( );
	// bool ProcessPSFASCIIUnSorted();

	bool PrepProcessTransientMagnitudes( std::vector<Magnitude*>* unsortedMags,
		std::vector<Magnitude*>* sortedMags, const std::string& spectreResultTrans );

	void set_export_processed_magnitudes(bool export_processed_magnitudes){
		this->export_processed_magnitudes = export_processed_magnitudes;
	}
	void set_format(int format){ this->format = format; }
	void set_log_file_path( const std::string& log_file_path ) {
		this->log_file_path = log_file_path;}
	void set_transient_file_path( const std::string& transient_file_path ) {
		this->transient_file_path = transient_file_path;}
	void set_processed_file_path( const std::string& processed_file_path ) {
		this->processed_file_path = processed_file_path;}
	void set_magnitudes( std::vector<Magnitude*>* magnitudes ) {
		this->magnitudes = magnitudes;}
	void set_is_golden( bool is_golden ){ this->is_golden = is_golden; }
	//Log manager
	void set_log_io( LogIO* log_io ){ this->log_io = log_io; }
	bool get_correctly_processed() const{ return correctly_processed ; }

private:
	LogIO* log_io;
	std::string transient_file_path;
	std::string processed_file_path;
	std::string log_file_path;
	std::vector<Magnitude*>* magnitudes;
	int format;
	bool correctly_processed;
	bool export_processed_magnitudes;
	bool is_golden;

	void RecreateMagnitudesVector();
	bool ExportMagnitudes2File();
	bool CheckRequirements();

	// Spectre output files
	std::string kPSFAsciiTraceWord = "TRACE";
	std::string kPSFAsciiValueWord = "VALUE";
	std::string kPSFAsciiEndWord = "END";
};

#endif /* RAWFORMATPROCESSOR_H */
