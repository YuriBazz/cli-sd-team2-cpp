#include "environment.hpp"

#include <cstdlib>
#include <cstring>

Environment::Environment() { loadEnv(); }

void Environment::loadEnv() {
    extern char** environ;
    for (char** env = environ; *env; ++env) {
        char* eq = strchr(*env, '=');
        if (eq) {
            std::string key(*env, eq - *env);
            std::string value(eq + 1);
            variables[key] = value;
        }
    }
}

std::string Environment::get(const std::string& var) const {
    auto it = variables.find(var);
    if (it != variables.end()) return it->second;
    return "";
}

void Environment::set(const std::string& var, const std::string& value) { variables[var] = value; }

std::string Environment::expand(const Argument* arg) const {
    if (arg->type == Argument::VARIABLE) {
        return get(arg->value);
    }
    return arg->value;
}