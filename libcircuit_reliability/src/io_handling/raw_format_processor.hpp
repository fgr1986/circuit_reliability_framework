/*
 * Metric.h
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
#include "../metric_modeling/metric.hpp"

class RAWFormatProcessor {
public:
	RAWFormatProcessor();
	RAWFormatProcessor( std::vector<Metric*>* metrics,
		const std::string& transient_file_path, const std::string& processed_file_path, const std::string& log_file_path  );
	virtual ~RAWFormatProcessor();

	bool ProcessSpectreOutputs();

	bool PrepProcessTransientMetrics( std::vector<Metric*>* unsortedMags,
		std::vector<Metric*>* sortedMags, const std::string& spectreResultTrans, const std::string& spectreLog );

	void set_export_processed_metrics(bool export_processed_metrics){
		this->export_processed_metrics = export_processed_metrics;
	}
	void set_format(int format){ this->format = format; }
	void set_log_file_path( const std::string& log_file_path ) {
		this->log_file_path = log_file_path;}
	void set_transient_file_path( const std::string& transient_file_path ) {
		this->transient_file_path = transient_file_path;}
	void set_processed_file_path( const std::string& processed_file_path ) {
		this->processed_file_path = processed_file_path;}
	void set_montecarlo_eval_file_path( const std::string& montecarlo_eval_file_path ) {
		this->montecarlo_eval_file_path = montecarlo_eval_file_path;}
	void set_metrics( std::vector<Metric*>* metrics ) {
		this->metrics = metrics;}
	void set_is_golden( bool is_golden ){ this->is_golden = is_golden; }
void set_is_montecarlo_nested_simulation( bool is_montecarlo_nested_simulation ){ this->is_montecarlo_nested_simulation = is_montecarlo_nested_simulation; }
	//Log manager
	void set_log_io( LogIO* log_io ){ this->log_io = log_io; }
	bool get_correctly_processed() const{ return correctly_processed ; }

private:
	LogIO* log_io;
	std::string transient_file_path;
	std::string processed_file_path;
	std::string log_file_path;
	std::string montecarlo_eval_file_path;
	std::vector<Metric*>* metrics;
	int format;
	bool correctly_processed;
	bool export_processed_metrics;
	bool is_golden;
	bool is_montecarlo_nested_simulation;

	void RecreateMetricsVector();
	bool ExportMetrics2File();
	bool CheckRequirements();
	bool ProcessPSFASCII();
	bool ProcessSpectreLogs();
	bool ProcessMontecarloEvals();

	// Spectre output files
	std::string kOceanEvalExportWord1 = "Export:";
	std::string kOceanEvalExportWord2 = "Export: ";
	std::string kPSFAsciiTraceWord = "TRACE";
	std::string kPSFAsciiValueWord = "VALUE";
	std::string kPSFAsciiEndWord = "END";
};

#endif /* RAWFORMATPROCESSOR_H */
