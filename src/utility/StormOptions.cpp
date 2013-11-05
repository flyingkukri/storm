#include "src/utility/StormOptions.h"

bool storm::utility::StormOptions::optionsRegistered = storm::settings::Settings::registerNewModule([] (storm::settings::Settings* settings) -> bool {
	settings->addOption(storm::settings::OptionBuilder("StoRM Main", "help", "h", "Shows all available Options, Arguments and Descriptions").build());

	settings->addOption(storm::settings::OptionBuilder("StoRM Main", "verbose", "v", "Be verbose").build());

	settings->addOption(storm::settings::OptionBuilder("StoRM Main", "debug", "", "Be very verbose (intended for debugging)").build());

	settings->addOption(storm::settings::OptionBuilder("StoRM Main", "trace", "", "Be extremly verbose (intended for debugging, heavy performance impacts)").build());

	settings->addOption(storm::settings::OptionBuilder("StoRM Main", "logfile", "l", "If specified, the log output will also be written to this file").addArgument(storm::settings::ArgumentBuilder::createStringArgument("logFileName", "The path and name of the File to write to").build()).build());

	settings->addOption(storm::settings::OptionBuilder("StoRM Main", "configfile", "c", "If specified, this file will be read and parsed for additional configuration settings").addArgument(storm::settings::ArgumentBuilder::createStringArgument("configFileName", "The path and name of the File to read from").addValidationFunctionString(storm::settings::ArgumentValidators::existingReadableFileValidator()).build()).build());

	settings->addOption(storm::settings::OptionBuilder("StoRM Main", "explicit", "", "Explicit parsing from Transition- and Labeling Files").addArgument(storm::settings::ArgumentBuilder::createStringArgument("transitionFileName", "The path and name of the File to read the transition system from").addValidationFunctionString(storm::settings::ArgumentValidators::existingReadableFileValidator()).build()).addArgument(storm::settings::ArgumentBuilder::createStringArgument("labelingFileName", "The path and name of the File to read the labeling from").addValidationFunctionString(storm::settings::ArgumentValidators::existingReadableFileValidator()).build()).build());

	settings->addOption(storm::settings::OptionBuilder("StoRM Main", "symbolic", "", "Parse the given PRISM File").addArgument(storm::settings::ArgumentBuilder::createStringArgument("prismFileName", "The path and name of the File to read the PRISM Model from").addValidationFunctionString(storm::settings::ArgumentValidators::existingReadableFileValidator()).build()).build());

	settings->addOption(storm::settings::OptionBuilder("StoRM Main", "prctl", "", "Evaluates the PRCTL Formulas given in the File").addArgument(storm::settings::ArgumentBuilder::createStringArgument("prctlFileName", "The path and name of the File to read PRCTL Formulas from").addValidationFunctionString(storm::settings::ArgumentValidators::existingReadableFileValidator()).build()).build());

	settings->addOption(storm::settings::OptionBuilder("StoRM Main", "csl", "", "Evaluates the CSL Formulas given in the File").addArgument(storm::settings::ArgumentBuilder::createStringArgument("cslFileName", "The path and name of the File to read CSL Formulas from").addValidationFunctionString(storm::settings::ArgumentValidators::existingReadableFileValidator()).build()).build());

	settings->addOption(storm::settings::OptionBuilder("StoRM Main", "ltl", "", "Evaluates the LTL Formulas given in the File").addArgument(storm::settings::ArgumentBuilder::createStringArgument("ltlFileName", "The path and name of the File to read LTL Formulas from").addValidationFunctionString(storm::settings::ArgumentValidators::existingReadableFileValidator()).build()).build());

	settings->addOption(storm::settings::OptionBuilder("StoRM Main", "counterExample", "", "Generates a counterexample for the given formulas if not satisfied by the model").addArgument(storm::settings::ArgumentBuilder::createStringArgument("prctlFileName", "The path and name of the File to read the PRCTL Formulas to be used in \n\t\t\t\t\t\t the couterexample generation from").addValidationFunctionString(storm::settings::ArgumentValidators::existingReadableFileValidator()).build()).addArgument(storm::settings::ArgumentBuilder::createStringArgument("outputPath", "The path to the directory to write the generated counterexample files to.").build()).build());

	settings->addOption(storm::settings::OptionBuilder("StoRM Main", "transitionRewards", "", "If specified, the model will have these transition rewards").addArgument(storm::settings::ArgumentBuilder::createStringArgument("transitionRewardsFileName", "The path and name of the File to read the Transition Rewards from").addValidationFunctionString(storm::settings::ArgumentValidators::existingReadableFileValidator()).build()).build());

	settings->addOption(storm::settings::OptionBuilder("StoRM Main", "stateRewards", "", "If specified, the model will have these state rewards").addArgument(storm::settings::ArgumentBuilder::createStringArgument("stateRewardsFileName", "The path and name of the File to read the State Rewards from").addValidationFunctionString(storm::settings::ArgumentValidators::existingReadableFileValidator()).build()).build());

	settings->addOption(storm::settings::OptionBuilder("StoRM Main", "fixDeadlocks", "", "Insert Self-Loops for States with no outgoing transitions").build());

	std::vector<std::string> matrixLibrarys;
	matrixLibrarys.push_back("gmm++");
	matrixLibrarys.push_back("native");
	settings->addOption(storm::settings::OptionBuilder("StoRM Main", "matrixLibrary", "m", "Sets which matrix library is preferred for numerical operations.").addArgument(storm::settings::ArgumentBuilder::createStringArgument("matrixLibraryName", "The name of a matrix library. Valid values are gmm++ and native.").addValidationFunctionString(storm::settings::ArgumentValidators::stringInListValidator(matrixLibrarys)).setDefaultValueString("gmm++").build()).build());

	return true;
});
