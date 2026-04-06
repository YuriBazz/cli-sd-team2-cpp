#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/wait.h>
#include <unistd.h>

#include <fstream>

#include "ast.hpp"
#include "environment.hpp"

class ExternalCommandTest : public ::testing::Test {
   protected:
    void SetUp() override { env.set("PATH", "/bin:/usr/bin"); }

    Environment env;
};

TEST_F(ExternalCommandTest, LsCommand) {
    ArgumentList* args = new ArgumentList();
    args->push_back(new Argument(Argument::WORD, "-la"));

    ExternalCommand cmd("ls", args);

    int pipefd[2];
    ASSERT_EQ(pipe(pipefd), 0);

    pid_t pid = fork();
    if (pid == 0) {
        cmd.execute(env, 0, pipefd[1]);
        exit(0);
    } else {
        waitpid(pid, nullptr, 0);
        close(pipefd[1]);

        char buffer[4096] = {0};
        read(pipefd[0], buffer, sizeof(buffer) - 1);
        close(pipefd[0]);

        std::string output(buffer);
        EXPECT_THAT(output, ::testing::HasSubstr("bash_tests"));
    }
}

TEST_F(ExternalCommandTest, EchoCommand) {
    ArgumentList* args = new ArgumentList();
    args->push_back(new Argument(Argument::WORD, "Hello from external"));

    ExternalCommand cmd("echo", args);

    int pipefd[2];
    ASSERT_EQ(pipe(pipefd), 0);

    pid_t pid = fork();
    if (pid == 0) {
        cmd.execute(env, 0, pipefd[1]);
        exit(0);
    } else {
        waitpid(pid, nullptr, 0);
        close(pipefd[1]);

        char buffer[4096] = {0};
        read(pipefd[0], buffer, sizeof(buffer) - 1);
        close(pipefd[0]);

        std::string output(buffer);
        EXPECT_THAT(output, ::testing::HasSubstr("Hello from external"));
    }
}

TEST_F(ExternalCommandTest, NonExistentCommand) {
    ArgumentList* args = new ArgumentList();

    ExternalCommand cmd("nonexistentcommand123456", args);

    int pipefd[2];
    ASSERT_EQ(pipe(pipefd), 0);

    int stderr_backup = dup(STDERR_FILENO);
    dup2(pipefd[1], STDERR_FILENO);

    pid_t pid = fork();
    if (pid == 0) {
        cmd.execute(env, 0, 1);
        exit(0);
    } else {
        int status;
        waitpid(pid, &status, 0);
        fflush(stderr);
        dup2(stderr_backup, STDERR_FILENO);
        close(pipefd[1]);

        char buffer[4096] = {0};
        read(pipefd[0], buffer, sizeof(buffer) - 1);
        close(pipefd[0]);
        close(stderr_backup);

        std::string output(buffer);
        EXPECT_THAT(output, ::testing::HasSubstr("failed"));
        EXPECT_TRUE(WIFEXITED(status));
        EXPECT_EQ(WEXITSTATUS(status), 127);
    }
}

TEST_F(ExternalCommandTest, WithArgumentsAndVariables) {
    env.set("TEST_VAR", "test_value");

    ArgumentList* args = new ArgumentList();
    args->push_back(new Argument(Argument::WORD, "Variable value:"));
    args->push_back(new Argument(Argument::VARIABLE, "TEST_VAR"));

    ExternalCommand cmd("echo", args);

    int pipefd[2];
    ASSERT_EQ(pipe(pipefd), 0);

    pid_t pid = fork();
    if (pid == 0) {
        cmd.execute(env, 0, pipefd[1]);
        exit(0);
    } else {
        waitpid(pid, nullptr, 0);
        close(pipefd[1]);

        char buffer[4096] = {0};
        read(pipefd[0], buffer, sizeof(buffer) - 1);
        close(pipefd[0]);

        std::string output(buffer);
        EXPECT_THAT(output, ::testing::HasSubstr("Variable value: test_value"));
    }
}

TEST_F(ExternalCommandTest, PwdCommand) {
    ArgumentList* args = new ArgumentList();

    ExternalCommand cmd("pwd", args);

    int pipefd[2];
    ASSERT_EQ(pipe(pipefd), 0);

    pid_t pid = fork();
    if (pid == 0) {
        cmd.execute(env, 0, pipefd[1]);
        exit(0);
    } else {
        waitpid(pid, nullptr, 0);
        close(pipefd[1]);

        char buffer[4096] = {0};
        read(pipefd[0], buffer, sizeof(buffer) - 1);
        close(pipefd[0]);

        std::string output(buffer);
        char cwd[1024];
        getcwd(cwd, sizeof(cwd));
        EXPECT_THAT(output, ::testing::HasSubstr(cwd));
    }
}

TEST_F(ExternalCommandTest, CatCommand) {
    std::string testFile = "temp_test_cat.txt";
    std::ofstream file(testFile);
    file << "cmake_minimum_required(VERSION 3.16)\n";
    file << "project(Test)\n";
    file.close();

    ArgumentList* args = new ArgumentList();
    args->push_back(new Argument(Argument::WORD, testFile.c_str()));

    ExternalCommand cmd("cat", args);

    int pipefd[2];
    ASSERT_EQ(pipe(pipefd), 0);

    pid_t pid = fork();
    if (pid == 0) {
        cmd.execute(env, 0, pipefd[1]);
        exit(0);
    } else {
        waitpid(pid, nullptr, 0);
        close(pipefd[1]);

        char buffer[4096] = {0};
        read(pipefd[0], buffer, sizeof(buffer) - 1);
        close(pipefd[0]);

        std::string output(buffer);
        EXPECT_THAT(output, ::testing::HasSubstr("cmake_minimum_required"));
    }

    unlink(testFile.c_str());
}
