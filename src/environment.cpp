#include "environment.hpp"

#include <cstdlib>
#include <cstring>
#include <sstream>
#include <unistd.h>

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
    if (var == "?") {
        return std::to_string(lastExitCode);
    }
    auto it = variables.find(var);
    if (it != variables.end()) return it->second;
    const char* env_val = getenv(var.c_str());
    if (env_val) return std::string(env_val);
    return "";
}

void Environment::set(const std::string& var, const std::string& value) {
    variables[var] = value;
    setenv(var.c_str(), value.c_str(), 1);
}

std::string Environment::expand(const Argument* arg) const {
    if (!arg) return "";
    if (arg->type == Argument::VARIABLE) {
        return get(arg->value);
    }
    if (arg->type == Argument::COMPOSITE) {
        std::string result;
        for (auto part : arg->parts) {
            result += expand(part);
        }
        return result;
    }
    return arg->value;
}
