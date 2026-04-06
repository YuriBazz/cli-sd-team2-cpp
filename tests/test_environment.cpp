#include <gtest/gtest.h>

#include "environment.hpp"

class EnvironmentTest : public ::testing::Test {
   protected:
    void SetUp() override {
        setenv("TEST_ENV_VAR_1", "value1", 1);
        setenv("TEST_ENV_VAR_2", "value2", 1);
    }

    void TearDown() override {
        unsetenv("TEST_ENV_VAR_1");
        unsetenv("TEST_ENV_VAR_2");
    }
};

TEST_F(EnvironmentTest, LoadEnvVariables) {
    Environment env;
    env.loadEnv();

    EXPECT_EQ(env.get("TEST_ENV_VAR_1"), "value1");
    EXPECT_EQ(env.get("TEST_ENV_VAR_2"), "value2");
}

TEST_F(EnvironmentTest, VariableExpansion) {
    Environment env;
    env.set("MY_VAR", "expanded_value");

    Argument* varArg = new Argument(Argument::VARIABLE, "MY_VAR");
    Argument* wordArg = new Argument(Argument::WORD, "static");
    Argument* stringArg = new Argument(Argument::STRING, "quoted string");

    EXPECT_EQ(env.expand(varArg), "expanded_value");
    EXPECT_EQ(env.expand(wordArg), "static");
    EXPECT_EQ(env.expand(stringArg), "quoted string");

    delete varArg;
    delete wordArg;
    delete stringArg;
}

TEST_F(EnvironmentTest, GetNonExistentVariable) {
    Environment env;
    EXPECT_EQ(env.get("NON_EXISTENT_VAR"), "");
}

TEST_F(EnvironmentTest, SetAndGetVariable) {
    Environment env;
    env.set("NEW_VAR", "new_value");
    EXPECT_EQ(env.get("NEW_VAR"), "new_value");
}

TEST_F(EnvironmentTest, OverwriteVariable) {
    Environment env;
    env.set("VAR", "first");
    EXPECT_EQ(env.get("VAR"), "first");

    env.set("VAR", "second");
    EXPECT_EQ(env.get("VAR"), "second");
}

TEST_F(EnvironmentTest, EmptyVariableValue) {
    Environment env;
    env.set("EMPTY_VAR", "");
    EXPECT_EQ(env.get("EMPTY_VAR"), "");
}

TEST_F(EnvironmentTest, ShouldExitFlag) {
    Environment env;
    EXPECT_FALSE(env.shouldExit());

    env.setExit(true);
    EXPECT_TRUE(env.shouldExit());

    env.setExit(false);
    EXPECT_FALSE(env.shouldExit());
}

TEST_F(EnvironmentTest, GetVariables) {
    Environment env;
    env.getVariables().clear();
    env.set("VAR1", "value1");
    env.set("VAR2", "value2");

    const auto& vars = env.getVariables();
    EXPECT_EQ(vars.size(), 2);
    EXPECT_EQ(vars.at("VAR1"), "value1");
    EXPECT_EQ(vars.at("VAR2"), "value2");
}
