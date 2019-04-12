/*
 * Copyright (C) 2008  Remko Troncon
 * See COPYING for license details.
 */

#ifndef RANDRANDOMNUMBERGENERATOR_H
#define RANDRANDOMNUMBERGENERATOR_H

#include "xmpp/base/randomnumbergenerator.h"

namespace XMPP {
	class RandRandomNumberGenerator : public RandomNumberGenerator
	{
		public:
			RandRandomNumberGenerator() {}

			double generateNumber() const override {
				return rand();
			}

			double getMaximumGeneratedNumber() const override {
				return RAND_MAX;
			}
	};
}

#endif
