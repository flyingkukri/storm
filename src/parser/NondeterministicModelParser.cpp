#include "src/parser/NondeterministicModelParser.h"

#include <string>
#include <vector>

#include "src/parser/NondeterministicSparseTransitionParser.h"
#include "src/parser/AtomicPropositionLabelingParser.h"
#include "src/parser/SparseStateRewardParser.h"

namespace storm {
	namespace parser {

		NondeterministicModelParser::Result NondeterministicModelParser::parseNondeterministicModel(std::string const & transitionsFilename, std::string const & labelingFilename, std::string const & stateRewardFilename, std::string const & transitionRewardFilename) {

			// Parse the transitions.
			storm::storage::SparseMatrix<double> transitions(std::move(storm::parser::NondeterministicSparseTransitionParser::parseNondeterministicTransitions(transitionsFilename)));

			uint_fast64_t stateCount = transitions.getColumnCount();

			// Parse the state labeling.
			storm::models::sparse::StateLabeling labeling(std::move(storm::parser::AtomicPropositionLabelingParser::parseAtomicPropositionLabeling(stateCount, labelingFilename)));

			// Only parse state rewards if a file is given.
			boost::optional<std::vector<double>> stateRewards;
			if (stateRewardFilename != "") {
				stateRewards = storm::parser::SparseStateRewardParser::parseSparseStateReward(stateCount, stateRewardFilename);
			}

			// Only parse transition rewards if a file is given.
			boost::optional<storm::storage::SparseMatrix<double>> transitionRewards;
			if (transitionRewardFilename != "") {
				transitionRewards = storm::parser::NondeterministicSparseTransitionParser::parseNondeterministicTransitionRewards(transitionRewardFilename, transitions);
			}

			// Construct the result.
			Result result(std::move(transitions), std::move(labeling));
			result.stateRewards = stateRewards;
			result.transitionRewards = transitionRewards;

			return result;
		}

        storm::models::sparse::Mdp<double> NondeterministicModelParser::parseMdp(std::string const & transitionsFilename, std::string const & labelingFilename, std::string const & stateRewardFilename, std::string const & transitionRewardFilename) {
            Result parserResult = parseNondeterministicModel(transitionsFilename, labelingFilename, stateRewardFilename, transitionRewardFilename);
            
            std::map<std::string, storm::models::sparse::StandardRewardModel<double>> rewardModels;
            rewardModels.insert(std::make_pair("", storm::models::sparse::StandardRewardModel<double>(parserResult.stateRewards, boost::optional<std::vector<double>>(), parserResult.transitionRewards)));
            return storm::models::sparse::Mdp<double>(std::move(parserResult.transitionSystem), std::move(parserResult.labeling), std::move(rewardModels), boost::optional<std::vector<boost::container::flat_set<uint_fast64_t>>>());
		}

	} /* namespace parser */
} /* namespace storm */
