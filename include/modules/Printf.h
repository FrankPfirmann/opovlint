//
// Created by frank on 7/31/19.
//

#ifndef MODULES_PRINT_H
#define MODULES_PRINT_H


#include <core/module/ASTMatcherModule.h>

namespace opov {
namespace module {

class Printf : public opov::ASTMatcherModule {
private:
    std::vector<std::string> functions_n;
public:
    Printf();

    void setupOnce(const Configuration *config) override;

    void setupMatcher() override;

    void run(const clang::ast_matchers::MatchFinder::MatchResult &result) override;

    std::string moduleName() override;

    std::string moduleDescription() override;

    ~Printf() override;
};

} /* namespace module */
} /* namespace opov */

#endif  // MODULES_PRINT_H
