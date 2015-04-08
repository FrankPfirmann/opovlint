/*
 * ModuleConsumer.cpp
 *
 *  Created on: May 7, 2014
 *      Author: ahueck
 */
#include <core/Logger.h>
#include <core/ModuleContext.h>
#include <ModuleConsumer.h>

namespace opov {

ModuleConsumer::ModuleConsumer(Module* module,
		ModuleContext* mcontext) :
		AbstractModuleConsumer(module, mcontext) {

}

ModuleConsumer::~ModuleConsumer() {
	// TODO Auto-generated destructor stub
	//llvm::outs() << "Killed ModuleConsumer: " << this << "\n";
	LOG_DEBUG("Killed ModuleConsumer: " << this);
}

} /* namespace opov */