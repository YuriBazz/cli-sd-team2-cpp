#include "ast.hpp"
#include "environment.hpp"
#include "executor.hpp"
#include "parser.tab.h"

extern FILE* yyin;
Executor* g_executor = nullptr;

int main() {
    Environment env;
    Executor executor(env);
    g_executor = &executor;

    yyin = stdin;
    yyparse();

    return 0;
}
