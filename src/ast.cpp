#include "ast.hpp"
#include "environment.hpp"
#include <unistd.h>
#include <sys/wait.h>

void Assignment::execute(Environment& env) {
    if (args->empty()) {
        env.set(varName, "");
        return;
    }

    std::string value;
    for (auto arg : *args) {
        std::string expanded = env.expand(arg);
        if (!value.empty()) value += " ";
        value += expanded;
        delete arg;
    }
    env.set(varName, value);
    args->clear();
}

void Pipeline::execute(Environment& env) {
    int numCommands = commands.size();
    if (numCommands == 0) return;

    int prevFd = 0;
    int pipefd[2];

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
            if (prevFd != 0) close(prevFd);
            if (outputFd != 1) close(outputFd);
            prevFd = pipefd[0];
        } else {
            perror("fork");
            return;
        }
    }
    for (size_t i = 0; i < commands.size(); ++i) {
        wait(nullptr);
    }
}