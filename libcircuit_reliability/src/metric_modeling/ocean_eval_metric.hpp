/**
 * @file OceanEvalMetric.hpp
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

#ifndef OCEAN_EVAL_METRIC_H
#define OCEAN_EVAL_METRIC_H

#include "metric.hpp"

class OceanEvalMetric : public Metric {
public:
	/// Default Constructor
	OceanEvalMetric( const std::string& name );
	/// Copy Constructor
	OceanEvalMetric( const OceanEvalMetric& orig );
	OceanEvalMetric( const OceanEvalMetric& orig, const bool copyValues );

	OceanEvalMetric* GetCopy() override;
	virtual ~OceanEvalMetric();

	virtual double get_value_at( const int index ) override;

	double get_value() const{ return value; }
	double get_error_margin_up() const{ return error_margin_up; }
	double get_error_margin_down() const{ return error_margin_down; }
	std::string get_ocean_eval_expression() const{ return ocean_eval_expression; }

	void set_value( double value ){this->value = value;}
	void set_error_margin_up( const double error_margin_up ){this->error_margin_up = error_margin_up;}
	void set_error_margin_down( const double error_margin_down ){this->error_margin_down = error_margin_down;}

	void set_ocean_eval_expression( const std::string& ocean_eval_expression ){this->ocean_eval_expression = ocean_eval_expression;}

private:
	double value;
	double error_margin_up;
	double error_margin_down;
	std::string ocean_eval_expression;
};

#endif /* OCEAN_EVAL_METRIC_H */
