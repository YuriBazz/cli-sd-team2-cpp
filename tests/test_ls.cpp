#include <gtest/gtest.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstdio>
#include <cstring>
#include <fstream>

#include "ast.hpp"
#include "environment.hpp"

class LsCommandTest : public ::testing::Test {
   protected:
    void SetUp() override {
        original_cwd = get_current_dir_name();

        testDir = "test_ls_temp_dir";
        mkdir(testDir.c_str(), 0755);

        std::ofstream f1((testDir + "/file1.txt").c_str());
        f1 << "test1\n";
        f1.close();
        std::ofstream f2((testDir + "/file2.txt").c_str());
        f2 << "test2\n";
        f2.close();

        subDir = testDir + "/subdir";
        mkdir(subDir.c_str(), 0755);

        emptyDir = testDir + "/empty_dir";
        mkdir(emptyDir.c_str(), 0755);
    }

    void TearDown() override {
        unlink((testDir + "/file1.txt").c_str());
        unlink((testDir + "/file2.txt").c_str());
        rmdir(subDir.c_str());
        rmdir(emptyDir.c_str());
        rmdir(testDir.c_str());

        chdir(original_cwd);
        free(original_cwd);
    }

    std::string runLs(ArgumentList* args) {
        Environment env;
        LsCommand cmd(args);

        int pipefd[2];
        if (pipe(pipefd) == -1) return "PIPE_ERROR";

        int stdout_save = dup(STDOUT_FILENO);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);

        cmd.execute(env, 0, 1);

        fflush(stdout);
        dup2(stdout_save, STDOUT_FILENO);
        close(stdout_save);

        char buffer[65536] = {0};
        ssize_t n = read(pipefd[0], buffer, sizeof(buffer) - 1);
        close(pipefd[0]);

        return (n > 0) ? std::string(buffer, n) : "";
    }

    ArgumentList* createArgList(const std::vector<std::string>& args_str) {
        auto* list = new ArgumentList();
        for (const auto& s : args_str) {
            list->push_back(new Argument(Argument::WORD, s.c_str()));
        }
        return list;
    }

    char* original_cwd = nullptr;
    std::string testDir;
    std::string subDir;
    std::string emptyDir;
};

bool containsAll(const std::string& output, const std::vector<std::string>& items) {
    for (const auto& item : items) {
        if (output.find(item) == std::string::npos) return false;
    }
    return true;
}

TEST_F(LsCommandTest, ListCurrentDirectory) {
    chdir(testDir.c_str());
    ArgumentList* args = new ArgumentList();
    std::string output = runLs(args);

    EXPECT_TRUE(containsAll(output, {"file1.txt", "file2.txt", "subdir", "empty_dir"}));
    EXPECT_EQ(output.back(), '\n');
}

TEST_F(LsCommandTest, ListSpecificDirectory) {
    ArgumentList* args = createArgList({testDir});
    std::string output = runLs(args);
    EXPECT_TRUE(containsAll(output, {"file1.txt", "file2.txt", "subdir", "empty_dir"}));
}

TEST_F(LsCommandTest, ListEmptyDirectory) {
    ArgumentList* args = createArgList({emptyDir});
    std::string output = runLs(args);
    EXPECT_EQ(output, "\n");
}

TEST_F(LsCommandTest, ListNonExistentFile) {
    ArgumentList* args = createArgList({"nonexistent_file_xyz"});
    std::string output = runLs(args);

    EXPECT_FALSE(output.empty());
    EXPECT_TRUE(output.find("Нет такого файла или каталога") != std::string::npos ||
                output.find("No such file") != std::string::npos);
}

TEST_F(LsCommandTest, ListParentDirectory) {
    chdir(subDir.c_str());
    ArgumentList* args = createArgList({".."});
    std::string output = runLs(args);
    EXPECT_TRUE(containsAll(output, {"file1.txt", "file2.txt", "subdir", "empty_dir"}));
    chdir(original_cwd);
}

TEST_F(LsCommandTest, ListCurrentDirectoryWithDot) {
    chdir(testDir.c_str());
    ArgumentList* args = createArgList({"."});
    std::string output = runLs(args);
    EXPECT_TRUE(containsAll(output, {"file1.txt", "file2.txt", "subdir", "empty_dir"}));
    chdir(original_cwd);
}

TEST_F(LsCommandTest, NoArguments) {
    chdir(testDir.c_str());
    ArgumentList* args = new ArgumentList();
    std::string output = runLs(args);
    EXPECT_TRUE(containsAll(output, {"file1.txt", "file2.txt", "subdir", "empty_dir"}));
    chdir(original_cwd);
}

TEST_F(LsCommandTest, AbsolutePath) {
    char cwd[4096];
    getcwd(cwd, sizeof(cwd));
    std::string absPath = std::string(cwd) + "/" + testDir;

    ArgumentList* args = createArgList({absPath});
    std::string output = runLs(args);
    EXPECT_TRUE(containsAll(output, {"file1.txt", "file2.txt"}));
}

TEST_F(LsCommandTest, MultipleFiles) {
    std::string file1 = testDir + "/file1.txt";
    std::string file2 = testDir + "/file2.txt";

    ArgumentList* args = createArgList({file1, file2});
    std::string output = runLs(args);

    EXPECT_TRUE(output.find("file1.txt") != std::string::npos);
    EXPECT_TRUE(output.find("file2.txt") != std::string::npos);
    EXPECT_EQ(output.back(), '\n');
}

TEST_F(LsCommandTest, SubdirectoryContents) {
    ArgumentList* args = createArgList({subDir});
    std::string output = runLs(args);
    EXPECT_EQ(output, "\n");
}