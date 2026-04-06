#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <unistd.h>

#include <fstream>

#include "ast.hpp"
#include "environment.hpp"

class CatCommandTest : public ::testing::Test {
   protected:
    void SetUp() override {
        testFile = "test_cat_temp.txt";
        std::ofstream file(testFile);
        file << "Hello world\n";
        file << "This is a test file\n";
        file << "Hello again\n";
        file.close();

        utf8File = "test_cat_utf8.txt";
        std::ofstream utf8(utf8File);
        utf8 << "Привет мир\n";
        utf8 << "Это тестовый файл\n";
        utf8.close();

        emptyFile = "test_cat_empty.txt";
        std::ofstream empty(emptyFile);
        empty.close();
    }

    void TearDown() override {
        unlink(testFile.c_str());
        unlink(utf8File.c_str());
        unlink(emptyFile.c_str());
    }

    std::string testFile;
    std::string utf8File;
    std::string emptyFile;
};

TEST_F(CatCommandTest, WithExistingFile) {
    Environment env;
    ArgumentList* args = new ArgumentList();
    args->push_back(new Argument(Argument::WORD, testFile.c_str()));

    CatCommand cmd(args);

    int pipefd[2];
    ASSERT_EQ(pipe(pipefd), 0);

    cmd.execute(env, 0, pipefd[1]);
    close(pipefd[1]);

    char buffer[4096] = {0};
    read(pipefd[0], buffer, sizeof(buffer) - 1);
    close(pipefd[0]);

    std::string output(buffer);
    EXPECT_THAT(output, ::testing::HasSubstr("Hello world"));
    EXPECT_THAT(output, ::testing::HasSubstr("This is a test file"));
    EXPECT_THAT(output, ::testing::HasSubstr("Hello again"));
}

TEST_F(CatCommandTest, WithMultipleFiles) {
    Environment env;
    ArgumentList* args = new ArgumentList();
    args->push_back(new Argument(Argument::WORD, testFile.c_str()));
    args->push_back(new Argument(Argument::WORD, utf8File.c_str()));

    CatCommand cmd(args);

    int pipefd[2];
    ASSERT_EQ(pipe(pipefd), 0);

    cmd.execute(env, 0, pipefd[1]);
    close(pipefd[1]);

    char buffer[8192] = {0};
    read(pipefd[0], buffer, sizeof(buffer) - 1);
    close(pipefd[0]);

    std::string output(buffer);
    EXPECT_THAT(output, ::testing::HasSubstr("Hello world"));
    EXPECT_THAT(output, ::testing::HasSubstr("Привет мир"));
}

TEST_F(CatCommandTest, WithNonExistentFile) {
    Environment env;
    ArgumentList* args = new ArgumentList();
    args->push_back(new Argument(Argument::WORD, "nonexistent_cat_file.txt"));

    CatCommand cmd(args);

    int pipefd[2];
    ASSERT_EQ(pipe(pipefd), 0);

    cmd.execute(env, 0, pipefd[1]);
    close(pipefd[1]);

    char buffer[4096] = {0};
    read(pipefd[0], buffer, sizeof(buffer) - 1);
    close(pipefd[0]);

    std::string output(buffer);
    EXPECT_THAT(output, ::testing::HasSubstr("No such file or directory"));
}

TEST_F(CatCommandTest, WithoutArguments) {
    Environment env;
    ArgumentList* args = new ArgumentList();

    CatCommand cmd(args);

    int pipefd[2];
    ASSERT_EQ(pipe(pipefd), 0);

    std::string input = "Test input data for cat\n";
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

    EXPECT_EQ(std::string(buffer), input);
}

TEST_F(CatCommandTest, WithEmptyFile) {
    Environment env;
    ArgumentList* args = new ArgumentList();
    args->push_back(new Argument(Argument::WORD, emptyFile.c_str()));

    CatCommand cmd(args);

    int pipefd[2];
    ASSERT_EQ(pipe(pipefd), 0);

    cmd.execute(env, 0, pipefd[1]);
    close(pipefd[1]);

    char buffer[4096] = {0};
    read(pipefd[0], buffer, sizeof(buffer) - 1);
    close(pipefd[0]);

    EXPECT_EQ(std::string(buffer), "");
}
