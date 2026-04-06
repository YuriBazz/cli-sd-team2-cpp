#include <gtest/gtest.h>
#include <unistd.h>

#include "ast.hpp"
#include "environment.hpp"

class PwdCommandTest : public ::testing::Test {
   protected:
    void SetUp() override { original_cwd = get_current_dir_name(); }

    void TearDown() override { free(original_cwd); }

    char* original_cwd;
};

TEST_F(PwdCommandTest, ExecutesSuccessfully) {
    Environment env;
    ArgumentList* args = new ArgumentList();

    PwdCommand cmd(args);

    int pipefd[2];
    ASSERT_EQ(pipe(pipefd), 0);

    cmd.execute(env, 0, pipefd[1]);
    close(pipefd[1]);

    char buffer[4096] = {0};
    read(pipefd[0], buffer, sizeof(buffer) - 1);
    close(pipefd[0]);

    std::string output(buffer);
    EXPECT_FALSE(output.empty());
    EXPECT_EQ(output.back(), '\n');

    char expected_cwd[1024];
    getcwd(expected_cwd, sizeof(expected_cwd));
    std::string expected(expected_cwd);
    expected += "\n";

    EXPECT_EQ(output, expected);
}

TEST_F(PwdCommandTest, ReturnsCurrentDirectory) {
    Environment env;
    ArgumentList* args = new ArgumentList();

    PwdCommand cmd(args);

    int pipefd[2];
    ASSERT_EQ(pipe(pipefd), 0);

    cmd.execute(env, 0, pipefd[1]);
    close(pipefd[1]);

    char buffer[4096] = {0};
    read(pipefd[0], buffer, sizeof(buffer) - 1);
    close(pipefd[0]);

    std::string output(buffer);
    output.pop_back();  // Remove newline

    char cwd[1024];
    getcwd(cwd, sizeof(cwd));

    EXPECT_EQ(output, std::string(cwd));
}

TEST_F(PwdCommandTest, OutputContainsNewline) {
    Environment env;
    ArgumentList* args = new ArgumentList();

    PwdCommand cmd(args);

    int pipefd[2];
    ASSERT_EQ(pipe(pipefd), 0);

    cmd.execute(env, 0, pipefd[1]);
    close(pipefd[1]);

    char buffer[4096] = {0};
    ssize_t n = read(pipefd[0], buffer, sizeof(buffer) - 1);
    close(pipefd[0]);

    ASSERT_GT(n, 0);
    EXPECT_EQ(buffer[n - 1], '\n');
}
