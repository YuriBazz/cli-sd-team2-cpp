#include <gtest/gtest.h>

#include "ast.hpp"
#include "environment.hpp"

class AssignmentTest : public ::testing::Test {
   protected:
    void SetUp() override { env.set("EXISTING_VAR", "existing_value"); }

    Environment env;
};

TEST_F(AssignmentTest, BasicAssignment) {
    ArgumentList* args = new ArgumentList();
    args->push_back(new Argument(Argument::WORD, "test_value"));

    Assignment assign("TEST_VAR", args);
    assign.execute(env);

    EXPECT_EQ(env.get("TEST_VAR"), "test_value");
}

TEST_F(AssignmentTest, EmptyValue) {
    ArgumentList* args = new ArgumentList();

    Assignment assign("EMPTY_VAR", args);
    assign.execute(env);

    EXPECT_EQ(env.get("EMPTY_VAR"), "");
}

TEST_F(AssignmentTest, MultipleArguments) {
    ArgumentList* args = new ArgumentList();
    args->push_back(new Argument(Argument::WORD, "value1"));
    args->push_back(new Argument(Argument::WORD, "value2"));
    args->push_back(new Argument(Argument::WORD, "value3"));

    Assignment assign("MULTI_VAR", args);
    assign.execute(env);

    EXPECT_EQ(env.get("MULTI_VAR"), "value1 value2 value3");
}

TEST_F(AssignmentTest, WithVariableExpansion) {
    ArgumentList* args = new ArgumentList();
    args->push_back(new Argument(Argument::VARIABLE, "EXISTING_VAR"));

    Assignment assign("NEW_VAR", args);
    assign.execute(env);

    EXPECT_EQ(env.get("NEW_VAR"), "existing_value");
}

TEST_F(AssignmentTest, OverwriteExistingVariable) {
    env.set("EXISTING_VAR", "old_value");

    ArgumentList* args = new ArgumentList();
    args->push_back(new Argument(Argument::WORD, "new_value"));

    Assignment assign("EXISTING_VAR", args);
    assign.execute(env);

    EXPECT_EQ(env.get("EXISTING_VAR"), "new_value");
}

TEST_F(AssignmentTest, WithSpacesInValue) {
    ArgumentList* args = new ArgumentList();
    args->push_back(new Argument(Argument::WORD, "value"));
    args->push_back(new Argument(Argument::WORD, "with"));
    args->push_back(new Argument(Argument::WORD, "spaces"));

    Assignment assign("SPACE_VAR", args);
    assign.execute(env);

    EXPECT_EQ(env.get("SPACE_VAR"), "value with spaces");
}

TEST_F(AssignmentTest, WithStringArgument) {
    ArgumentList* args = new ArgumentList();
    args->push_back(new Argument(Argument::STRING, "quoted value with spaces"));

    Assignment assign("STRING_VAR", args);
    assign.execute(env);

    EXPECT_EQ(env.get("STRING_VAR"), "quoted value with spaces");
}

TEST_F(AssignmentTest, VariableNameWithUnderscore) {
    ArgumentList* args = new ArgumentList();
    args->push_back(new Argument(Argument::WORD, "test"));

    Assignment assign("VAR_WITH_UNDERSCORE", args);
    assign.execute(env);

    EXPECT_EQ(env.get("VAR_WITH_UNDERSCORE"), "test");
}
