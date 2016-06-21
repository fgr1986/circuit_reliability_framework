/**
 * @file OceanEvalMagnitude.hpp
 *
 * @date Created on: December 16, 2013
 *
 * @author Author: Fernando García <fgarcia@die.upm.es> <fernando.garca@gmail.com>
 *
 * @section DESCRIPTION
 *
 * This Class is responsible of the simulations results files as a web page.
 * Several html web pages are created (hierarchical analysis,
 * dependency analysis, simulations results analysis).
 *
 */

#ifndef OCEAN_EVAL_MAGNITUDE_H
#define OCEAN_EVAL_MAGNITUDE_H

#include "magnitude.hpp"

class OceanEvalMagnitude : public Magnitude {
public:
	/// Default Constructor
	OceanEvalMagnitude( const std::string& name );
	/// Copy Constructor
	OceanEvalMagnitude( const OceanEvalMagnitude& orig );

	virtual ~OceanEvalMagnitude();

	double get_value() const{ return value; }
	double get_abs_error_margin() const{ return abs_error_margin; }

	void set_value( double value ){this->value = value;}
	void set_abs_error_margin( double abs_error_margin ){this->abs_error_margin = abs_error_margin;}

private:
	double value;
	double abs_error_margin;
};

#endif /* OCEAN_EVAL_MAGNITUDE_H */
