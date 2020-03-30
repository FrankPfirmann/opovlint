/*
 * ASTMatcherModule.cpp
 *
 *  Created on: May 26, 2014
 *      Author: ahueck
 */

#include <core/module/ASTMatcherModule.h>
#include <core/configuration/Configuration.h>
#include <core/module/ModuleContext.h>

namespace opov {

ASTMatcherModule::ASTMatcherModule() {
}

void ASTMatcherModule::init(const Configuration* config) {
  config->getVector("global:type", type_s);
  config->getValue("global:transform", transform, false);
  setupOnce(config);
  setupMatcher();
}

void ASTMatcherModule::execute(ModuleContext* context) {
  AbstractModule::execute(context);
  // setupMatcher();
  finder.matchAST(context->getASTContext());
}

ASTMatcherModule::~ASTMatcherModule() = default;

} /* namespace opov */
