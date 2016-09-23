#To do list **Milestone 4.x**

##Major

* **Required Mixed processing for MCND and MCCritND**
* **To do: Latex/Html export**
* **¿TO DO? Regex for large components (ports)**
* **Instance. Nodes instead absolute nodes**
* Weighted Nodes

##Minor
* If (obj) delete obj (test if the object exist before the deletion)
* New conditional parsing scheme: REGEX:
	* if () statement
	* if () { statements }
	* if () statement elseif () statement
	* if () { statements } elseif () statement
	* if () { statements } elseif () { statementd }
	* if () { statements } else { statementd }
	* if () { statements } elseif () { statements } else {statements}
	* if () { statements } elseif () { statements } else statement
	* if () { statements } else { statementd }
* New model parsing scheme: Group of models:

# Changelog of version 3.3.0
* Parameter design space exploration ( 10^ is not e^!!) has been revised.
* Improved **MontecarloNDParametersSweepSimulation**
* **OceanEvalMetrics** generate maps with both error and VALUES (*MetricErrors metric_value* takes *OceanEvalMetric value*).
* *metric_montecarlo_results_t* includes **OceanEvalMetrics** values

# Version 3.x.y

## Version 3.2.y

* Changelog of version 3.3.0
	* Parameter design space exploration ( 10^ is not e^!!) has been revised.
	* Improved **MontecarloNDParametersSweepSimulation**
	* **OceanEvalMetrics** generate maps with both error and VALUES (*MetricErrors metric_value* takes *OceanEvalMetric value*).
	* *metric_montecarlo_results_t* includes **OceanEvalMetrics** values

* Changelog of version 3.2.2
	* Summary results processor bug fixes.

* Changelog of version 3.2.1
	* Metric error logging has been improved.
	* 3D plot improvements

* Changelog of version 3.2.0
	* **GoldenNDParametersSweepSimulation** now exclusively simulate those GoldenSimulation that are required.
	* Critical parameter evolution plotting is controlled by **plot_critical_parameter_value_evolution**.
	* 3D plot Improvements

## Version 3.1.y

* Changelog of version 3.1.3
	* Radiation summary reports now include parameter versus parameter planes.
	* Result files PLANE structures: [ M m -] stands for Max Min and mean

				Crit ND->
				Partial Plane:	... magName	magMetricError	magMGlobalError
				General Plane:	... ''			[ M m -]				[ M m -]

				MC Crit ND->
				Partial Plane:	... magName	magMetricError [ M m - median q12 q34 ]	magMGlobalError [ M m -]
				General Plane:	... ''			[ M m - median q12 q34 ]				[ M m -]

	* Radiation summary reports now include parameter versus parameter planes.
	* *Fixed Bug:* included files not only containing subcircuits
	* **Explicitly save critical parameter injections**
	* **Global results**
	* MC Variability plots

* Changelog of version 3.1.2
	* **LogIO** handling improvements
	* Major bug fixes in **RAWFormatProcessor, CircuitIOHandler and CircuitRadiator** classes.
	* Radiation summary reports now include parameter versus parameter planes.

* Changelog of version 3.1.1
	* Fixed **ProcessLine** trim
	* **Metric** handling improvements

* Changelog of version 3.1.0
	* Major improvements in RAWFormatProcessor class
	* **Metric** has two derived classes, **Magnitude** and **OceanEvalMetric**
	* More efficient number2String
	* Allows *OceanEval* expressions using **OceanEvalMetric**
	* Explicit *std::scientific* export where needed

## Version 3.0.y

* Changelog of version 3.0.0
	* Clean build for *smacd_2016*, including all *v2.9.y* improvements and bug fixes.
	* **CreateFolder** Method is a global template function
	* **NDMagnitudesStructure** now includes constant pointers to useful vectors (plottable and metric magnitudes)
	* Minor *profile scatter* plotting bug has been corrected.

# Version 2.x.y

## Version 2.9.y (3.0 betas)

* Changelog of version 2.9.3_beta
	* Intermediate interface **RunProfile** has been removed
	* Mayor improvements handling handling **thread_group** items
	* Deprecated folder **deprecated** has been created, inside with deprecated classes are organized maintaining their previous structure. **Within this folder no items are automatically compiled neither linked**.
	* Class **MontecarloStandardSimulation** has been deprecated (files *deprecated_montecarlo_standard_simulation.cpp* and *deprecated_montecarlo_standard_simulation.hpp*)
	* Done: *Spectre errors at altered scenarios should at least simulate critical parameter boundaries, both of them. And what happens when an error appears?*

* Changelog of version 2.9.2_beta
	* Title/legend improvements in plots
	* **montecarlo_critical_parameter_nd_parameters_sweep_simulation** simulation mode
	* **montecarlo_critical_parameter_nd_parameters_sweep_simulation** creates **montecarlo_critical_parameter_simulation** threads.
	* **montecarlo_critical_parameter_simulation** creates **critical_parameter_value_simulation** threads,
	* **critical_parameter_value_simulation** transients now may -or not, depending on wheter montecarlo analysis is present- include sufixes **001**, **002**, etc,
	* Introduced **montecarlo_iterations** as a simulation parameter:

		<!-- new in 2.9.2_beta -->
		<!-- required by montecarlo_critical_parameter_n_d_parameters_sweep_analysis -->
		<simulation_parameter>
			<simulation_parameter_name>montecarlo_iterations</simulation_parameter_name>
			<simulation_parameter_default_value>3</simulation_parameter_default_value>
			<simulation_parameter_fixed>true</simulation_parameter_fixed>
			<simulation_parameter_golden_fixed>true</simulation_parameter_golden_fixed>
		</simulation_parameter>


* Changelog of version 2.9.1_beta
	* Improvements in data plotting and handling.
	* Reserving memory in most **std::vectors**.
	* Bug fixes in golden scenarios creation.
	* Improvement when handling Spectre errors in nested simulations.
	* Now with profile->simulation_parameters translation list.
	* Transforming **TransientSimulationResults** to **BasicSimulationResults**
	* Included **ResultsProcessor**, **PlaneResultsStructure** and **SimulationResults** classes, handling multidimensional results.
	* Posible full simulation ID: scenario_profile_mc or scenario_profile_parameter_variation_count, so **s_prof_pvc_mc**
	* Using **PlaneProfileIndexesStructure** for planes-profile indexes storing

			/**
			* Auxiliar Template,
			* PlaneProfileIndexesStructure is a Vector of pointers to vectors of any_number
			*/
			template <class any_number>
			using PlaneProfileIndexesStructure = std::vector<std::vector<any_number>*>;

* Changelog of version 2.9.0_beta
	* New **n-dimensional** analysis:

			a1 b1 c1 d1
			a1 b1 c1 d2
			a1 b1 c1 d3
			a1 b1 c2 d1
			a1 b1 c2 d2
			a1 b1 c2 d3
			a1 b2 c1 d1
			....
	* Uses **NDMagnitudesStructure** structures.
	* **n_d_index** is param_index*param_sweep_length + sweep_index

## Version 2.3.y

* Changelog of version v2.3.5
	* **correctly_simulated** has been implemented using lambdas
	* Introducing a better files management (using libboost instead system calls)
	* Additional labels: **delete_spectre_folders save_spectre_transients save_processed_transients**
	* Results handling bugs fixed.
	* Transient and processed files handling bugs fixed.
	* Montecarlo simulations disk usage bug fixed

* Changelog of version v2.3.4
	* Results handling bugs fixed.
	* Better handling of technology sections.
	* Montecarlo simulations disk usage bug fixed
	* Magnitude errors statistics have been improved: added global maximum error in magnitudes.
	* Magnitude errors statistics have been improved: fixed mag max error in user metric error.
	* Removed 2D gnuplot maps.
	* Results handling bugs corrected

* Changelog from version v2.3.3
	* Introducing technologies with no sections (PT_65nm, for example) defined in xml files.
	* **export_magnitude_errors** feature fixed
	* Minor bugs (plot related) fixed

* Changelog from version v2.3.2
	* Clean Build for MR_2015 (also in bitbucket)

* Changelog from version v2.3.1
	* Some warnings have been suppresed.
	* Scenarios ids mixing bug has been solved.
	* Include max parallel scenario threads support
	* Include minimal report: scenarios list
	* Build for MR_2015

* Changelog from version v2.3.0
	* **Different error regions definition:** Golden simulation decides the comparison region.
	* Minor Gnuplot bugs have been corrected.
	* Several spectre_simulation handling bugs have been corrected.
	* Build for MR_2015


## Version 2.2.y

* Changelog from version v2.2.5 [a->e]
	* Process -> Interpolate -> Plot -> Delete scheme for spectre_simulation threads.
	This affects critical_parameter_1d_parameters_sweep_simulation, golden_simulation,
	montecarlo_critical_parameter_1d_parameters_sweep_simulation,
	* Fixed time window for magnitude evaluation
	* std::endl replaced by '\n' for improving speed in raw_format_processor.cpp.
	* Allows static values for sweep parameters only in golden scenarios.
	* minor bug solved: gnuplot call from critical_parameter_1d_parameters_sweep_simulation summary creation.

* Changelog from version v2.2.4
	* Montecarlo_critical_parameter_1d_parameters_sweep_simulation developed
	* Includes time window for user-metric analysis

* Changelog from version v2.2.3
	* Starting developing novel montecarlo_critical_parameter_1d_parameters_sweep_simulation

* Changelog from version v2.2.2
	* Improvements in Montecarlo 1d param sweep simulations

* Changelog from version v2.2.1
	* Improvements in AHDL Circuits

* Changelog from version v2.2.0
	* **First version of Variability Simulator*
	* **TO do:**
		* % upsets mean
		* Montecarlo without Qcrit
		* Full sweep without Qcrit
	* Improvements in Linear/Logaritmic parameter change

## Version 2.1.y

* Changelog from version v2.1.5
	* **Mayor changes in simulation parameter handling**
	* **Golden depends on tt, while experiment can depend on other models section**
	* **No latex-html export support**

* Changelog from version v2.1.4
	* **No latex-html export support**
	* **TO do: % upsets mean**
	* **To do: Montecarlo without Qcrit**
	* **To do: Full sweep without Qcrit**

* Changelog from version v2.1.3
	* Bug fixes

* Changelog from version v2.1.2
	* Golden plotting (useful for sweeps)
	* Bug fixes
	* **Improved Parameter variations: linear or logaritmic**

* Changelog from version v2.1.1
	* **It does not simulate full-sweep at montecarlo-sweep**
	* **Montecarlo mode bug fixes**
	* Plotting fixes

* Changelog from version v2.1.0
	* **Done: SingleSpectreSimulation->SpectreSimulation**
	* **Done: SpectreSimulation simulation_parameters is independent on each thread**
	* **Done: SpectreSimulation critical_parameter -> golden_critical_parameter**
	* **Done: SpectreSimulation paired_parameter -> golden_paired_parameter**
		* Changed how param_sweep alters the parameter?
		* Removed embebed parameters from golden
	* **Done: SpectreSimulation should have:**
		* critical_parameter (can be null)
		* paired_parameter (can be null)
		* methods such as I/O
		* methods such as ExportParametersCircuit
		* methods such as UpdateSimulationParameterPointer
	* **Done: Sweep on golden scenario**
		* Prior AHDL Golden compilation
		* golden scenario depends on parameters scs
		* If sweep param affects golden (such as tª) sweep over golden
	* **Done:** golden_magnitudes_structure created and implemented.


## Version 2.0.y

* Changelog from version v2.0.5
	* Sync repos
	* Minor bugs
	* Parameter Sweep Simulation now can select whether plot last iteration transient

* Changelog from version v2.0.4
	* Parameter-sweep simulations destructor has been corrected
	* Minor bugs

* Changelog from version v2.0.3
	* Allows to plot signals generated only in altered scenario but not in the golden one.
	* Minor bugs

* Changelog from version v2.0.2
	* Critical parameter evolution report.
	* Minor bugs
	* Report update in novel sweep parameter simulation mode.

* Changelog from version v2.0.1
	* Minor bugs
	* Report update in novel sweep parameter simulation mode.

* Changelog from version v2.0.0
	* Library renamed from Spectre Handler Library to **Circuit Reliability Library**.

# Version 1.x.y

## Version 1.2.y

* Changelog from version v1.2.0
		* Improved installation script
		* Experiment title taken from xml conf file
		* Improved Simulations: Full parameter sweep, fast parameter sweep and montecarlo sweep.
		* Results Object Design:
			* TransientSimulationResults [old SimulationResults]. Minimal results object, related to a single transient. Each single simulation has associated one but the montecarlo_sweep_simulation.
			* TransientSimulationResultsVector [old SimulationResultsVector]. 1D array of TransientSimulationResults.
		* ParameterSweepSimulation: New mode
			* In essence, a CriticalParameterValueSimulation vector.
			* Each CriticalParameterValueSimulation stands as an independent thread.
			* Results: SummaryResults Vector, while each CriticalParameterValueSimulation results are accessible.
			*  foreach Parameter
				* TransientSimulationResultsVectorGroup

## Version 1.1.y

* Changelog from version v1.1.17
		* Improved parallelization features
		* Several improvements in code organization.
		* Mayor and minor bugs.

* Changelog from version v1.1.16
		* Additional Parallelization Level in Montecarlo Full Sweeps
			- [Novel] For each parameter a new thread
			- [Novel] For each altered circuit a new thread
		* Several improvements in code organization.
		* Several improvements in logging system.

* Changelog from version v1.1.15
		* Additional Parallelization Level in Full Sweeps
			- [Novel] For each parameter a new thread
			- [Novel] For each altered circuit a new thread
		* Several improvements in code organization.

* Changelog from version v1.1.14
		* Improved installation script
		* Experiment title taken from xml conf file
		* Results Object Design:
			* TransientSimulationResults [old SimulationResults]. Minimal results object, related to a single transient. Each single simulation has associated one but the montecarlo_sweep_simulation.
			* TransientSimulationResultsVector [old SimulationResultsVector]. 1D array of TransientSimulationResults.
		* Several improvements in code organization.

* Changelog from version v1.1.13
		* Environment variables now on the project folder.
		* Allowed sweep of layout/general parameters
		* Mallorca version, septiembre 2014

* Changelog from version v1.1.12
		* Installation, deployment... scripts now depends on environment variables.
		* Minor bugs related to SimulationModesHandler
		* Repo should be up to date, therefore this version

* Changelog from version v1.1.11
		* Scenario results summary improvements.
		* Several bugs have been fixed
		* Pre-vacations version

* Changelog from version v1.1.10
		* Scenario results summary improvements.
		* Logarithm axis for non-lineal parameter variations
		* Several bugs have been fixed

* Changelog from version v1.1.9
		* Data results organized in specific folders
		* Scenario results summary improvements.
		* Scenario result summary: Fourth Stage

* Changelog from version v1.1.8
		* Scenario result summary: Third Stage

* Changelog from version v1.1.7b
		* Scenario result summary: Second Stage
		* 3D Montecarlo Maps improved.

* Changelog from version v1.1.7
	* Scenario result summary: Second Stage
	* 3D Montecarlo Maps improved.

* Changelog from version v1.1.6
	* Scenario result summary: Initial Stage
	* Summary reports: max/mean maps.
	* 3D Montecarlo Maps improved.

* Changelog from version v1.1.5
	* Global reports: max/mean maps: Stage 2

* Changelog from version v1.1.4
	* Global reports: max/mean maps: Initial Stage

* Changelog from version v1.1.3
	* save control statement exportation bug fixed

* Changelog from version v1.1.2
	* Improved Parameters Parsing Method: [Deep] netlists:

		(?:^|\s)((\b|\\)[\wçÇñÑáÁéÉíÍóÓúÚ@%_:;,<>/\.\{\}\(\)\\\*\-\+\?\|\^\$]+\b|(\b|\\)[\wçÇñÑáÁéÉíÍóÓúÚ@%_:;,<>/\.\{\}\(\)\\\*\-\+\?\|\^\$]+\b\s*=\s*[\s"\wçÇñÑáÁéÉíÍóÓúÚ@%_:;,<>/\.\{\}\(\)\\\*\-\+\?\|\^\$(==)(>=)(<=)]+?(\b|\)|\})|(\b|\\)[\wçÇñÑáÁéÉíÍóÓúÚ@%_:;,<>/\.\{\}\(\)\\\*\-\+\?\|\^\$]+\b\s*=\s*\[[\s"\wçÇñÑáÁéÉíÍóÓúÚ@%_:;,<>/\.\{\}\(\)\\\*\-\+\?\|\^\$(==)(>=)(<=)]+?(\b|\)|\})\]|(\b|\\)[\wçÇñÑáÁéÉíÍóÓúÚ@%_:;,<>/\.\{\}\(\)\\\*\-\+\?\|\^\$]+\b\s*=\s*\[([\s"\wçÇñÑáÁéÉíÍóÓúÚ@%_:;,<>/\.\{\}\(\)\\\*\-\+\?\|\^\$(==)(>=)(<=)]+|(\[[\s"\wçÇñÑáÁéÉíÍóÓúÚ@%_:;,<>/\.\{\}\(\)\\\*\-\+\?\|\^\$(==)(>=)(<=)]+?(\b|\)|\})\]))+\])(?=\s*$|\s+(\b|\\)[\wçÇñÑáÁéÉíÍóÓúÚ@%_:;,<>/\.\{\}\(\)\\\*\-\+\?\|\^\$]+\b\s*=|\s+(\b|\\)[\wçÇñÑáÁéÉíÍóÓúÚ@%_:;,<>/\.\{\}\(\)\\\*\-\+\?\|\^\$]+\b)

* Changelog from version v1.1.1
	* Improved Parameters Parsing Method.

* Changelog from version v1.1.0
	* Novel Parameters Parsing Method:

		// parameters regex idea
	 	(?:^|\s)(\b[a-zA-Z0-9]+\b|\b[a-zA-Z0-9]+\b\s*=\s*\b[a-zA-Z0-9\s\-\*\+\(\)(==)(>=)(<=)]+?\b)(?=\s+\b[a-zA-Z0-9]+\b\s*=|\s*$|\s+\b[a-zA-Z0-9]+\b)

	 	// full param regex
	 	(?:^|\s)(\b[A-Za-z0-9çÇñÑáÁéÉíÍóÓúÚ@%_:;,<>/\.\{\}\(\)\\\*\-\+\?\|\^\$]+\b|\b[A-Za-z0-9çÇñÑáÁéÉíÍóÓúÚ@%_:;,<>/\.\{\}\(\)\\\*\-\+\?\|\^\$]+\b\s*=\s*\b[\s"A-Za-z0-9çÇñÑáÁéÉíÍóÓúÚ@%_:;,<>/\.\{\}\(\)\\\*\-\+\?\|\^\$(==)(>=)(<=)]+?\b)(?=\s+\b[A-Za-z0-9çÇñÑáÁéÉíÍóÓúÚ@%_:;,<>/\.\{\}\(\)\\\*\-\+\?\|\^\$]+\b\s*=|\s*$|\s+\b[A-Za-z0-9çÇñÑáÁéÉíÍóÓúÚ@%_:;,<>/\.\{\}\(\)\\\*\-\+\?\|\^\$]+\b)

## Version 1.0.y

* Changelog from version v1.0.2
	* Minor bugs fixed

* Changelog from version v1.0.1
	* Read **README.md** for more documentation.
	* Library installer improved.

* Changelog from version v1.0.0
	* Read **README.md** for more documentation.
	* Library installer. Please, run


		# some privileges may be needed
		sh installation.sh


	* All the features inherited from **radiation_simulator**, version *v1.7.0* have been included.
	* This changelog includes bellow the changes accomplished while developing **radiation_simulator**, version *v1.x.y* as library versions *0.x.y".
	* This changelog includes bellow the changes accomplished while developing **radiation_simulator**, version *v0.x.y* as library versions *0.0.x.y".
	* Removed statements XML exportation.

* Changelog from version v1.0.0[Beta]
	* All the features inherited from **radiation_simulator**, version *v1.7.0* have been included.
	* This changelog includes bellow the changes accomplished while developing **radiation_simulator**, version *v1.x.y* as library versions *0.x.y".
	* This changelog includes bellow the changes accomplished while developing **radiation_simulator**, version *v0.x.y* as library versions *0.0.x.y".
	* Removed statements XML exportation.

## Version 0.x.y

* Changelog from version v0.7.0
	* Latex Report Generation
	* Minor bugs have been fixed

* Changelog from version v0.6.9
	* Results documentation: Latex. Stage 9
	* Latex hyperlinks in references

* Changelog from version v0.6.8
	* Results documentation: Latex. Stage 8
	* Improved 2D/3D color maps in non-altered scenarios.

* Changelog from version v0.6.7
	* Results documentation: Latex. Stage 5: csv support.
	* Results documentation: Latex. Stage 6: Initial hierarchy support.
	* Results documentation: Latex. Stage 7: Improved latex document type.

* Changelog from version v0.6.6
	* Sweep/Monte Carlo statistics have been improved.
	* Results documentation: Latex. Stage 4: csv initial support.
	* New diagrams in documentation folder.

* Changelog from version v0.6.5
	* HTML exportation has been improved.
	* Results documentation: Latex. Stage 3.

* Changelog from version v0.6.4
	* Experiment class is now responsible for results reports.
	* Folders structure:

	>	Main results folder
	>		->generated images
	>		->generated tables (sweep and montecarlo)?
	>		->generated html
	>		->generated pdf

	* Gnuplot latex exportation
	* Gnuplot multiplot images have been split into single ones.
	* Results documentation: Latex. Stage 2.


* Changelog from version v0.6.3
	* TO DO: folders organization
	* TO DO: export both svg/pdf or svg/latex images
	* Html results generation improved
	* Results documentation: Latex. Stage 1.

* Changelog from version v0.6.2
	* Better sweep 2D plots.
	* Single transient results report selection.
	* 2D/3D interpolation selection.


* Changelog from version v0.6.1
	* Retrieve data scripts
	* Gnuplot paletes
	* Better 3D plot and 2D plots.

* Changelog from version v0.6.0
	* Monte Carlo simulation mode.
	* Full support of 3D plot and 2D plots for Monte Carlo results
	* Improved results appearance.

## Version 1.5.y

* Changelog from version v0.5.6
	* TODO: Monte Carlo simulation mode development: stage 5
	* 3D plot and 2D plots for Monte Carlo results

* Changelog from version v0.5.5
	* TODO: Monte Carlo simulation mode development: stage 4
	* Several minor bugs fixed.
	* 3D plot beta scheme

* Changelog from version v0.5.4
	* TODO: montecarlo-sweep and sweep bugs.
	* Monte Carlo simulation mode development: stage 3.
	* SimulationResults systematic generation in weep and montecarlo-sweep modes.

* Changelog from version v0.5.3
	* TODO: montecarlo-sweep and sweep bugs.
	* Monte Carlo simulation mode development: stage 2.
	* *simulation_results* refers:
		* A single SimulationResult object (standard simulations, singular simulations).
		* A vector of SimulationResult objects (critical value simulations).
		* A vector of vectors of SimulationResult objects (sweep and montecarlo-sweep simulations).
	* HTML sweep/montegarlo results generation bugs have been fixed.

* Changelog from version v0.5.2
	* TODO: montecarlo-sweep and sweep bugs.
	* Each SingleSimulation is responsible of the radiation-paramenters exportation.
	* Monte Carlo simulation mode development: stage 1.

* Changelog from version v0.5.1
	* TODO: montecarlo-sweep and sweep bugs.
	* Control Statements xml importing bugs have been fixed
	* Control Statements scs parsing bugs have been fixed
	* Improved SingularSimulation handling
	* Improved AnalysisStatement exportation scheme

* Changelog from version v0.5.0
	* Improved magnitudes interpolation
	* Punctual errors detection scheme has been fixed
	* Magnitudes monitors (*save* control statements) are automatically added. *Save allpub* or similars are no more needed.

## Version 1.4.y

* Changelog from version v0.4.6
	* Improved transient images: Multi-Magnitudes analysis.
	* Minor bugs have been fixed.
	* Document and report of each alteration.

* Changelog from version v0.4.5
	* Full path of alteration: the user is able to directly know which statement has been altered.
	* Minor bugs have been fixed.

* Changelog from version v0.4.4
	* Html results generation have been improved.
	* Major bugs have been fixed.

* Changelog from version v0.4.3
	* Error detection: define three different regions: logical '1', '0' or 'default'. Magnitude example:

			<magnitude>
				<magnitude_name>output_s</magnitude_name>
				<magnitude_analyzable>true</magnitude_analyzable>
				<magnitude_analyze_error_in_time>true</magnitude_analyze_error_in_time>
				<!-- Binary ones error detection -->
				<magnitude_abs_error_margin_ones>0.5</magnitude_abs_error_margin_ones>
				<magnitude_error_time_span_ones>0.5e-9</magnitude_error_time_span_ones>
				<magnitude_error_threshold_ones>2</magnitude_error_threshold_ones>
				<magnitude_ommit_upper_threshold>false</magnitude_ommit_upper_threshold>
				<!-- Zeros error detection -->
				<magnitude_abs_error_margin_zeros>0.5</magnitude_abs_error_margin_zeros>
				<magnitude_error_time_span_zeros>0.5e-9</magnitude_error_time_span_zeros>
				<magnitude_error_threshold_zeros>0.5</magnitude_error_threshold_zeros>
				<magnitude_ommit_lower_threshold>false</magnitude_ommit_lower_threshold>
				<!-- Default error detection -->
				<magnitude_abs_error_margin_default>0.5</magnitude_abs_error_margin_default>
				<magnitude_error_time_span_default>0.5e-9</magnitude_error_time_span_default>
			</magnitude>

	* Remove unused transient files (this option is enabled by the **save_all_simulations** property in experiment conf xml file).
	* Several bugs fixed

* Changelog from version v0.4.2
	* Dist folder: binary/conf files of each usable version.
	* Improved sweep plots:
		* Sweep plots show non-affected scenarios with "x".
		* Sweep plots show non-affected scenarios with "o".
		* Pointer color refers the value of the critical parameter at this point.
	* Minor bugs fixed
	* Several new **Simulation parameters modes** has been developed.

			//a is the minimum parameter value
			//b is the maximum parameter value
			//n is the iteration index
			// the minimum/maximum parameter values are always simulated.
			0. lineal increments in both sweep and critical value modes.
			1. X(n) = sqrt( X(n-1), b ), a < b in both critical value and sweep modes.
			2. X(n) = sqrt( X(n-1), a ), a < b in sweep mode.
			3. X(n) = X(n-1) + (X(n-1)+b)/2, a < b in sweep mode.
			4. X(n) = X(n-1) + (X(n-1)+a)/2, a < b in sweep mode.

* Changelog from version v0.4.1
	* HTML simulations dynamically loads scenario variations
	* Graphical transient analysis:
		* GnuPlot integration
		* Html report integration
	* Minor bugs fixed

* Changelog from version v0.4.0
	* **Sweep analysis mode:**
	* Graphical sweep analysis:
		* GnuPlot integration
		* Html report integration
	* Minor bugs fixed

## Version 1.3.y

* Changelog from version v0.3.4
	* **Sweep analysis mode:** Scheme developed from xml configuration until simulation.
	* To Do: Process results in sweep mode.

* Changelog from version v0.3.3
	* Statements order during circuit exportation

			simulator lang=spectre
			global 0 vdd!
			parameters myCap=10f
			// parameters AUXtau1=0.1n AUXtau2=2n AUXQcoll=3p AUXI0=AUXQcoll/(AUXtau2-AUXtau1) AUXtImpact=2n
			include "/opt/cadence/tsmc40nm/tsmcN40/../models/spectre/toplevel.scs" section=top_tt


* Changelog from version v0.3.2
	* HTML Results exportation
	* Altered statement full path report
	* Save injection (using full path)

* Changelog from version v0.3.1
	* More complex simulations: new classes derived from **SingleSpectreSimulation**
		* Standard simulation **StandardSimulation** Class
		* Singular simulation **SingularSimulation** Class
		* Critical parameter value simulation **CriticalParameterValueSimulation** Class
	* More complex simulation results: **SimulationResults** Class
		* Simulated parameters
		* Spectre result
		* Radiation result
		* Magnitudes affected by radiation list
		* Error timing: start and stop times
		* Magnitudes transient (in file)

* Changelog from version v0.3.0
	* Substitution scheme allows the critical parameter value simulation mode.
	* Radiation sources are imported from xml files.
	* Radiation sources netlists are deprecated.
	* Simulation Analysis, as well as control statements, are read from the experiment xml conf file.
	* New **SimulationModesHandler** Class. Manages the different simulation modes, including the available radiation modes.
	* New **SimulationMode** Class. Manages the simulation mode attributes, as well as the selected radiation mode.
	* New **RadiationMode** Class. Manages the selected radiation source(s) and its refereed (un)alterable statement lists.
	* Improved XML parsing.
	* SpectreHandler Class improved.

## Version 1.2.y

* Changelog from version v0.2.3
	* Main analysis modes are specified in experiment xml conf file.
	* XML Importation NOT WORKING
	* **Doxygen documentation** already documented:
		* CircuitIOHandler
		* CircuitRadiator
		* Experiment Environment
		* ModeSelector
		* LogIO Class
		* HTMLIO Class
		* XMLIOManager Class
		* Experiment Class
		* DependenceTreesHandler Class
		* Main.cpp

* Changelog from version v0.2.2
	* New **ModelSelector** class.
	* **BSourceStatement** class has been removed.
	* Radiation sources parameters are exported/not exported in a cleaner way.
	* We can select whether export or not the analysis included in the netlists.
	* **Doxygen documentation:** Src comments in doxygen comments:
		* CircuitIOHandler
		* CircuitRadiator
		* Experiment Environment
		* ModeSelector
	* Already documented:
		* LogIO Class
		* HTMLIO Class
		* XMLIOManager Class
		* Experiment Class
		* DependenceTreesHandler Class
		* Main.cpp

* Changelog from version v0.2.1
	* Magnitudes values checkers during xml importation
	* New **src** structure
	* **Doxygen documentation:** Src comments in doxygen comments:
		* LogIO Class
		* HTMLIO Class
		* XMLIOManager Class
		* Experiment Class
		* DependenceTreesHandler Class
		* Main.cpp

* Changelog from version v0.2.0
	* global_constants.hpp file has been slitted in several files (all of them stored in global_functions_and_constants folder).
	* **Find critical value mode** has been improved

## Version 1.1.y

* Changelog from version v0.1.3
	* New attribute: **save_all_simulations**, in experiment.conf file, allows saving all the intermediate simulations
	* Bug fixed in Html Experiments exportation

* Changelog from version v0.1.2
	* HTML Experiments and hierarchy exportation
	* Bug fixed in Html dependency exportation

* Changelog from version v0.1.1
	* Html hierarchy exportation
	* [To do] Bug Html dependency exportation

* Changelog from version v0.1.0
	* Html dependency exportation
	* Bug fixes (section-library export)

## Version 1.0.y

* Changelog from version v0.0.0
	* **Results Analysis**.
	* [FULL] New simulation methods:
		* **Standard Mode**: Finds sensible nodes with default radiation parameters.
		* **Critical Radiation Parameter Mode**: Finds sensible nodes with finding the critical radiation parameter value.
	* Bug fixes.

# Version 0.x.y

## Versions 0.10.y

* Changelog from version v0.0.10.2
	* Added the monitor saving the injected charge

* Changelog from version v0.0.10.1
	* **Granularity**
		* Non-analyzed folders.
		* Non-alterable statements: can't be substituted/injected, as well as its children.
		* List of injectable/replaceable statements.
		* Non-injectable technology/experiment nodes.
	* **[BETA 2] Results Analysis**.

* Changelog from version v0.0.10.0
	* New simulation methods:
		* **Standard Mode**: Finds sensible nodes with default radiation parameters.
		* **Critical Radiation Parameter Mode**: Finds sensible nodes with finding the critical radiation parameter value.
	* Several Threads bugs have been fixed.
	* Improved AHDL/Golden simulations.
	* **Results Interpolation**.
	* **[BETA] Results Analysis**.

## Versions 0.9.y

* Changelog from version v0.0.9.3
	* Minor bug correction in logs generation.
	* Major bugs fixed: Finding dependency/global dependency
	* Major bugs correction in several-deep-level subcircuit injection scheme.
	* **Substitution radiation scheme implemented**
	* Improved log system

* Changelog from version v0.0.9.2
	* Major bugs correction in several-deep-level subcircuit injection scheme.

* Changelog from version v0.0.9.1
	* Major bugs correction in several-deep-level subcircuit injection scheme.
	Take the time to analyze the Substitution scheme, it's pretty cool.
	* Minor bugs correction in circuit_io_handler, preventing files to be read from non-existing paths.

* Changelog from version v0.0.9.0
	* Environment variables handling in each thread
	* Matlab Scripts generation
	* AHDL radiation sources precompilation

## Versions 0.8.y

* Changelog from version v0.0.8.8
	* **RadiationParameter** inclusion, in order to allow sweeps
	* XML Importer has been improved
	* Scripts improved
	* Magnitudes are selected dynamically from xml a file

* Changelog from version v0.0.8.7
	* SpectreHandler improvements
	* AHDL radiation sources support
	* AHDL reuse support: We make use of the following environment variables
		* **$CDS_AHDLCMI_ENABLE** (YES)
		* **$CDS_AHDLCMI_SHIPDB_COPY** (YES)
		* **$CDS_AHDLCMI_SHIPDB_DIR** (~/ahdlCompiled/SHIPDB)
		* **$CDS_AHDLCMI_SIMDB_DIR** (~/ahdlCompiled/SIMDB)

* Changelog from version v0.0.8.6
	* ASCII Raw format processing.
	* The results are stored in instances of **Magnitudes**

* Changelog from version v0.0.8.5
	* Control Statement Parser bug fixes.
	* Ommited files in specified folders.
	* Granularity:
		* Reading limits.
	* Improvements in log system
	* Improvements in dependency system

* Changelog from version v0.0.8.4
	* Improved REGEX for bigger chains.
	* Unexpected EOF while parsing Sections/Libraries: We have decided that, given that several PDKs like **tsmc** do not fully meet spectre sintax.
	* Relative paths: are read from the parent folder, not from the main netlist folder.

* Changelog from version v0.0.8.3
	* Handling with Windows EOL chars
	* Better handling of non-spectre and non-spice files.
	* Non-altered inclusion in cases where the files are non-spectre/non-spice-

* Changelog from version v0.0.8.2
	* By using **boost::thread** we are able to instantiate paralell spectre simulations.
	* Each simulation has its own resources (temp, log and result folders);

* Changelog from version v0.0.8.1
	* Relative paths in included files: Now the relative paths of the included files have been altered depending on the main netlist folder:

			// Path relative/absolute alterations
			boost::filesystem::path pn( netlistFile );
			if( isMain || pn.is_absolute() ){
				log_io->ReportPlain2Log( "Absolute path: " + netlistFile );
			}else{
				log_io->ReportPlain2Log( "Relative path: " + netlistFile );
				netlistFile = mainCircuitFolder + kFolderSeparator + netlistFile;
				log_io->ReportPlain2Log( "Relative path changed to: " + netlistFile );
			}

* Changelog from version v0.0.8.0
	* Language definitions while parsing (omitting spice/verilog langs)

## Versions 0.7.y

* Changelog from version v0.0.7.5
	* Introducing spectre simulations handling

* Changelog from version v0.0.7.4
	* Several fixes in escenarios exportation

* Changelog from version v0.0.7.3
	* Several fixes in escenarios exportation
	* Better scope/circuit/naming scheme

* Changelog from version v0.0.7.2
	* Makefile parallel comipling improvement:

			export MAKEFLAGS="-j $(grep -c ^processor /proc/cpuinfo)"

	* Minor bug fixes.
	* New circuit/scope numeration scheme.
	* Include statements now refer to an already parsed circuit, easying the related circuit exportation. Cast needed when used.

* Changelog from version v0.0.7.1
	* **VERBOSE** mode defined in **log_io.cpp**, in order to define wheter or not **rs_log.log** file includes the log/debug messages. The program needs to be recompilated if **VERBOSE** mode is changed.
	* Injection scheme almost done.
	* To do: include-scheme in exported files.

* Changelog from version v0.0.7beta
	* Experiment has access to all the involved golden circuits.
	* When injecting a subcircuit node, we generate an additional circuit which contains the altered copies of the models/subcircuits involved.

## Versions 0.6.y

* Changelog from version v0.0.6.15
	* Inline Subcircuits -> Subcircuits with an attribute.
	* Injection scheme for instances of (inline) subcircuits/ or (analog) models of (inline) subcircuits


			// Being Radiation Instance 'i', instance of 's' (inline) subcircuit

			InjectNodeOfSubcircuitChild( node n, Instance 'i', bool already_injected ){
				Create radiated subcircuit ('s2') of the parent ('s') of 'i'
				ReplaceSubcircuitInstances( 's', s2' );
			}

			ReplaceSubcircuitInstances( Statement 'originalSubcircuit', Statement 'alteredSubcircuit' ){
				Search instances/(analog) models ('s') dependent on 'originalSubcircuit' ->
				Foreach s € S{
					Create a copy ('c2') of the circuit including 's' ('c')
					SubstituteStatement( 's', 'originalSubcircuit', 'alteredSubcircuit', 'c2' );
				}
			}

			ReplaceInstance( Statement 's', 'originalSubcircuit', 'alteredSubcircuit', 'c2' ){
				if( 's' is an instance of the (inline) subcircuit 'originalSubcircuit'
						&& 's' parent is (another) (inline) subcircuit ){
					Create subcircuit ('ss2') of the parent ('ss1') of 's'
					Change the master_name of 's' from 'originalSubcircuit' to 'alteredSubcircuit'
					ReplaceSubcircuitInstances( 'ss1', 'ss2' );
				}else if( 's' is an instance of the (inline) subcircuit 'originalSubcircuit' ){
					Change the master_name of 's' from 'originalSubcircuit' to 'alteredSubcircuit'
					add'alteredSubcircuit' to 'c2'
					export( 'c2' )
				} else { // it is an (analog) model
					while( 's' is an instance of an (analog) model ('m') ){
						Create a copy ('m2') of the model 'm'
						change the master_name from of 'm' to 'm2'
						add 'm2' to 'c2'
						's' = 'm2'
						ReplaceInstance( 's', 'c2' );
					}
				}
			}


* Changelog from version v0.0.6.14
	* New **Circuit Radiator Class** manages the creation of the radiated circuits.
	* We inject the scope nodes, not a statement.
	* Circuit Radiation Methods have been atomized.
	* Circuit Subcircuit
* Changelog from version v0.0.6.13
	* Major changes in Experiment Class:
		* Report methods have been delegated to LogIO Class.
		* Import/Export Circuit methods have been delegated to CircuitIOHandler Class.

* Changelog from version v0.0.6.12
	* New 'log' file: **rs_analysis.log** including parent-child relations and dependencies between statements.* Primitives handling: Specified by the xml.conf file:
	* Instances Statements new attribute: **is_a_primitive_instance** (replacing the functionality of **is_instance_of_subcircuit**). We consider that an instance is a primitive if we search within the accessible statements and we found that it instantiates a non-accessible instance/(analog)model/(inline)subcircuit or a primitive.
	* Model Statements new attribute: **is_a_primitive_model** (replacing the functionality of **is_instance_of_subcircuit**). We consider that a model is a primitive if we search within the accessible statements and we found that it instantiates a non-accessible instance/(analog)model/(inline)subcircuit or a primitive.
	* Statements new attributes:
		* **master_statement**:	A pointer to the statement which instantiates.
		* **master_scanned**:	A boolean specifying if the instance has already been scanned (for DSF search method).
	* The new trees (primitive_statemetents) will be constructed once parsed the different circuits.
	* The new pointers (master_statement) will be defined once parsed the different circuits.
	* Circuit Statements new attribute **primitive_statemetents**:

			<primitives>
					<primitive_name>a2d</primitive_name>
					<primitive_name>angelov</primitive_name>
					<primitive_name>assert</primitive_name>
					....
			</primitives>

			// Instance or (inline) Subcircuit Statements acting as primitives:
			//	* Instanciated by (analog) models (if (inline) subcircuit or instance)
			//	* Instanciated by instances (if (inline) subcircuit)

* Changelog from version v0.0.6.11
	* Defined algorithm BUT NOT IMPLEMENTED:
	Only instance/transistor statements can be injected BUT
	a model can be a model of an instance.
	Therefore we need to search the hierarchy:

			Altered subcircuit:
			I where i are instances of (inline) subcircuit
			M where m are models of members of i

* Changelog from version v0.0.6.10
	* **Scopes** has the new attribute **statement_name**.
	This attribute refers the name of the parent statement in which the children have the same scope
	* (Inline) subcircuit statements have been fixed (the own_scope is updated when parsing).

* Changelog from version v0.0.6.9
	* **Scopes** has the new attribute **is_subcircuit_scope**.
	Therefore, the children of the conditional statements inside a subcircuit are also be handled as a child of a subcircuit whould in the method **InjectStatement**.

	* This way, the medium to do task "Instance Statements within Conditional Statements within ... within Subcircuit Statements should be treated as Instance Statements within Subcircuit Statements" is done.

* Changelog from version v0.0.6.8
	* Instance Statements new attribute **is_instance_of_subcircuit**
	* Statements new attribute **parent_is_subcircuit**:
	This property can be extracted from the parent has_own_scope attribute and the statement type
	If the parent is an (inline) subcircuit . Only circuit statements, subcircuit statements and inline subcircuit statements have own scope

* Changelog from version v0.0.6.7
	* Removed Statements new attribute **parent_is_subcircuit**

* Changelog from version v0.0.6.6
	* Bug Fixed: **Subcircuit problem: it did not take the first instance in subcircuit, conditional statements...**
	* After the circuits have been parsed, we scan the different instances in order to:
		a) Set the analysis granularity. **TO DO!!**
		b) Describe whether the primitive parent is an (inline) subcircuit.
	* Minor method organization changes
	* PrimitiveStatements now called **InstanceStatements**

* Changelog from version v0.0.6.5
	* Circuit Statement has several lists of the different statements included in its file.
	* These lists are classified depending on the statements type.

* Changelog from version v0.0.6.4
	* New statement property: **altered**. A subcircuit can not be injected (contrary to its children) but, however, can be altered (analyzed and its children altered).
	* An statement gets altered when all the nodes perceptible of being injected have been injected.
	* Refined node handling

* Changelog from version v0.0.6.3b
	* Logging is being externalized
	* Scope handles better the nodes
	* Colorful output
	* Scope Destructor

* Changelog from version v0.0.6.2
	* Transistor Statement parsing using regex

* Changelog from version v0.0.6.1
	* README.md MarkDown sintax
	* Altered vs injected
	* Transistor Statement inclusion for substitutions (instead injections)

* Changelog from version v0.0.6.0
	* Separated XML conf files
	* Scope handling
