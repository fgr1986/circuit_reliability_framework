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
#include "../magnitude_errors.hpp"
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
	this->process_magnitudes = true;
	this->export_processed_magnitudes = false;
	this->export_magnitude_errors = false;
	this->correctly_simulated = false;
	this->correctly_processed = false;
	this->is_nested_simulation = false;
	this->simulation_id = kNotDefinedString;
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
	if( threadsCount == maxThreads-1 ){
		// Wait for threads
		#ifdef SPECTRE_SIMULATIONS_VERBOSE
		log_io->ReportPlainStandard( "Scenario # " + simulation_id + " Waiting for resources until " + number2String(thread2BeWaited-1) );
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
bool SpectreSimulation::InterpolateAndAnalyzeMagnitudes(
	TransientSimulationResults& transientSimulationResults,
	std::vector<Magnitude*>& simulatedMagnitudes, const unsigned int index, const std::string partialId  ){
	bool reliabilityError = false;
	// obtain std::vector<Magnitude*>* magnitudes depending on its profile index
	auto golden_magnitudes = golden_magnitudes_structure->GetMagnitudesVector( index );
	if( simulatedMagnitudes.size()<2 ){
		log_io->ReportError2AllLogs( "Magnitudes size (including time):" + number2String(simulatedMagnitudes.size())
			+ " at the scenario #" + partialId);
			return false;
	}
	if( golden_magnitudes->size()<2 ){
		log_io->ReportError2AllLogs( "golden_magnitudes size (including time):" + number2String(golden_magnitudes->size())
			+ " at the scenario #" + partialId);
			return false;
	}
	std::vector<Magnitude*>::iterator it_m = ++(simulatedMagnitudes.begin());
	for(std::vector<Magnitude*>::iterator it_mg = ++(golden_magnitudes->begin());
		it_mg != golden_magnitudes->end(); it_mg++){
		if( (*it_mg)->get_analyzable() && (*it_mg)->is_transient_magnitude() ){
			#ifdef SPECTRE_SIMULATIONS_VERBOSE
				log_io->ReportPlain2Log( kTab + (*it_mg)->get_name()
					+ " magnitude of scenario #" + simulation_id + " partial_id=" + partialId + " will be analyzed.");
			#endif
			if( !InterpolateAndAnalyzeMagnitude( transientSimulationResults, reliabilityError,
					**it_mg, **it_m, **(golden_magnitudes->begin()), **(simulatedMagnitudes.begin()),
					partialId )) {
				log_io->ReportError2AllLogs( "Error interpolating magnitude " + (*it_mg)->get_name()
					+ " at the scenario #" + partialId);
				transientSimulationResults.set_reliability_result( kScenarioSensitive );
				return false;
			}
		}else if( (*it_mg)->get_analyzable() ){

		}
		// update simulated magnitude to match golden magnitude
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

bool SpectreSimulation::Get2AnalyzableTimeWindow(
	std::vector<double>::iterator& itGoldenTime, std::vector<double>::iterator& itGoldenTimeEnd,
	std::vector<double>::iterator& itSimulatedTime, std::vector<double>::iterator& itSimulatedTimeEnd,
	std::vector<double>::iterator& itGoldenMagnitude, std::vector<double>::iterator& itSimulatedMagnitude,
	double analyzableTimeWindowT0 ){
	// starting time is the window t0
	while( itGoldenTime < itGoldenTimeEnd && *itGoldenTime < analyzableTimeWindowT0 ){
		++itGoldenTime;
		++itGoldenMagnitude;
	}
	if( itGoldenTime == itGoldenTimeEnd ){
		// log_io->ReportError2AllLogs( "Invalid t0 window for simulation #" + partialId );
		return false;
	}
	while( itSimulatedTime < itSimulatedTimeEnd && *itSimulatedTime < analyzableTimeWindowT0 ){
		++itSimulatedTime;
		++itSimulatedMagnitude;
	}
	if( itSimulatedTime == itSimulatedTimeEnd ){
		// log_io->ReportError2AllLogs( "Invalid t0 window for simulation #" + partialId );
		return false;
	}
	return true;
}

bool SpectreSimulation::CheckEndOfWindow(
	const bool magnitudeAnalizableInTWindow, const double& analyzableTimeWindowTF,
	std::vector<double>::iterator& itGoldenTime, std::vector<double>::iterator& itSimulatedTime ){
	// fgarcia note:
	//best performance I think
	if( magnitudeAnalizableInTWindow
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
	std::vector<double>::iterator& itGoldenMagnitude, std::vector<double>::iterator& itSimulatedMagnitude,
	const Magnitude& simulatedMagnitude ){
	#ifdef RESULTS_ANALYSIS_VERBOSE
		log_io->ReportPlain2Log("****************************" );
		log_io->ReportPlain2Log("****************************" );
		log_io->ReportPlain2Log("itGoldenTime at start: " + number2String(*itGoldenTime) );
		log_io->ReportPlain2Log("itSimulatedTime at start: " + number2String(*itSimulatedTime) );

		log_io->ReportPlain2Log("****************************" );
		log_io->ReportPlain2Log("currentGoldenValue at start: " + number2String(*itGoldenMagnitude) );
		log_io->ReportPlain2Log("currentSimulatedValue at start: " + number2String(*itSimulatedMagnitude) );
		log_io->ReportPlain2Log("****************************" );

		log_io->ReportPlain2Log("get_abs_error_margin_default : " + number2String(simulatedMagnitude.get_abs_error_margin_default()) );
		log_io->ReportPlain2Log("get_error_time_span_default : " + number2String(simulatedMagnitude.get_error_time_span_default()) );
		log_io->ReportPlain2Log("get_error_threshold_ones : " + number2String(simulatedMagnitude.get_error_threshold_ones()) );
		log_io->ReportPlain2Log("get_error_time_span_ones : " + number2String(simulatedMagnitude.get_error_time_span_ones()) );
		log_io->ReportPlain2Log("get_abs_error_margin_zeros : " + number2String(simulatedMagnitude.get_abs_error_margin_zeros()) );
		log_io->ReportPlain2Log("get_error_time_span_zeros : " + number2String(simulatedMagnitude.get_error_time_span_zeros()) );
		log_io->ReportPlain2Log("****************************" );
		log_io->ReportPlain2Log("****************************" );
	#endif
}

void SpectreSimulation::VerboseReliabilityError( const std::string& errorType,
	TransientSimulationResults& transientSimulationResults, const std::string& partialId,
	const std::string& magName, const double& currentTime, const double& currentMagnitudeError,
	const double& currentSimulatedValue, const double& currentGoldenValue, const double& backSimulatedValue, const double& backGoldenValue,
	std::vector<double>::iterator& itSimulatedMagnitude, std::vector<double>::iterator& itGoldenMagnitude ){
	#ifdef RESULTS_ANALYSIS_VERBOSE
		log_io->ReportPlain2Log( k2Tab + "[RELIABILITY ERROR] #" + partialId
			+ " Scenario, #" + transientSimulationResults.get_full_id()
			+ " variation. Found reliability error, " + errorType + ", at time=" + number2String(currentTime)
			+ " magnitude " + magName
			+ " currentMagnitudeError:" + number2String(currentMagnitudeError)
			+ kEmptyLine
			+ " currentSimulatedValue:" + number2String(currentSimulatedValue)
			+ " backSimulatedValue: "+ number2String(backSimulatedValue)
			+ " *itSimulatedMagnitude:" + number2String(*itSimulatedMagnitude)
			+ kEmptyLine
			+ " currentGoldenValue:" + number2String(currentGoldenValue)
			+ " backGoldenValue: "+ number2String(backGoldenValue)
			+ " *itGoldenMagnitude:" + number2String(*itGoldenMagnitude) );
	#endif
}

bool SpectreSimulation::InterpolateAndAnalyzeMagnitude( TransientSimulationResults& transientSimulationResults,
	bool& reliabilityError, Magnitude& goldenMagnitude, Magnitude& simulatedMagnitude,
	Magnitude& goldenTime, Magnitude& simulatedTime, const std::string& partialId ){

	#ifdef RESULTS_ANALYSIS_VERBOSE
		log_io->ReportPlain2Log( kTab + "#" + simulation_id + " " + partialId + " -> Interpolating/Analyzing magnitude " + simulatedMagnitude.get_name() );
	#endif
	// check simulation lengths
	if( goldenTime.get_values()->front() != simulatedTime.get_values()->front()
		|| goldenTime.get_values()->back() != simulatedTime.get_values()->back() ){
		log_io->ReportPlain2Log("Simulation initial-final times, in scenario # " + partialId + " do not coincide with golden scenario timing.");
		if( abs(simulatedTime.get_values()->back() - goldenTime.get_values()->back()) > goldenTime.get_values()->back()*kSpectreMaxAllowedSimDiffCoeff ){
			ReportSimulationsLengthError( goldenTime, simulatedTime, partialId );
			return false;
		}
	}
	// MagnitudeError
	MagnitudeErrors* magnitudeErrors = new MagnitudeErrors();
	magnitudeErrors->set_magnitude_name( simulatedMagnitude.get_name() );
	// add it
	transientSimulationResults.AddMagnitudeErrors( magnitudeErrors );

	// Values Pointers: goldenTime, simulatedTime, goldenMagnitude, simulatedMagnitude
	// all are std::vector<double>::iterator
	auto itGoldenTime = goldenTime.get_values()->begin();
	auto itSimulatedTime = simulatedTime.get_values()->begin();
	auto itGoldenMagnitude = goldenMagnitude.get_values()->begin();
	auto itSimulatedMagnitude = simulatedMagnitude.get_values()->begin();
	// End pointers
	auto itGoldenTimeEnd = goldenTime.get_values()->end();
	auto itSimulatedTimeEnd = simulatedTime.get_values()->end();

	// magnitudes errors
	bool magnitudeError = false;
	bool onGoingError = false;
	double errorInit = 0.0;
	// statistics
	double maxAbsErrorGlobal = 0;
	double maxAbsError = 0;
	// partial result
	bool partialResult = true;
	std::string computedErrorFilePath = top_folder + kFolderSeparator
		+ kResultsFolder + kFolderSeparator + kResultsDataFolder + kFolderSeparator + kTransientResultsFolder + kFolderSeparator
		+ simulation_id + "_" + partialId
		+ "_mag_" + goldenMagnitude.get_file_name() + "_error_stats_" + kDataSufix;
	std::ofstream computedErrorFile;
	try{
		// export mag errors
		if( export_magnitude_errors ){
			computedErrorFile.open( computedErrorFilePath.c_str() );
			computedErrorFile << "# ID: " << partialId << "\n";
			computedErrorFile << "# Mag: " << goldenMagnitude.get_name() << "\n";
			computedErrorFile << "# time currentAbsError maxAbsError maxAbsErrorGlobal goldenValue simulatedValue\n";
		}
		// current error etc
		double currentMagnitudeError;
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
		// init magnitudes values
		backGoldenValue = *itGoldenMagnitude++;
		backSimulatedValue = *itSimulatedMagnitude++;
		currentSimulatedValue = *itSimulatedMagnitude;
		currentGoldenValue = *itGoldenMagnitude;

		// Interpolation
		// InterpolateValue: Params double x1, double x2, double y1, double y2, double a
		// (a - x1)*(y2 - y1)/(x2 - x1) + y1, with x2>x1

		// time window
		if( goldenMagnitude.get_analyze_error_in_time_window() ){
			if( !Get2AnalyzableTimeWindow( itGoldenTime, itGoldenTimeEnd,
				itSimulatedTime, itSimulatedTimeEnd,
				itGoldenMagnitude, itSimulatedMagnitude,
				goldenMagnitude.get_analyzable_time_window_t0()) ){
				log_io->ReportError2AllLogs( "Invalid t0 window for simulation #" + partialId );
				return false;
			}
		}
		// update current/backtime
		currentTime = std::min( *itGoldenTime, *itSimulatedTime );
		backTime = currentTime;
		currentSimulatedValue = *itSimulatedMagnitude;
		currentGoldenValue = *itGoldenMagnitude;
		backSimulatedValue = currentSimulatedValue;
		backGoldenValue = currentGoldenValue;
		// report if needed
		VerboseStartMagnitudeAnalysis( itGoldenTime, itSimulatedTime,
			itGoldenMagnitude, itSimulatedMagnitude, simulatedMagnitude );

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
				// magnitude
				currentSimulatedValue = *itSimulatedMagnitude++;
				currentGoldenValue = *itGoldenMagnitude++;
			} else if ( *itGoldenTime < *itSimulatedTime ) {
				// time
				currentTime = *itGoldenTime++;
				// magnitudes
				currentGoldenValue = *itGoldenMagnitude++;
				currentSimulatedValue = InterpolateValue( interpolationError, backTime, *itSimulatedTime,
					backSimulatedValue, *itSimulatedMagnitude, currentTime);
			} else {
				// time
				currentTime = *itSimulatedTime++;
				// magnitudes
				currentSimulatedValue = *itSimulatedMagnitude++;
				currentGoldenValue = InterpolateValue( interpolationError, backTime, *itGoldenTime,
					backGoldenValue, *itGoldenMagnitude, currentTime);
			}
			// Update back variables
			backTime = currentTime;
			backSimulatedValue = currentSimulatedValue;
			backGoldenValue = currentGoldenValue;
			// Error
			currentMagnitudeError = std::abs( currentGoldenValue - currentSimulatedValue );
			// Max Abs Error Global
			if( maxAbsErrorGlobal<currentMagnitudeError ){
				maxAbsErrorGlobal = currentMagnitudeError;
			}
			// for statistics
			if( export_magnitude_errors ){
				computedErrorFile << currentTime << " " << currentMagnitudeError << " "
				<< maxAbsError << " " << maxAbsErrorGlobal << " " << currentGoldenValue << " " << currentSimulatedValue << "\n";
			}
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
						currentMagnitudeError, absErrorMargin ) ) {
					magnitudeErrors->AddErrorTiming( number2String(currentTime), "");
					reliabilityError = true;
					magnitudeError = true;
					updateMaxAbsError = true;
					VerboseReliabilityError( "punctual error", transientSimulationResults, partialId, simulatedMagnitude.get_name(),
						currentTime, currentMagnitudeError, currentSimulatedValue, currentGoldenValue, backSimulatedValue, backGoldenValue,
						itSimulatedMagnitude, itGoldenMagnitude );
				}
			}else{
				if ( CheckError( simulatedMagnitude, currentSimulatedValue, currentGoldenValue, currentMagnitudeError, absErrorMargin ) ) {
					if( onGoingError ){
						if ( currentTime - errorInit > absErrorTimeSpan ) {
							if( !reliabilityError ){
								VerboseReliabilityError( "non punctual error", transientSimulationResults, partialId, simulatedMagnitude.get_name(),
									currentTime, currentMagnitudeError, currentSimulatedValue, currentGoldenValue, backSimulatedValue, backGoldenValue,
									itSimulatedMagnitude, itGoldenMagnitude );
							}
							magnitudeErrors->AddErrorTiming( number2String(errorInit), number2String(currentTime));
							reliabilityError = true;
							magnitudeError = true;
							updateMaxAbsError = true;
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
			if( updateMaxAbsError && maxAbsError<currentMagnitudeError ){
				maxAbsError = currentMagnitudeError;
			}
		} // end of while
		if(interpolationError){
			log_io->ReportError2AllLogs( "-> #" + partialId + " Interpolation error at magnitude " + simulatedMagnitude.get_name());
			magnitudeErrors->set_has_errors( true );
			transientSimulationResults.set_has_magnitudes_errors( true );
			transientSimulationResults.AddMagnitudeErrors( magnitudeErrors );
			return false;
		}
		#ifdef SPECTRE_SIMULATIONS_VERBOSE
			log_io->ReportPlain2Log( "-> #" + partialId + " Scenario, #" + transientSimulationResults.get_full_id()
						+ " variation, magnitude " + simulatedMagnitude.get_name() + " Analyzed.");
		#endif
		// report results
		magnitudeErrors->set_max_abs_error( maxAbsError );
		magnitudeErrors->set_max_abs_error_global( maxAbsErrorGlobal );
		magnitudeErrors->set_has_errors( magnitudeError );
		if( magnitudeError ){
			transientSimulationResults.set_has_magnitudes_errors( true );
		}
	}catch (std::exception const& ex) {
		std::string ex_what = ex.what();
		log_io->ReportError2AllLogs( "Exception in " + simulation_id + " " + partialId + " -> Interpolating/Analyzing magnitude " + simulatedMagnitude.get_name() );
		log_io->ReportError2AllLogs( ex_what );
		transientSimulationResults.set_has_magnitudes_errors( true );
		partialResult = false;
	}
	if( export_magnitude_errors ){
		computedErrorFile.close();
	}
	return partialResult;
}

bool SpectreSimulation::CheckError( const Magnitude& magnitude, const double& currentSimulatedValue,
	const double& currentGoldenValue, const double& currentMagnitudeError, const double& absErrorMargin ){
// fgarcia needs a revision, we require from a default '1' and '0' level
	return  ( currentMagnitudeError > absErrorMargin ) &&
				!( (magnitude.get_ommit_upper_threshold() && currentGoldenValue > magnitude.get_error_threshold_ones() )
					|| (magnitude.get_ommit_lower_threshold() && currentGoldenValue < magnitude.get_error_threshold_zeros()) );
	// return  currentMagnitudeError > absErrorMargin;
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
	std::vector<Magnitude*>& myParameterMagnitudes, const bool& isGolden ){
	// obtain paths
	std::string spectreResultsFilePath = GetSpectreResultsFilePath( currentFolder, processMainTransient);
	std::string spectreLogFilePath = GetSpectreLogFilePath( currentFolder );
	std::string processedResultsFilePath = GetProcessedResultsFilePath( currentFolder,
		localSimulationId, processMainTransient);
	transientSimulationResults.set_original_file_path( spectreResultsFilePath );
	transientSimulationResults.set_processed_file_path( processedResultsFilePath );
	// Raw Format Processor
	RAWFormatProcessor* rfp = new RAWFormatProcessor();
	rfp->set_format( kGnuPlot );
	rfp->set_magnitudes( &myParameterMagnitudes );
	rfp->set_transient_file_path( spectreResultsFilePath );
	rfp->set_log_file_path( spectreLogFilePath );
	rfp->set_processed_file_path( processedResultsFilePath ) ;
	rfp->set_export_processed_magnitudes( export_processed_magnitudes );
	rfp->set_is_golden( isGolden );
	#ifdef SPECTRE_SIMULATIONS_VERBOSE
		if(!is_nested_simulation){
			log_io->ReportPlainStandard( k3Tab + "#" + localSimulationId + " scenario: processing spectre output data." + " path: '" + spectreResultsFilePath + "'");
		}
	#endif
	correctly_processed = rfp->ProcessPSFASCII();
	if( !correctly_processed ){
		log_io->ReportError2AllLogs("ProcessPSFASCII failed. " + localSimulationId);
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

std::vector<Magnitude*>* SpectreSimulation::CreateMagnitudesVectorFromGoldenMagnitudes(
	const unsigned int& profileIndex ){
	#ifdef SPECTRE_SIMULATIONS_VERBOSE
		log_io->ReportPlain2Log( k2Tab + "#" + simulation_id + " scenario: Setting up Magnitudes" );
		log_io->ReportPlain2Log( k2Tab + "#param_profile: " + number2String(profileIndex) );
	#endif
	if( profileIndex>=golden_magnitudes_structure->GetTotalElementsLength() ){
		log_io->ReportError2AllLogs( k2Tab + "-> Error: size is "
			+ number2String(golden_magnitudes_structure->GetTotalElementsLength()) );
		log_io->ReportError2AllLogs( k2Tab + "-> Accesing " + number2String(profileIndex)  );
		return nullptr;
	}
	std::vector<Magnitude*>* golden_magnitudes =
		golden_magnitudes_structure->GetMagnitudesVector( profileIndex );

	std::vector<Magnitude*>* parameterMagnitudes = new std::vector<Magnitude*>();
	// reserve memory
	parameterMagnitudes->reserve( golden_magnitudes->size() );
	for( auto const &m : *golden_magnitudes){
		// Create an empty copy of each magnitude
		parameterMagnitudes->push_back( new Magnitude(*m, false) );
	}
	return parameterMagnitudes;
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
	gnuplotScriptFile << "set style line 11 lc rgb '#808080' lt 1\n";
	gnuplotScriptFile << "set border 3 back ls 11\n";
	gnuplotScriptFile << "set tics nomirror\n";
	// line style
	gnuplotScriptFile <<  "set style line 1 lc rgb '#8b1a0e' pt 1 ps 1 lt 1 lw 2 # --- red\n";
	gnuplotScriptFile <<  "set style line 2 lc rgb '#5e9c36' pt 6 ps 1 lt 1 lw 2 # --- green\n";
	// plots
	int indexCount = 2;
	std::vector<Magnitude*>* golden_magnitudes =
		golden_magnitudes_structure->GetMagnitudesVector( index );
	std::string goldenFilePath  = golden_magnitudes_structure->GetFilePath( index );
	for(std::vector<Magnitude*>::iterator it_mg = ++(golden_magnitudes->begin());
		it_mg != golden_magnitudes->end(); it_mg++){
		if( (*it_mg)->get_plottable()  && (*it_mg)->get_found_in_results() ){
			std::string mgName = (*it_mg)->get_title_name();
			//std::string mgName = FilenameWithoutIllegalChars( (*it_mg)->get_file_name() );
			std::string outputImagePath = top_folder + kFolderSeparator
				+ kResultsFolder + kFolderSeparator + kResultsImagesFolder + kFolderSeparator
				+ kTransientResultsFolder + kFolderSeparator + fullSimId + "_m_" + (*it_mg)->get_file_name() + kGnuplotTransientSVGSufix;
			// Svg
			gnuplotScriptFile <<  "set term svg  size "<< kSvgImageWidth << ","
				<< kSvgImageHeight << " fname " << kSvgFont << "\n";
			gnuplotScriptFile <<  "set output \"" << outputImagePath  << "\"\n";

			// magnitudes
			gnuplotScriptFile <<  "set xlabel \"time\"\n";
			gnuplotScriptFile <<  "set ylabel \"" << mgName <<  "\"\n";
			gnuplotScriptFile <<  "set title \"" << mgName << " \"\n";
			gnuplotScriptFile <<  "set key bottom right\n";
			// Plot
			gnuplotScriptFile <<  "plot '" << transientSimulationResults.get_processed_file_path() << "' u 1:"
				+ number2String(indexCount) << " w lp ls 1 title 'altered', \\\n";
			gnuplotScriptFile <<  " '" + goldenFilePath + "' u 1:"
				+ number2String(indexCount) << " w lp ls 2 title 'golden'\n";
			gnuplotScriptFile << "unset output\n";
			// Add images path
			transientSimulationResults.AddTransientImage( outputImagePath, mgName );
		}else if( (*it_mg)->get_plottable() ){
			log_io->ReportError2AllLogs( (*it_mg)->get_name() + " not found in results." );
		}
		++indexCount;
	}
	gnuplotScriptFile << "\nquit\n";
	// close file
	gnuplotScriptFile.close();

	std::string execCommand = kGnuplotCommand + gnuplotScriptFilePath + kGnuplotEndCommand;
	return std::system( execCommand.c_str() );
}
