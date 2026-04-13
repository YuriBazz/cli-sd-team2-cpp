#include <sys/wait.h>
#include <unistd.h>

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <vector>

#include "ast.hpp"
#include "environment.hpp"

static std::vector<std::string> expandArgs(const ArgumentList* args, const Environment& env) {
    std::vector<std::string> result;
    if (!args) return result;
    for (const auto* arg : *args) {
        std::string expanded = env.expand(arg);
        if (arg->type == Argument::VARIABLE) {
            std::istringstream iss(expanded);
            std::string word;
            while (iss >> word) {
                result.push_back(word);
            }
        } else {
            result.push_back(expanded);
        }
    }
    return result;
}

void CatCommand::execute(Environment& env, int inputFd, int outputFd) {
    auto args = expandArgs(this->args, env);
    if (args.empty()) {
        char buf[4096];
        ssize_t n;
        while ((n = read(inputFd, buf, sizeof(buf))) > 0) {
            write(outputFd, buf, n);
        }
        return;
    }

    for (const auto& fname : args) {
        std::ifstream file(fname);
        if (!file) {
            std::string err = "cat: " + fname + ": No such file or directory\n";
            write(outputFd, err.c_str(), err.size());
            continue;
        }
        std::string line;
        while (std::getline(file, line)) {
            line += '\n';
            write(outputFd, line.c_str(), line.size());
        }
    }
}

void EchoCommand::execute(Environment& env, int inputFd, int outputFd) {
    auto args = expandArgs(this->args, env);
    std::string out;
    for (size_t i = 0; i < args.size(); ++i) {
        if (i != 0) out += " ";
        out += args[i];
    }
    out += "\n";
    write(outputFd, out.c_str(), out.size());
}

void WcCommand::execute(Environment& env, int inputFd, int outputFd) {
    auto args = expandArgs(this->args, env);
    if (args.empty()) {
        std::string content;
        char buf[4096];
        ssize_t n;
        while ((n = read(inputFd, buf, sizeof(buf))) > 0) {
            content.append(buf, n);
        }
        size_t lines = std::count(content.begin(), content.end(), '\n');
        size_t words = 0;
        bool inWord = false;
        for (char c : content) {
            if (std::isspace(c)) {
                if (inWord) words++;
                inWord = false;
            } else {
                inWord = true;
            }
        }
        if (inWord) words++;
        size_t bytes = content.size();
        char result[256];
        snprintf(result, sizeof(result), "%zu %zu %zu\n", lines, words, bytes);
        write(outputFd, result, strlen(result));
        return;
    }
    for (const auto& fname : args) {
        std::ifstream file(fname, std::ios::binary);
        if (!file) {
            std::string err = "wc: " + fname + ": No such file\n";
            write(outputFd, err.c_str(), err.size());
            continue;
        }
        std::string content((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());
        size_t lines = std::count(content.begin(), content.end(), '\n');
        size_t words = 0;
        bool inWord = false;
        for (char c : content) {
            if (std::isspace(c)) {
                if (inWord) words++;
                inWord = false;
            } else {
                inWord = true;
            }
        }
        if (inWord) words++;
        size_t bytes = content.size();
        char result[256];
        snprintf(result, sizeof(result), "%zu %zu %zu %s\n", lines, words, bytes, fname.c_str());
        write(outputFd, result, strlen(result));
    }
}

void PwdCommand::execute(Environment& env, int inputFd, int outputFd) {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd))) {
        std::string out = std::string(cwd) + "\n";
        write(outputFd, out.c_str(), out.size());
    } else {
        std::string err = "pwd: error retrieving current directory\n";
        write(outputFd, err.c_str(), err.size());
    }
}

void ExitCommand::execute(Environment& env, int inputFd, int outputFd) {
    int code = 0;
    if (args && !args->empty()) {
        std::string codeStr = env.expand(args->front());
        try {
            code = std::stoi(codeStr);
        } catch (...) {
            code = 0;
        }
    }
    write(outputFd, "exit\n", 5);
    env.setExit(true, code);
    exit(code);
}

void GrepCommand::execute(Environment& env, int inputFd, int outputFd) {
    auto args = expandArgs(this->args, env);
    if (args.empty()) {
        std::string err = "grep: missing pattern\n";
        write(outputFd, err.c_str(), err.size());
        return;
    }

    bool opt_i = false, opt_c = false, opt_l = false, opt_w = false;
    int opt_A = 0;
    std::string pattern;
    std::vector<std::string> files;

    for (size_t i = 0; i < args.size(); ++i) {
        const std::string& arg = args[i];
        if (arg == "-i")
            opt_i = true;
        else if (arg == "-c")
            opt_c = true;
        else if (arg == "-l")
            opt_l = true;
        else if (arg == "-w")
            opt_w = true;
        else if (arg == "-A" && i + 1 < args.size()) {
            opt_A = std::stoi(args[++i]);
        } else if (pattern.empty()) {
            pattern = arg;
        } else {
            files.push_back(arg);
        }
    }

    if (pattern.empty()) {
        std::string err = "grep: pattern not specified\n";
        write(outputFd, err.c_str(), err.size());
        return;
    }

    std::regex::flag_type flags = std::regex::ECMAScript;
    if (opt_i) flags |= std::regex::icase;
    std::regex re(pattern, flags);

    auto match_word = [&](const std::string& line) -> bool {
        if (!opt_w) return std::regex_search(line, re);
        std::regex word_re("\\b" + pattern + "\\b",
                           opt_i ? std::regex::icase : std::regex::ECMAScript);
        return std::regex_search(line, word_re);
    };

    auto process_stream = [&](std::istream& is, const std::string& filename, bool isFile) {
        std::vector<std::string> lines;
        std::string line;
        int matchCount = 0;
        bool fileHasMatch = false;

        while (std::getline(is, line)) {
            lines.push_back(line);
            if (match_word(line)) {
                matchCount++;
                fileHasMatch = true;
            }
        }

        if (opt_l) {
            if (fileHasMatch) {
                std::string out = filename + "\n";
                write(outputFd, out.c_str(), out.size());
            }
            return;
        }

        if (opt_c) {
            char buf[64];
            snprintf(buf, sizeof(buf), "%d\n", matchCount);
            if (files.size() > 1) {
                std::string out = filename + ":" + buf;
                write(outputFd, out.c_str(), out.size());
            } else {
                write(outputFd, buf, strlen(buf));
            }
            return;
        }

        for (size_t i = 0; i < lines.size(); ++i) {
            if (match_word(lines[i])) {
                if (files.size() > 1) {
                    std::string prefix = filename + ":";
                    write(outputFd, prefix.c_str(), prefix.size());
                }
                std::string out = lines[i] + "\n";
                write(outputFd, out.c_str(), out.size());

                if (opt_A > 0) {
                    for (int k = 1; k <= opt_A && i + k < lines.size(); ++k) {
                        std::string ctx = "-" + lines[i + k] + "\n";
                        write(outputFd, ctx.c_str(), ctx.size());
                    }
                }
            }
        }
    };

    if (files.empty()) {
        std::istream* is = &std::cin;
        if (inputFd != 0) {
            std::string content;
            char buf[4096];
            ssize_t n;
            while ((n = read(inputFd, buf, sizeof(buf))) > 0) {
                content.append(buf, n);
            }
            std::istringstream iss(content);
            process_stream(iss, "(standard input)", false);
        } else {
            process_stream(*is, "(standard input)", false);
        }
    } else {
        for (const auto& fname : files) {
            std::ifstream file(fname);
            if (!file) {
                std::string err = "grep: " + fname + ": No such file\n";
                write(outputFd, err.c_str(), err.size());
                continue;
            }
            process_stream(file, fname, true);
        }
    }
}

std::string ExternalCommand::findInPath(const std::string& cmd, const Environment& env) {
    if (cmd.find('/') != std::string::npos) {
        if (access(cmd.c_str(), X_OK) == 0) return cmd;
        return "";
    }

    std::string pathEnv = env.get("PATH");
    if (pathEnv.empty()) pathEnv = getenv("PATH") ? getenv("PATH") : "";

    size_t start = 0;
    size_t end;
    while ((end = pathEnv.find(':', start)) != std::string::npos) {
        std::string dir = pathEnv.substr(start, end - start);
        if (!dir.empty()) {
            std::string fullPath = dir + "/" + cmd;
            if (access(fullPath.c_str(), X_OK) == 0) return fullPath;
        }
        start = end + 1;
    }
    if (start < pathEnv.length()) {
        std::string dir = pathEnv.substr(start);
        if (!dir.empty()) {
            std::string fullPath = dir + "/" + cmd;
            if (access(fullPath.c_str(), X_OK) == 0) return fullPath;
        }
    }
    return "";
}

void ExternalCommand::execute(Environment& env, int inputFd, int outputFd) {
    std::string expandedCommand;
    if (!command.empty() && command[0] == '$') {
        std::string varName = command.substr(1);
        expandedCommand = env.get(varName);
        if (expandedCommand.empty()) {
            expandedCommand = command;
        }
    } else {
        expandedCommand = command;
    }

    std::vector<std::string> expandedArgs;
    if (args) {
        expandedArgs.reserve(args->size());
        for (const auto arg : *args) {
            expandedArgs.push_back(env.expand(arg));
        }
    }

    std::string cmdPath = findInPath(expandedCommand, env);
    if (cmdPath.empty()) cmdPath = expandedCommand;

    expandedArgs.insert(expandedArgs.begin(), cmdPath);

    std::vector<char*> argv;
    argv.reserve(expandedArgs.size() + 1);
    for (auto& arg : expandedArgs) {
        argv.push_back(const_cast<char*>(arg.c_str()));
    }
    argv.push_back(nullptr);

    if (inputFd != 0) {
        dup2(inputFd, STDIN_FILENO);
        close(inputFd);
    }
    if (outputFd != 1) {
        dup2(outputFd, STDOUT_FILENO);
        close(outputFd);
    }

    for (const auto& pair : env.getVariables()) {
        setenv(pair.first.c_str(), pair.second.c_str(), 1);
    }

    execvp(argv[0], argv.data());

    std::string err = "external command failed: " + expandedCommand + "\n";
    write(STDERR_FILENO, err.c_str(), err.size());
    exit(127);
}
