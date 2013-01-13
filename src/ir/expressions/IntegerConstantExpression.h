/*
 * IntegerConstantExpression.h
 *
 *  Created on: 04.01.2013
 *      Author: chris
 */

#ifndef INTEGERCONSTANTEXPRESSION_H_
#define INTEGERCONSTANTEXPRESSION_H_

#include "ConstantExpression.h"

namespace storm {

namespace ir {

namespace expressions {

class IntegerConstantExpression : public ConstantExpression {
public:
	IntegerConstantExpression(std::string constantName) : ConstantExpression(int_, constantName), defined(false), value(0) {

	}

	virtual ~IntegerConstantExpression() {

	}

	virtual int_fast64_t getValueAsInt(std::vector<bool> const& booleanVariableValues, std::vector<int_fast64_t> const& integerVariableValues) const {
		if (!defined) {
			throw storm::exceptions::ExpressionEvaluationException() << "Cannot evaluate expression: "
					<< "Integer constant '" << this->getConstantName() << "' is undefined.";
		} else {
			return value;
		}
	}

	virtual std::string toString() const {
		std::string result = this->constantName;
		if (defined) {
			result += "[" + boost::lexical_cast<std::string>(value) + "]";
		}
		return result;
	}

	bool isDefined() {
		return defined;
	}

	int getValue() {
		return value;
	}

	void define(int_fast64_t value) {
		defined = true;
		this->value = value;
	}

private:
	bool defined;
	int_fast64_t value;
};

}

}

}

#endif /* INTEGERCONSTANTEXPRESSION_H_ */
