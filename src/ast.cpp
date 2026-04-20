#include "ast.hpp"

#include <sys/wait.h>
#include <unistd.h>

#include "environment.hpp"

void Assignment::execute(Environment& env) {
    if (!args) {
        env.set(varName, "");
        return;
    }

    if (args->empty()) {
        env.set(varName, "");
        return;
    }

    std::string value;
    for (const auto arg : *args) {
        std::string expanded = env.expand(arg);
        if (!value.empty()) value += " ";
        value += expanded;
    }
    env.set(varName, value);
}

void Pipeline::execute(Environment& env) {
    int numCommands = commands.size();
    if (numCommands == 0) return;

    if (numCommands == 1) {
        if (dynamic_cast<CdCommand*>(commands[0]) || dynamic_cast<ExitCommand*>(commands[0])) {
            commands[0]->execute(env, 0, 1);
            return;
        }
    }

    int prevFd = 0;
    int pipefd[2];
    std::vector<pid_t> children;
    int last_status = 0;

    for (size_t i = 0; i < commands.size(); ++i) {
        if (dynamic_cast<ExitCommand*>(commands[i])) {
            commands[i]->execute(env, 0, 1);
            return;
        }

        int outputFd = 1;
        if (i < commands.size() - 1) {
            if (pipe(pipefd) == -1) {
                perror("pipe");
                return;
            }
            outputFd = pipefd[1];
        }

        pid_t pid = fork();
        if (pid == 0) {
            if (prevFd != 0) {
                dup2(prevFd, 0);
                close(prevFd);
            }
            if (outputFd != 1) {
                dup2(outputFd, 1);
                close(outputFd);
            }
            commands[i]->execute(env, 0, 1);
            exit(0);
        } else if (pid > 0) {
            children.push_back(pid);
            if (prevFd != 0) close(prevFd);
            if (outputFd != 1) close(outputFd);
            if (i < commands.size() - 1) {
                prevFd = pipefd[0];
            } else {
                prevFd = 0;
            }
        } else {
            perror("fork");
            return;
        }
    }

    for (pid_t pid : children) {
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            last_status = WEXITSTATUS(status);
        }
    }

    env.setLastExitCode(last_status);
}
