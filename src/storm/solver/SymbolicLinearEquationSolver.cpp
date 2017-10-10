#include "storm/solver/SymbolicLinearEquationSolver.h"

#include "storm/storage/dd/DdManager.h"
#include "storm/storage/dd/Add.h"

#include "storm/utility/dd.h"

#include "storm/solver/SymbolicEliminationLinearEquationSolver.h"
#include "storm/solver/SymbolicNativeLinearEquationSolver.h"
#include "storm/solver/SolverSelectionOptions.h"

#include "storm/settings/SettingsManager.h"
#include "storm/settings/modules/CoreSettings.h"

#include "storm/utility/macros.h"

#include "storm/adapters/RationalFunctionAdapter.h"

namespace storm {
    namespace solver {

        template<storm::dd::DdType DdType, typename ValueType>
        SymbolicLinearEquationSolver<DdType, ValueType>::SymbolicLinearEquationSolver(storm::dd::Add<DdType, ValueType> const& A, storm::dd::Bdd<DdType> const& allRows, std::set<storm::expressions::Variable> const& rowMetaVariables, std::set<storm::expressions::Variable> const& columnMetaVariables, std::vector<std::pair<storm::expressions::Variable, storm::expressions::Variable>> const& rowColumnMetaVariablePairs) : SymbolicLinearEquationSolver(allRows, rowMetaVariables, columnMetaVariables, rowColumnMetaVariablePairs) {
            this->setMatrix(A);
        }
        
        template<storm::dd::DdType DdType, typename ValueType>
        SymbolicLinearEquationSolver<DdType, ValueType>::SymbolicLinearEquationSolver(storm::dd::Bdd<DdType> const& allRows, std::set<storm::expressions::Variable> const& rowMetaVariables, std::set<storm::expressions::Variable> const& columnMetaVariables, std::vector<std::pair<storm::expressions::Variable, storm::expressions::Variable>> const& rowColumnMetaVariablePairs) : allRows(allRows), rowMetaVariables(rowMetaVariables), columnMetaVariables(columnMetaVariables), rowColumnMetaVariablePairs(rowColumnMetaVariablePairs) {
            // Intentionally left empty.
        }
        
        template<storm::dd::DdType DdType, typename ValueType>
        storm::dd::Add<DdType, ValueType> SymbolicLinearEquationSolver<DdType, ValueType>::multiply(storm::dd::Add<DdType, ValueType> const& x, storm::dd::Add<DdType, ValueType> const* b, uint_fast64_t n) const {
            storm::dd::Add<DdType, ValueType> xCopy = x;
            
            // Perform matrix-vector multiplication while the bound is met.
            for (uint_fast64_t i = 0; i < n; ++i) {
                xCopy = xCopy.swapVariables(this->rowColumnMetaVariablePairs);
                xCopy = this->A.multiplyMatrix(xCopy, this->columnMetaVariables);
                if (b != nullptr) {
                    xCopy += *b;
                }
            }
            
            return xCopy;
        }
        
        template<storm::dd::DdType DdType, typename ValueType>
        void SymbolicLinearEquationSolver<DdType, ValueType>::setMatrix(storm::dd::Add<DdType, ValueType> const& newA) {
            this->A = newA;
        }
        
        template<storm::dd::DdType DdType, typename ValueType>
        storm::dd::DdManager<DdType>& SymbolicLinearEquationSolver<DdType, ValueType>::getDdManager() const {
            return this->allRows.getDdManager();
        }
        
        template<storm::dd::DdType DdType, typename ValueType>
        std::unique_ptr<storm::solver::SymbolicLinearEquationSolver<DdType, ValueType>> SymbolicLinearEquationSolverFactory<DdType, ValueType>::create(storm::dd::Add<DdType, ValueType> const& A, storm::dd::Bdd<DdType> const& allRows, std::set<storm::expressions::Variable> const& rowMetaVariables, std::set<storm::expressions::Variable> const& columnMetaVariables, std::vector<std::pair<storm::expressions::Variable, storm::expressions::Variable>> const& rowColumnMetaVariablePairs) const {
            std::unique_ptr<storm::solver::SymbolicLinearEquationSolver<DdType, ValueType>> solver = this->create(allRows, rowMetaVariables, columnMetaVariables, rowColumnMetaVariablePairs);
            solver->setMatrix(A);
            return solver;
        }
        
        template<storm::dd::DdType DdType, typename ValueType>
        std::unique_ptr<storm::solver::SymbolicLinearEquationSolver<DdType, ValueType>> GeneralSymbolicLinearEquationSolverFactory<DdType, ValueType>::create(storm::dd::Bdd<DdType> const& allRows, std::set<storm::expressions::Variable> const& rowMetaVariables, std::set<storm::expressions::Variable> const& columnMetaVariables, std::vector<std::pair<storm::expressions::Variable, storm::expressions::Variable>> const& rowColumnMetaVariablePairs) const {
            storm::solver::EquationSolverType equationSolver = storm::settings::getModule<storm::settings::modules::CoreSettings>().getEquationSolver();
            switch (equationSolver) {
                case storm::solver::EquationSolverType::Elimination: return std::make_unique<storm::solver::SymbolicEliminationLinearEquationSolver<DdType, ValueType>>(allRows, rowMetaVariables, columnMetaVariables, rowColumnMetaVariablePairs);
                    break;
                case storm::solver::EquationSolverType::Native: return std::make_unique<storm::solver::SymbolicNativeLinearEquationSolver<DdType, ValueType>>(allRows, rowMetaVariables, columnMetaVariables, rowColumnMetaVariablePairs);
                    break;
                default:
                    STORM_LOG_WARN("The selected equation solver is not available in the dd engine. Falling back to native solver.");
                    return std::make_unique<storm::solver::SymbolicNativeLinearEquationSolver<DdType, ValueType>>(allRows, rowMetaVariables, columnMetaVariables, rowColumnMetaVariablePairs);
            }
        }
        
        template<storm::dd::DdType DdType>
        std::unique_ptr<storm::solver::SymbolicLinearEquationSolver<DdType, storm::RationalNumber>> GeneralSymbolicLinearEquationSolverFactory<DdType, storm::RationalNumber>::create(storm::dd::Bdd<DdType> const& allRows, std::set<storm::expressions::Variable> const& rowMetaVariables, std::set<storm::expressions::Variable> const& columnMetaVariables, std::vector<std::pair<storm::expressions::Variable, storm::expressions::Variable>> const& rowColumnMetaVariablePairs) const {

            auto const& coreSettings = storm::settings::getModule<storm::settings::modules::CoreSettings>();
            storm::solver::EquationSolverType equationSolver = coreSettings.getEquationSolver();
            if (coreSettings.isEquationSolverSetFromDefaultValue() && equationSolver != storm::solver::EquationSolverType::Elimination) {
                STORM_LOG_WARN("Selecting the elimination solver to guarantee exact results. If you want to override this, please explicitly specify a different equation solver.");
                equationSolver = storm::solver::EquationSolverType::Elimination;
            }
            
            if (equationSolver != storm::solver::EquationSolverType::Elimination) {
                STORM_LOG_WARN("The chosen equation solver does not guarantee precise results despite using exact arithmetic. Consider using the elimination solver instead.");
            }

            switch (equationSolver) {
                case storm::solver::EquationSolverType::Elimination: return std::make_unique<storm::solver::SymbolicEliminationLinearEquationSolver<DdType, storm::RationalNumber>>(allRows, rowMetaVariables, columnMetaVariables, rowColumnMetaVariablePairs);
                    break;
                case storm::solver::EquationSolverType::Native:
                    return std::make_unique<storm::solver::SymbolicNativeLinearEquationSolver<DdType, storm::RationalNumber>>(allRows, rowMetaVariables, columnMetaVariables, rowColumnMetaVariablePairs);
                    break;
                default:
                    STORM_LOG_WARN("The selected equation solver is not available in the dd engine. Falling back to elimination solver.");
                    return std::make_unique<storm::solver::SymbolicEliminationLinearEquationSolver<DdType, storm::RationalNumber>>(allRows, rowMetaVariables, columnMetaVariables, rowColumnMetaVariablePairs);
            }
        }
        
        template<storm::dd::DdType DdType>
        std::unique_ptr<storm::solver::SymbolicLinearEquationSolver<DdType, storm::RationalFunction>> GeneralSymbolicLinearEquationSolverFactory<DdType, storm::RationalFunction>::create(storm::dd::Bdd<DdType> const& allRows, std::set<storm::expressions::Variable> const& rowMetaVariables, std::set<storm::expressions::Variable> const& columnMetaVariables, std::vector<std::pair<storm::expressions::Variable, storm::expressions::Variable>> const& rowColumnMetaVariablePairs) const {
            
            storm::solver::EquationSolverType equationSolver = storm::settings::getModule<storm::settings::modules::CoreSettings>().getEquationSolver();
            switch (equationSolver) {
                case storm::solver::EquationSolverType::Elimination: return std::make_unique<storm::solver::SymbolicEliminationLinearEquationSolver<DdType, storm::RationalFunction>>(allRows, rowMetaVariables, columnMetaVariables, rowColumnMetaVariablePairs);
                    break;
                default:
                    STORM_LOG_WARN("The selected equation solver is not available in the DD setting. Falling back to elimination solver.");
                    return std::make_unique<storm::solver::SymbolicEliminationLinearEquationSolver<DdType, storm::RationalFunction>>(allRows, rowMetaVariables, columnMetaVariables, rowColumnMetaVariablePairs);
            }            
        }
        
        template class SymbolicLinearEquationSolver<storm::dd::DdType::CUDD, double>;
        template class SymbolicLinearEquationSolver<storm::dd::DdType::CUDD, storm::RationalNumber>;

        template class SymbolicLinearEquationSolver<storm::dd::DdType::Sylvan, double>;
        template class SymbolicLinearEquationSolver<storm::dd::DdType::Sylvan, storm::RationalNumber>;
        template class SymbolicLinearEquationSolver<storm::dd::DdType::Sylvan, storm::RationalFunction>;
        
        template class SymbolicLinearEquationSolverFactory<storm::dd::DdType::CUDD, double>;
        template class SymbolicLinearEquationSolverFactory<storm::dd::DdType::CUDD, storm::RationalNumber>;
        template class SymbolicLinearEquationSolverFactory<storm::dd::DdType::Sylvan, double>;
        template class SymbolicLinearEquationSolverFactory<storm::dd::DdType::Sylvan, storm::RationalNumber>;
        template class SymbolicLinearEquationSolverFactory<storm::dd::DdType::Sylvan, storm::RationalFunction>;
        
        template class GeneralSymbolicLinearEquationSolverFactory<storm::dd::DdType::CUDD, double>;
        template class GeneralSymbolicLinearEquationSolverFactory<storm::dd::DdType::CUDD, storm::RationalNumber>;
        template class GeneralSymbolicLinearEquationSolverFactory<storm::dd::DdType::Sylvan, double>;
        template class GeneralSymbolicLinearEquationSolverFactory<storm::dd::DdType::Sylvan, storm::RationalNumber>;
        template class GeneralSymbolicLinearEquationSolverFactory<storm::dd::DdType::Sylvan, storm::RationalFunction>;
        
    }
}
