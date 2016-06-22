#ifndef FILES_FOLDERS_IO_CONSTANTS_H
#define FILES_FOLDERS_IO_CONSTANTS_H

// Files
static const std::string kFileSeparator = "_";
static const std::string kFolderSeparator = "/";

static const std::string kMatlabScript = "matlab_test.m";
static const std::string kSpectreStandardLogsFile = "spectre_standard_out.log";
static const std::string kReadMeFile = "readme.txt";
static const std::string kErrorFile = "rs_error.log";
static const std::string kDependencyHTMLFile = "statement_dependency.html";
static const std::string kHierarchyHTMLFile = "statements_hierarchy.html";
static const std::string kSimulationResultsHTMLFile = "simulation_results.html";
static const std::string kSummaryHTMLFile = "summary.html";
static const std::string kLogFile = "rs_log.log";
static const std::string kLogFileResults = "rs_results.log";
static const std::string kSpectreLogFile = "spectre_log.log";
static const std::string kTransientSufix = ".tran";
static const std::string kMCDATASufix = ".mcdata";
static const std::string kProcessedPrefix = "processed_";

static const std::string kDataSufix = ".data";
static const std::string kGnuPlotScriptSufix = ".gnuplot";
static const std::string kCsvSufix = ".csv";
static const std::string kSvgSufix = ".svg";

static const std::string kScenariosSummaryFile = "scenarios_list.txt";
static const std::string kSummaryFile = "main_summary.data";
static const std::string kProfilesSummaryFile = "profiles_list.txt";

static const std::string kProcessedTransientFile = "processed" + kDataSufix;
static const std::string kGnuplotTransientSVGSufix = "_tran" + kSvgSufix;
// Simulation Folders
static const std::string kSimulationsFolder = "simulations";
static const std::string kIntermediateSimulationsFolder = "intermediate_values";
static const std::string kCompiledComponentsFolder = "ahdl_compiled_components";
static const std::string kSpectreResultsFolder = "spectre_simulation";
static const std::string kAlterationSubcircuitAHDLFolder = "altered_scenarios_compiled_components_circuit";
static const std::string kGoldenFolder = "golden";
static const std::string kXMLFolder = "xml";
static const std::string kAlterationScenariosFolder = "altered_scenarios";
static const std::string kAlterationInjectorName = "radiation_source_injector";
static const std::string kAlterationScenarioFolderPrefix = "altered_scenario";
static const std::string kAlterationSourceCircuitFolder = "radiation_source_circuit";

// Netlist files
static const std::string kAlterationSubcircuitAHDLFile = kAlterationSubcircuitAHDLId + kCircutFileSufix;
static const std::string kAlterationSourceNetlistFile = kAlterationSourcesCircuitStringId + kCircutFileSufix;
static const std::string kAlteredStatementsNetlistFile = kAlteredStatementsCircuitStringId + kCircutFileSufix;
static const std::string kMainNetlistFile = kMainCircuitStringId + kCircutFileSufix;
static const std::string kParametersCircuitFile = kParametersCircuitStringId + kCircutFileSufix;

// Transient export formats
static const int kMatlab = 0;
static const int kGnuPlot = 1;
static const int kCSV = 2;

// results
static const std::string kResultsFolder = "results";

// processed data (summary, sweep and montecarlo results)
// transients are stored at each spectre_simulation folder
static const std::string kResultsDataFolder = "generated_data";
// gnuplot_scripts
static const std::string kResultsGnuplotScriptsFolder = "generated_gnuplot_scripts";
// images
static const std::string kResultsImagesFolder = "generated_images";
// sub_folders
static const std::string kSummaryResultsFolder = "summary_results";
static const std::string kCriticalParameterNDParametersSweepResultsFolder = "critical_parameter_n_d_parameters_sweep_results";
static const std::string kMontecarloNDParametersSweepResultsFolder = "montecarlo_n_d_parameters_sweep_results";
static const std::string kMontecarloNDParametersSweepResultsFolderSubProfiles = kMontecarloNDParametersSweepResultsFolder + "/profiles";
static const std::string kMontecarloCriticalParameterNDParametersSweepResultsFolder = "montecarlo_critical_parameter_n_d_parameters_sweep_results";
static const std::string kMontecarloCriticalParameterNDParametersSweepResultsFolderSubProfiles = kMontecarloCriticalParameterNDParametersSweepResultsFolder + "/profiles";
static const std::string kTransientResultsFolder = "trans_results";
static const std::string kCriticalParameterEvolutionFolder = "critical_parameter_evolution_results";
// sub_prefix
// sweep
static const std::string kSummarySweepStatsFilePrefix = "stats_map";
static const std::string kSummarySweepScenariosFilePrefix = "scenarios_map";
// full and montecarlo
static const std::string kSummaryFullSweepMaxMapFilePrefix = "max_map";
static const std::string kSummaryFullSweepMeanMapFilePrefix = "mean_map";
static const std::string kSummaryFullSweepScenariosFilePrefix = "scenarios_map";
// montecarlo
static const std::string kSummaryMontecarloSweepMaxMapFilePrefix = "max_map";
static const std::string kSummaryMontecarloSweepMeanMapFilePrefix = "mean_map";
static const std::string kSummaryMontecarloSweepScenariosFilePrefix = "scenarios_map";


// html5
static const std::string kResultsHtmlFolder = "generated_html";
// from the html file
static const std::string kHtmlUpFolders2Top = "../../../";
static const std::string kHtmlResources = "../html_resources";
static const std::string kHTML5HeadFile = kHtmlResources + kFolderSeparator + "kHTML5Head.txt";
static const std::string kHTML5BodyStartFile = kHtmlResources + kFolderSeparator + "kHTML5BodyStart.txt";
static const std::string kHTML5BodyEndFile = kHtmlResources + kFolderSeparator + "kHTML5BodyEnd.txt";

// latex
static const std::string kLatexTmp = "generated_latex_tmp";
static const std::string kResultsLatexFolder = "generated_doc";
static const std::string kLatexResources = "../latex_resources";
static const std::string kDefaultLatexSummaryPath = "summary.tex";
// from where the program is run
// static const std::string kLatexUpFolders2Top = "../../../";
static const std::string kLatexUpFolders2Top = "";
static const std::string kLatexOutputImagesSufix = ".pdf";
static const std::string kLatexOutputSufix = ".tex";
static const std::string kLatexMainFile = "main.tex";
static const std::string kLatexPDFFile = "main.pdf";
static const std::string kLatexCircuitAnalysisFile ="circuit_analysis.tex";
static const std::string kLatexSimulationResultsFile ="simulation_results.tex";
static const std::string kLatexSimulationResultsAppendixFile ="simulation_results_appendix.tex";
static const std::string kLatexDefinitionsFile ="definitions.tex";

#endif /* FILES_FOLDERS_IO_CONSTANTS_H */
