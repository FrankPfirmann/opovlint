//
// Created by frank on 9/7/19.
//

#ifndef MODULES_ASSIGN_H
#define MODULES_ASSIGN_H

#include <core/module/ASTMatcherModule.h>

namespace opov {
namespace module {

class Assign : public opov::ASTMatcherModule {
private:
    std::string tolerated_type;
public:
    Assign();

    void setupOnce(const Configuration *config) override;

    void setupMatcher() override;

    void run(const clang::ast_matchers::MatchFinder::MatchResult &result) override;

    std::string moduleName() override;

    std::string moduleDescription() override;

    ~Assign() override;
};

} /* namespace module */
} /* namespace opov */



#endif //MODULES_ASSIGN_H
