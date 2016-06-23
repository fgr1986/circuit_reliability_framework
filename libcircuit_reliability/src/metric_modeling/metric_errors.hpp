 /**
 * @file metric_errors.hpp
 *
 * @date Created on: March 28, 2014
 *
 * @author Author: Fernando García <fgarcia@die.upm.es> <fernando.garca@gmail.com>
 *
 * @section DESCRIPTION
 *
 * This Class is a container of the simulation results.
 *
 */

#ifndef METRIC_ERRORS_H
#define METRIC_ERRORS_H

// c++ std libraries
#include <string>
// #include <iostream>
#include <map>

class MetricErrors {
public:
	MetricErrors();
	virtual ~MetricErrors();

	/**
	 * @brief Add an error start-stop timing
	 * @details
	 *
	 * @param start
	 * @param stop
	 */
	void AddErrorTiming( const std::string& start, const std::string& stop );

	/**
	 * @brief set metric_name
	 *
	 * @param metric_name
	 */
	void set_metric_name( const std::string& metric_name ){ this->metric_name = metric_name; }

	/**
	 * @brief set max_abs_error
	 *
	 * @param max_abs_error
	 */
	void set_max_abs_error( const double max_abs_error ){
		this->max_abs_error = max_abs_error; }

	/**
	 * @brief set max_abs_error_global
	 *
	 * @param max_abs_error_global
	 */
	void set_max_abs_error_global( const double max_abs_error_global ){
		this->max_abs_error_global = max_abs_error_global; }

	/**
	 * @brief set has_errors
	 *
	 * @param has_errors
	 */
	void set_has_errors( const bool has_errors ){ this->has_errors = has_errors; }

	/**
	 * @brief gets metric_name
	 * @return metric_name
	 */
	std::string get_metric_name() const { return metric_name; }

	/**
	 * @brief gets max_abs_error
	 * @return max_abs_error
	 */
	double get_max_abs_error() const { return max_abs_error; }

	/**
	 * @brief gets max_abs_error_global
	 * @return max_abs_error_global
	 */
	double get_max_abs_error_global() const { return max_abs_error_global; }

	/**
	 * @brief gets has_errors
	 * @return has_errors
	 */
	bool get_has_errors() const { return has_errors; }

	/**
	 * @brief gets the pairs error start- error stop
	 * @return error_timing
	 */
	std::map<std::string,std::string>* get_error_timing(){ return &error_timing; }

private:
	/// max error in whole transient,
	/// including punctual and recovered errors
	double max_abs_error_global;
	/// max error in user metric error
	/// if no-punctual-mode is defined in the metric, then not considering punctual errors
	double max_abs_error;
	/// metric name
	std::string metric_name;
	/// metric has_errors
	bool has_errors;
	/// error timing: start-end
	std::map<std::string,std::string> error_timing;
};

#endif /* METRIC_ERRORS_H */
