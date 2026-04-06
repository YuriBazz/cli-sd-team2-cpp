#include <gtest/gtest.h>
#include <unistd.h>

#include "ast.hpp"
#include "environment.hpp"

class ExitCommandTest : public ::testing::Test {
   protected:
    Environment env;
};

TEST_F(ExitCommandTest, SetsExitFlag) {
    ArgumentList* args = new ArgumentList();

    ExitCommand cmd(args);

    int pipefd[2];
    ASSERT_EQ(pipe(pipefd), 0);

    cmd.execute(env, 0, pipefd[1]);
    close(pipefd[1]);

    EXPECT_TRUE(env.shouldExit());

    char buffer[4096] = {0};
    read(pipefd[0], buffer, sizeof(buffer) - 1);
    close(pipefd[0]);

    std::string output(buffer);
    EXPECT_NE(output.find("exit"), std::string::npos);
}

TEST_F(ExitCommandTest, WithArguments) {
    ArgumentList* args = new ArgumentList();
    args->push_back(new Argument(Argument::WORD, "extra"));
    args->push_back(new Argument(Argument::WORD, "args"));

    ExitCommand cmd(args);

    int pipefd[2];
    ASSERT_EQ(pipe(pipefd), 0);

    cmd.execute(env, 0, pipefd[1]);
    close(pipefd[1]);

    EXPECT_TRUE(env.shouldExit());

    char buffer[4096] = {0};
    read(pipefd[0], buffer, sizeof(buffer) - 1);
    close(pipefd[0]);

    std::string output(buffer);
    EXPECT_NE(output.find("exit"), std::string::npos);
}

TEST_F(ExitCommandTest, EmptyArguments) {
    ArgumentList* args = new ArgumentList();

    ExitCommand cmd(args);

    int pipefd[2];
    ASSERT_EQ(pipe(pipefd), 0);

    cmd.execute(env, 0, pipefd[1]);
    close(pipefd[1]);

    EXPECT_TRUE(env.shouldExit());
}
