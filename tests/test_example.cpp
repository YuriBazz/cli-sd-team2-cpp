#include <gtest/gtest.h>

TEST(Module1Test, BasicFunctionality) {
    EXPECT_EQ(1 + 1, 2);
}

/*TEST(Module1Test, AnotherTest) {

}*/

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
