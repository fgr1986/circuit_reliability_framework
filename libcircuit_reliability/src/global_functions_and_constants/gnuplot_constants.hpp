#ifndef GLOBAL_GNUPLOT_CONSTANTS_H
#define GLOBAL_GNUPLOT_CONSTANTS_H

#include <string>

static const std::string kGnuplotCommand    = "gnuplot ";
// static const std::string kGnuplotEndCommand = " > /dev/null 2>&1  ";
static const std::string kGnuplotEndCommand = "";
// static const std::string kGnuplotEndCommand = ">> $RELIABILITY_GNUPLOT_LOG";
//LineStiles
static const std::string kElegantLine = "set style line 100 lc rgb '#333333' lt 1 lw 0.3";

// Color palettes
static const std::string kBlueForPalettes = "#C1DAE8";
static const std::string kDarkBlueForPalettes = "#4d69ab";
// Minimal for flat data or non existing data
static const std::string kMinimalPalette = "set palette defined (  0 \"" + kBlueForPalettes + "\", 1e-3 \"#fffaef\", 0.5 \"#ffd35a\", 1 \"#bd2c29\")"
											" \n set cbrange [0:1]";
// standard palettes
static const std::string kMathematicaPalette = "set palette defined ( 0 \"#bd2c29\", 50 \"#ffd35a\", 100 \"#fffaef\")";
static const std::string kMathematicaInvPalette = "set palette defined (  0 \"#fffaef\", 50 \"#ffd35a\", 100 \"#bd2c29\")";
static const std::string kBlueRedPalette = "set palette defined ( 0 \"" + kDarkBlueForPalettes + "\", 50 \"#ffd35a\", 100 \"#ed2c29\")";
// Our palette
// requires from min_critical_value existance (gnuplot variable)
static const std::string kRadiationPalette = "set palette defined ( 0 \"" + kBlueForPalettes + "\", min_critical_value \"#fffaef\", "
	"middle_critical_value \"#ffd35a\", max_critical_value \"#ed2c29\")";
static const std::string kRadiationInvertedPalette = "set palette defined ( 0 \"" + kBlueForPalettes + "\", min_critical_value \"#ed2c29\", "
	"middle_critical_value \"#ffd35a\", max_critical_value \"#fffaef\")";
static const std::string kUpsetsPalette = "set palette defined ( 0 \"" + kBlueForPalettes + "\", 1e-9 \"#fffaef\", 50 \"#ffd35a\", 100 \"#ed2c29\")";
static const std::string kUpsetsInvertedPalette = "set palette defined ( 0 \"" + kBlueForPalettes + "\", 1e-9 \"#ed2c29\", 50 \"#ffd35a\", 100 \"#fffaef\")";

static const std::string kSimilarLinesPalette =
		"# color definitions \n"
		"set style line 5  lc rgb '#0025ad' lt 1 lw 2\n"
		"set style line 6  lc rgb '#0042ad' lt 1 lw 2\n"
		"set style line 7  lc rgb '#0060ad' lt 1 lw 2\n"
		"set style line 8  lc rgb '#007cad' lt 1 lw 2\n"
		"set style line 9  lc rgb '#0099ad' lt 1 lw 2\n"
		"set style line 10 lc rgb '#00ada4' lt 1 lw 2\n"
		"set style line 11 lc rgb '#00ad88' lt 1 lw 2\n"
		"set style line 12 lc rgb '#00ad6b' lt 1 lw 2\n"
		"set style line 13 lc rgb '#00ad4e' lt 1 lw 2\n"
		"set style line 14 lc rgb '#00ad31' lt 1 lw 2\n"
		"set style line 15 lc rgb '#00ad14' lt 1 lw 2\n"
		"set style line 16 lc rgb '#09ad00' lt 1 lw 2\n";

// Background
static const std::string kWholeBackground = "set object 1 rect from screen 0.0, 0.0, 0 to screen 1, 1, 0 behind \n"
	"set object 1 rect fc rgb \"white\" fillstyle solid 1.0 noborder";
static const std::string kUpperBackground = "set object 1 rect from screen 0.0, 0.5, 0 to screen 1, 1, 0 behind \n"
	"set object 1 rect fc rgb \"white\" fillstyle solid 1.0 noborder";
static const std::string kLowerBackground = "set object 1 rect from screen 0, 0, 0 to screen 1, 0.5, 0 behind \n"
	"set object 1 rect fc rgb \"white\" fillstyle solid 1.0 noborder";

// Transparency
static const std::string kFillObjects = "set style fill solid 1.0 border";
static const std::string kTransparentObjects = "set style fill transparent solid 0.8 border";

// Latex terminal
static const std::string kLatexTerminal = "pdfcairo";
static const std::string kLatexFont = "\"Times-New Roman,13\"";
// svg terminal
static const std::string kSvgFont = "'Times' fsize 12";


// Latex terminal default width (cm)
static const double kLatexImageWidth = 20;
static const double kLatexImageHeight = 15;
// Svg terminal default width (px)
static const int kSvgImageWidth = 1200;
static const int kSvgImageHeight = 900;

#endif /* GLOBAL_GNUPLOT_CONSTANTS_H */
