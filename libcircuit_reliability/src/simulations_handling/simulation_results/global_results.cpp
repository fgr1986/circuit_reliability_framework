/**
 * results_summary.cpp
 *
 * Created on: March 30, 2015
 *   Author: fernando
 */

#include <iostream>

#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
// Radiation simulator
#include "global_results.hpp"
#include "../../global_functions_and_constants/global_constants.hpp"
#include "../../global_functions_and_constants/files_folders_io_constants.hpp"
#include "../../global_functions_and_constants/global_template_functions.hpp"
#include "../../global_functions_and_constants/gnuplot_constants.hpp"
// results_processor
#include "../../io_handling/results_processor.hpp"
// simulation modes
#include "../spectre_simulations/standard_simulation.hpp"
#include "../spectre_simulations/critical_parameter_value_simulation.hpp"
#include "../spectre_simulations/critical_parameter_nd_parameters_sweep_simulation.hpp"
#include "../spectre_simulations/montecarlo_critical_parameter_nd_parameters_sweep_simulation.hpp"

bool GlobalResults::ProcessScenarioStatistics(){
	if( simulations==nullptr || simulation_mode==nullptr ){
		log_io->ReportError2AllLogs("null simulation vector or simulation_mode in GlobalResults");
		return false;
	}
	gnuplot_script_folder = top_folder + "/" + kResultsFolder  + "/" + kResultsGnuplotScriptsFolder + "/" + kSummaryResultsFolder;
	images_folder = top_folder + "/" + kResultsFolder + "/" + kResultsImagesFolder + "/" + kSummaryResultsFolder;
	data_folder = top_folder + "/" + kResultsFolder + "/" + kResultsDataFolder + "/" + kSummaryResultsFolder;
	bool partialResult = true;
	// Process results
	switch( simulation_mode->get_id() ){
		case kStandardMode: {
				partialResult = ProcessStandardSimulationMode();
			}
		break;
		case kCriticalValueMode: {
				partialResult = ProcessCriticalParameterValueSimulationMode();
			}
		break;
		case kCriticalParameterNDParametersSweepMode: {
				partialResult = ProcessCriticalParameterNDParametersSweepSimulationMode();
			}
		break;
		case kMontecarloCriticalParameterNDParametersSweepMode: {
				partialResult = ProcessMontecarloCriticalParameterNDParametersSweepMode();
			}
		break;
		default: {
			log_io->ReportRedStandard("[GlobalResults] default mode:" + number2String(simulation_mode->get_id()) );
			partialResult = false;
		}
		break;
	}
	return partialResult;
}

bool GlobalResults::ProcessStandardSimulationMode(){
	// Export output
	bool correctlyExported = true;
	std::ofstream outputFile;
	try{
		outputFile.open( data_folder + "/" + kSummaryFile.c_str() );
		if (!outputFile){
			log_io->ReportError2AllLogs( kTab + "error writing " + top_folder + "/" + kResultsFolder + "/" + kSummaryFile.c_str());
			return false;
		}
		outputFile << "# Summary file for standard simulation mode\n";
		outputFile << "# altered_scenario_index simulation_id altered_element_path"
			<< " correctlySimulated reliability_result folder\n";
		for( auto const &s : *simulations ){
			auto pSS = dynamic_cast<StandardSimulation*>(s);
			std::string correctlySimulated = pSS->get_correctly_simulated() ? "true " : "false ";
			outputFile << pSS->get_altered_scenario_index()
				<< " " << pSS->get_simulation_id()
				<< " " << pSS->get_altered_statement_path()
				<< " " << correctlySimulated
				<< " " << (pSS->get_basic_simulation_results())->get_s_reliability_result()
				<< " " << pSS->get_folder() << "\n";
		}
	}catch (std::exception const& ex) {
		std::string ex_what = ex.what();
		log_io->ReportError2AllLogs( "Exception while parsing the file: ex-> " + ex_what );
		correctlyExported = false;
	}
	outputFile.close();
	return correctlyExported;
}

bool GlobalResults::ProcessCriticalParameterValueSimulationMode(){
	// Export output
	bool correctlyExported = true;
	std::ofstream outputFile;
	std::string outputFilePath = data_folder + "/" + kSummaryFile.c_str();
	SimulationParameter* criticalParameter = nullptr;
	std::vector<Metric*>* auxMetrics = nullptr;
	unsigned int firstMagOffset = 9;
	unsigned int critParamOffset = 8;
	unsigned int dataColumnsPerMetric = 3;
	try{
		outputFile.open( outputFilePath );
		if (!outputFile){
			log_io->ReportError2AllLogs( kTab + "error writing " + top_folder + "/" + kResultsFolder + "/" + kSummaryFile.c_str());
			return false;
		}
		// aux variables
		auto auxSim = dynamic_cast<CriticalParameterValueSimulation*> (simulations->front());
		criticalParameter = auxSim->get_golden_critical_parameter();
		// aux mags
		auxMetrics = auxSim->get_golden_metrics_structure()->GetMetricsVector( 0 );
		if( criticalParameter == nullptr || auxMetrics==nullptr ){
			log_io->ReportError2AllLogs( "null criticalParameter or auxMetrics");
			return false;
		}
		outputFile << "# Summary file for standard simulation mode\n";
		outputFile << "#altered_scenario_index #simulation_id #altered_element_path";
		outputFile << " #folder #correctlySimulated #affected_by_min_value #affected_by_min_value"
			<< " #critical_parameter_value" << " #MAG_i_name #MAG_i_maxErrorGlobal #MAG_i_maxErrorMetric\n";
		for( auto const &s : *simulations ){
			auto pCPVS = dynamic_cast<CriticalParameterValueSimulation*>(s);
			std::string correctlySimulated = pCPVS->get_correctly_simulated() ? "true " : "false ";
			std::string affectedByMinVal = pCPVS->get_affected_by_min_value() ? "true " : "false ";
			std::string affectedByMaxVal = pCPVS->get_affected_by_min_value() ? "true " : "false ";
			outputFile << pCPVS->get_altered_scenario_index()
				<< " " << pCPVS->get_simulation_id()
				<< " " << pCPVS->get_altered_statement_path()
				<< " " << pCPVS->get_folder()
				<< " " << correctlySimulated
				<< " " << affectedByMinVal
				<< " " << affectedByMaxVal
				<< " " << pCPVS->get_critical_parameter_value();
			// mag errors
			auto magErrors = pCPVS->get_last_valid_transient_simulation_results()->get_metrics_errors();
			for( auto const &m : *magErrors ){
				outputFile << " " << m->get_metric_name() << " " << m->get_max_abs_error_global()
					<< " " << m->get_max_abs_error();
			}
			outputFile << "\n";
		}
	}catch (std::exception const& ex) {
		std::string ex_what = ex.what();
		log_io->ReportError2AllLogs( "Exception while parsing the file: ex-> " + ex_what );
		correctlyExported = false;
	}
	// close file
	outputFile.close();
	// plot it!
	if( correctlyExported ){
		correctlyExported = correctlyExported && PlotCriticalParameterValueSimulationMode(
			*auxMetrics, critParamOffset, firstMagOffset,
			dataColumnsPerMetric, *criticalParameter, outputFilePath);
	}
	// fgarcia, is it already finished?
	return correctlyExported;
}

bool GlobalResults::PlotCriticalParameterValueSimulationMode(
		const std::vector<Metric*>& analyzedMetrics, const unsigned int &critParamOffset,
		const unsigned int& firstMagOffset, const unsigned int& dataColumnsPerMetric,
		const SimulationParameter& criticalParameter, const std::string& gnuplotDataFile ){
	int partialResult = 0;
	bool correctlyExported = true;
	// plot critical parameter value
	std::string generalGSFPath = gnuplot_script_folder + kFolderSeparator + "scenarios_general" + kGnuPlotScriptSufix;
	std::string generalIPath = images_folder + kFolderSeparator + "scenarios_general" + kSvgSufix;
	std::string title = "[Scenarios General] " + criticalParameter.get_title_name();
	// Generate scripts
	std::ofstream generalGSF;
	try{
		generalGSF.open( generalGSFPath.c_str() );
		// Svg
		generalGSF << "set term svg noenhanced size " << kSvgImageWidth << ","<< kSvgImageHeight << " fname " << kSvgFont << "\n";
		generalGSF << "set output \"" << generalIPath << "\"\n";
		generalGSF << "set title \" " << title << " \"\n";
		generalGSF << "set grid\n";
		// Axis
		generalGSF << "set format x \"%g\"\n";
		generalGSF << "set format y \"%g\"\n";
		generalGSF << "set xlabel \"Scenario\"\n";
		generalGSF << "set ylabel \""  << criticalParameter.get_title_name() << "\"\n";
		// # remove border on top and right and set color to gray
		generalGSF << kCustomBorders;
		generalGSF << "set y2tics\n";
		// palete range
		generalGSF << kTransientSimilarLinesPalette << "\n";
		generalGSF <<  "plot '" << gnuplotDataFile << "' using 1:" << critParamOffset << " axis x1y1 with lp ls 2 title '" << criticalParameter.get_title_name() << "'\n";
		// legend
		generalGSF <<  "set key top left\n";
		generalGSF << "unset output\n";
		// close file
		generalGSF << "quit\n";
	}catch (std::exception const& ex) {
		std::string ex_what = ex.what();
		log_io->ReportError2AllLogs( "Exception while parsing the file: ex-> " + ex_what );
		correctlyExported = false;
	}
	generalGSF.close();
	// Exec comand
	std::string execCommand = kGnuplotCommand + generalGSFPath + kGnuplotEndCommand;
	partialResult += std::system( execCommand.c_str() );
	// plot metrics
	unsigned int magCount = 0;
	for( auto const &m : analyzedMetrics ){
		if( m->get_analyzable() ){
			// Files
			std::string gnuplotScriptFilePath = gnuplot_script_folder + kFolderSeparator
				+ "scenarios_general_mag_" + number2String(magCount) + kGnuPlotScriptSufix;
			std::string outputImagePath = images_folder + kFolderSeparator
				+ "scenarios_general_mag_" + number2String(magCount) + kSvgSufix;
			std::string title = "[General] " + m->get_title_name() + " errors & " + criticalParameter.get_title_name() + " Statistics";
			// Generate scripts
			std::ofstream gnuplotScriptFile;
			try{
				gnuplotScriptFile.open( gnuplotScriptFilePath.c_str() );
				// Svg
				gnuplotScriptFile << "set term svg noenhanced size " << kSvgImageWidth << ","<< kSvgImageHeight
					<< " fname " << kSvgFont << "\n";
				gnuplotScriptFile << "set output \"" << outputImagePath << "\"\n";
				gnuplotScriptFile << "set title \" " << title << " \"\n";
				gnuplotScriptFile << "set grid\n";
				// Axis
				gnuplotScriptFile << "set format x \"%g\"\n";
				gnuplotScriptFile << "set format y \"%g\"\n";
				gnuplotScriptFile << "set format y2 \"%g\"\n";
				gnuplotScriptFile << "set xlabel \"Profile\"\n";
				gnuplotScriptFile << "set y2label \"" << criticalParameter.get_title_name() << " \"\n";
				gnuplotScriptFile << "set ylabel \"Error in metric "  << m->get_title_name() << "\"\n";
				// # remove border on top and right and set color to gray
				gnuplotScriptFile << kCustomBorders;
				gnuplotScriptFile << "set y2tics\n";
				// palete range
				// Color Paletes
				gnuplotScriptFile << kProfilesPalette;
				int magDataIndex = firstMagOffset + dataColumnsPerMetric*magCount; // title
				gnuplotScriptFile <<  "plot '" << gnuplotDataFile << "' using 1:"<< critParamOffset <<" axis x1y2 with filledcurve x1 ls 4 title '" << criticalParameter.get_title_name() << "_max', \\\n";
				gnuplotScriptFile <<  " '" << gnuplotDataFile << "' using 1:"<< (critParamOffset+2) <<" axis x1y2 with filledcurve x1 ls 6 title '" << criticalParameter.get_title_name() << "_mean', \\\n";
				gnuplotScriptFile <<  " '" << gnuplotDataFile << "' using 1:"<< (critParamOffset+1) <<" axis x1y2 with filledcurve x1 ls 5 title '" << criticalParameter.get_title_name() << "_min', \\\n";
				// mags
				// metric
				gnuplotScriptFile <<  "     '" << gnuplotDataFile << "' u 1:" << (magDataIndex+3)  <<  ":" << (magDataIndex+2) << ":" << (magDataIndex+1) << " axis x1y1  w errorbars ls 1 notitle, \\\n";
				gnuplotScriptFile <<  "     '" << gnuplotDataFile << "' u 1:" << (magDataIndex+1) << " axis x1y1  w lp ls 2 title '" << m->get_title_name() << "  (max_error_metric)', \\\n";
				// global
				gnuplotScriptFile <<  "     '" << gnuplotDataFile << "' u 1:" << (magDataIndex+6)  <<  ":" << (magDataIndex+5) << ":" << (magDataIndex+4) << " axis x1y1  w errorbars ls 3 notitle, \\\n";
				gnuplotScriptFile <<  "     '" << gnuplotDataFile << "' u 1:" << (magDataIndex+6) << " axis x1y1  w lp ls 3 title '" << m->get_title_name() << "  (max_error_global)'\n";
				// legend
				gnuplotScriptFile <<  "set key top left\n";

				gnuplotScriptFile << "unset output\n";
				// close file
				gnuplotScriptFile << "quit\n";
			}catch (std::exception const& ex) {
				std::string ex_what = ex.what();
				log_io->ReportError2AllLogs( "Exception while parsing the file: ex-> " + ex_what );
				correctlyExported = false;
			}
			gnuplotScriptFile.close();
			// Exec comand
			std::string execCommand = kGnuplotCommand + gnuplotScriptFilePath + kGnuplotEndCommand;
			partialResult += std::system( execCommand.c_str() );
			++magCount;
		}
	}
	#ifdef RESULTS_POST_PROCESSING_VERBOSE
		if( partialResult>0 ){
			log_io->ReportPlain2Log("Gnuplot may have return warnings or errors in GlobalResults");
		}
	#endif
	return correctlyExported;
}

bool GlobalResults::ProcessCriticalParameterNDParametersSweepSimulationMode(){
	bool partialResult = true;
	if( simulations->size()<1 ){
		log_io->ReportError2AllLogs( "Exception simulations->size()<1" );
		return false;
	}
	// 1) critical parameter value for each profile (mean between scenarios)
	std::ofstream outputFile;
	std::string outputFilePath = data_folder + "/critical_parameter_" + kSummaryFile.c_str();
	// scenario paths
	std::map<std::string, std::string> paths;
	// simulationsNDSimulationResults
	std::vector<NDSimulationResults*> simulationsNDSimulationResults;
	ResultsProcessor rp;
	auto auxSim = dynamic_cast<CriticalParameterNDParameterSweepSimulation*> (simulations->front());
	auto pFirstMagProfileCIMean = auxSim->get_out_profile_c_i_mean();
	auto pFirstMagGPCIMax= auxSim->get_out_p_c_i_max();
	auto pFirstMagGPCIMin= auxSim->get_out_p_c_i_min();
	auto pFirstMagGPCIMean= auxSim->get_out_p_c_i_mean();

	for( auto const &s : *simulations ){
		auto pCPNDPSS = dynamic_cast<CriticalParameterNDParameterSweepSimulation*>(s);
		paths.insert( std::make_pair(
			(pCPNDPSS->get_main_nd_simulation_results())->get_general_data_path(), pCPNDPSS->get_simulation_id() ) );
		simulationsNDSimulationResults.push_back( pCPNDPSS->get_main_nd_simulation_results() );
	}
	// process results
	partialResult = partialResult && rp.StatisticProcessResultsFiles( &paths, outputFilePath, std::move(*pFirstMagProfileCIMean) );
	// plot it!
	if( partialResult ){
		SimulationParameter* criticalParameter = nullptr;
		std::vector<Metric*>* auxMetrics = nullptr;
		unsigned int critParamOffset = auxSim->get_out_gnuplot_crit_param_offset();
		unsigned int profileFirstMagMetricOffset =  auxSim->get_out_profile_gnuplot_first_mag_metric_offset();
		unsigned int profileFirstMagGlobalOffset =  auxSim->get_out_profile_gnuplot_first_mag_global_offset();
		unsigned int planeFirstMagMetricOffset =  auxSim->get_out_plane_gnuplot_first_mag_metric_offset();
		unsigned int planeFirstMagGlobalOffset =  auxSim->get_out_plane_gnuplot_first_mag_global_offset();
		unsigned int dataColumnsPerMetric =  auxSim->get_out_data_per_metric_per_line(); // name + MAG_i_maxErrorMetric*3 + MAG_i_maxErrorGlobal*3
		// aux variables
		criticalParameter = auxSim->get_golden_critical_parameter();
		// aux mags
		auxMetrics = auxSim->get_golden_metrics_structure()->GetMetricsVector( 0 );
		if( criticalParameter == nullptr || auxMetrics==nullptr ){
			log_io->ReportError2AllLogs( "null criticalParameter or auxMetrics");
			return false;
		}
		// create gnuplot scatter map graphs
		partialResult = partialResult && PlotCriticalParameterNDParametersSweepSimulationMode(
			*auxMetrics, critParamOffset, profileFirstMagMetricOffset, profileFirstMagGlobalOffset, dataColumnsPerMetric, *criticalParameter, outputFilePath);
		// planes
		auto simulatedParameters = auxSim->get_simulation_parameters();
		// fgarcia
		auto deleteAfter = GetGoldenProfiles2Simulate(* simulatedParameters);
		delete deleteAfter;
		partialResult = partialResult && GenerateAndPlotParameterPairResults(
			ResultsProcessor::kStatisticProcessStatisticsFiles, auxSim->get_golden_critical_parameter()->get_title_name(),
			critParamOffset, planeFirstMagMetricOffset, planeFirstMagGlobalOffset, dataColumnsPerMetric, *auxMetrics, *simulatedParameters,
			std::move(*pFirstMagGPCIMax), std::move(*pFirstMagGPCIMin), std::move(*pFirstMagGPCIMean), simulationsNDSimulationResults );
	}
	// fgarcia
	// critical parameter value for each Scenario (mean between profiles)

	return partialResult;
}

bool GlobalResults::PlotCriticalParameterNDParametersSweepSimulationMode(
		const std::vector<Metric*>& analyzedMetrics, const unsigned int &critParamOffset,
		const unsigned int& profileFirstMagMetricOffset, const unsigned int& profileFirstMagGlobalOffset, const unsigned int& dataColumnsPerMetric,
		const SimulationParameter& criticalParameter, const std::string& gnuplotDataFile ){
	// partial results
	int partialResult = 0;
	bool correctlyExported = true;
	// Critical parameter
	// Files
	std::string generalGSFPath = gnuplot_script_folder + kFolderSeparator + "scenarios_general" + kGnuPlotScriptSufix;
	std::string generalIPath = images_folder + kFolderSeparator + "scenarios_general" + kSvgSufix;
	std::string title = "[Scenarios General] " + criticalParameter.get_title_name();
	// Generate scripts
	std::ofstream generalGSF;
	try{
		generalGSF.open( generalGSFPath.c_str() );
		// Svg
		generalGSF << "set term svg noenhanced size " << kSvgImageWidth << ","<< kSvgImageHeight
			 << " fname " << kSvgFont << "\n";
		generalGSF << "set output \"" << generalIPath << "\"\n";
		generalGSF << "set title \" " << title
			 << " vs " << criticalParameter.get_title_name() << " \"\n";
		generalGSF << "set grid\n";
		// Axis
		if( criticalParameter.get_value_change_mode()!=kSPLineal ){
			 generalGSF << "set logscale y\n";
		}
		generalGSF << "set format x \"%g\"\n";
		generalGSF << "set format y \"%g\"\n";
		generalGSF << "set xlabel \""  << "Profile" << "\"\n";
		generalGSF << "set ylabel \""  << criticalParameter.get_title_name() << "\"\n";
		// Color Paletes
		generalGSF << kUpsetsPalette;
		// # remove border on top and right and set color to gray
		generalGSF << kCustomBorders;
		// line style
		generalGSF <<  kTransientSimilarLinesPalette;
		// // ranges
		// generalGSF << "set yrange [" << criticalParameter.get_value_min() << ":" << criticalParameter.get_value_max() << "]\n";
		// generalGSF << "set xrange [" << pairedParameter.get_value_min() << ":" << pairedParameter.get_value_max() << "]\n";
		// legend
		generalGSF <<  "set key bottom right\n";
		generalGSF <<  "ntics = 10\n";
		// Plot
		generalGSF <<  "plot '" << gnuplotDataFile << "' u 1:" << (critParamOffset+2) << " w lp ls 2 title '" << criticalParameter.get_title_name() <<  "', \\\n";
		generalGSF <<  "     '" << gnuplotDataFile << "' u 1:" << (critParamOffset+2) << ":" << (critParamOffset+1) <<  ":" << critParamOffset << " w errorbars ls 1 notitle\n";
		generalGSF <<  "set xtics ntics\n";
		generalGSF << "unset output\n";
		// close file
		generalGSF << "quit\n";
		generalGSF.close();
		// Exec comand
	}catch (std::exception const& ex) {
		std::string ex_what = ex.what();
		log_io->ReportError2AllLogs( "Exception while parsing the file: ex-> " + ex_what );
		correctlyExported = false;
	}
	generalGSF.close();
	// Exec comand
	std::string execCommand = kGnuplotCommand + generalGSFPath + kGnuplotEndCommand;
	partialResult += std::system( execCommand.c_str() );
	// plot metrics
	unsigned int magCount = 0;
	for( auto const &m : analyzedMetrics ){
		if( m->get_analyzable() ){
			// Files
			std::string gnuplotScriptFilePath = gnuplot_script_folder + kFolderSeparator
				+ "scenarios_general_mag_" + number2String(magCount) + kGnuPlotScriptSufix;
			std::string outputImagePath = images_folder + kFolderSeparator
				+ "scenarios_general_mag_" + number2String(magCount) + kSvgSufix;
			std::string title = "[General] " + m->get_title_name() + " errors & " + criticalParameter.get_title_name() + " Statistics";
			// Generate scripts
			std::ofstream gnuplotScriptFile;
			try{
				gnuplotScriptFile.open( gnuplotScriptFilePath.c_str() );
				// Svg
				gnuplotScriptFile << "set term svg noenhanced size " << kSvgImageWidth << ","<< kSvgImageHeight
					<< " fname " << kSvgFont << "\n";
				gnuplotScriptFile << "set output \"" << outputImagePath << "\"\n";
				gnuplotScriptFile << "set title \" " << title << " \"\n";
				gnuplotScriptFile << "set grid\n";
				// Axis
				gnuplotScriptFile << "set format x \"%g\"\n";
				gnuplotScriptFile << "set format y \"%g\"\n";
				gnuplotScriptFile << "set format y2 \"%g\"\n";
				gnuplotScriptFile << "set xlabel \"Profile\"\n";
				gnuplotScriptFile << "set y2label \"" << criticalParameter.get_title_name() << " \"\n";
				gnuplotScriptFile << "set ylabel \"Error in metric "  << m->get_title_name() << "\"\n";
				// # remove border on top and right and set color to gray
				gnuplotScriptFile << kCustomBorders;
				gnuplotScriptFile << "set y2tics\n";
				// Color Paletes
				gnuplotScriptFile << kProfilesPalette;
				int magDataMetricIndex = profileFirstMagMetricOffset + dataColumnsPerMetric*magCount; // max
				int magDataGlobalIndex = profileFirstMagGlobalOffset + dataColumnsPerMetric*magCount; // max
				// crit param
				gnuplotScriptFile <<  "plot '" << gnuplotDataFile << "' using 1:" << (critParamOffset) << " axis x1y2 with filledcurve x1 ls 4 title '" << criticalParameter.get_title_name() << "_max', \\\n";
				gnuplotScriptFile <<  " '" << gnuplotDataFile << "' using 1:" << (critParamOffset+2) << " axis x1y2 with filledcurve x1 ls 6 title '" << criticalParameter.get_title_name() << "_mean', \\\n";
				gnuplotScriptFile <<  " '" << gnuplotDataFile << "' using 1:" << (critParamOffset+1) << " axis x1y2 with filledcurve x1 ls 5 title '" << criticalParameter.get_title_name() << "_min', \\\n";
				// candlesticks  # Data columns: X Min 1stQuartile Median 3rdQuartile Max
				// metric
				gnuplotScriptFile <<  "     '" << gnuplotDataFile << "' u 1:" << (magDataMetricIndex+2)  <<  ":" << (magDataMetricIndex+1) << ":" << magDataMetricIndex << " axis x1y1  w errorbars ls 1 notitle, \\\n";
				gnuplotScriptFile <<  "     '" << gnuplotDataFile << "' u 1:" << (magDataMetricIndex+2) << " axis x1y1  w lp ls 2 title '" << m->get_title_name() << " (max_error_metric)', \\\n";
				// global
				gnuplotScriptFile <<  "     '" << gnuplotDataFile << "' u 1:" << (magDataGlobalIndex+2)  <<  ":" << (magDataGlobalIndex+1) << ":" << magDataGlobalIndex << " axis x1y1  w errorbars ls 3 notitle, \\\n";
				gnuplotScriptFile <<  "     '" << gnuplotDataFile << "' u 1:" << (magDataGlobalIndex+2) << " axis x1y1  w lp ls 3 title '" << m->get_title_name() << " (max_error_global)'\n";
				// legend
				gnuplotScriptFile <<  "set key top left\n";

				gnuplotScriptFile << "unset output\n";
				// close file
				gnuplotScriptFile << "quit\n";
			}catch (std::exception const& ex) {
				std::string ex_what = ex.what();
				log_io->ReportError2AllLogs( "Exception while parsing the file: ex-> " + ex_what );
				correctlyExported = false;
			}
			gnuplotScriptFile.close();
			// Exec comand
			std::string execCommand = kGnuplotCommand + gnuplotScriptFilePath + kGnuplotEndCommand;
			partialResult += std::system( execCommand.c_str() );
			++magCount;
		}
	} // end of mags
	#ifdef RESULTS_POST_PROCESSING_VERBOSE
		if( partialResult>0 ){
			log_io->ReportPlain2Log("Gnuplot may have return warnings or errors in GlobalResults");
		}
	#endif

	return correctlyExported;
}

bool GlobalResults::ProcessMontecarloCriticalParameterNDParametersSweepMode(){
	bool partialResult = true;
	if( simulations->size()<1 ){
		log_io->ReportError2AllLogs( "Exception simulations->size()<1" );
		return false;
	}
	// 1) critical parameter value for each profile (mean between scenarios)
	std::ofstream outputFile;
	std::string outputFilePath = data_folder + "/critical_parameter_" + kSummaryFile.c_str();
	// scenario paths
	std::map<std::string, std::string> paths;
	std::vector<NDSimulationResults*> simulationsNDSimulationResults;
	ResultsProcessor rp;
	auto auxSim = dynamic_cast<MontecarloCriticalParameterNDParametersSweepSimulation*> (simulations->front());
	auto pFirstMagProfileCIMax = auxSim->get_out_profile_c_i_max();
	auto pFirstMagProfileCIMin = auxSim->get_out_profile_c_i_min();
	auto pFirstMagProfileCIMean = auxSim->get_out_profile_c_i_mean();
	auto pFirstMagGPCIMax= auxSim->get_out_p_c_i_max();
	auto pFirstMagGPCIMin= auxSim->get_out_p_c_i_min();
	auto pFirstMagGPCIMean= auxSim->get_out_p_c_i_mean();

	for( auto const &s : *simulations ){
		auto pMCPNDPSS = dynamic_cast<MontecarloCriticalParameterNDParametersSweepSimulation*>(s);
		paths.insert( std::make_pair(
			(pMCPNDPSS->get_main_nd_simulation_results())->get_general_data_path(), pMCPNDPSS->get_simulation_id() ) );
		simulationsNDSimulationResults.push_back( pMCPNDPSS->get_main_nd_simulation_results() );
	}
	// process results
	partialResult = partialResult && rp.StatisticProcessStatisticsFiles(
		&paths, outputFilePath, std::move(*pFirstMagProfileCIMax), std::move(*pFirstMagProfileCIMin), std::move(*pFirstMagProfileCIMean) );
	// plot it!
	if( partialResult ){
		SimulationParameter* criticalParameter = nullptr;
		std::vector<Metric*>* auxMetrics = nullptr;
		// # note that each computed column will have max, min and mean values, including critical parameter
		// #profCount #Profile #Qcoll #MAG_i_name #MAG_i_maxErrorGlobal #MAG_i_maxErrorMetric
		unsigned int critParamOffset = auxSim->get_out_gnuplot_crit_param_offset();
		unsigned int profileFirstMagMetricOffset =  auxSim->get_out_profile_gnuplot_first_mag_metric_offset();
		unsigned int profileFirstMagGlobalOffset =  auxSim->get_out_profile_gnuplot_first_mag_global_offset();
		unsigned int planeFirstMagMetricOffset =  auxSim->get_out_plane_gnuplot_first_mag_metric_offset();
		unsigned int planeFirstMagGlobalOffset =  auxSim->get_out_plane_gnuplot_first_mag_global_offset();
		unsigned int dataColumnsPerMetric =  auxSim->get_out_data_per_metric_per_line(); // 1+6+3
		// aux variables
		criticalParameter = (* simulations->begin())->get_golden_critical_parameter();
		// aux mags
		auxMetrics = ((* simulations->begin())->get_golden_metrics_structure())->GetMetricsVector( 0 );
		if( criticalParameter == nullptr || auxMetrics==nullptr ){
			log_io->ReportError2AllLogs( "null criticalParameter or auxMetrics");
			return false;
		}
		// create gnuplot scatter map graphs
		partialResult = partialResult && PlotMontecarloCriticalParameterNDParametersSweepMode(
			*auxMetrics, critParamOffset, profileFirstMagMetricOffset, profileFirstMagGlobalOffset,
			dataColumnsPerMetric, *criticalParameter, outputFilePath);
		// planes
		auto auxSim = dynamic_cast<MontecarloCriticalParameterNDParametersSweepSimulation*> (simulations->front());
		auto simulatedParameters = simulations->front()->get_simulation_parameters();

		partialResult = partialResult && GenerateAndPlotParameterPairResults(
			ResultsProcessor::kStatisticProcessStatisticsFiles, auxSim->get_golden_critical_parameter()->get_title_name(),
			critParamOffset, planeFirstMagMetricOffset, planeFirstMagGlobalOffset, dataColumnsPerMetric, *auxMetrics, *simulatedParameters,
			std::move(*pFirstMagGPCIMax), std::move(*pFirstMagGPCIMin), std::move(*pFirstMagGPCIMean), simulationsNDSimulationResults );
	}
	// // fgarcia
	// // critical parameter value for each Scenario (mean between profiles)
	return partialResult;
}

bool GlobalResults::PlotMontecarloCriticalParameterNDParametersSweepMode(
		const std::vector<Metric*>& analyzedMetrics, const unsigned int &critParamOffset,
		const unsigned int& profileFirstMagMetricOffset, const unsigned int& profileFirstMagGlobalOffset,
		const unsigned int& dataColumnsPerMetric,
		const SimulationParameter& criticalParameter, const std::string& gnuplotDataFile ){
	int partialResult = 0;
	bool correctlyExported = true;
	// Critical parameter
	// Files
	std::string generalGSFPath = gnuplot_script_folder + kFolderSeparator + "scenarios_general" + kGnuPlotScriptSufix;
	std::string generalIPath = images_folder + kFolderSeparator + "scenarios_general" + kSvgSufix;
	std::string title = "[Scenarios General] " + criticalParameter.get_title_name();
	// Generate scripts
	std::ofstream generalGSF;
	try{
		generalGSF.open( generalGSFPath.c_str() );
		// Svg
		generalGSF << "set term svg noenhanced size " << kSvgImageWidth << ","<< kSvgImageHeight
			 << " fname " << kSvgFont << "\n";
		generalGSF << "set output \"" << generalIPath << "\"\n";
		generalGSF << "set title \" " << title
			 << " vs " << criticalParameter.get_title_name() << " \"\n";
		generalGSF << "set grid\n";
		// Axis
		if( criticalParameter.get_value_change_mode()!=kSPLineal ){
			 generalGSF << "set logscale y\n";
		}
		generalGSF << "set format x \"%g\"\n";
		generalGSF << "set format y \"%g\"\n";
		generalGSF << "set xlabel \""  << "Profile" << "\"\n";
		generalGSF << "set ylabel \""  << criticalParameter.get_title_name() << "\"\n";
		// Color Paletes
		generalGSF << kProfilesPalette;
		// # remove border on top and right and set color to gray
		generalGSF << kCustomBorders;
		// line style
		generalGSF <<  kTransientSimilarLinesPalette;
		// legend
		generalGSF <<  "set key bottom right\n";
		generalGSF <<  "ntics = 10\n";
		// Plot
		generalGSF <<  "plot '" << gnuplotDataFile << "' u 1:" << (critParamOffset+2) << " w lp ls 2 title '" << criticalParameter.get_title_name() <<  "', \\\n";
		generalGSF <<  "     '" << gnuplotDataFile << "' u 1:" << (critParamOffset+2) << ":" << (critParamOffset+1) <<  ":" << (critParamOffset) << " w errorbars ls 1 notitle\n";
		generalGSF <<  "set xtics ntics\n";
		generalGSF << "unset output\n";
		// close file
		generalGSF << "quit\n";
		generalGSF.close();
		// Exec comand
	}catch (std::exception const& ex) {
		std::string ex_what = ex.what();
		log_io->ReportError2AllLogs( "Exception while parsing the file: ex-> " + ex_what );
		correctlyExported = false;
	}
	generalGSF.close();
	// Exec comand
	std::string execCommand = kGnuplotCommand + generalGSFPath + kGnuplotEndCommand;
	partialResult += std::system( execCommand.c_str() );
	// plot metrics
	unsigned int magCount = 0;
	for( auto const &m : analyzedMetrics ){
		if( m->get_analyzable() ){
			// Files
			std::string gnuplotScriptFilePath = gnuplot_script_folder + kFolderSeparator
				+ "scenarios_general_mag_" + number2String(magCount) + kGnuPlotScriptSufix;
			std::string outputImagePath = images_folder + kFolderSeparator
				+ "scenarios_general_mag_" + number2String(magCount) + kSvgSufix;
			std::string title = "[General] " + m->get_title_name() + " errors & " + criticalParameter.get_title_name() + " Statistics";
			// Generate scripts
			std::ofstream gnuplotScriptFile;
			try{
				gnuplotScriptFile.open( gnuplotScriptFilePath.c_str() );
				// Svg
				gnuplotScriptFile << "set term svg noenhanced size " << kSvgImageWidth << ","<< kSvgImageHeight
					<< " fname " << kSvgFont << "\n";
				gnuplotScriptFile << "set output \"" << outputImagePath << "\"\n";
				gnuplotScriptFile << "set title \" " << title << " \"\n";
				gnuplotScriptFile << "set grid\n";
				// Axis
				gnuplotScriptFile << "set format x \"%g\"\n";
				gnuplotScriptFile << "set format y \"%g\"\n";
				gnuplotScriptFile << "set format y2 \"%g\"\n";
				gnuplotScriptFile << "set xlabel \"Profile\"\n";
				gnuplotScriptFile << "set y2label \"" << criticalParameter.get_title_name() << " \"\n";
				gnuplotScriptFile << "set ylabel \"Error in metric "  << m->get_title_name() << "\"\n";
				// # remove border on top and right and set color to gray
				gnuplotScriptFile << kCustomBorders;
				gnuplotScriptFile << "set y2tics\n";
				// Color Paletes
				gnuplotScriptFile << kProfilesPalette;
				int magDataMetricIndex = profileFirstMagMetricOffset + dataColumnsPerMetric*magCount; // title
				int magDataGlobalIndex = profileFirstMagGlobalOffset + dataColumnsPerMetric*magCount; // title
				// crit param
				gnuplotScriptFile <<  "plot '" << gnuplotDataFile << "' using 1:" << (critParamOffset) << " axis x1y2 with filledcurve x1 ls 4 title '" << criticalParameter.get_title_name() << "_max', \\\n";
				gnuplotScriptFile <<  " '" << gnuplotDataFile << "' using 1:" << (critParamOffset+2) << " axis x1y2 with filledcurve x1 ls 6 title '" << criticalParameter.get_title_name() << "_mean', \\\n";
				gnuplotScriptFile <<  " '" << gnuplotDataFile << "' using 1:" << (critParamOffset+1) << " axis x1y2 with filledcurve x1 ls 5 title '" << criticalParameter.get_title_name() << "_min', \\\n";
				// candlesticks  # Data columns: X Min 1stQuartile Median 3rdQuartile Max
				// metric
				gnuplotScriptFile <<  "     '" << gnuplotDataFile << "' u 1:" << (magDataMetricIndex+2)  <<  ":" << (magDataMetricIndex+1) << ":" << magDataMetricIndex << " axis x1y1  w errorbars ls 1 notitle, \\\n";
				gnuplotScriptFile <<  "     '" << gnuplotDataFile << "' u 1:" << (magDataMetricIndex+2) << " axis x1y1  w lp ls 2 title '" << m->get_title_name() << " (max_error_metric)', \\\n";
				// global
				gnuplotScriptFile <<  "     '" << gnuplotDataFile << "' u 1:" << (magDataGlobalIndex+2)  <<  ":" << (magDataGlobalIndex+1) << ":" << magDataGlobalIndex << " axis x1y1  w errorbars ls 3 notitle, \\\n";
				gnuplotScriptFile <<  "     '" << gnuplotDataFile << "' u 1:" << (magDataGlobalIndex+2) << " axis x1y1  w lp ls 3 title '" << m->get_title_name() << " (max_error_global)'\n";
				// legend
				gnuplotScriptFile <<  "set key top left\n";

				gnuplotScriptFile << "unset output\n";
				// close file
				gnuplotScriptFile << "quit\n";
			}catch (std::exception const& ex) {
				std::string ex_what = ex.what();
				log_io->ReportError2AllLogs( "Exception while parsing the file: ex-> " + ex_what );
				correctlyExported = false;
			}
			gnuplotScriptFile.close();
			// Exec comand
			std::string execCommand = kGnuplotCommand + gnuplotScriptFilePath + kGnuplotEndCommand;
			partialResult += std::system( execCommand.c_str() );
			++magCount;
		}
	} // end of mags
	#ifdef RESULTS_POST_PROCESSING_VERBOSE
		if( partialResult>0 ){
			log_io->ReportPlain2Log("Gnuplot may have return warnings or errors in GlobalResults");
		}
	#endif

	return correctlyExported;
}

bool GlobalResults::GenerateAndPlotParameterPairResults(
		const unsigned int statisticMethod, const std::string& criticalParameterName,
		const unsigned int critParamOffset, const unsigned int magMetricColumnOffset, const unsigned int magGlobalColumnOffset,
		const unsigned int dataPerMetricPerLine, const std::vector<Metric*>& analyzedMetrics,
		const std::vector<SimulationParameter*>& simulationParameters,
		const std::vector<unsigned int>&& magGPCIMax, const std::vector<unsigned int>&& magGPCIMin,
		const std::vector<unsigned int>&& magGPCIMean, const std::vector<NDSimulationResults*>& simulationsNDSimulationResults ){
	bool correctlyPlotted = true;
	std::vector<SimulationParameter*> parameters2sweep;
	for( auto const &p : simulationParameters ){
		if( p->get_allow_sweep() ){
			parameters2sweep.push_back( p );
		}
	}
	log_io->ReportGreenStandard( "Generating Summary 3d p1 vs p2 planes. Parameters2sweep size: " + number2String(parameters2sweep.size()));

	std::string planesMapsFolder;
	std::string planesGnuplotScriptFolder;
	std::string planesImagesFolder;
	std::string generalParameterResultsFile;
	std::map<std::string,std::string> planeStructurePaths;
	// planeStructurePaths.reserve( simulationsNDSimulationResults.size() );
	// fgarcia:
	auto auxNDSimulationResults = simulationsNDSimulationResults.front();
	unsigned int planeCount = 0;
	ResultsProcessor rp;
	std::vector<std::string> paramTokens;
	std::string planeId;
	for ( auto const &planeStructure : * auxNDSimulationResults->get_plane_results_structures() ){
		// create folders
		planesMapsFolder.clear();
		planesGnuplotScriptFolder.clear();
		planesImagesFolder.clear();
		planeId = planeStructure->get_plane_id();
		planesMapsFolder =  data_folder + kFolderSeparator + planeId;
		planesGnuplotScriptFolder =  gnuplot_script_folder + kFolderSeparator + planeId;
		planesImagesFolder = images_folder + kFolderSeparator + planeId;
		if( !CreateFolder(planesMapsFolder, true ) || !CreateFolder(planesImagesFolder, true ) || !CreateFolder(planesGnuplotScriptFolder, true ) ){
			log_io->ReportError2AllLogs( k2Tab + "-> Error creating folders: '" + planesMapsFolder + " and " + planesImagesFolder + "'." );
			log_io->ReportError2AllLogs( "Error GenerateAndPlotResults" );
			return false;
		}
		// process statistics
		generalParameterResultsFile.clear();
		generalParameterResultsFile = planesMapsFolder + kFolderSeparator +  planeId + "_general_scenarios" + kDataSufix;
		planeStructurePaths.clear();
		for( auto const &pNDSR : simulationsNDSimulationResults ){
			planeStructurePaths.insert( std::make_pair(
				pNDSR->GetPlaneResultsStructure(planeCount)->get_general_data_path(), pNDSR->GetPlaneResultsStructure(planeCount)->get_plane_id()) );
		}
		++planeCount;
		// generate data
		switch( statisticMethod ){
			case ResultsProcessor::kMeanProcessResultsFiles: {
				correctlyPlotted = correctlyPlotted && rp.MeanProcessResultsFiles(
					&planeStructurePaths, generalParameterResultsFile, std::move(magGPCIMean));
			}
			break;
			case ResultsProcessor::kStatisticProcessResultsFiles: {
				correctlyPlotted = correctlyPlotted && rp.StatisticProcessResultsFiles(
					&planeStructurePaths, generalParameterResultsFile, std::move(magGPCIMean));
			}
			break;
			case ResultsProcessor::kStatisticProcessStatisticsFiles: {
				correctlyPlotted = correctlyPlotted && rp.StatisticProcessStatisticsFiles(
					&planeStructurePaths, generalParameterResultsFile,
					std::move(magGPCIMax) , std::move(magGPCIMin), std::move(magGPCIMean));
			}
			break;
			default:{
				log_io->ReportError2AllLogs( "Unexpected statisticMethod variable: " + number2String(statisticMethod) );
				correctlyPlotted = false;
			}
			break;
		}
		paramTokens.clear();
		boost::split( paramTokens, planeId, boost::is_any_of("_"), boost::token_compress_on );
		if( paramTokens.size()!=2 ){
			log_io->ReportError2AllLogs( planeId + "paramTokens error" );
			break;
		}
		auto p1 = parameters2sweep.at( std::atoi( paramTokens.at(0).c_str() ) );
		auto p2 = parameters2sweep.at( std::atoi( paramTokens.at(1).c_str() ) );
		// metrics
		int gnuplotResult = GnuplotCriticalParameterValuePlane(
			critParamOffset, criticalParameterName, *p1, *p2, planeId,
			generalParameterResultsFile, planesGnuplotScriptFolder, planesImagesFolder );
		gnuplotResult += GnuplotPlaneMetricResults(
			magMetricColumnOffset, magGlobalColumnOffset, dataPerMetricPerLine, analyzedMetrics,
			*p1, *p2, planeId, generalParameterResultsFile, planesGnuplotScriptFolder, planesImagesFolder );
		if( gnuplotResult > 0 ){
			log_io->ReportError2AllLogs( "Unexpected gnuplot result: " + number2String(gnuplotResult) );
		}

	} // end of p1 vs p2 3d plot
	return correctlyPlotted;
}

int GlobalResults::GnuplotCriticalParameterValuePlane(
	const unsigned int critParamOffset, const std::string& criticalParameterName,
	const SimulationParameter& p1, const SimulationParameter& p2,
	const std::string& partialPlaneId, const std::string& gnuplotDataFile,
	const std::string& gnuplotScriptFolder, const std::string& imagesFolder ){
	// Files
	std::string gnuplotScriptFilePath = gnuplotScriptFolder + kFolderSeparator + partialPlaneId + "_" + kGnuPlotScriptSufix;
	std::string outputImagePath = imagesFolder + kFolderSeparator + partialPlaneId + "_3D" + kSvgSufix;
	// Generate scripts
	std::ofstream gnuplotScriptFile;
	gnuplotScriptFile.open( gnuplotScriptFilePath.c_str() );
	gnuplotScriptFile << "set term svg noenhanced size " << kSvgImageWidth << ","<< kSvgImageHeight << " fname " << kSvgFont << "\n";
	gnuplotScriptFile << "set output \"" << outputImagePath << "\"\n";

	gnuplotScriptFile << "set grid\n";
	// Axis
	if( p1.get_value_change_mode()!=kSPLineal ){
		gnuplotScriptFile << "set logscale x\n";
	}
	if( p2.get_value_change_mode()!=kSPLineal ){
		gnuplotScriptFile << "set logscale y\n";
	}
	gnuplotScriptFile << "set format x \"%g\"\n";
	gnuplotScriptFile << "set format y \"%g\"\n";
	gnuplotScriptFile << "set xlabel \""  << p1.get_title_name() << "\"\n";
	gnuplotScriptFile << "set ylabel \""  << p2.get_title_name() << "\"\n";
	gnuplotScriptFile << "set zlabel \"" << criticalParameterName << "\" rotate by 90\n";
	gnuplotScriptFile << "set title \"" << criticalParameterName << ", " << partialPlaneId << " \"\n";
	// Color Paletes
	gnuplotScriptFile << kUpsetsPalette;
	gnuplotScriptFile << kTransientSimilarLinesPalette;
	gnuplotScriptFile << k3DProperties;

	gnuplotScriptFile << "splot '" << gnuplotDataFile << "' u 1:2:" << (critParamOffset) << " title 'max_" << criticalParameterName << "' w lp ls 1, \\\n";
	gnuplotScriptFile << " '" << gnuplotDataFile << "' u 1:2:" << (critParamOffset+1) << " title 'min_" << criticalParameterName << "' w lp ls 1, \\\n";
	gnuplotScriptFile << " '" << gnuplotDataFile << "' u 1:2:" << (critParamOffset+2) << " title 'mean_" << criticalParameterName << "' " << kElegantLine3D << " w pm3d\n";

	gnuplotScriptFile << "unset output\n";
	// close file
	gnuplotScriptFile << "quit\n";
	gnuplotScriptFile.close();

	// Exec comand
	std::string execCommand = kGnuplotCommand + gnuplotScriptFilePath + kGnuplotEndCommand;
	return std::system( execCommand.c_str() );
}

int GlobalResults::GnuplotPlaneMetricResults(
	const unsigned int magMetricColumnOffset,
	const unsigned int magGlobalColumnOffset,
	const unsigned int dataPerMetricPerLine,
	const std::vector<Metric*>& analyzedMetrics,
	const SimulationParameter& p1, const SimulationParameter& p2,
	const std::string& partialPlaneId, const std::string& gnuplotDataFile,
	const std::string& gnuplotScriptFolder, const std::string& imagesFolder ){
	unsigned int magCount = 0;
	int partialResults = 0;
	for( auto const &m : analyzedMetrics ){
		if( m->get_analyzable() ){
			// Files
			std::string gnuplotScriptFilePath = gnuplotScriptFolder + kFolderSeparator
				 + partialPlaneId + "_mag_" + number2String(magCount) + "_" + kGnuPlotScriptSufix;
			std::string outputImagePath = imagesFolder + kFolderSeparator
				 + partialPlaneId + "_mag_" + number2String(magCount) + "_3D" + kSvgSufix;
			std::string title = m->get_title_name() + ", " + partialPlaneId;
			// Generate scripts
			std::ofstream gnuplotScriptFile;
			gnuplotScriptFile.open( gnuplotScriptFilePath.c_str() );
			gnuplotScriptFile << "set term svg noenhanced size " << kSvgImageWidth << ","<< kSvgImageHeight << " fname " << kSvgFont << "\n";
			gnuplotScriptFile << "set output \"" << outputImagePath  << "\"\n";
			gnuplotScriptFile << "set grid\n";
			// Axis
			if( p1.get_value_change_mode()!=kSPLineal ){
				gnuplotScriptFile << "set logscale x\n";
			}
			if( p2.get_value_change_mode()!=kSPLineal ){
				gnuplotScriptFile << "set logscale y\n";
			}
			gnuplotScriptFile << "set format x \"%g\"\n";
			gnuplotScriptFile << "set format y \"%g\"\n";
			gnuplotScriptFile << "set xlabel \"" << p1.get_title_name() << "\"\n";
			gnuplotScriptFile << "set ylabel \"" << p2.get_title_name() << "\"\n";
			gnuplotScriptFile << "set zlabel \"" << m->get_title_name() << "\" rotate by 90\n";
			// Format
			gnuplotScriptFile << "set format cb \"%g\"\n";
			gnuplotScriptFile << "set title \"" << title << " \"\n";
			// Color Paletes
			gnuplotScriptFile << kUpsetsPalette ;
			gnuplotScriptFile << kTransientSimilarLinesPalette;
			gnuplotScriptFile << k3DProperties;

			int magMetricDataIndex = magMetricColumnOffset + dataPerMetricPerLine*magCount; // max val
			int magGlobalDataIndex = magGlobalColumnOffset + dataPerMetricPerLine*magCount; // max val

			gnuplotScriptFile << "set multiplot layout 2, 1 \n";

			gnuplotScriptFile << "splot '" << gnuplotDataFile << "' u 1:2:" << (magMetricDataIndex+1) << " notitle w lp ls 2, \\\n";
			gnuplotScriptFile << " '" << gnuplotDataFile << "' u 1:2:" << (magMetricDataIndex) << " notitle w lp ls 1, \\\n";
			gnuplotScriptFile << " '" << gnuplotDataFile << "' u 1:2:" << (magMetricDataIndex+2) << " title 'max_err_metric_" << m->get_title_name() << "' " << kElegantLine3D << " w pm3d\n";
			gnuplotScriptFile << "# Uncomment for global error \n";
			gnuplotScriptFile << "splot '" << gnuplotDataFile << "' u 1:2:" << (magGlobalDataIndex+1) << " notitle w lp ls 2, \\\n";
			gnuplotScriptFile << " '" << gnuplotDataFile << "' u 1:2:" << (magGlobalDataIndex) << " notitle w lp ls 1, \\\n";
			gnuplotScriptFile << " '" << gnuplotDataFile << "' u 1:2:" << (magGlobalDataIndex+2) << " title 'max_err_global_" << m->get_title_name() << "' " << kElegantLine3D << " w pm3d\n";

			gnuplotScriptFile << "unset multiplot\n";
			// close file
			gnuplotScriptFile << "quit\n";
			gnuplotScriptFile.close();

			// Exec comand
			std::string execCommand = kGnuplotCommand + gnuplotScriptFilePath + kGnuplotEndCommand;
			partialResults += std::system( execCommand.c_str() );
			// update counter
			++magCount;
		}
	}
	return partialResults;
}
