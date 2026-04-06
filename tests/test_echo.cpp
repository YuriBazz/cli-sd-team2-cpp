#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <unistd.h>

#include "ast.hpp"
#include "environment.hpp"

class EchoCommandTest : public ::testing::Test {
   protected:
    void SetUp() override {
        env.set("TEST_VAR", "expanded_value");
        env.set("ANOTHER_VAR", "another_value");
    }

    Environment env;
};

TEST_F(EchoCommandTest, WithMultipleArguments) {
    ArgumentList* args = new ArgumentList();
    args->push_back(new Argument(Argument::WORD, "Hello"));
    args->push_back(new Argument(Argument::WORD, "World"));
    args->push_back(new Argument(Argument::WORD, "Test"));

    EchoCommand cmd(args);

    int pipefd[2];
    ASSERT_EQ(pipe(pipefd), 0);

    cmd.execute(env, 0, pipefd[1]);
    close(pipefd[1]);

    char buffer[4096] = {0};
    read(pipefd[0], buffer, sizeof(buffer) - 1);
    close(pipefd[0]);

    EXPECT_EQ(std::string(buffer), "Hello World Test\n");
}

TEST_F(EchoCommandTest, WithSingleArgument) {
    ArgumentList* args = new ArgumentList();
    args->push_back(new Argument(Argument::WORD, "Hello"));

    EchoCommand cmd(args);

    int pipefd[2];
    ASSERT_EQ(pipe(pipefd), 0);

    cmd.execute(env, 0, pipefd[1]);
    close(pipefd[1]);

    char buffer[4096] = {0};
    read(pipefd[0], buffer, sizeof(buffer) - 1);
    close(pipefd[0]);

    EXPECT_EQ(std::string(buffer), "Hello\n");
}

TEST_F(EchoCommandTest, WithEmptyArguments) {
    ArgumentList* args = new ArgumentList();

    EchoCommand cmd(args);

    int pipefd[2];
    ASSERT_EQ(pipe(pipefd), 0);

    cmd.execute(env, 0, pipefd[1]);
    close(pipefd[1]);

    char buffer[4096] = {0};
    read(pipefd[0], buffer, sizeof(buffer) - 1);
    close(pipefd[0]);

    EXPECT_EQ(std::string(buffer), "\n");
}

TEST_F(EchoCommandTest, WithVariableExpansion) {
    ArgumentList* args = new ArgumentList();
    args->push_back(new Argument(Argument::VARIABLE, "TEST_VAR"));

    EchoCommand cmd(args);

    int pipefd[2];
    ASSERT_EQ(pipe(pipefd), 0);

    cmd.execute(env, 0, pipefd[1]);
    close(pipefd[1]);

    char buffer[4096] = {0};
    read(pipefd[0], buffer, sizeof(buffer) - 1);
    close(pipefd[0]);

    EXPECT_EQ(std::string(buffer), "expanded_value\n");
}

TEST_F(EchoCommandTest, WithMixedArgumentsAndVariables) {
    ArgumentList* args = new ArgumentList();
    args->push_back(new Argument(Argument::WORD, "Value:"));
    args->push_back(new Argument(Argument::VARIABLE, "TEST_VAR"));
    args->push_back(new Argument(Argument::WORD, "and"));
    args->push_back(new Argument(Argument::VARIABLE, "ANOTHER_VAR"));

    EchoCommand cmd(args);

    int pipefd[2];
    ASSERT_EQ(pipe(pipefd), 0);

    cmd.execute(env, 0, pipefd[1]);
    close(pipefd[1]);

    char buffer[4096] = {0};
    read(pipefd[0], buffer, sizeof(buffer) - 1);
    close(pipefd[0]);

    EXPECT_EQ(std::string(buffer), "Value: expanded_value and another_value\n");
}

TEST_F(EchoCommandTest, WithSpecialCharacters) {
    ArgumentList* args = new ArgumentList();
    args->push_back(new Argument(Argument::STRING, "Hello\tWorld\nTest"));

    EchoCommand cmd(args);

    int pipefd[2];
    ASSERT_EQ(pipe(pipefd), 0);

    cmd.execute(env, 0, pipefd[1]);
    close(pipefd[1]);

    char buffer[4096] = {0};
    read(pipefd[0], buffer, sizeof(buffer) - 1);
    close(pipefd[0]);

    EXPECT_EQ(std::string(buffer), "Hello\tWorld\nTest\n");
}
