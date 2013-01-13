/*
 * Expression.h
 *
 *  Created on: 03.01.2013
 *      Author: Christian Dehnert
 */

#ifndef STORM_IR_EXPRESSIONS_BASEEXPRESSION_H_
#define STORM_IR_EXPRESSIONS_BASEEXPRESSION_H_

#include "src/exceptions/ExpressionEvaluationException.h"
#include "src/exceptions/NotImplementedException.h"

#include <string>
#include <vector>

namespace storm {

namespace ir {

namespace expressions {

class BaseExpression {

public:
	enum ReturnType {undefined, bool_, int_, double_};

	BaseExpression() : type(undefined) {

	}

	BaseExpression(ReturnType type) : type(type) {

	}

	virtual ~BaseExpression() {

	}

	virtual int_fast64_t getValueAsInt(std::vector<bool> const& booleanVariableValues, std::vector<int_fast64_t> const& integerVariableValues) const {
		if (type != int_) {
			throw storm::exceptions::ExpressionEvaluationException() << "Cannot evaluate expression of type '"
					<< this->getTypeName() << "' as 'int'.";
		}
		throw storm::exceptions::NotImplementedException() << "Cannot evaluate expression of type '"
					<< this->getTypeName() << " because evaluation implementation is missing.";
	}

	virtual bool getValueAsBool(std::vector<bool> const& booleanVariableValues, std::vector<int_fast64_t> const& integerVariableValues) const {
		if (type != bool_) {
			throw storm::exceptions::ExpressionEvaluationException() << "Cannot evaluate expression of type '"
					<< this->getTypeName() << "' as 'bool'.";
		}
		throw storm::exceptions::NotImplementedException() << "Cannot evaluate expression of type '"
					<< this->getTypeName() << " because evaluation implementation is missing.";
	}

	virtual double getValueAsDouble(std::vector<bool> const& booleanVariableValues, std::vector<int_fast64_t> const& integerVariableValues) const {
		if (type != bool_) {
			throw storm::exceptions::ExpressionEvaluationException() << "Cannot evaluate expression of type '"
					<< this->getTypeName() << "' as 'double'.";
		}
		throw storm::exceptions::NotImplementedException() << "Cannot evaluate expression of type '"
					<< this->getTypeName() << " because evaluation implementation is missing.";
	}

	virtual std::string toString() const = 0;

	std::string getTypeName() const {
		switch(type) {
		case bool_: return std::string("bool");
		case int_: return std::string("int");
		case double_: return std::string("double");
		default: return std::string("undefined");
		}
	}

	ReturnType getType() const {
		return type;
	}

private:
	ReturnType type;
};

} // namespace expressions

} // namespace ir

} // namespace storm

#endif /* STORM_IR_EXPRESSIONS_BASEEXPRESSION_H_ */
