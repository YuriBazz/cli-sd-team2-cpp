#ifndef EXECUTOR_HPP
#define EXECUTOR_HPP

#include "ast.hpp"
#include "environment.hpp"

class Executor {
public:
    Executor(Environment& env) : env(env) {}
    void execute(Statement* stmt);
    
    bool shouldExit() const { return env.shouldExit(); }
private:
    Environment& env;
};

#endif