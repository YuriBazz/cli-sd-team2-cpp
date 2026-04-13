#ifndef ENVIRONMENT_HPP
#define ENVIRONMENT_HPP

#include <string>
#include <unordered_map>

#include "ast.hpp"

class Environment {
   public:
    Environment();
    std::string get(const std::string& var) const;
    void set(const std::string& var, const std::string& value);
    std::string expand(const Argument* arg) const;
    std::string expandToString(const Argument* arg) const;
    void loadEnv();
    bool shouldExit() const { return exitFlag; }
    void setExit(bool flag, int code = 0) { exitFlag = flag; exitCode = code; }
    int getExitCode() const { return exitCode; }
    int getLastExitCode() const { return lastExitCode; }
    void setLastExitCode(int code) { lastExitCode = code; }
    std::unordered_map<std::string, std::string>& getVariables() { return variables; }
    const std::unordered_map<std::string, std::string>& getVariables() const { return variables; }

   private:
    std::unordered_map<std::string, std::string> variables;
    bool exitFlag = false;
    int exitCode = 0;
    int lastExitCode = 0;
};

#endif
