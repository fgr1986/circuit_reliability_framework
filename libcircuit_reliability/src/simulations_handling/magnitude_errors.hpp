 /**
 * @file magnitude_errors.hpp
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

#ifndef MAGNITUDE_ERRORS_h
#define MAGNITUDE_ERRORS_h

// c++ std libraries
#include <string>
#include <map>

class MagnitudeErrors {
public:
	MagnitudeErrors();
	virtual ~MagnitudeErrors();

	/**
	 * @brief Add an error start-stop timing
	 * @details
	 *
	 * @param start
	 * @param stop
	 */
	void AddErrorTiming( std::string start, std::string stop );

	/**
	 * @brief set magnitude_name
	 *
	 * @param magnitude_name
	 */
	void set_magnitude_name( std::string magnitude_name ){ this->magnitude_name = magnitude_name; }

	/**
	 * @brief set max_abs_error
	 *
	 * @param max_abs_error
	 */
	void set_max_abs_error( double max_abs_error ){ this->max_abs_error = max_abs_error; }

	/**
	 * @brief set max_abs_error_global
	 *
	 * @param max_abs_error_global
	 */
	void set_max_abs_error_global( double max_abs_error_global ){
		this->max_abs_error_global = max_abs_error_global; }

	/**
	 * @brief set has_errors
	 *
	 * @param has_errors
	 */
	void set_has_errors( bool has_errors ){ this->has_errors = has_errors; }

	/**
	 * @brief gets magnitude_name
	 * @return magnitude_name
	 */
	std::string get_magnitude_name() const { return magnitude_name; }

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
	/// magnitude name
	std::string magnitude_name;
	/// magnitude has_errors
	bool has_errors;
	/// error timing: start-end
	std::map<std::string,std::string> error_timing;
};

#endif /* MAGNITUDE_ERRORS_h */
