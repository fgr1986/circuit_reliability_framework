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
		// case kMontecarloCriticalParameterNDParametersSweepMode: {
		// 		if( critical_parameter==nullptr ){
		// 			log_io->ReportError2AllLogs( "This mode does not allow a nullptr critical parameter" );
		// 			return false;
		// 		}
		// 		sss = new MontecarloCriticalParameterNDParametersSweepSimulation();
		// 		MontecarloCriticalParameterNDParametersSweepSimulation* pMCPNDPSS =
		// 			dynamic_cast<MontecarloCriticalParameterNDParametersSweepSimulation*>(sss);
		// 		// get numruns and set it to montecarlo_iterations
		// 		pMCPNDPSS->set_montecarlo_iterations( montecarlo_iterations );
		// 		pMCPNDPSS->set_max_parallel_profile_instances( max_parallel_profile_instances );
		// 		pMCPNDPSS->set_plot_last_transients( plot_last_transients );
		// 		pMCPNDPSS->set_max_parallel_montecarlo_instances( max_parallel_montecarlo_instances );
		// 	}
		// break;
		default: {
			log_io->ReportRedStandard("[GlobalResults] default mode:" + number2String(simulation_mode->get_id()) );
			return false;
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
	std::vector<Magnitude*>* auxMagnitudes = nullptr;
	unsigned int firstMagOffset = 9;
	unsigned int critParamOffset = 8;
	unsigned int dataPerMagnitudePerLine = 3;
	try{
		outputFile.open( outputFilePath );
		if (!outputFile){
			log_io->ReportError2AllLogs( kTab + "error writing " + top_folder + "/" + kResultsFolder + "/" + kSummaryFile.c_str());
			return false;
		}
		// aux variables
		criticalParameter = (* simulations->begin())->get_golden_critical_parameter();
		// aux mags
		auxMagnitudes = ((* simulations->begin())->get_golden_magnitudes_structure())->GetMagnitudesVector( 0 );
		if( criticalParameter == nullptr || auxMagnitudes==nullptr ){
			log_io->ReportError2AllLogs( "null criticalParameter or auxMagnitudes");
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
			auto magErrors = pCPVS->get_last_valid_transient_simulation_results()->get_magnitudes_errors();
			for( auto const &m : *magErrors ){
				outputFile << " " << m->get_magnitude_name() << " " << m->get_max_abs_error_global()
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
		correctlyExported = correctlyExported & PlotCriticalParameterValueSimulationMode(
			*auxMagnitudes, critParamOffset, firstMagOffset,
			dataPerMagnitudePerLine, *criticalParameter, outputFilePath);
	}
	// fgarcia, is it already finished?
	return correctlyExported;
}

bool GlobalResults::PlotCriticalParameterValueSimulationMode(
		const std::vector<Magnitude*>& analyzedMagnitudes, const unsigned int &critParamOffset,
		const unsigned int& firstMagOffset, const unsigned int& dataPerMagnitudePerLine,
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
		generalGSF << "set term svg  size " << kSvgImageWidth << ","<< kSvgImageHeight << " fname " << kSvgFont << std::endl;
		generalGSF << "set output \"" << generalIPath << "\"" << std::endl;
		generalGSF << "set title \" " << title << " \"" << std::endl;
		generalGSF << "set grid" << std::endl;
		// Axis
		generalGSF << "set format x \"%g\"" << std::endl;
		generalGSF << "set format y \"%g\"" << std::endl;
		generalGSF << "set xlabel \"Scenario\"" << std::endl;
		generalGSF << "set ylabel \""  << criticalParameter.get_title_name() << "\"" << std::endl;
		// # remove border on top and right and set color to gray
		generalGSF << "set style line 11 lc rgb '#808080' lt 1" << std::endl;
		generalGSF << "set border 3 back ls 11" << std::endl;
		generalGSF << "set tics nomirror" << std::endl;
		generalGSF << "set y2tics" << std::endl;
		// palete range
		generalGSF << kUpsetsPalette << std::endl;
		// line style
		generalGSF <<  "set style line 1 lc rgb '#ff3a00' lt 1 lw 1 pt 7 ps 1  # --- red" << std::endl;
		generalGSF <<  "set style line 2 lc rgb '#0060ad' lt 1 lw 3 pt 7 ps 1.5  # --- blue" << std::endl;
		generalGSF <<  "set style line 3 lc rgb '#ffd35a' lt 3 lw 1 pt 9 ps 1  # --- yellow" << std::endl;
		generalGSF <<  "set style line 4 lc rgb '#ff3a00' lt 3 lw 0 pt 9 ps 1  # --- red" << std::endl;
		generalGSF <<  "set style line 5 lc rgb '#666666' lt 3 lw 2 pt 9 ps 1  # --- grey " << std::endl;
		generalGSF <<  "set boxwidth 0.5 relative" << std::endl;
		// generalGSF <<  "set style fill transparent solid 0.7 noborder" << std::endl;
		generalGSF <<  "set style fill transparent solid 0.5" << std::endl;
		// // Background
		generalGSF << kWholeBackground << std::endl;
		// #altered_scenario_index #simulation_id #altered_element_path
		// #folder #correctlySimulated #affected_by_min_value #affected_by_min_value
	 	// #critical_parameter_value" << " #MAG_i_name #MAG_i_maxErrorGlobal #MAG_i_maxErrorMetric
		generalGSF <<  "plot '" << gnuplotDataFile << "' using 1:" << critParamOffset
			<< " axis x1y1 with lp ls 2 title '" << criticalParameter.get_title_name() << "'" << std::endl;
		// legend
		generalGSF <<  "set key top left" << std::endl;
		generalGSF << "unset output" << std::endl;
		// close file
		generalGSF << "quit" << std::endl;
	}catch (std::exception const& ex) {
		std::string ex_what = ex.what();
		log_io->ReportError2AllLogs( "Exception while parsing the file: ex-> " + ex_what );
		correctlyExported = false;
	}
	generalGSF.close();
	// Exec comand
	std::string execCommand = kGnuplotCommand + generalGSFPath + kGnuplotEndCommand;
	partialResult += std::system( execCommand.c_str() );
	// plot magnitudes
	unsigned int magCount = 0;
	for( auto const &m : analyzedMagnitudes ){
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
				gnuplotScriptFile << "set term svg  size " << kSvgImageWidth << ","<< kSvgImageHeight
					<< " fname " << kSvgFont << std::endl;
				gnuplotScriptFile << "set output \"" << outputImagePath << "\"" << std::endl;
				gnuplotScriptFile << "set title \" " << title << " \"" << std::endl;
				gnuplotScriptFile << "set grid" << std::endl;
				// Axis
				gnuplotScriptFile << "set format x \"%g\"" << std::endl;
				gnuplotScriptFile << "set format y \"%g\"" << std::endl;
				gnuplotScriptFile << "set format y2 \"%g\"" << std::endl;
				gnuplotScriptFile << "set xlabel \"Profile\"" << std::endl;
				gnuplotScriptFile << "set y2label \"" << criticalParameter.get_title_name() << " \%\"" << std::endl;
				gnuplotScriptFile << "set ylabel \"Error in magnitude "  << m->get_title_name() << "\"" << std::endl;
				// # remove border on top and right and set color to gray
				gnuplotScriptFile << "set style line 11 lc rgb '#808080' lt 1" << std::endl;
				gnuplotScriptFile << "set border 3 back ls 11" << std::endl;
				gnuplotScriptFile << "set tics nomirror" << std::endl;
				gnuplotScriptFile << "set y2tics" << std::endl;
				// palete range
				// Color Paletes
				gnuplotScriptFile << kUpsetsPalette << std::endl;
				// line style
				gnuplotScriptFile <<  "set style line 1 lc rgb '#ff3a00' lt 1 lw 1 pt 7 ps 1  # --- red" << std::endl;
				gnuplotScriptFile <<  "set style line 2 lc rgb '#0060ad' lt 1 lw 3 pt 7 ps 1.5  # --- blue" << std::endl;
				gnuplotScriptFile <<  "set style line 3 lc rgb '#ffd35a' lt 3 lw 1 pt 9 ps 1  # --- yellow" << std::endl;
				gnuplotScriptFile <<  "set style line 4 lc rgb '#ff3a00' lt 3 lw 0 pt 9 ps 1  # --- red" << std::endl;
				gnuplotScriptFile <<  "set style line 5 lc rgb '#666666' lt 3 lw 2 pt 9 ps 1  # --- grey " << std::endl;
				gnuplotScriptFile <<  "set boxwidth 0.5 relative" << std::endl;
				// gnuplotScriptFile <<  "set style fill transparent solid 0.7 noborder" << std::endl;
				gnuplotScriptFile <<  "set style fill transparent solid 0.5" << std::endl;
				// // Background
				gnuplotScriptFile << kWholeBackground << std::endl;
				int magDataIndex = firstMagOffset + dataPerMagnitudePerLine*magCount; // title
				gnuplotScriptFile <<  "plot '" << gnuplotDataFile << "' using 1:"<< critParamOffset
					<<" axis x1y2 with filledcurve x1 ls 3 title '\% "
					<< criticalParameter.get_title_name() << "', \\" << std::endl;
				gnuplotScriptFile <<  "     '" << gnuplotDataFile << "' u 1:" << (magDataIndex+1) << " axis x1y1  w lp ls 1 title '"
					<< m->get_title_name() << "  (max\\_error\\_global)', \\" << std::endl;
				gnuplotScriptFile <<  "     '" << gnuplotDataFile << "' u 1:" << (magDataIndex+2) << " axis x1y1  w lp ls 2 title '"
					<< m->get_title_name() << "  (max\\_error\\_metric)'" << std::endl;
				// legend
				gnuplotScriptFile <<  "set key top left" << std::endl;

				gnuplotScriptFile << "unset output" << std::endl;
				// close file
				gnuplotScriptFile << "quit" << std::endl;
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
	ResultsProcessor rp;
	auto pFirstMagColumnIndexes= ( dynamic_cast<CriticalParameterNDParameterSweepSimulation*>(
		(*simulations->begin())) )->get_magnitude_column_indexes();
	for( auto const &s : *simulations ){
		auto pCPNDPSS = dynamic_cast<CriticalParameterNDParameterSweepSimulation*>(s);
		paths.insert( std::make_pair(
			(pCPNDPSS->get_main_nd_simulation_results())->get_general_data_path(), pCPNDPSS->get_simulation_id() ) );
	}
	// process results
	partialResult = partialResult && rp.StatisticProcessResultsFiles(
		&paths, outputFilePath, std::move(*pFirstMagColumnIndexes) );
	// plot it!
	if( partialResult ){
		SimulationParameter* criticalParameter = nullptr;
		std::vector<Magnitude*>* auxMagnitudes = nullptr;
		// # not that each computed column will have mean, max and min values
		// #profCount #Profile #Qcoll #MAG_i_name #MAG_i_maxErrorGlobal #MAG_i_maxErrorMetric
		unsigned int firstMagOffset = 6;
		unsigned int critParamOffset = 3;
		unsigned int dataPerMagnitudePerLine = 3;
		// aux variables
		criticalParameter = (* simulations->begin())->get_golden_critical_parameter();
		// aux mags
		auxMagnitudes = ((* simulations->begin())->get_golden_magnitudes_structure())->GetMagnitudesVector( 0 );
		if( criticalParameter == nullptr || auxMagnitudes==nullptr ){
			log_io->ReportError2AllLogs( "null criticalParameter or auxMagnitudes");
			return false;
		}
		// create gnuplot scatter map graphs
		partialResult = partialResult & PlotCriticalParameterNDParametersSweepSimulationMode(
			*auxMagnitudes, critParamOffset, firstMagOffset,
			dataPerMagnitudePerLine, *criticalParameter, outputFilePath);
	}
	// fgarcia
	// critical parameter value for each Scenario (mean between profiles)

	return partialResult;
}

bool GlobalResults::PlotCriticalParameterNDParametersSweepSimulationMode(
		const std::vector<Magnitude*>& analyzedMagnitudes, const unsigned int &critParamOffset,
		const unsigned int& firstMagOffset, const unsigned int& dataPerMagnitudePerLine,
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
		generalGSF << "set term svg  size " << kSvgImageWidth << ","<< kSvgImageHeight
			 << " fname " << kSvgFont << std::endl;
		generalGSF << "set output \"" << generalIPath << "\"" << std::endl;
		generalGSF << "set title \" " << title
			 << " vs " << criticalParameter.get_title_name() << " \"" << std::endl;
		generalGSF << "set grid" << std::endl;
		// Axis
		if( criticalParameter.get_value_change_mode()!=kSPLineal ){
			 generalGSF << "set logscale y" << std::endl;
		}
		generalGSF << "set format x \"%g\"" << std::endl;
		generalGSF << "set format y \"%g\"" << std::endl;
		// generalGSF << "set xlabel \""  << pairedParameter.get_title_name() << "\"" << std::endl;
		generalGSF << "set xlabel \""  << "Profile" << "\"" << std::endl;
		generalGSF << "set ylabel \""  << criticalParameter.get_title_name() << "\"" << std::endl;
		// Color Paletes
		generalGSF << kUpsetsPalette << std::endl;
		// # remove border on top and right and set color to gray
		generalGSF << "set style line 11 lc rgb '#808080' lt 1" << std::endl;
		generalGSF << "set border 3 back ls 11" << std::endl;
		generalGSF << "set tics nomirror" << std::endl;
		// line style
		generalGSF <<  "set style line 1 lc rgb '#8b1a0e' pt 1 ps 1 lt 1 lw 2 # --- red" << std::endl;
		generalGSF <<  "set style line 2 lc rgb '#5e9c36' pt 6 ps 1 lt 1 lw 2 # --- green" << std::endl;
		generalGSF <<  "set style fill solid" << std::endl;
		// // Background
		generalGSF << kWholeBackground << std::endl;
		// // ranges
		// generalGSF << "set yrange [" << criticalParameter.get_value_min() << ":" << criticalParameter.get_value_max() << "]" << std::endl;
		// generalGSF << "set xrange [" << pairedParameter.get_value_min() << ":" << pairedParameter.get_value_max() << "]" << std::endl;
		// legend
		generalGSF <<  "set key bottom right" << std::endl;
		generalGSF <<  "ntics = 10" << std::endl;
		// Plot
		generalGSF <<  "plot '" << gnuplotDataFile << "' u 1:" << critParamOffset << " w lp ls 2 title '"
			<< criticalParameter.get_title_name() <<  "', \\" << std::endl;
		generalGSF <<  "     '" << gnuplotDataFile << "' u 1:" << critParamOffset << ":"
			<< (critParamOffset+2) <<  ":" << (critParamOffset+1) << " w errorbars ls 1 notitle" << std::endl;
		generalGSF <<  "set xtics ntics" << std::endl;
		generalGSF << "unset output" << std::endl;
		// close file
		generalGSF << "quit" << std::endl;
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
	// plot magnitudes
	unsigned int magCount = 0;
	for( auto const &m : analyzedMagnitudes ){
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
				gnuplotScriptFile << "set term svg  size " << kSvgImageWidth << ","<< kSvgImageHeight
					<< " fname " << kSvgFont << std::endl;
				gnuplotScriptFile << "set output \"" << outputImagePath << "\"" << std::endl;
				gnuplotScriptFile << "set title \" " << title << " \"" << std::endl;
				gnuplotScriptFile << "set grid" << std::endl;
				// Axis
				gnuplotScriptFile << "set format x \"%g\"" << std::endl;
				gnuplotScriptFile << "set format y \"%g\"" << std::endl;
				gnuplotScriptFile << "set format y2 \"%g\"" << std::endl;
				gnuplotScriptFile << "set xlabel \"Profile\"" << std::endl;
				gnuplotScriptFile << "set y2label \"" << criticalParameter.get_title_name() << " \%\"" << std::endl;
				gnuplotScriptFile << "set ylabel \"Error in magnitude "  << m->get_title_name() << "\"" << std::endl;
				// # remove border on top and right and set color to gray
				gnuplotScriptFile << "set style line 11 lc rgb '#808080' lt 1" << std::endl;
				gnuplotScriptFile << "set border 3 back ls 11" << std::endl;
				gnuplotScriptFile << "set tics nomirror" << std::endl;
				gnuplotScriptFile << "set y2tics" << std::endl;
				// Color Paletes
				gnuplotScriptFile << kUpsetsPalette << std::endl;
				// line style
				gnuplotScriptFile <<  "set style line 1 lc rgb '#ffd35a' lt 3 lw 1 pt 9 ps 1  # --- yellow" << std::endl;
				gnuplotScriptFile <<  "set style line 2 lc rgb '#0060ad' lt 1 lw 3 pt 7 ps 1.5  # --- blue" << std::endl;
				gnuplotScriptFile <<  "set style line 3 lc rgb '#0060ad' lt 1 lw 1 pt 7 ps 1.5  # --- blue" << std::endl;
				gnuplotScriptFile <<  "set style line 4 lc rgb '#ff3a00' lt 1 lw 1 pt 7 ps 1  # --- red" << std::endl;
				// gnuplotScriptFile <<  "set style line 5 lc rgb '#ff3a00' lt 3 lw 0 pt 9 ps 1  # --- red" << std::endl;
				// gnuplotScriptFile <<  "set style line 6 lc rgb '#666666' lt 3 lw 2 pt 9 ps 1  # --- grey " << std::endl;
				gnuplotScriptFile <<  "set boxwidth 0.5 relative" << std::endl;
				// gnuplotScriptFile <<  "set style fill transparent solid 0.7 noborder" << std::endl;
				gnuplotScriptFile <<  "set style fill transparent solid 0.5" << std::endl;
				// // Background
				gnuplotScriptFile << kWholeBackground << std::endl;
				int magDataIndex = firstMagOffset + dataPerMagnitudePerLine*magCount; // title
				// crit param
				gnuplotScriptFile <<  "plot '" << gnuplotDataFile << "' using 1:"<< critParamOffset
					<<" axis x1y2 with filledcurve x1 ls 1 title '\% "
					<< criticalParameter.get_title_name() << "', \\" << std::endl;
				// mean of max errors_global in (mean) in magDataIndex+1
				gnuplotScriptFile <<  "     '" << gnuplotDataFile << "' u 1:" << (magDataIndex+1) << " axis x1y1  w lp ls 2 title '"
					<< m->get_title_name() << "  (max\\_error\\_global)', \\" << std::endl;
					gnuplotScriptFile <<  "     '" << gnuplotDataFile << "' u 1:" << (magDataIndex+1) << ":"
						<< (magDataIndex+3) <<  ":" << (magDataIndex+2) << " w errorbars ls 3 notitle, \\" << std::endl;
				// mean of max error_metric in (mean) in magDataIndex+4
				gnuplotScriptFile <<  "     '" << gnuplotDataFile << "' u 1:" << (magDataIndex+4) << ":"
						<< (magDataIndex+6) <<  ":" << (magDataIndex+5) << " w errorbars ls 4 notitle, \\" << std::endl;
				gnuplotScriptFile <<  "     '" << gnuplotDataFile << "' u 1:" << (magDataIndex+4) << " axis x1y1  w lp ls 4 title '"
						<< m->get_title_name() << "  (max\\_error\\_metric)'" << std::endl;
				// legend
				gnuplotScriptFile <<  "set key top left" << std::endl;

				gnuplotScriptFile << "unset output" << std::endl;
				// close file
				gnuplotScriptFile << "quit" << std::endl;
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
