// #include <NetworkService.h>
#include <unity.h>

NetworkService networkService;

void test_scanNetworks(void) {
    TEST_ASSERT_EQUAL(3, networkService.scan().length);
}
int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_scanNetworks);
    UNITY_END();

    return 0;
}