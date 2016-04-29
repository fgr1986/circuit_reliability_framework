/*
 * analysis_statement.h
 *
 *  Created on: April 1, 2013
 *  Author: fernando
 */

#ifndef ANALYSIS_STATEMENT_H
#define ANALYSIS_STATEMENT_H

#include <vector>
#include <string>
#include <istream>
#include <fstream>

#include "statement.hpp"

class AnalysisStatement : public Statement{
public:
	/**
	 * @brief Default constructor
	 */
	AnalysisStatement();

	/**
	 * @brief Constructor
	 * 
	 * @param belonging_circuit 
	 * @param log_io 
	 * @param belonging_scope 
	 */
	AnalysisStatement( Statement* belonging_circuit,
		LogIO* log_io, Scope* belonging_scope );

	/**
	 * @brief Copy constructor
	 * 
	 * @param orig 
	 */
	AnalysisStatement(const AnalysisStatement& orig);

	/**
	 * @brief Default destructor
	 * 
	 */
	virtual ~AnalysisStatement();

	/**
	 * @brief get if it is an advanced analysis
	 * @return if it is and advanced analysis
	 */
	bool get_advanced_analysis() const {return advanced_analysis; }

	/**
	 * @brief sets if it is an advanced analysis
	 * 
	 * @param advanced_analysis
	 */
	void set_advanced_analysis(bool advanced_analysis) { this->advanced_analysis = advanced_analysis; }

	/**
	 * @brief Gets a new analysis statement (deep) copy
	 * @return new analysis
	 */
	virtual AnalysisStatement* GetCopy(); 

	/**
	 * @brief gets the spectre netlist code of the analysis (and its children)
	 * 
	 * @param indentation 
	 * @return spectre netlist code of the analysis (and its children)
	 */
	virtual std::string ExportCircuitStatement(std::string indentation);

	/**
	 * @brief Parse a spectre netlist statement, filling the analysis attributes
	 * 
	 * @param global_scope_parent 
	 * @param file 
	 * @param lineTockens 
	 * @param statementCode 
	 * @param currentReadLine 
	 * @param statementCount 
	 * @param endOfFile 
	 * @param parsingSpectreCode 
	 * @param permissiveParsingMode 
	 * @return 
	 */
	bool ParseAnalysisStatement( Statement& global_scope_parent, std::ifstream* file,
		std::vector<std::string>& lineTockens, std::string & statementCode,
		std::string& currentReadLine, int& statementCount, bool& endOfFile,
		bool& parsingSpectreCode, const bool permissiveParsingMode );

	/**
	 * @brief set if it is the main transient analysis
	 * 
	 * @param main_transient
	 */
	void set_main_transient( bool main_transient ){ this->main_transient = main_transient; }

	/**
	 * @brief test if the analysis is the main transient one
	 * @return true if it is the circuit main transient analysis
	 */
	bool get_main_transient(){ return main_transient; }

	/**
	 * @brief set if it is an essential analysis
	 * 
	 * @param essential_analysis
	 */
	void set_essential_analysis( bool essential_analysis ){ this->essential_analysis = essential_analysis; }

	/**
	 * @brief Mutes the exportation of all non essential (radiation_simulator) analysis
	 * @details Mute the exportation of all non essential (radiation_simulator) analysis
	 * Is overriden in analysis_statement.hpp class
	 */
	virtual void MuteAllNonEssentialAnalysis();

	/**
	 * @brief Mutes the exportation of all analysis which are not the main transient
	 * Is overriden in analysis_statement.hpp class
	 */
	virtual void MuteNonMainTransientAnalysis();
	
private:
	/// wheter is and advanced analysis (and therefore has children)
	bool advanced_analysis;
	/// wheter or not the analysis is introduced by the application
	/// or by contrary it was previously defined in the (main) scs
	bool essential_analysis;
	/// key transient analysis
	/// can be nested into a sweep/montecarlo analysis
	bool main_transient;
};

#endif /* ANALYSIS_STATEMENT_H */