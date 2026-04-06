// tests/test_wc.cpp
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <unistd.h>

#include <fstream>

#include "ast.hpp"
#include "environment.hpp"

class WcCommandTest : public ::testing::Test {
   protected:
    void SetUp() override {
        testFile = "test_wc_temp.txt";
        std::ofstream file(testFile);
        file << "Hello world\n";
        file << "This is a test file\n";
        file << "Hello again\n";
        file << "Another line\n";
        file << "Last line\n";
        file.close();

        emptyFile = "test_wc_empty.txt";
        std::ofstream empty(emptyFile);
        empty.close();

        singleLineFile = "test_wc_single.txt";
        std::ofstream single(singleLineFile);
        single << "Just one line with multiple words here\n";
        single.close();
    }

    void TearDown() override {
        unlink(testFile.c_str());
        unlink(emptyFile.c_str());
        unlink(singleLineFile.c_str());
    }

    std::string testFile;
    std::string emptyFile;
    std::string singleLineFile;
};

TEST_F(WcCommandTest, WithFile) {
    Environment env;
    ArgumentList* args = new ArgumentList();
    args->push_back(new Argument(Argument::WORD, testFile.c_str()));

    WcCommand cmd(args);

    int pipefd[2];
    ASSERT_EQ(pipe(pipefd), 0);

    cmd.execute(env, 0, pipefd[1]);
    close(pipefd[1]);

    char buffer[4096] = {0};
    read(pipefd[0], buffer, sizeof(buffer) - 1);
    close(pipefd[0]);

    std::string output(buffer);
    EXPECT_THAT(output, ::testing::HasSubstr("5"));
    EXPECT_THAT(output, ::testing::HasSubstr(testFile));
}

TEST_F(WcCommandTest, WithEmptyFile) {
    Environment env;
    ArgumentList* args = new ArgumentList();
    args->push_back(new Argument(Argument::WORD, emptyFile.c_str()));

    WcCommand cmd(args);

    int pipefd[2];
    ASSERT_EQ(pipe(pipefd), 0);

    cmd.execute(env, 0, pipefd[1]);
    close(pipefd[1]);

    char buffer[4096] = {0};
    read(pipefd[0], buffer, sizeof(buffer) - 1);
    close(pipefd[0]);

    std::string output(buffer);
    EXPECT_THAT(output, ::testing::HasSubstr("0 0 0"));
    EXPECT_THAT(output, ::testing::HasSubstr(emptyFile));
}

TEST_F(WcCommandTest, WithSingleLineFile) {
    Environment env;
    ArgumentList* args = new ArgumentList();
    args->push_back(new Argument(Argument::WORD, singleLineFile.c_str()));

    WcCommand cmd(args);

    int pipefd[2];
    ASSERT_EQ(pipe(pipefd), 0);

    cmd.execute(env, 0, pipefd[1]);
    close(pipefd[1]);

    char buffer[4096] = {0};
    read(pipefd[0], buffer, sizeof(buffer) - 1);
    close(pipefd[0]);

    std::string output(buffer);
    EXPECT_THAT(output, ::testing::HasSubstr("1"));
    EXPECT_THAT(output, ::testing::HasSubstr("7"));
}

TEST_F(WcCommandTest, WithoutArguments) {
    Environment env;
    ArgumentList* args = new ArgumentList();

    WcCommand cmd(args);

    int pipefd[2];
    ASSERT_EQ(pipe(pipefd), 0);

    std::string input = "Line one\nLine two\nLine three\n";
    int inputPipe[2];
    ASSERT_EQ(pipe(inputPipe), 0);
    write(inputPipe[1], input.c_str(), input.size());
    close(inputPipe[1]);

    cmd.execute(env, inputPipe[0], pipefd[1]);
    close(pipefd[1]);
    close(inputPipe[0]);

    char buffer[4096] = {0};
    read(pipefd[0], buffer, sizeof(buffer) - 1);
    close(pipefd[0]);

    std::string output(buffer);
    EXPECT_THAT(output, ::testing::HasSubstr("3"));
    EXPECT_THAT(output, ::testing::HasSubstr("6"));
}

TEST_F(WcCommandTest, WithNonExistentFile) {
    Environment env;
    ArgumentList* args = new ArgumentList();
    args->push_back(new Argument(Argument::WORD, "nonexistent_wc_file.txt"));

    WcCommand cmd(args);

    int pipefd[2];
    ASSERT_EQ(pipe(pipefd), 0);

    cmd.execute(env, 0, pipefd[1]);
    close(pipefd[1]);

    char buffer[4096] = {0};
    read(pipefd[0], buffer, sizeof(buffer) - 1);
    close(pipefd[0]);

    std::string output(buffer);
    EXPECT_THAT(output, ::testing::HasSubstr("No such file"));
}

TEST_F(WcCommandTest, WithMultipleFiles) {
    Environment env;
    ArgumentList* args = new ArgumentList();
    args->push_back(new Argument(Argument::WORD, testFile.c_str()));
    args->push_back(new Argument(Argument::WORD, singleLineFile.c_str()));

    WcCommand cmd(args);

    int pipefd[2];
    ASSERT_EQ(pipe(pipefd), 0);

    cmd.execute(env, 0, pipefd[1]);
    close(pipefd[1]);

    char buffer[4096] = {0};
    read(pipefd[0], buffer, sizeof(buffer) - 1);
    close(pipefd[0]);

    std::string output(buffer);
    EXPECT_THAT(output, ::testing::HasSubstr(testFile));
    EXPECT_THAT(output, ::testing::HasSubstr(singleLineFile));
}
