#ifndef GLOBAL_GNUPLOT_CONSTANTS_H
#define GLOBAL_GNUPLOT_CONSTANTS_H

#include <string>

static const std::string kGnuplotCommand    = "gnuplot ";
// static const std::string kGnuplotEndCommand = " > /dev/null 2>&1  ";
static const std::string kGnuplotEndCommand = "";
// static const std::string kGnuplotEndCommand = ">> $RELIABILITY_GNUPLOT_LOG";
//LineStiles
static const std::string kElegantLine = "# elegant line \n"
		"set style line 102 lc rgb '#333333' lt 1 lw 0.3\n\n";

// Color palettes
static const std::string kBlueForPalettes = "#C1DAE8";
static const std::string kDarkBlueForPalettes = "#4d69ab";
// Minimal for flat data or non existing data
static const std::string kMinimalPalette = "set palette defined (  0 \"" + kBlueForPalettes + "\", 1e-3 \"#fffaef\", 0.5 \"#ffd35a\", 1 \"#bd2c29\")\n\n"
											" \n set cbrange [0:1]\n";
// standard palettes
static const std::string kMathematicaPalette = "set palette defined ( 0 \"#bd2c29\", 50 \"#ffd35a\", 100 \"#fffaef\")\n\n";
static const std::string kMathematicaInvPalette = "set palette defined (  0 \"#fffaef\", 50 \"#ffd35a\", 100 \"#bd2c29\")\n\n";
static const std::string kBlueRedPalette = "set palette defined ( 0 \"" + kDarkBlueForPalettes + "\", 50 \"#ffd35a\", 100 \"#ed2c29\")\n\n";
// Our palette
// requires from min_critical_value existance (gnuplot variable)
static const std::string kRadiationPalette = "set palette defined ( 0 \"" + kBlueForPalettes + "\", min_critical_value \"#fffaef\", "
	"middle_critical_value \"#ffd35a\", max_critical_value \"#ed2c29\")\n\n";
static const std::string kRadiationInvertedPalette = "set palette defined ( 0 \"" + kBlueForPalettes + "\", min_critical_value \"#ed2c29\", "
	"middle_critical_value \"#ffd35a\", max_critical_value \"#fffaef\")\n\n";
static const std::string kUpsetsPalette = "set palette defined ( 0 \"" + kBlueForPalettes + "\", 1e-19 \"#fffaef\", 50 \"#ffd35a\", 100 \"#ed2c29\")\n\n";
static const std::string kUpsetsInvertedPalette = "set palette defined ( 0 \"" + kBlueForPalettes + "\", 1e-9 \"#ed2c29\", 50 \"#ffd35a\", 100 \"#fffaef\")\n\n";

static const std::string k3DProperties =
		"set style line 102 lc rgb '#333333' lt 1 lw 0.3\n"
		"set style line 101 lc rgb '#333333' lt 1 lw 0.3\n"
		"set style line 103 lc rgb '#333333' lt 1 lw 0.3\n"
		"set pm3d interpolate 5,5\n"
		"set pm3d depthorder hidden3d 102\n"
		"set hidden3d\n\n";

static const std::string kCustomBorders =
	// # remove border on top and right and set color to gray
		"set style line 11 lc rgb '#777777' lt 1\n"
		"set border 3 back ls 11\n"
		"set tics nomirror\n\n";

static const std::string kTransientSimilarLinesPalette =
		"# color definitions \n"
		"set style line 1  lc rgb '#cf3a00' lt 1 lw 1 pt 6 ps 1 # ---red\n"
		"set style line 2  lc rgb '#0025ad' lt 1 lw 2 pt 7 # -- remaining blues and greens\n"
		"set style line 3  lc rgb '#0042ad' lt 1 lw 2\n"
		"set style line 4  lc rgb '#0060ad' lt 1 lw 2\n"
		"set style line 5  lc rgb '#007cad' lt 1 lw 2\n"
		"set style line 6  lc rgb '#0099ad' lt 1 lw 2\n"
		"set style line 7 lc rgb '#00ada4' lt 1 lw 2\n"
		"set style line 8 lc rgb '#00ad88' lt 1 lw 2\n"
		"set style line 9 lc rgb '#00ad6b' lt 1 lw 2\n"
		"set style line 10 lc rgb '#00ad4e' lt 1 lw 2\n"
		"set style line 11 lc rgb '#00ad31' lt 1 lw 2\n"
		"set style line 12 lc rgb '#00ad14' lt 1 lw 2\n"
		"set style line 13 lc rgb '#09ad00' lt 1 lw 2\n"
		"set style fill solid \n";

static const std::string kProfilesPalette =
		"# color definitions \n"
		"set style line 1 lc rgb '#0060ad' lt 1 lw 1 pt 7 ps 1  # --- blue for candle metric \n"
		"set style line 2 lc rgb '#0060ad' lt 1 lw 3 pt 7 ps 1.5  # --- blue for metric\n" // metric
		"set style line 3 lc rgb '#cf3a00' lt 1 lw 1 pt 7 ps 1  # --- red for global or errorbars\n" // global
		"set style line 4 lc rgb '#ffd35a' lt 3 lw 1 pt 9 ps 1  # --- yellow filled crit_param_max\n" // crit vals
		"set style line 5 lc rgb '#ff6666' lt 3 lw 1 pt 9 ps 1  # --- red filled for crit_param_min (worst) \n"
		"set style line 6 lc rgb '#ffb35a' lt 3 lw 1 pt 9 ps 1  # --- orange filled for crit_param_mean\n"
		"set style line 7 lc rgb '#666666' lt 3 lw 1 pt 9 ps 1  # --- grey\n"
		"set boxwidth 0.5 relative\n"
		"set style fill transparent solid 0.4\n";

// Transparency
// static const std::string kFillObjects = "set style fill solid 1.0 border\n\n";
static const std::string kTransparent3DObjects = "set style fill transparent solid 0.5 border\n\n";

// Background
// static const std::string kWholeBackground = "set object 1 rect from screen 0.0, 0.0, 0 to screen 1, 1, 0 behind \n"
// 	"set object 1 rect fc rgb \"white\" fillstyle solid 1.0 noborder\n\n";
// static const std::string kUpperBackground = "set object 1 rect from screen 0.0, 0.5, 0 to screen 1, 1, 0 behind \n"
// 	"set object 1 rect fc rgb \"white\" fillstyle solid 1.0 noborder\n\n";
// static const std::string kLowerBackground = "set object 1 rect from screen 0, 0, 0 to screen 1, 0.5, 0 behind \n"
// 	"set object 1 rect fc rgb \"white\" fillstyle solid 1.0 noborder\n\n";

// Latex terminal
static const std::string kLatexTerminal = "pdfcairo";
static const std::string kLatexFont = "\"Times-New Roman,13\"";
// svg terminal
static const std::string kSvgFont = "'Times' fsize 18";


// Latex terminal default width (cm)
static const double kLatexImageWidth = 20;
static const double kLatexImageHeight = 15;
// Svg terminal default width (px)
static const int kSvgImageWidth = 1200;
static const int kSvgImageHeight = 900;

#endif /* GLOBAL_GNUPLOT_CONSTANTS_H */
