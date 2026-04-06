#include <gtest/gtest.h>
#include <unistd.h>

#include <fstream>

#include "ast.hpp"
#include "environment.hpp"

class GrepCommandTest : public ::testing::Test {
   protected:
    void SetUp() override {
        testFile = "test_grep_temp.txt";
        std::ofstream file(testFile);
        file << "Hello world\n";
        file << "This is a test file\n";
        file << "Hello again\n";
        file << "Another line with Hello\n";
        file << "hello lowercase\n";
        file << "HELLO UPPERCASE\n";
        file << "No match here\n";
        file << "Last line\n";
        file.close();

        multiLineFile = "test_grep_multiline.txt";
        std::ofstream multi(multiLineFile);
        multi << "Line 1\n";
        multi << "Line 2 with pattern\n";
        multi << "Line 3\n";
        multi << "Line 4 with pattern again\n";
        multi << "Line 5\n";
        multi.close();
    }

    void TearDown() override {
        unlink(testFile.c_str());
        unlink(multiLineFile.c_str());
    }

    std::string testFile;
    std::string multiLineFile;
};

TEST_F(GrepCommandTest, BasicSearch) {
    Environment env;
    ArgumentList* args = new ArgumentList();
    args->push_back(new Argument(Argument::WORD, "Hello"));
    args->push_back(new Argument(Argument::WORD, testFile.c_str()));

    GrepCommand cmd(args);

    int pipefd[2];
    ASSERT_EQ(pipe(pipefd), 0);

    cmd.execute(env, 0, pipefd[1]);
    close(pipefd[1]);

    char buffer[4096] = {0};
    read(pipefd[0], buffer, sizeof(buffer) - 1);
    close(pipefd[0]);

    std::string output(buffer);
    EXPECT_NE(output.find("Hello world"), std::string::npos);
    EXPECT_NE(output.find("Hello again"), std::string::npos);
    EXPECT_NE(output.find("Another line with Hello"), std::string::npos);
    EXPECT_EQ(output.find("This is a test file"), std::string::npos);
}

TEST_F(GrepCommandTest, CaseInsensitive) {
    Environment env;
    ArgumentList* args = new ArgumentList();
    args->push_back(new Argument(Argument::WORD, "-i"));
    args->push_back(new Argument(Argument::WORD, "hello"));
    args->push_back(new Argument(Argument::WORD, testFile.c_str()));

    GrepCommand cmd(args);

    int pipefd[2];
    ASSERT_EQ(pipe(pipefd), 0);

    cmd.execute(env, 0, pipefd[1]);
    close(pipefd[1]);

    char buffer[4096] = {0};
    read(pipefd[0], buffer, sizeof(buffer) - 1);
    close(pipefd[0]);

    std::string output(buffer);
    EXPECT_NE(output.find("Hello world"), std::string::npos);
    EXPECT_NE(output.find("hello lowercase"), std::string::npos);
    EXPECT_NE(output.find("HELLO UPPERCASE"), std::string::npos);
}

TEST_F(GrepCommandTest, CountMatches) {
    Environment env;
    ArgumentList* args = new ArgumentList();
    args->push_back(new Argument(Argument::WORD, "-c"));
    args->push_back(new Argument(Argument::WORD, "Hello"));
    args->push_back(new Argument(Argument::WORD, testFile.c_str()));

    GrepCommand cmd(args);

    int pipefd[2];
    ASSERT_EQ(pipe(pipefd), 0);

    cmd.execute(env, 0, pipefd[1]);
    close(pipefd[1]);

    char buffer[4096] = {0};
    read(pipefd[0], buffer, sizeof(buffer) - 1);
    close(pipefd[0]);

    std::string output(buffer);
    EXPECT_NE(output.find("3"), std::string::npos);
}

TEST_F(GrepCommandTest, ListFilesWithMatch) {
    Environment env;
    ArgumentList* args = new ArgumentList();
    args->push_back(new Argument(Argument::WORD, "-l"));
    args->push_back(new Argument(Argument::WORD, "Hello"));
    args->push_back(new Argument(Argument::WORD, testFile.c_str()));

    GrepCommand cmd(args);

    int pipefd[2];
    ASSERT_EQ(pipe(pipefd), 0);

    cmd.execute(env, 0, pipefd[1]);
    close(pipefd[1]);

    char buffer[4096] = {0};
    read(pipefd[0], buffer, sizeof(buffer) - 1);
    close(pipefd[0]);

    std::string output(buffer);
    EXPECT_NE(output.find(testFile), std::string::npos);
    EXPECT_EQ(output.find("Hello world"), std::string::npos);
}

TEST_F(GrepCommandTest, WordBoundary) {
    Environment env;
    ArgumentList* args = new ArgumentList();
    args->push_back(new Argument(Argument::WORD, "-w"));
    args->push_back(new Argument(Argument::WORD, "Hello"));
    args->push_back(new Argument(Argument::WORD, testFile.c_str()));

    GrepCommand cmd(args);

    int pipefd[2];
    ASSERT_EQ(pipe(pipefd), 0);

    cmd.execute(env, 0, pipefd[1]);
    close(pipefd[1]);

    char buffer[4096] = {0};
    read(pipefd[0], buffer, sizeof(buffer) - 1);
    close(pipefd[0]);

    std::string output(buffer);
    EXPECT_NE(output.find("Hello world"), std::string::npos);
    EXPECT_NE(output.find("Hello again"), std::string::npos);
    EXPECT_NE(output.find("Another line with Hello"), std::string::npos);
}

TEST_F(GrepCommandTest, ContextLines) {
    Environment env;
    ArgumentList* args = new ArgumentList();
    args->push_back(new Argument(Argument::WORD, "-A"));
    args->push_back(new Argument(Argument::WORD, "1"));
    args->push_back(new Argument(Argument::WORD, "pattern"));
    args->push_back(new Argument(Argument::WORD, multiLineFile.c_str()));

    GrepCommand cmd(args);

    int pipefd[2];
    ASSERT_EQ(pipe(pipefd), 0);

    cmd.execute(env, 0, pipefd[1]);
    close(pipefd[1]);

    char buffer[4096] = {0};
    read(pipefd[0], buffer, sizeof(buffer) - 1);
    close(pipefd[0]);

    std::string output(buffer);
    EXPECT_NE(output.find("Line 2 with pattern"), std::string::npos);
    EXPECT_NE(output.find("Line 3"), std::string::npos);
    EXPECT_NE(output.find("Line 4 with pattern again"), std::string::npos);
    EXPECT_NE(output.find("Line 5"), std::string::npos);
}

TEST_F(GrepCommandTest, WithoutPattern) {
    Environment env;
    ArgumentList* args = new ArgumentList();

    GrepCommand cmd(args);

    int pipefd[2];
    ASSERT_EQ(pipe(pipefd), 0);

    cmd.execute(env, 0, pipefd[1]);
    close(pipefd[1]);

    char buffer[4096] = {0};
    read(pipefd[0], buffer, sizeof(buffer) - 1);
    close(pipefd[0]);

    std::string output(buffer);
    EXPECT_NE(output.find("missing pattern"), std::string::npos);
}

TEST_F(GrepCommandTest, WithStdin) {
    Environment env;
    ArgumentList* args = new ArgumentList();
    args->push_back(new Argument(Argument::WORD, "test"));

    GrepCommand cmd(args);

    int pipefd[2];
    ASSERT_EQ(pipe(pipefd), 0);

    int inputPipe[2];
    ASSERT_EQ(pipe(inputPipe), 0);
    std::string input = "This is a test line\nAnother line without\nThird test line\n";
    write(inputPipe[1], input.c_str(), input.size());
    close(inputPipe[1]);

    cmd.execute(env, inputPipe[0], pipefd[1]);
    close(pipefd[1]);
    close(inputPipe[0]);

    char buffer[4096] = {0};
    read(pipefd[0], buffer, sizeof(buffer) - 1);
    close(pipefd[0]);

    std::string output(buffer);
    EXPECT_NE(output.find("This is a test line"), std::string::npos);
    EXPECT_NE(output.find("Third test line"), std::string::npos);
    EXPECT_EQ(output.find("Another line without"), std::string::npos);
}

TEST_F(GrepCommandTest, MultipleFiles) {
    Environment env;
    ArgumentList* args = new ArgumentList();
    args->push_back(new Argument(Argument::WORD, "Hello"));
    args->push_back(new Argument(Argument::WORD, testFile.c_str()));
    args->push_back(new Argument(Argument::WORD, multiLineFile.c_str()));

    GrepCommand cmd(args);

    int pipefd[2];
    ASSERT_EQ(pipe(pipefd), 0);

    cmd.execute(env, 0, pipefd[1]);
    close(pipefd[1]);

    char buffer[8192] = {0};
    read(pipefd[0], buffer, sizeof(buffer) - 1);
    close(pipefd[0]);

    std::string output(buffer);
    EXPECT_NE(output.find(testFile + ":Hello world"), std::string::npos);
    EXPECT_NE(output.find(testFile + ":Hello again"), std::string::npos);
}

TEST_F(GrepCommandTest, ComplexPattern) {
    Environment env;
    ArgumentList* args = new ArgumentList();
    args->push_back(new Argument(Argument::WORD, "H.*o"));
    args->push_back(new Argument(Argument::WORD, testFile.c_str()));

    GrepCommand cmd(args);

    int pipefd[2];
    ASSERT_EQ(pipe(pipefd), 0);

    cmd.execute(env, 0, pipefd[1]);
    close(pipefd[1]);

    char buffer[4096] = {0};
    read(pipefd[0], buffer, sizeof(buffer) - 1);
    close(pipefd[0]);

    std::string output(buffer);
    EXPECT_NE(output.find("Hello world"), std::string::npos);
    EXPECT_NE(output.find("Hello again"), std::string::npos);
}
