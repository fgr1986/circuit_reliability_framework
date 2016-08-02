/*
 * single_Spectre_simulation.cpp
 *
 * Created on: Feb 03, 2014
 *	Author: fernando
 */

#include <algorithm>

#include "boost/filesystem.hpp" // includes all needed Boost.Filesystem declarations

// Reliability simulator
#include "spectre_simulation.hpp"
#include "../../metric_modeling/metric_errors.hpp"
#include "../../io_handling/raw_format_processor.hpp"
// constants
#include "../../global_functions_and_constants/gnuplot_constants.hpp"
#include "../../global_functions_and_constants/global_constants.hpp"
#include "../../global_functions_and_constants/global_template_functions.hpp"
#include "../../global_functions_and_constants/ahdl_constants.hpp"
#include "../../global_functions_and_constants/files_folders_io_constants.hpp"

SpectreSimulation::SpectreSimulation() {
	this->altered_scenario_index = kNotDefinedInt;
	this->folder = kNotDefinedString;
	this->top_folder = kNotDefinedString;
	this->delete_spectre_transients = true;
	this->delete_processed_transients = true;
	this->delete_spectre_folders = false;
	this->ahdl_simdb_env = kNotDefinedString;
	this->ahdl_shipdb_env = kNotDefinedString;
	this->process_metrics = true;
	this->export_processed_metrics = false;
	this->export_metric_errors = false;
	this->correctly_simulated = false;
	this->correctly_processed = false;
	this->is_nested_simulation = false;
	this->simulation_id = kNotDefinedString;
	// injection mode related
	this->has_additional_injection = false;
	// n_dimensional
	this->n_dimensional = false;
	this->n_d_profile_index = kNotDefinedInt;
	// montecarlo
	this->is_montecarlo_nested_simulation = false;
	this->montecarlo_transient_sufix = kNotDefinedString;
	// plot
	this->plot_scatters = false;
	this->plot_transients = false;

	this->golden_critical_parameter = nullptr;
	this->simulation_parameters = nullptr;
}

SpectreSimulation::~SpectreSimulation(){
	#ifdef DESTRUCTORS_VERBOSE
		std::cout << "SpectreSimulation destructor" << "\n";;
	#endif
	if( simulation_parameters ){
		#ifdef DESTRUCTORS_VERBOSE
			std::cout << "deleteContentsOfVectorOfPointers( *simulation_parameters ) in SpectreSimulation destructor\n";
		#endif
		deleteContentsOfVectorOfPointers( *simulation_parameters );
	}
}

void SpectreSimulation::AddAdditionalSimulationParameter(
		SimulationParameter* simulationParameter ){
	simulation_parameters->push_back( simulationParameter );
}

void SpectreSimulation::WaitForResources( unsigned int& threadsCount,
	const unsigned int maxThreads, boost::thread_group& tg, const unsigned int thread2BeWaited ){
	if( threadsCount == maxThreads ){
		// Wait for threads
		#ifdef SPECTRE_SIMULATIONS_VERBOSE
		log_io->ReportPlainStandard( "Scenario # " + simulation_id + " Waiting for resources until thread #" + number2String(thread2BeWaited) );
		#endif
		tg.join_all();
		#ifdef SPECTRE_SIMULATIONS_VERBOSE
		log_io->ReportPlainStandard( "Scenario # " + simulation_id + ". OK, next thread " + number2String(thread2BeWaited) );
		#endif
		threadsCount = 0;
	}
}

bool SpectreSimulation::UpdateParameterValue( SimulationParameter& updatedParameter,
	std::string newValue ){
	if(simulation_parameters == nullptr){
		return false;
	}
	for( auto const &p : *simulation_parameters ){
		if( p->get_name().compare( updatedParameter.get_name() ) == 0 ){
			p->set_value(newValue);
			return true;
		}
	}
	return false;
}

bool SpectreSimulation::UpdateGoldenCriticalParameter( SimulationParameter& originalGoldenParameter ){
	if(simulation_parameters == nullptr){
		return false;
	}
	for( auto const &p : *simulation_parameters ){
		if( p->get_name().compare( originalGoldenParameter.get_name() ) == 0 ){
			golden_critical_parameter = p;
			return true;
		}
	}
	return false;
}

void SpectreSimulation::CopySimulationParameters(
		std::vector<SimulationParameter*>& simulationParameters ){
	// in case simulation parameters is not empty
	simulation_parameters = new std::vector<SimulationParameter*>();
	// reserve memory
	simulation_parameters->reserve( simulationParameters.size() );
	deepCopyVectorOfPointers( simulationParameters, *simulation_parameters );
}

bool SpectreSimulation::ExportParametersCircuit( std::string currentFolder,
	int parameterVariationCount ){
	if(simulation_parameters == nullptr){
		log_io->ReportError2AllLogs( "simulation_parameters vector is null ");
		return false;
	}
	#ifdef SPECTRE_SIMULATIONS_VERBOSE
	log_io->ReportPlain2Log( k2Tab + "#" + simulation_id + ", Exporting parameters file..." );
	#endif
	bool success = true;
	std::string oParametersFile;
	oParametersFile = currentFolder + kFolderSeparator + kParametersCircuitFile;
	std::ofstream outputFile;
	outputFile.open( oParametersFile.c_str() );

	outputFile << kCommentWord1 << "Parameter variation #" << number2String(parameterVariationCount) << kEmptyLine;
	if( golden_critical_parameter!=nullptr ){
		outputFile << kCommentWord1 << "Parameter " << golden_critical_parameter->get_name()
			<< ", min_val:" << golden_critical_parameter->get_value_min()
			<< ", max_val:" << golden_critical_parameter->get_value_max() << kEmptyLine;
	}
	outputFile << kLanguageSentence << kEmptyLine;
	if( simulation_parameters==nullptr ){
		log_io->ReportError2AllLogs( "nullptr simulation_parameters, ");
		return false;
	}
	outputFile << kEmptyLine << kParametersWord;
	for( auto const &p : *simulation_parameters ){
		outputFile << kDelimiter << p->get_name() << kEqualsWord << p->get_value();
	}
	outputFile.close();
	#ifdef SPECTRE_SIMULATIONS_VERBOSE
	log_io->ReportPlain2Log( k2Tab + "#" + simulation_id + " scenario: parameters file exported." );
	#endif
	return success;
}

// paramIndex and sweepIndex response to param_index and sweep_index for nested simulations
// but while called from 'parent' simulations, they are different parameters
bool SpectreSimulation::InterpolateAndAnalyzeMetrics(
	TransientSimulationResults& transientSimulationResults,
	std::vector<Metric*>& simulatedMetrics, const unsigned int index, const std::string partialId  ){
	bool reliabilityError = false;
	// obtain std::vector<Metric*>* metrics depending on its profile index
	auto golden_metrics = golden_metrics_structure->GetMetricsVector( index );
	if( simulatedMetrics.size()<2 ){
		log_io->ReportError2AllLogs( "Metrics size (including time):" + number2String(simulatedMetrics.size())
			+ " at the scenario #" + partialId);
			return false;
	}
	if( golden_metrics->size()<2 ){
		log_io->ReportError2AllLogs( "golden_metrics size (including time):" + number2String(golden_metrics->size())
			+ " at the scenario #" + partialId);
			return false;
	}
	auto pTimeGolden = static_cast<Magnitude*>(*(golden_metrics->begin()));
	auto pTimeSim = static_cast<Magnitude*>(*(simulatedMetrics.begin()));
	std::vector<Metric*>::iterator it_m = ++(simulatedMetrics.begin());
	// for each metric
	for(std::vector<Metric*>::iterator it_mg = ++(golden_metrics->begin()); it_mg != golden_metrics->end(); ++it_mg){
		if( (*it_mg)->get_analyzable() && (*it_mg)->is_transient_magnitude() ){
			auto pMagGolden = static_cast<Magnitude*>(*it_mg);
			auto pMagSim = static_cast<Magnitude*>(*it_m);
			#ifdef SPECTRE_SIMULATIONS_VERBOSE
				log_io->ReportPlain2Log( kTab + (*it_mg)->get_name() + " metric of scenario #" + simulation_id + " partial_id=" + partialId + " will be analyzed.");
			#endif
			if( !InterpolateAndAnalyzeMagnitude( transientSimulationResults, reliabilityError,
					*pMagGolden, *pMagSim, *pTimeGolden, *pTimeSim, partialId )) {
				log_io->ReportError2AllLogs( "Error interpolating magnitude " + (*it_mg)->get_name() + " at the scenario #" + partialId);
				transientSimulationResults.set_reliability_result( kScenarioSensitive );
				return false;
			}
		}else if( (*it_mg)->get_analyzable() ){
			#ifdef SPECTRE_SIMULATIONS_VERBOSE
				log_io->ReportPlain2Log( kTab + (*it_mg)->get_name() + " metric of scenario #" + simulation_id + " partial_id=" + partialId + " will be analyzed.");
			#endif
			auto pOceanEvalMetricGolden = static_cast<OceanEvalMetric*>(*it_mg);
			auto pOceanEvalMetricSim = static_cast<OceanEvalMetric*>(*it_m);
			if( !AnalyzeOceanEvalMetric( transientSimulationResults, reliabilityError,
					*pOceanEvalMetricGolden, *pOceanEvalMetricSim, partialId )) {
				log_io->ReportError2AllLogs( "Error interpolating magnitude " + (*it_mg)->get_name() + " at the scenario #" + partialId);
				transientSimulationResults.set_reliability_result( kScenarioSensitive );
				return false;
			}
		}
		// update simulated metric to match golden metric
		++it_m;
	}
	if( reliabilityError ){
		transientSimulationResults.set_reliability_result( kScenarioSensitive );
		#ifdef RESULTS_ANALYSIS_VERBOSE
		log_io->ReportRedStandard( "Reliability error at scenario #" + partialId + " simulation_result #" + transientSimulationResults.get_full_id() );
		#endif
	}else{
		transientSimulationResults.set_reliability_result( kScenarioNotSensitive);
		#ifdef RESULTS_ANALYSIS_VERBOSE
		log_io->ReportCyanStandard( "No reliability error at scenario #" + partialId + " simulation_result #" + transientSimulationResults.get_full_id() );
		#endif
	}
	return true;
}

bool SpectreSimulation::AnalyzeOceanEvalMetric(TransientSimulationResults& transientSimulationResults,
	bool& reliabilityError, OceanEvalMetric& goldenMetric, OceanEvalMetric& simulatedMetric, const std::string & partialId ){
	#ifdef SPECTRE_SIMULATIONS_VERBOSE
		log_io->ReportPlain2Log( "-> #" + partialId + " Scenario, #" + transientSimulationResults.get_full_id()
					+ " variation, metric " + simulatedMetric.get_name() + " Analyzed.");
	#endif
	MetricErrors* metricErrors = new MetricErrors();
	metricErrors->set_metric_name( simulatedMetric.get_name() );
	// add it
	transientSimulationResults.AddMetricErrors( metricErrors );
	// report results
	double error = std::abs( goldenMetric.get_value() - simulatedMetric.get_value());
	metricErrors->set_max_abs_error( error );
// log_io->ReportPlainStandard( "[debug]> #" + partialId + " simulated: "+ number2String(simulatedMetric.get_value()) + " golden: " + number2String(goldenMetric.get_value()) );
// std::cout << "[debug] errorC: "<< std::scientific << metricErrors->get_max_abs_error() << "\n";
	metricErrors->set_max_abs_error_global( error );
	metricErrors->set_has_errors( error > goldenMetric.get_abs_error_margin() );
	if( metricErrors->get_has_errors() ){
		transientSimulationResults.set_has_metrics_errors( true );
		reliabilityError = true;
	}
	return true;
}

bool SpectreSimulation::Get2AnalyzableTimeWindow(
	std::vector<double>::iterator& itGoldenTime, std::vector<double>::iterator& itGoldenTimeEnd,
	std::vector<double>::iterator& itSimulatedTime, std::vector<double>::iterator& itSimulatedTimeEnd,
	std::vector<double>::iterator& itGoldenMetric, std::vector<double>::iterator& itSimulatedMetric,
	double analyzableTimeWindowT0 ){
	// starting time is the window t0
	while( itGoldenTime < itGoldenTimeEnd && *itGoldenTime < analyzableTimeWindowT0 ){
		++itGoldenTime;
		++itGoldenMetric;
	}
	if( itGoldenTime == itGoldenTimeEnd ){
		// log_io->ReportError2AllLogs( "Invalid t0 window for simulation #" + partialId );
		return false;
	}
	while( itSimulatedTime < itSimulatedTimeEnd && *itSimulatedTime < analyzableTimeWindowT0 ){
		++itSimulatedTime;
		++itSimulatedMetric;
	}
	if( itSimulatedTime == itSimulatedTimeEnd ){
		// log_io->ReportError2AllLogs( "Invalid t0 window for simulation #" + partialId );
		return false;
	}
	return true;
}

bool SpectreSimulation::CheckEndOfWindow(
	const bool metricAnalizableInTWindow, const double& analyzableTimeWindowTF,
	std::vector<double>::iterator& itGoldenTime, std::vector<double>::iterator& itSimulatedTime ){
	// fgarcia note:
	//best performance I think
	if( metricAnalizableInTWindow
			&& ( *itGoldenTime > analyzableTimeWindowTF || *itSimulatedTime > analyzableTimeWindowTF ) ){
			#ifdef RESULTS_ANALYSIS_VERBOSE
				log_io->ReportPlain2Log("reached window tf: " + number2String(analyzableTimeWindowTF) );
				log_io->ReportPlain2Log("\t current itGoldenTime: " + number2String(*itGoldenTime) );
				log_io->ReportPlain2Log("\t current itSimulatedTime: " + number2String(*itSimulatedTime) );
			#endif
		return true; // break while
	}
	return false;
}

void SpectreSimulation::ReportSimulationsLengthError( Magnitude& goldenTime,
	Magnitude& simulatedTime, const std::string& partialId ){

	#ifdef RESULTS_ANALYSIS_VERBOSE
	log_io->ReportError2AllLogs("Maybe there was an Spectre error previous to this step.");
	log_io->ReportError2AllLogs("Golden size:" + number2String( goldenTime.get_values_size() ));
	log_io->ReportError2AllLogs("Altered size:" + number2String( simulatedTime.get_values_size()));
	if( goldenTime.get_values()->front() != simulatedTime.get_values()->front() ){
		log_io->ReportError2AllLogs("Golden initial time:" + number2String(goldenTime.get_values()->front()));
		log_io->ReportError2AllLogs("Altered scenario initial time:" + number2String(simulatedTime.get_values()->front()));
		log_io->ReportError2AllLogs("Diff:"
			+ number2String(simulatedTime.get_values()->front() - goldenTime.get_values()->front() ));
	}
	if( goldenTime.get_values()->back() != simulatedTime.get_values()->back() ){
		log_io->ReportError2AllLogs("Golden final time:" + number2String(goldenTime.get_values()->back()));
		log_io->ReportError2AllLogs("Altered scenario final time:" + number2String(simulatedTime.get_values()->back()));
		log_io->ReportError2AllLogs("Diff:"
			+ number2String(simulatedTime.get_values()->back() - goldenTime.get_values()->back() ));
	}
	#endif
	log_io->ReportError2AllLogs("Simulation initial-final times, in scenario # "
		+ partialId + " do not coincide with golden scenario timing.");
}

void SpectreSimulation::VerboseStartMagnitudeAnalysis(
	std::vector<double>::iterator& itGoldenTime, std::vector<double>::iterator& itSimulatedTime,
	std::vector<double>::iterator& itGoldenMetric, std::vector<double>::iterator& itSimulatedMetric,
	const Magnitude& simulatedMetric ){
	#ifdef RESULTS_ANALYSIS_VERBOSE
		log_io->ReportPlain2Log("****************************" );
		log_io->ReportPlain2Log("****************************" );
		log_io->ReportPlain2Log("itGoldenTime at start: " + number2String(*itGoldenTime) );
		log_io->ReportPlain2Log("itSimulatedTime at start: " + number2String(*itSimulatedTime) );

		log_io->ReportPlain2Log("****************************" );
		log_io->ReportPlain2Log("currentGoldenValue at start: " + number2String(*itGoldenMetric) );
		log_io->ReportPlain2Log("currentSimulatedValue at start: " + number2String(*itSimulatedMetric) );
		log_io->ReportPlain2Log("****************************" );

		log_io->ReportPlain2Log("get_abs_error_margin_default : " + number2String(simulatedMetric.get_abs_error_margin_default()) );
		log_io->ReportPlain2Log("get_error_time_span_default : " + number2String(simulatedMetric.get_error_time_span_default()) );
		log_io->ReportPlain2Log("get_error_threshold_ones : " + number2String(simulatedMetric.get_error_threshold_ones()) );
		log_io->ReportPlain2Log("get_error_time_span_ones : " + number2String(simulatedMetric.get_error_time_span_ones()) );
		log_io->ReportPlain2Log("get_abs_error_margin_zeros : " + number2String(simulatedMetric.get_abs_error_margin_zeros()) );
		log_io->ReportPlain2Log("get_error_time_span_zeros : " + number2String(simulatedMetric.get_error_time_span_zeros()) );
		log_io->ReportPlain2Log("****************************" );
		log_io->ReportPlain2Log("****************************" );
	#endif
}

void SpectreSimulation::VerboseReliabilityError( const std::string& errorType,
	TransientSimulationResults& transientSimulationResults, const std::string& partialId,
	const std::string& magName, const double& currentTime, const double& currentMetricError,
	const double& currentSimulatedValue, const double& currentGoldenValue, const double& backSimulatedValue, const double& backGoldenValue,
	std::vector<double>::iterator& itSimulatedMetric, std::vector<double>::iterator& itGoldenMetric ){
	#ifdef RESULTS_ANALYSIS_VERBOSE
		log_io->ReportPlain2Log( k2Tab + "[RELIABILITY ERROR] #" + partialId
			+ " Scenario, #" + transientSimulationResults.get_full_id()
			+ " variation. Found reliability error, " + errorType + ", at time=" + number2String(currentTime)
			+ " metric " + magName
			+ " currentMetricError:" + number2String(currentMetricError)
			+ kEmptyLine
			+ " currentSimulatedValue:" + number2String(currentSimulatedValue)
			+ " backSimulatedValue: "+ number2String(backSimulatedValue)
			+ " *itSimulatedMetric:" + number2String(*itSimulatedMetric)
			+ kEmptyLine
			+ " currentGoldenValue:" + number2String(currentGoldenValue)
			+ " backGoldenValue: "+ number2String(backGoldenValue)
			+ " *itGoldenMetric:" + number2String(*itGoldenMetric) );
	#endif
}

bool SpectreSimulation::InterpolateAndAnalyzeMagnitude( TransientSimulationResults& transientSimulationResults,
	bool& reliabilityError, Magnitude& goldenMagnitude, Magnitude& simulatedMagnitude,
	Magnitude& goldenTime, Magnitude& simulatedTime, const std::string& partialId ){

	#ifdef RESULTS_ANALYSIS_VERBOSE
		log_io->ReportPlain2Log( kTab + "#" + simulation_id + " " + partialId + " -> Interpolating/Analyzing metric " + simulatedMagnitude.get_name() );
	#endif
	// check simulation lengths
	if( goldenTime.get_values()->front() != simulatedTime.get_values()->front()
		|| goldenTime.get_values()->back() != simulatedTime.get_values()->back() ){
		log_io->ReportPlain2Log("Simulation initial-final times, in scenario # " + partialId + " do not coincide with golden scenario timing.");
		if( std::abs(simulatedTime.get_values()->back() - goldenTime.get_values()->back()) > goldenTime.get_values()->back()*kSpectreMaxAllowedSimDiffCoeff ){
			ReportSimulationsLengthError( goldenTime, simulatedTime, partialId );
			return false;
		}
	}
	// MetricError
	MetricErrors* metricErrors = new MetricErrors();
	metricErrors->set_metric_name( simulatedMagnitude.get_name() );
	// add it
	transientSimulationResults.AddMetricErrors( metricErrors );

	// Values Pointers: goldenTime, simulatedTime, goldenMagnitude, simulatedMagnitude
	// all are std::vector<double>::iterator
	auto itGoldenTime = goldenTime.get_values()->begin();
	auto itSimulatedTime = simulatedTime.get_values()->begin();
	auto itGoldenMetric = goldenMagnitude.get_values()->begin();
	auto itSimulatedMetric = simulatedMagnitude.get_values()->begin();
	// End pointers
	auto itGoldenTimeEnd = goldenTime.get_values()->end();
	auto itSimulatedTimeEnd = simulatedTime.get_values()->end();

	// metrics errors
	bool metricError = false;
	bool onGoingError = false;
	double errorInit = 0.0;
	// statistics
	double maxAbsErrorGlobal = 0;
	double maxAbsError = 0;
	// partial result
	bool partialResult = true;
	std::string computedErrorFilePath = top_folder + kFolderSeparator
		+ kResultsFolder + kFolderSeparator + kResultsDataFolder + kFolderSeparator + kTransientResultsFolder + kFolderSeparator
		+ simulation_id + "_" + partialId + "_mag_" + goldenMagnitude.get_file_name() + "_error_stats_" + kDataSufix;
	std::ofstream computedErrorFile;
	try{
		// export mag errors
		if( export_metric_errors ){
			computedErrorFile.open( computedErrorFilePath.c_str() );
			computedErrorFile << "# ID: " << partialId << "\n";
			computedErrorFile << "# Mag: " << goldenMagnitude.get_name() << "\n";
			computedErrorFile << "# time currentAbsError maxAbsError maxAbsErrorGlobal goldenValue simulatedValue PUNCTUAL \n";
			#ifdef RESULTS_ANALYSIS_VERBOSE
				// [fgarcia] for statistics
				if( export_metric_errors ){
					computedErrorFile << "# All punctual error is recorded as RESULTS_ANALYSIS_VERBOSE (compile time) is set.\n";
				}
			#endif
		}
		// current error etc
		double currentMetricError;
		double currentSimulatedValue;
		double currentGoldenValue;
		double backSimulatedValue;
		double backGoldenValue;
		double currentTime;
		double backTime;
		// Interpolation error control
		bool interpolationError = false;

		// init time
		// First step omitted (time=0.0)
		currentTime = *(++itGoldenTime);
		++itSimulatedTime;
		// init metrics values
		backGoldenValue = *itGoldenMetric++;
		backSimulatedValue = *itSimulatedMetric++;
		currentSimulatedValue = *itSimulatedMetric;
		currentGoldenValue = *itGoldenMetric;

		// Interpolation
		// InterpolateValue: Params double x1, double x2, double y1, double y2, double a
		// (a - x1)*(y2 - y1)/(x2 - x1) + y1, with x2>x1

		// time window
		if( goldenMagnitude.get_analyze_error_in_time_window() ){
			if( !Get2AnalyzableTimeWindow( itGoldenTime, itGoldenTimeEnd,
				itSimulatedTime, itSimulatedTimeEnd,
				itGoldenMetric, itSimulatedMetric,
				goldenMagnitude.get_analyzable_time_window_t0()) ){
				log_io->ReportError2AllLogs( "Invalid t0 window for simulation #" + partialId );
				return false;
			}
		}
		// update current/backtime
		currentTime = std::min( *itGoldenTime, *itSimulatedTime );
		backTime = currentTime;
		currentSimulatedValue = *itSimulatedMetric;
		currentGoldenValue = *itGoldenMetric;
		backSimulatedValue = currentSimulatedValue;
		backGoldenValue = currentGoldenValue;
		// report if needed
		VerboseStartMagnitudeAnalysis( itGoldenTime, itSimulatedTime,
			itGoldenMetric, itSimulatedMetric, simulatedMagnitude );

		// main while loop
		while ( itGoldenTime < itGoldenTimeEnd && itSimulatedTime < itSimulatedTimeEnd && !interpolationError ) {
			// check if end of observable window
			if( CheckEndOfWindow( goldenMagnitude.get_analyze_error_in_time_window(),
					goldenMagnitude.get_analyzable_time_window_tf(), itGoldenTime, itSimulatedTime) ){
				break; // while
			}

			// Comparison of time-axis
			if ( *itGoldenTime == *itSimulatedTime ) {
				// time
				currentTime = *itGoldenTime++;
				++itSimulatedTime;
				// metric
				currentSimulatedValue = *itSimulatedMetric++;
				currentGoldenValue = *itGoldenMetric++;
			} else if ( *itGoldenTime < *itSimulatedTime ) {
				// time
				currentTime = *itGoldenTime++;
				// metrics
				currentGoldenValue = *itGoldenMetric++;
				currentSimulatedValue = InterpolateValue( interpolationError, backTime, *itSimulatedTime,
					backSimulatedValue, *itSimulatedMetric, currentTime);
			} else {
				// time
				currentTime = *itSimulatedTime++;
				// metrics
				currentSimulatedValue = *itSimulatedMetric++;
				currentGoldenValue = InterpolateValue( interpolationError, backTime, *itGoldenTime,
					backGoldenValue, *itGoldenMetric, currentTime);
			}
			// Update back variables
			backTime = currentTime;
			backSimulatedValue = currentSimulatedValue;
			backGoldenValue = currentGoldenValue;
			// Error
			currentMetricError = std::abs( currentGoldenValue - currentSimulatedValue );
			// Max Abs Error Global
			if( maxAbsErrorGlobal<currentMetricError ){
				maxAbsErrorGlobal = currentMetricError;
			}
			#ifdef RESULTS_ANALYSIS_VERBOSE
				// [fgarcia] for statistics
				if( export_metric_errors ){
					computedErrorFile << currentTime << " " << currentMetricError << " " << maxAbsError << " " << maxAbsErrorGlobal << " " << currentGoldenValue << " " << currentSimulatedValue << " 1\n";
				}
			#endif
			// Analyze error
			double absErrorMargin = simulatedMagnitude.get_abs_error_margin_default();
			double absErrorTimeSpan = simulatedMagnitude.get_error_time_span_default();
			if( currentGoldenValue > simulatedMagnitude.get_error_threshold_ones() ){
				absErrorMargin = simulatedMagnitude.get_abs_error_margin_ones();
				absErrorTimeSpan = simulatedMagnitude.get_error_time_span_ones();
			}else if( currentGoldenValue < simulatedMagnitude.get_error_threshold_zeros() ) {
				absErrorMargin = simulatedMagnitude.get_abs_error_margin_zeros();
				absErrorTimeSpan = simulatedMagnitude.get_error_time_span_zeros();
			}
			// for abs error stats
			bool updateMaxAbsError = false;
			// Punctual errors
			if ( !simulatedMagnitude.get_analyze_error_in_time() ) {
				if ( CheckError( simulatedMagnitude, currentSimulatedValue, currentGoldenValue,
						currentMetricError, absErrorMargin ) ) {
					metricErrors->AddErrorTiming( number2String(currentTime), "");
					reliabilityError = true;
					metricError = true;
					updateMaxAbsError = true;
					VerboseReliabilityError( "punctual error", transientSimulationResults, partialId, simulatedMagnitude.get_name(),
						currentTime, currentMetricError, currentSimulatedValue, currentGoldenValue, backSimulatedValue, backGoldenValue,
						itSimulatedMetric, itGoldenMetric );
					// export metric errors
					if( export_metric_errors ){
						computedErrorFile << currentTime << " " << currentMetricError << " " << maxAbsError << " " << maxAbsErrorGlobal << " " << currentGoldenValue << " " << currentSimulatedValue << " 0\n";
					}
				}
			}else{
				if ( CheckError( simulatedMagnitude, currentSimulatedValue, currentGoldenValue, currentMetricError, absErrorMargin ) ) {
					if( onGoingError ){
						if ( currentTime - errorInit > absErrorTimeSpan ) {
							if( !reliabilityError ){
								VerboseReliabilityError( "non punctual error", transientSimulationResults, partialId, simulatedMagnitude.get_name(),
									currentTime, currentMetricError, currentSimulatedValue, currentGoldenValue, backSimulatedValue, backGoldenValue,
									itSimulatedMetric, itGoldenMetric );
							}
							metricErrors->AddErrorTiming( number2String(errorInit), number2String(currentTime));
							reliabilityError = true;
							metricError = true;
							updateMaxAbsError = true;
							// export metric errors
							if( export_metric_errors ){
								computedErrorFile << currentTime << " " << currentMetricError << " " << maxAbsError << " " << maxAbsErrorGlobal << " " << currentGoldenValue << " " << currentSimulatedValue << " 0\n";
							}
						}
					} else {
						onGoingError = true;
						errorInit = currentTime;
						#ifdef RESULTS_ANALYSIS_VERBOSE
							log_io->ReportPlain2Log( k2Tab + "[POSIBLE RELIABILITY ERROR*] #" + partialId + " Scenario, #" + transientSimulationResults.get_full_id()
								+ " variation. Found reliability error at time=" + number2String(currentTime) + ". Analyzing if it is only punctual" );
						#endif
					}
				}else if( onGoingError ) {
					#ifdef RESULTS_ANALYSIS_VERBOSE
					log_io->ReportPlain2Log( k2Tab + " #" + partialId + " Scenario, #" + transientSimulationResults.get_full_id() + " variation. Ongoing reliability error fixed at " + number2String(currentTime) );
					#endif
					onGoingError = false;
				}
			}

			// Max Abs Error
			if( updateMaxAbsError && maxAbsError<currentMetricError ){
				maxAbsError = currentMetricError;
			}
		} // end of while
		if(interpolationError){
			log_io->ReportError2AllLogs( "-> #" + partialId + " Interpolation error at metric " + simulatedMagnitude.get_name());
			metricErrors->set_has_errors( true );
			transientSimulationResults.set_has_metrics_errors( true );
			return false;
		}
		#ifdef SPECTRE_SIMULATIONS_VERBOSE
			log_io->ReportPlain2Log( "-> #" + partialId + " Scenario, #" + transientSimulationResults.get_full_id()
						+ " variation, metric " + simulatedMagnitude.get_name() + " Analyzed.");
		#endif
		// report results
		metricErrors->set_max_abs_error( maxAbsError );
		metricErrors->set_max_abs_error_global( maxAbsErrorGlobal );
		metricErrors->set_has_errors( metricError );
		if( metricError ){
			transientSimulationResults.set_has_metrics_errors( true );
		}
	}catch (std::exception const& ex) {
		std::string ex_what = ex.what();
		log_io->ReportError2AllLogs( "Exception in " + simulation_id + " " + partialId + " -> Interpolating/Analyzing metric " + simulatedMagnitude.get_name() );
		log_io->ReportError2AllLogs( ex_what );
		transientSimulationResults.set_has_metrics_errors( true );
		partialResult = false;
	}
	if( export_metric_errors ){
		computedErrorFile.close();
	}
	return partialResult;
}

bool SpectreSimulation::CheckError( const Magnitude& magnitude, const double& currentSimulatedValue,
	const double& currentGoldenValue, const double& currentMetricError, const double& absErrorMargin ){
// fgarcia needs a revision, we require from a default '1' and '0' level
	return  ( currentMetricError > absErrorMargin ) &&
				!( (magnitude.get_ommit_upper_threshold() && currentGoldenValue > magnitude.get_error_threshold_ones() )
					|| (magnitude.get_ommit_lower_threshold() && currentGoldenValue < magnitude.get_error_threshold_zeros()) );
	// return  currentMetricError > absErrorMargin;
}

double SpectreSimulation::InterpolateValue( bool& interpolationError,
	const double& x1, const double& x2,
	const double& y1, const double& y2, const double& a){
	if( x2<=x1 ){
		interpolationError = true;
		log_io->ReportError2AllLogs( "Scenario #" + simulation_id + " x2<=x1");
		return 0;
	}else if( a>x2 || a<x1 ){
		interpolationError = true;
		log_io->ReportError2AllLogs( "Scenario #" + simulation_id + " a>x2 || a<x1: a:"
			+ number2String(a) + " x1:" + number2String(x1) + " x2:" + number2String(x2) );
		return 0;
	}
	return (a - x1)*(y2 - y1)/(x2 - x1) + y1;
}

void SpectreSimulation::ConfigureEnvironmentVariables(){
	// AHDL compiled components environment configuration
	#ifdef SPECTRE_SIMULATIONS_VERBOSE
	log_io->ReportPlain2Log( " #" + simulation_id + " AHDL compiled components environment configuration." );
	#endif
	int env_result = putenv( (char*)kEnableAHDL.c_str() );
	// SIMDB
	env_result += putenv( (char*)ahdl_simdb_env.c_str() );
	// SHIDB
	env_result += putenv( (char*)kEnable_AHDLCMI_SHIPDB_COPY.c_str() );
	env_result += putenv( (char*)ahdl_shipdb_env.c_str() );
	if( env_result>0 ){
		log_io->ReportRedStandard("[Warning] ConfigureEnvironmentVariables may have failed.");
	}
}

void SpectreSimulation::ShowEnvironmentVariables(){
	#ifdef SPECTRE_SIMULATIONS_VERBOSE
	std::string message;
	const char* env_variable;
	message = " #" + simulation_id + " $CDS_AHDLCMI_ENABLE: ";
	if( (env_variable=std::getenv("CDS_AHDLCMI_ENABLE")) != nullptr){
		message += env_variable ;
	}
	log_io->ReportPlain2Log( message );
	message = " #" + simulation_id + " $CDS_AHDLCMI_SIMDB_DIR: ";
	if( (env_variable=std::getenv("CDS_AHDLCMI_SIMDB_DIR")) != nullptr){
		message += env_variable;
	}
	log_io->ReportPlain2Log( message );
	message = " #" + simulation_id + " $CDS_AHDLCMI_SHIPDB_COPY: ";
	if( (env_variable=std::getenv("CDS_AHDLCMI_SHIPDB_COPY")) != nullptr){
		message += env_variable ;
	}
	log_io->ReportPlain2Log( message );
	message = " #" + simulation_id + " $CDS_AHDLCMI_SHIPDB_DIR: ";
	if( (env_variable=std::getenv("CDS_AHDLCMI_SHIPDB_DIR")) != nullptr){
		message += env_variable;
	}
	log_io->ReportPlain2Log( message );
	#endif
}

std::string SpectreSimulation::GetSpectreLogFilePath( const std::string& currentFolder ){
	return currentFolder + kFolderSeparator + kSpectreStandardLogsFile;
}

std::string SpectreSimulation::GetSpectreResultsFilePath(const std::string& currentFolder,
	const bool& processMainTransient){
	std::string analysisFinalName = main_analysis->get_name();
	if( processMainTransient ){
		analysisFinalName = main_transient_analysis->get_name();
	}
	return currentFolder + kFolderSeparator + kSpectreResultsFolder
		+ kFolderSeparator + analysisFinalName + kTransientSufix;
}

std::string SpectreSimulation::GetSpectreMontecarloEvalFilePath(const std::string& currentFolder ){
	std::string analysisFinalName = main_analysis->get_name() ;
	return currentFolder + kFolderSeparator + kSpectreResultsFolder
		+ kFolderSeparator + analysisFinalName + kMCDATASufix;
}

std::string SpectreSimulation::GetProcessedResultsFilePath(const std::string& currentFolder,
	const std::string& localSimulationId, const bool& processMainTransient){
	std::string analysisFinalName = main_analysis->get_name();
	if( processMainTransient ){
		analysisFinalName = main_transient_analysis->get_name();
	}
	return top_folder + kFolderSeparator + kResultsFolder + kFolderSeparator + kResultsDataFolder
		+ kFolderSeparator + kTransientResultsFolder + kFolderSeparator
		+ localSimulationId + "_" + kProcessedTransientFile;
}

bool SpectreSimulation::ProcessSpectreResults( const std::string& currentFolder, const std::string& localSimulationId,
	TransientSimulationResults& transientSimulationResults, const bool& processMainTransient,
	std::vector<Metric*>& myParameterMetrics, const bool& isGolden, const bool& isMontecarloNested ){
	// obtain paths
	std::string spectreResultsFilePath = GetSpectreResultsFilePath( currentFolder, processMainTransient);
	std::string processedResultsFilePath = GetProcessedResultsFilePath( currentFolder,
		localSimulationId, processMainTransient);
	transientSimulationResults.set_original_file_path( spectreResultsFilePath );
	transientSimulationResults.set_processed_file_path( processedResultsFilePath );
	// Raw Format Processor
	RAWFormatProcessor* rfp = new RAWFormatProcessor();
	//
	rfp->set_additional_save( has_additional_injection );
	rfp->set_format( kGnuPlot );
	rfp->set_metrics( &myParameterMetrics );
	rfp->set_transient_file_path( spectreResultsFilePath );
	rfp->set_log_file_path( GetSpectreLogFilePath( currentFolder ) );
	if( isMontecarloNested ){
		rfp->set_montecarlo_eval_file_path( GetSpectreMontecarloEvalFilePath( currentFolder ) );
	}
	rfp->set_processed_file_path( processedResultsFilePath ) ;
	rfp->set_export_processed_metrics( export_processed_metrics );
	rfp->set_is_golden( isGolden );
	rfp->set_is_montecarlo_nested_simulation( isMontecarloNested );
	#ifdef SPECTRE_SIMULATIONS_VERBOSE
		if(!is_nested_simulation){
			log_io->ReportPlainStandard( k3Tab + "#" + localSimulationId + " scenario: processing spectre output data." + " path: '" + spectreResultsFilePath + "'");
		}
	#endif
	correctly_processed = rfp->ProcessSpectreOutputs();
	if( !correctly_processed ){
		log_io->ReportError2AllLogs("ProcessSpectreResults failed. " + localSimulationId);
	}else{
		#ifdef SPECTRE_SIMULATIONS_VERBOSE
			log_io->ReportPlain2Log( k3Tab + "#" + localSimulationId + " scenario: output data exported. [DELETE] delete rfp;");
		#endif
	}
	delete rfp;
	return correctly_processed;
}

bool SpectreSimulation::PlotTransient( const std::string& localSimulationId,
	TransientSimulationResults& transientSimulationResults, const bool& isGolden ){
	if( !isGolden && plot_transients ){
		log_io->ReportPlain2Log( k3Tab + "Plotting #" + simulation_id + " transient.");
		int transGnuplot = CreateGnuplotTransientImages( localSimulationId,
			transientSimulationResults, n_d_profile_index, false, "" );
		if( transGnuplot > 0 ){
			log_io->ReportError2AllLogs( "Unexpected gnuplot result: " + number2String(transGnuplot) );
			return false;
		}
	}
	return true;
}

bool SpectreSimulation::ManageIndividualResultFiles(
	TransientSimulationResults& transientSimulationResults, bool isGolden ){
	bool handlingResult = true;
	if( delete_spectre_transients ){
		handlingResult = ManageIndividualSpectreFiles( transientSimulationResults );
	}
	if( !isGolden && delete_processed_transients ){
		handlingResult = handlingResult && ManageIndividualProcessedFiles( transientSimulationResults );
	}
	return handlingResult;
}

bool SpectreSimulation::ManageIndividualSpectreFiles( TransientSimulationResults& transientSimulationResults ){
	// delete spectre results, remove Returns:  false if p did not exist in the first place, otherwise true.
	boost::filesystem::remove(transientSimulationResults.get_original_file_path());
	return true;
}

bool SpectreSimulation::ManageIndividualProcessedFiles( TransientSimulationResults& transientSimulationResults ){
	// delete spectre results, remove Returns:  false if p did not exist in the first place, otherwise true.
	boost::filesystem::remove(transientSimulationResults.get_processed_file_path());
	return true;
}

bool SpectreSimulation::ManageSpectreFolder(){
	// delete spectre folders, remove_all returns the number of removed files
	if( delete_spectre_folders ){
	 	if( boost::filesystem::remove_all(folder) < 1 ){
			log_io->ReportError2AllLogs( "Unexpected ManageSpectreFolder: " + folder );
			return false;
		}
	}
	return true;
}

/// called from spectre_handler (variability_spectre_handler or radiation_spectre_handler)
void SpectreSimulation::HandleSpectreSimulation(){
	// simulation
	RunSimulation();
	// post simulation
	if( correctly_simulated ){
		ManageSpectreFolder();
	}
}

std::vector<Metric*>* SpectreSimulation::CreateMetricsVectorFromGoldenMetrics(
	const unsigned int& profileIndex ){
	#ifdef SPECTRE_SIMULATIONS_VERBOSE
		log_io->ReportPlain2Log( k2Tab + "#" + simulation_id + " scenario: Setting up Metrics" );
		log_io->ReportPlain2Log( k2Tab + "#param_profile: " + number2String(profileIndex) );
	#endif
	if( profileIndex>=golden_metrics_structure->GetTotalElementsLength() ){
		log_io->ReportError2AllLogs( k2Tab + "-> Error: size is "
			+ number2String(golden_metrics_structure->GetTotalElementsLength()) );
		log_io->ReportError2AllLogs( k2Tab + "-> Accesing " + number2String(profileIndex)  );
		return nullptr;
	}
	std::vector<Metric*>* golden_metrics =
		golden_metrics_structure->GetMetricsVector( profileIndex );

	std::vector<Metric*>* parameterMetrics = new std::vector<Metric*>();
	// reserve memory
	parameterMetrics->reserve( golden_metrics->size() );
	for( auto const &m : *golden_metrics){
		// Create an empty copy of each metric
		if( m->is_transient_magnitude() ){
			auto pMag = static_cast<Magnitude*>( m );
			parameterMetrics->push_back( new Magnitude(*pMag, false) );
		}else{
			auto pOceanEvalMag = static_cast<OceanEvalMetric*>( m );
			parameterMetrics->push_back( new OceanEvalMetric(*pOceanEvalMag, false) );
		}
	}
	return parameterMetrics;
}

int SpectreSimulation::CreateGnuplotTransientImages( const std::string& localSimulationId,
		TransientSimulationResults& transientSimulationResults,
		const unsigned int& index, const bool& hasOtherInfo,
		const std::string& otherInfo  ){
	std::string fullSimId = hasOtherInfo ? localSimulationId + "_" + otherInfo : localSimulationId;
	std::string gnuplotScriptFilePath = top_folder + kFolderSeparator
		+ kResultsFolder + kFolderSeparator + kResultsGnuplotScriptsFolder + kFolderSeparator + kTransientResultsFolder + kFolderSeparator
		+ fullSimId + kGnuPlotScriptSufix;
	// static script
	std::ofstream gnuplotScriptFile;
	gnuplotScriptFile.open( gnuplotScriptFilePath.c_str() );
	gnuplotScriptFile << "set grid\n";
	// Axis
	gnuplotScriptFile << "set format x \"%g\"\n";
	gnuplotScriptFile << "set format y \"%g\"\n";

	// # remove border on top and right and set color to gray
	gnuplotScriptFile << kCustomBorders << "\n";
	// line style
	gnuplotScriptFile << kTransientSimilarLinesPalette<<  "\n";
	// plots
	int indexCount = 2;
	std::vector<Metric*>* golden_metrics =
		golden_metrics_structure->GetMetricsVector( index );
	std::string goldenFilePath  = golden_metrics_structure->GetFilePath( index );
	for(std::vector<Metric*>::iterator it_mg = ++(golden_metrics->begin());
		it_mg != golden_metrics->end(); ++it_mg){
		if( (*it_mg)->is_transient_magnitude() ){
			auto pMag = static_cast<Magnitude*>(*it_mg);
			if( pMag->get_plottable()  && pMag->get_found_in_results() ){
				std::string mgName = (*it_mg)->get_title_name();
				//std::string mgName = FilenameWithoutIllegalChars( (*it_mg)->get_file_name() );
				std::string outputImagePath = top_folder + kFolderSeparator
					+ kResultsFolder + kFolderSeparator + kResultsImagesFolder + kFolderSeparator
					+ kTransientResultsFolder + kFolderSeparator + fullSimId + "_m_" + (*it_mg)->get_file_name() + kGnuplotTransientSVGSufix;
				// Svg
				gnuplotScriptFile <<  "set term svg noenhanced size "<< kSvgImageWidth << ","
					<< kSvgImageHeight << " fname " << kSvgFont << "\n";
				gnuplotScriptFile <<  "set output \"" << outputImagePath  << "\"\n";

				// metrics
				gnuplotScriptFile <<  "set xlabel \"time\"\n";
				gnuplotScriptFile <<  "set ylabel \"" << mgName <<  "\"\n";
				gnuplotScriptFile <<  "set title \"" << mgName << " \"\n";
				gnuplotScriptFile <<  "set key bottom right\n";
				// Plot
				gnuplotScriptFile <<  "plot '" << transientSimulationResults.get_processed_file_path() << "' u 1:" + number2String(indexCount) << " w lp ls 1 title 'altered', \\\n";
				gnuplotScriptFile <<  " '" + goldenFilePath + "' u 1:" + number2String(indexCount) << " w lp ls 2 title 'golden'\n";
				gnuplotScriptFile << "unset output\n";
				// Add images path
				transientSimulationResults.AddTransientImage( outputImagePath, mgName );
			}else if( pMag->get_plottable() ){
				log_io->ReportError2AllLogs( (*it_mg)->get_name() + " not found in results." );
			}
		}
		++indexCount;
	}
	gnuplotScriptFile << "\nquit\n";
	// close file
	gnuplotScriptFile.close();

	std::string execCommand = kGnuplotCommand + gnuplotScriptFilePath + kGnuplotEndCommand;
	return std::system( execCommand.c_str() );
}
