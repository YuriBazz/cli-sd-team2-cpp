#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <unistd.h>

#include <fstream>

#include "ast.hpp"
#include "environment.hpp"

class PipelineTest : public ::testing::Test {
   protected:
    void SetUp() override {
        // Создаем временный файл для тестов
        testFile = "test_pipeline_temp.txt";
        std::ofstream file(testFile);
        file << "apple\n";
        file << "banana\n";
        file << "cherry\n";
        file << "date\n";
        file.close();
    }

    void TearDown() override { unlink(testFile.c_str()); }

    std::string testFile;
    Environment env;
};

TEST_F(PipelineTest, EchoToGrep) {
    ArgumentList* echoArgs = new ArgumentList();
    echoArgs->push_back(new Argument(Argument::WORD, "Hello World\nTest Line"));
    EchoCommand* echo = new EchoCommand(echoArgs);

    ArgumentList* grepArgs = new ArgumentList();
    grepArgs->push_back(new Argument(Argument::WORD, "Hello"));
    GrepCommand* grep = new GrepCommand(grepArgs);

    Pipeline pipeline({echo, grep});

    int pipefd[2];
    ASSERT_EQ(pipe(pipefd), 0);

    int stdout_backup = dup(STDOUT_FILENO);
    dup2(pipefd[1], STDOUT_FILENO);

    pipeline.execute(env);

    fflush(stdout);
    dup2(stdout_backup, STDOUT_FILENO);
    close(pipefd[1]);

    char buffer[4096] = {0};
    read(pipefd[0], buffer, sizeof(buffer) - 1);
    close(pipefd[0]);
    close(stdout_backup);

    std::string output(buffer);
    EXPECT_THAT(output, ::testing::HasSubstr("Hello World"));
}

TEST_F(PipelineTest, CatToGrep) {
    ArgumentList* catArgs = new ArgumentList();
    catArgs->push_back(new Argument(Argument::WORD, testFile.c_str()));
    CatCommand* cat = new CatCommand(catArgs);

    ArgumentList* grepArgs = new ArgumentList();
    grepArgs->push_back(new Argument(Argument::WORD, "an"));
    GrepCommand* grep = new GrepCommand(grepArgs);

    Pipeline pipeline({cat, grep});

    int pipefd[2];
    ASSERT_EQ(pipe(pipefd), 0);

    int stdout_backup = dup(STDOUT_FILENO);
    dup2(pipefd[1], STDOUT_FILENO);

    pipeline.execute(env);

    fflush(stdout);
    dup2(stdout_backup, STDOUT_FILENO);
    close(pipefd[1]);

    char buffer[4096] = {0};
    read(pipefd[0], buffer, sizeof(buffer) - 1);
    close(pipefd[0]);
    close(stdout_backup);

    std::string output(buffer);
    EXPECT_THAT(output, ::testing::HasSubstr("banana"));
}

TEST_F(PipelineTest, ThreeCommandPipeline) {
    ArgumentList* catArgs = new ArgumentList();
    catArgs->push_back(new Argument(Argument::WORD, testFile.c_str()));
    CatCommand* cat = new CatCommand(catArgs);

    ArgumentList* grepArgs = new ArgumentList();
    grepArgs->push_back(new Argument(Argument::WORD, "a"));
    GrepCommand* grep = new GrepCommand(grepArgs);

    ArgumentList* wcArgs = new ArgumentList();
    WcCommand* wc = new WcCommand(wcArgs);

    Pipeline pipeline({cat, grep, wc});

    int pipefd[2];
    ASSERT_EQ(pipe(pipefd), 0);

    int stdout_backup = dup(STDOUT_FILENO);
    dup2(pipefd[1], STDOUT_FILENO);

    pipeline.execute(env);

    fflush(stdout);
    dup2(stdout_backup, STDOUT_FILENO);
    close(pipefd[1]);

    char buffer[4096] = {0};
    read(pipefd[0], buffer, sizeof(buffer) - 1);
    close(pipefd[0]);
    close(stdout_backup);

    std::string output(buffer);
    // apple, banana, date содержат 'a'
    EXPECT_THAT(output, ::testing::HasSubstr("3"));
}

TEST_F(PipelineTest, EmptyPipeline) {
    Pipeline pipeline({});

    // Не должно упасть
    pipeline.execute(env);
    SUCCEED();
}

TEST_F(PipelineTest, EchoToWc) {
    ArgumentList* echoArgs = new ArgumentList();
    echoArgs->push_back(new Argument(Argument::WORD, "one two three four five"));
    EchoCommand* echo = new EchoCommand(echoArgs);

    ArgumentList* wcArgs = new ArgumentList();
    WcCommand* wc = new WcCommand(wcArgs);

    Pipeline pipeline({echo, wc});

    int pipefd[2];
    ASSERT_EQ(pipe(pipefd), 0);

    int stdout_backup = dup(STDOUT_FILENO);
    dup2(pipefd[1], STDOUT_FILENO);

    pipeline.execute(env);

    fflush(stdout);
    dup2(stdout_backup, STDOUT_FILENO);
    close(pipefd[1]);

    char buffer[4096] = {0};
    read(pipefd[0], buffer, sizeof(buffer) - 1);
    close(pipefd[0]);
    close(stdout_backup);

    std::string output(buffer);
    EXPECT_THAT(output, ::testing::HasSubstr("1"));  // 1 line
    EXPECT_THAT(output, ::testing::HasSubstr("5"));  // 5 words
}
