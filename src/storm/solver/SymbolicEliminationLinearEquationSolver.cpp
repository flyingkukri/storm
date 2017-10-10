#include "storm/solver/SymbolicEliminationLinearEquationSolver.h"

#include "storm/storage/dd/DdManager.h"
#include "storm/storage/dd/Add.h"

#include "storm/utility/dd.h"

#include "storm/adapters/RationalFunctionAdapter.h"

namespace storm {
    namespace solver {
     
        template<storm::dd::DdType DdType, typename ValueType>
        SymbolicEliminationLinearEquationSolver<DdType, ValueType>::SymbolicEliminationLinearEquationSolver(storm::dd::Add<DdType, ValueType> const& A, storm::dd::Bdd<DdType> const& allRows, std::set<storm::expressions::Variable> const& rowMetaVariables, std::set<storm::expressions::Variable> const& columnMetaVariables, std::vector<std::pair<storm::expressions::Variable, storm::expressions::Variable>> const& rowColumnMetaVariablePairs) : SymbolicEliminationLinearEquationSolver(allRows, rowMetaVariables, columnMetaVariables, rowColumnMetaVariablePairs) {
            this->setMatrix(A);
        }
        
        template<storm::dd::DdType DdType, typename ValueType>
        SymbolicEliminationLinearEquationSolver<DdType, ValueType>::SymbolicEliminationLinearEquationSolver(storm::dd::Bdd<DdType> const& allRows, std::set<storm::expressions::Variable> const& rowMetaVariables, std::set<storm::expressions::Variable> const& columnMetaVariables, std::vector<std::pair<storm::expressions::Variable, storm::expressions::Variable>> const& rowColumnMetaVariablePairs) : SymbolicLinearEquationSolver<DdType, ValueType>(allRows, rowMetaVariables, columnMetaVariables, rowColumnMetaVariablePairs) {
            storm::dd::DdManager<DdType>& ddManager = allRows.getDdManager();
            
            // Create triple-layered meta variables for all original meta variables. We will use them later in the elimination process.
            for (auto const& metaVariablePair : this->rowColumnMetaVariablePairs) {
                auto rowVariable = metaVariablePair.first;
                
                storm::dd::DdMetaVariable<DdType> const& metaVariable = ddManager.getMetaVariable(rowVariable);
                
                std::vector<storm::expressions::Variable> newMetaVariables;
                
                // Find a suitable name for the temporary variable.
                uint64_t counter = 0;
                std::string newMetaVariableName = "tmp_" + metaVariable.getName();
                while (ddManager.hasMetaVariable(newMetaVariableName + std::to_string(counter))) {
                    ++counter;
                }
                
                newMetaVariables = ddManager.cloneVariable(metaVariablePair.first, newMetaVariableName + std::to_string(counter), 3);
                
                newRowVariables.insert(newMetaVariables[0]);
                newColumnVariables.insert(newMetaVariables[1]);
                helperVariables.insert(newMetaVariables[2]);
                
                newRowColumnMetaVariablePairs.emplace_back(newMetaVariables[0], newMetaVariables[1]);
                columnHelperMetaVariablePairs.emplace_back(newMetaVariables[1], newMetaVariables[2]);
                
                rowRowMetaVariablePairs.emplace_back(metaVariablePair.first, newMetaVariables[0]);
                columnColumnMetaVariablePairs.emplace_back(metaVariablePair.second, newMetaVariables[1]);
                
                oldToNewMapping.emplace_back(std::move(newMetaVariables));
            }
            
            oldNewMetaVariablePairs = rowRowMetaVariablePairs;
            for (auto const& entry : columnColumnMetaVariablePairs) {
                oldNewMetaVariablePairs.emplace_back(entry.first, entry.second);
            }
            
            shiftMetaVariablePairs = newRowColumnMetaVariablePairs;
            for (auto const& entry : columnHelperMetaVariablePairs) {
                shiftMetaVariablePairs.emplace_back(entry.first, entry.second);
            }
        }
        
        template<storm::dd::DdType DdType, typename ValueType>
        storm::dd::Add<DdType, ValueType> SymbolicEliminationLinearEquationSolver<DdType, ValueType>::solveEquations(storm::dd::Add<DdType, ValueType> const& x, storm::dd::Add<DdType, ValueType> const& b) const {
            storm::dd::DdManager<DdType>& ddManager = x.getDdManager();
            
            // Build diagonal BDD over new meta variables.
            storm::dd::Bdd<DdType> diagonal = storm::utility::dd::getRowColumnDiagonal(ddManager, this->rowColumnMetaVariablePairs);
            diagonal &= this->allRows;
            diagonal = diagonal.swapVariables(this->oldNewMetaVariablePairs);

            storm::dd::Add<DdType, ValueType> rowsAdd = this->allRows.swapVariables(rowRowMetaVariablePairs).template toAdd<ValueType>();
            storm::dd::Add<DdType, ValueType> diagonalAdd = diagonal.template toAdd<ValueType>();
            
            // Revert the conversion to an equation system and move it to the new meta variables.
            storm::dd::Add<DdType, ValueType> matrix = diagonalAdd - this->A.swapVariables(oldNewMetaVariablePairs);
            
            // Initialize solution over the new meta variables.
            storm::dd::Add<DdType, ValueType> solution = b.swapVariables(oldNewMetaVariablePairs);
            
            // As long as there are transitions, we eliminate them.
            uint64_t iterations = 0;
            while (!matrix.isZero()) {
                // Determine inverse loop probabilies.
                storm::dd::Add<DdType, ValueType> inverseLoopProbabilities = rowsAdd / (rowsAdd - (diagonalAdd * matrix).sumAbstract(newColumnVariables));
                
                // Scale all transitions with the inverse loop probabilities.
                matrix *= inverseLoopProbabilities;
                
                solution *= inverseLoopProbabilities;
                
                // Delete diagonal elements, i.e. remove self-loops.
                matrix = diagonal.ite(ddManager.template getAddZero<ValueType>(), matrix);
            
                // Update the one-step probabilities.
                solution += (matrix * solution.swapVariables(newRowColumnMetaVariablePairs)).sumAbstract(newColumnVariables);
                
                // Shortcut all transitions by eliminating one intermediate step.
                matrix = matrix.multiplyMatrix(matrix.permuteVariables(shiftMetaVariablePairs), newColumnVariables);
                matrix = matrix.swapVariables(columnHelperMetaVariablePairs);
                
                ++iterations;
                STORM_LOG_TRACE("Completed iteration " << iterations << " of elimination process.");
            }
            
            STORM_LOG_INFO("Elimination completed in " << iterations << " iterations.");
            
            return solution.swapVariables(rowRowMetaVariablePairs);
        }

        template<storm::dd::DdType DdType, typename ValueType>
        std::unique_ptr<storm::solver::SymbolicLinearEquationSolver<DdType, ValueType>> SymbolicEliminationLinearEquationSolverFactory<DdType, ValueType>::create(storm::dd::Bdd<DdType> const& allRows, std::set<storm::expressions::Variable> const& rowMetaVariables, std::set<storm::expressions::Variable> const& columnMetaVariables, std::vector<std::pair<storm::expressions::Variable, storm::expressions::Variable>> const& rowColumnMetaVariablePairs) const {
            return std::make_unique<SymbolicEliminationLinearEquationSolver<DdType, ValueType>>(allRows, rowMetaVariables, columnMetaVariables, rowColumnMetaVariablePairs);
        }
            
        template<storm::dd::DdType DdType, typename ValueType>
        SymbolicEliminationLinearEquationSolverSettings<ValueType>& SymbolicEliminationLinearEquationSolverFactory<DdType, ValueType>::getSettings() {
            return settings;
        }

        template<storm::dd::DdType DdType, typename ValueType>
        SymbolicEliminationLinearEquationSolverSettings<ValueType> const& SymbolicEliminationLinearEquationSolverFactory<DdType, ValueType>::getSettings() const {
            return settings;
        }
        
        template class SymbolicEliminationLinearEquationSolver<storm::dd::DdType::CUDD, double>;
        template class SymbolicEliminationLinearEquationSolver<storm::dd::DdType::CUDD, storm::RationalNumber>;
        template class SymbolicEliminationLinearEquationSolver<storm::dd::DdType::Sylvan, double>;
        template class SymbolicEliminationLinearEquationSolver<storm::dd::DdType::Sylvan, storm::RationalNumber>;
        template class SymbolicEliminationLinearEquationSolver<storm::dd::DdType::Sylvan, storm::RationalFunction>;

        template class SymbolicEliminationLinearEquationSolverFactory<storm::dd::DdType::CUDD, double>;
        template class SymbolicEliminationLinearEquationSolverFactory<storm::dd::DdType::CUDD, storm::RationalNumber>;
        template class SymbolicEliminationLinearEquationSolverFactory<storm::dd::DdType::Sylvan, double>;
        template class SymbolicEliminationLinearEquationSolverFactory<storm::dd::DdType::Sylvan, storm::RationalNumber>;
        template class SymbolicEliminationLinearEquationSolverFactory<storm::dd::DdType::Sylvan, storm::RationalFunction>;
        
    }
}
