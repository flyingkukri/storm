#ifndef DFTELEMENTS_H
#define	DFTELEMENTS_H

#include <set>
#include <vector>
#include <memory>
#include <string>
#include <cassert>
#include <cstdlib>
#include <iostream>

#include "DFTState.h"
#include "DFTStateSpaceGenerationQueues.h"
#include "src/utility/constants.h"
#include "src/adapters/CarlAdapter.h"

using std::size_t;

namespace storm {
    namespace storage {

        template<typename ValueType>
        class DFTGate;

        template<typename ValueType>
        class DFTElement {

            using DFTGatePointer = std::shared_ptr<DFTGate<ValueType>>;
            using DFTGateVector = std::vector<DFTGatePointer>;


        protected:
            size_t mId;
            std::string mName;
            size_t mRank = -1;
            DFTGateVector mParents;

        public:
            DFTElement(size_t id, std::string const& name) :
                    mId(id), mName(name)
            {}

            virtual ~DFTElement() {}

            
            virtual size_t id() const {
                return mId;
            }
            
            virtual void setRank(size_t rank) {
                mRank = rank;
            }
            
            virtual size_t rank() const {
                return mRank;
            }
            
            virtual bool isConstant() const {
                return false;
            }
            
            virtual bool isGate() const {
                return false;
            }
            
            virtual bool isBasicElement() const {
                return false;
            }
            
            virtual bool isColdBasicElement() const {
                return false;
            }
            
            virtual bool isSpareGate() const {
                return false;
            }
            
            virtual void setId(size_t newId) {
                mId = newId;
            }
            
            virtual std::string const& name() const {
                return mName;
            }
            
            bool addParent(DFTGatePointer const& e) {
                if(std::find(mParents.begin(), mParents.end(), e) != mParents.end()) {
                    return false;
                }
                else 
                {
                    mParents.push_back(e);
                    return true;
                }
            }

            bool hasParents() const {
                return !mParents.empty();
            }
            
            DFTGateVector const& parents() const {
                return mParents;
            }
            
            virtual void extendSpareModule(std::set<size_t>& elementsInModule) const;
            
            virtual size_t nrChildren() const = 0;

            virtual std::string toString() const = 0;

            virtual bool checkDontCareAnymore(storm::storage::DFTState<ValueType>& state, DFTStateSpaceGenerationQueues<ValueType>& queues) const;
            
            virtual std::vector<size_t> independentUnit() const = 0;
        
            virtual void extendUnit(std::set<size_t>& unit) const;

            void checkForSymmetricChildren() const;
        };


        enum class DFTElementTypes {AND, COUNTING, OR, VOT, BE, CONSTF, CONSTS, PAND, SPARE, POR, FDEP, SEQAND};

        inline bool isGateType(DFTElementTypes const& tp) {
            switch(tp) {
                case DFTElementTypes::AND:
                case DFTElementTypes::COUNTING:
                case DFTElementTypes::OR:
                case DFTElementTypes::VOT:
                case DFTElementTypes::PAND:
                case DFTElementTypes::SPARE:
                case DFTElementTypes::POR:
                case DFTElementTypes::SEQAND:
                    return true;
                case DFTElementTypes::BE:
                case DFTElementTypes::CONSTF:
                case DFTElementTypes::CONSTS:
                case DFTElementTypes::FDEP:
                    return false;
                default:
                    assert(false);
            }
        }



        template<typename ValueType>
        class DFTGate : public DFTElement<ValueType> {

            using DFTElementPointer = std::shared_ptr<DFTElement<ValueType>>;
            using DFTElementVector = std::vector<DFTElementPointer>;

        protected:
            DFTElementVector mChildren;

        public:
            DFTGate(size_t id, std::string const& name, DFTElementVector const& children) :
                    DFTElement<ValueType>(id, name), mChildren(children)
            {}
            
            virtual ~DFTGate() {}
            
            void pushBackChild(DFTElementPointer elem) {
                return mChildren.push_back(elem);
            }

            size_t nrChildren() const override {
                return mChildren.size();
            }

            DFTElementVector const& children() const {
                return mChildren;
            }
            
            virtual bool isGate() const override {
                return true;
            }
            
            
            virtual std::string typestring() const = 0;

            virtual void checkFails(storm::storage::DFTState<ValueType>& state, DFTStateSpaceGenerationQueues<ValueType>& queues) const = 0;

            virtual void checkFailsafe(storm::storage::DFTState<ValueType>& state, DFTStateSpaceGenerationQueues<ValueType>& queues) const = 0;
            
            virtual void extendSpareModule(std::set<size_t>& elementsInSpareModule) const override {
                DFTElement<ValueType>::extendSpareModule(elementsInSpareModule);
                for(auto const& child : mChildren) {
                    if(elementsInSpareModule.count(child->id()) == 0) {
                        elementsInSpareModule.insert(child->id());
                        child->extendSpareModule(elementsInSpareModule);
                    }
                }
            }
            
            virtual std::vector<size_t> independentUnit() const override {
                std::set<size_t> unit = {this->mId};
                for(auto const& child : mChildren) {
                    child->extendUnit(unit);
                }
                for(auto const& parent : this->mParents) {
                    if(unit.count(parent->id()) != 0) {
                        return {};
                    } 
                }
                return std::vector<size_t>(unit.begin(), unit.end());
            }
            
            virtual std::string toString() const override {
                std::stringstream stream;
                stream << "{" << this->name() << "} " << typestring() << "( ";
                typename DFTElementVector::const_iterator it = mChildren.begin();
                stream << (*it)->name();
                ++it;
                while(it != mChildren.end()) {
                    stream <<  ", " << (*it)->name();
                    ++it;
                }
                stream << ")";
                return stream.str();
            }

            virtual bool checkDontCareAnymore(storm::storage::DFTState<ValueType>& state, DFTStateSpaceGenerationQueues<ValueType>& queues) const override {
                if(DFTElement<ValueType>::checkDontCareAnymore(state, queues)) {
                    childrenDontCare(state, queues);
                    return true;
                }
                return false;
            }
            
            virtual void extendUnit(std::set<size_t>& unit) const override {
                DFTElement<ValueType>::extendUnit(unit);
                for(auto const& child : mChildren) {
                    child->extendUnit(unit);
                }
            }
            
        protected:

            void fail(DFTState<ValueType>& state, DFTStateSpaceGenerationQueues<ValueType>& queues) const {
                for(std::shared_ptr<DFTGate> parent : this->mParents) {
                    if(state.isOperational(parent->id())) {
                        queues.propagateFailure(parent);
                    }
                }
                state.setFailed(this->mId);
            }

            void failsafe(DFTState<ValueType>& state, DFTStateSpaceGenerationQueues<ValueType>& queues) const {
                for(std::shared_ptr<DFTGate> parent : this->mParents) {
                    if(state.isOperational(parent->id())) {
                        queues.propagateFailsafe(parent);
                    }
                }
                state.setFailsafe(this->mId);
            }

            void childrenDontCare(DFTState<ValueType>& state, DFTStateSpaceGenerationQueues<ValueType>& queues) const {
                queues.propagateDontCare(mChildren);
            }

            bool hasFailsafeChild(DFTState<ValueType>& state) const {
                for(auto const& child : mChildren) {
                    if(state.isFailsafe(child->id()))
                    {
                        return true;
                    }
                }
                return false;
            }

            bool hasFailedChild(DFTState<ValueType>& state) const {
                for(auto const& child : mChildren) {
                    if(state.hasFailed(child->id())) {
                        return true;
                    }
                }
                return false;
            }

        };
        
       

        template<typename ValueType>
        class DFTBE : public DFTElement<ValueType> {

            ValueType mActiveFailureRate;
            ValueType mPassiveFailureRate;

        public:
            DFTBE(size_t id, std::string const& name, ValueType failureRate, ValueType dormancyFactor) :
                    DFTElement<ValueType>(id, name), mActiveFailureRate(failureRate), mPassiveFailureRate(dormancyFactor * failureRate)
            {}
                        
            virtual size_t nrChildren() const {
                return 0;
            }

            ValueType const& activeFailureRate() const {
                return mActiveFailureRate;
            }

            ValueType const& passiveFailureRate() const {
                return mPassiveFailureRate;
            }
        
            std::string toString() const {
                std::stringstream stream;
                stream << *this;
                return stream.str();
            }
            
            bool isBasicElement() const {
                return true;
            }
            
            bool isColdBasicElement() const {
                return storm::utility::isZero(mPassiveFailureRate);
            }

            virtual std::vector<size_t> independentUnit() const {
                return {this->mId};
            }

            virtual bool checkDontCareAnymore(storm::storage::DFTState<ValueType>& state, DFTStateSpaceGenerationQueues<ValueType>& queues) const;
        };

        template<typename ValueType>
        inline std::ostream& operator<<(std::ostream& os, DFTBE<ValueType> const& be) {
            return os << "{" << be.name() << "} BE(" << be.activeFailureRate() << ", " << be.passiveFailureRate() << ")";
        }



        template<typename ValueType>
        class DFTConst : public DFTElement<ValueType> {

            bool mFailed;

        public:
            DFTConst(size_t id, std::string const& name, bool failed) :
                    DFTElement<ValueType>(id, name), mFailed(failed)
            {}
            
            bool failed() const {
                return mFailed;
            }
            
            virtual bool isConstant() const {
                return true;
            }
            
            virtual size_t nrChildren() const {
                return 0;
            }
            
        };



        template<typename ValueType>
        class DFTAnd : public DFTGate<ValueType> {
            
        public:
            DFTAnd(size_t id, std::string const& name, std::vector<std::shared_ptr<DFTElement<ValueType>>> const& children = {}) :
                    DFTGate<ValueType>(id, name, children)
            {}

            void checkFails(storm::storage::DFTState<ValueType>& state, DFTStateSpaceGenerationQueues<ValueType>& queues) const {
                if(state.isOperational(this->mId)) {
                    for(auto const& child : this->mChildren)
                    {
                        if(!state.hasFailed(child->id())) {
                            return;
                        }
                    }
                    this->fail(state, queues);
                }
            }

            void checkFailsafe(storm::storage::DFTState<ValueType>& state, DFTStateSpaceGenerationQueues<ValueType>& queues) const{
                assert(this->hasFailsafeChild(state));
                if(state.isOperational(this->mId)) {
                    this->failsafe(state, queues);
                    this->childrenDontCare(state, queues);
                }
            }
            
            std::string typestring() const {
                return "AND";
            }
        };

        template<typename ValueType>
        inline std::ostream& operator<<(std::ostream& os, DFTAnd<ValueType> const& gate) {
            return os << gate.toString();
        }

        

        template<typename ValueType>
        class DFTOr : public DFTGate<ValueType> {

        public:
            DFTOr(size_t id, std::string const& name, std::vector<std::shared_ptr<DFTElement<ValueType>>> const& children = {}) :
                    DFTGate<ValueType>(id, name, children)
            {}

            void checkFails(storm::storage::DFTState<ValueType>& state, DFTStateSpaceGenerationQueues<ValueType>& queues) const {
                assert(this->hasFailedChild(state));
                if(state.isOperational(this->mId)) {
                    this->fail(state, queues);
                }
            }

            void checkFailsafe(storm::storage::DFTState<ValueType>& state, DFTStateSpaceGenerationQueues<ValueType>& queues) const{
                 for(auto const& child : this->mChildren) {
                     if(!state.isFailsafe(child->id())) {
                         return;
                     }
                 }
                 this->failsafe(state, queues);
            }
            
            std::string typestring() const {
                return "OR";
            }
        };

        template<typename ValueType>
        inline std::ostream& operator<<(std::ostream& os, DFTOr<ValueType> const& gate) {
            return os << gate.toString();
        }



        template<typename ValueType>
        class DFTSeqAnd : public DFTGate<ValueType> {

        public:
            DFTSeqAnd(size_t id, std::string const& name, std::vector<std::shared_ptr<DFTElement<ValueType>>> const& children = {}) :
                    DFTGate<ValueType>(id, name, children)
            {}

            void checkFails(storm::storage::DFTState<ValueType>& state,  DFTStateSpaceGenerationQueues<ValueType>& queues) const {
                if(!state.hasFailed(this->mId)) {
                    bool childOperationalBefore = false;
                    for(auto const& child : this->mChildren)
                    {
                        if(!state.hasFailed(child->id())) {
                            childOperationalBefore = true;
                        }
                        else {
                            if(childOperationalBefore) {
                                state.markAsInvalid();
                                return;
                            }
                        }
                    }
                    if(!childOperationalBefore) {
                       fail(state, queues);
                    }
                  
                } 
            }

            void checkFailsafe(storm::storage::DFTState<ValueType>& state, DFTStateSpaceGenerationQueues<ValueType>& queues) const{
                assert(hasFailsafeChild(state));
                if(state.isOperational(this->mId)) {
                    failsafe(state, queues);
                    //return true;
                }
                //return false;
            }
            
            std::string  typestring() const {
                return "SEQAND";
            }
        };

        template<typename ValueType>
        inline std::ostream& operator<<(std::ostream& os, DFTSeqAnd<ValueType> const& gate) {
             return os << gate.toString();
        }



        template<typename ValueType>
        class DFTPand : public DFTGate<ValueType> {

        public:
            DFTPand(size_t id, std::string const& name, std::vector<std::shared_ptr<DFTElement<ValueType>>> const& children = {}) :
                    DFTGate<ValueType>(id, name, children)
            {}

            void checkFails(storm::storage::DFTState<ValueType>& state,  DFTStateSpaceGenerationQueues<ValueType>& queues) const {
                if(state.isOperational(this->mId)) {
                    bool childOperationalBefore = false;
                    for(auto const& child : this->mChildren)
                    {
                        if(!state.hasFailed(child->id())) {
                            childOperationalBefore = true;
                        } else if(childOperationalBefore && state.hasFailed(child->id())){
                            this->failsafe(state, queues);
                            this->childrenDontCare(state, queues);
                            return;
                        }
                    }
                    if(!childOperationalBefore) {
                        this->fail(state, queues);
                    }
                }
            }

            void checkFailsafe(storm::storage::DFTState<ValueType>& state, DFTStateSpaceGenerationQueues<ValueType>& queues) const{
                assert(this->hasFailsafeChild(state));
                if(state.isOperational(this->mId)) {
                    this->failsafe(state, queues);
                    this->childrenDontCare(state, queues);
                }
            }
            
            std::string typestring() const {
                return "PAND";
            }
        };
        
        template<typename ValueType>
        inline std::ostream& operator<<(std::ostream& os, DFTPand<ValueType> const& gate) {
             return os << gate.toString();
        }



        template<typename ValueType>
        class DFTPor : public DFTGate<ValueType> {
        public:
            DFTPor(size_t id, std::string const& name, std::vector<std::shared_ptr<DFTElement<ValueType>>> const& children = {}) :
                    DFTGate<ValueType>(id, name, children)
            {}

            void checkFails(storm::storage::DFTState<ValueType>& state, DFTStateSpaceGenerationQueues<ValueType>& queues) const {
                 assert(false);
            }

            void checkFailsafe(storm::storage::DFTState<ValueType>& state, DFTStateSpaceGenerationQueues<ValueType>& queues) const{
                assert(false);
            }
            
            std::string  typestring() const {
                return "POR";
            }
        };

        template<typename ValueType>
        inline std::ostream& operator<<(std::ostream& os, DFTPor<ValueType> const& gate) {
             return os << gate.toString();
        }



        template<typename ValueType>
        class DFTVot : public DFTGate<ValueType> {

        private:
            unsigned mThreshold;

        public:
            DFTVot(size_t id, std::string const& name, unsigned threshold, std::vector<std::shared_ptr<DFTElement<ValueType>>> const& children = {}) :
                    DFTGate<ValueType>(id, name, children), mThreshold(threshold)
            {}

            void checkFails(storm::storage::DFTState<ValueType>& state, DFTStateSpaceGenerationQueues<ValueType>& queues) const {
                if(state.isOperational(this->mId)) {
                    unsigned nrFailedChildren = 0;
                    for(auto const& child : this->mChildren)
                    {
                        if(state.hasFailed(child->id())) {
                            ++nrFailedChildren;
                            if(nrFailedChildren >= mThreshold) 
                            {
                                this->fail(state, queues);
                                return;
                            }
                        }
                    }
                  
                } 
            }

            void checkFailsafe(storm::storage::DFTState<ValueType>& state, DFTStateSpaceGenerationQueues<ValueType>& queues) const{
                assert(this->hasFailsafeChild(state));
                if(state.isOperational(this->mId)) {
                    unsigned nrFailsafeChildren = 0;
                    for(auto const& child : this->mChildren)
                    {
                        if(state.isFailsafe(child->id())) {
                            ++nrFailsafeChildren;
                            if(nrFailsafeChildren > this->nrChildren() - mThreshold)
                            {
                                this->failsafe(state, queues);
                                this->childrenDontCare(state, queues);
                                return;
                            }
                        }
                    }
                }
            }
            
            std::string typestring() const {
                return "VOT (" + std::to_string(mThreshold) + ")";
            }

        };

        template<typename ValueType>
        inline std::ostream& operator<<(std::ostream& os, DFTVot<ValueType> const& gate) {
            return os << gate.toString();
        }



        template<typename ValueType>
        class DFTSpare : public DFTGate<ValueType> {

        private:
            size_t mUseIndex;
            size_t mActiveIndex;
            
        public:
            DFTSpare(size_t id, std::string const& name, std::vector<std::shared_ptr<DFTElement<ValueType>>> const& children = {}) :
                    DFTGate<ValueType>(id, name, children)
            {}
            
            std::string typestring() const {
                return "SPARE";
            }
            
            bool isSpareGate() const {
                return true;
            }
            
            void setUseIndex(size_t useIndex) {
                mUseIndex = useIndex; 
            }
            
            void setActiveIndex(size_t activeIndex) {
                mActiveIndex = activeIndex;
            }

            void initializeUses(storm::storage::DFTState<ValueType>& state) {
                assert(this->mChildren.size() > 0);
                state.setUsesAtPosition(mUseIndex, this->mChildren[0]->id());
            }

            void checkFails(storm::storage::DFTState<ValueType>& state, DFTStateSpaceGenerationQueues<ValueType>& queues) const {
                if(state.isOperational(this->mId)) {
                    size_t uses = state.extractUses(mUseIndex);
                    if(!state.isOperational(uses)) {
                        // TODO compute children ids before.
                        std::vector<size_t> childrenIds;
                        for(auto const& child : this->mChildren) {
                            childrenIds.push_back(child->id());
                        }
                        
                        bool claimingSuccessful = state.claimNew(this->mId, mUseIndex, uses, childrenIds);
                        if(!claimingSuccessful) {
                            this->fail(state, queues);
                        }
                    }                  
                } 
            }

            void checkFailsafe(storm::storage::DFTState<ValueType>& state, DFTStateSpaceGenerationQueues<ValueType>& queues) const {
                if(state.isOperational(this->mId)) {
                    if(state.isFailsafe(state.extractUses((mUseIndex)))) {
                        this->failsafe(state, queues);
                        this->childrenDontCare(state, queues);
                    }
                }
            }
        };

    }
}



#endif	/* DFTELEMENTS_H */

