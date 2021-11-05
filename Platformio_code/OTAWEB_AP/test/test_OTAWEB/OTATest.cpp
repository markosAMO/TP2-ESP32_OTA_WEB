#include <Arduino.h>
#include <unity.h>
String STR_TO_TEST;

void setUp(void) {
    // set stuff up here
    STR_TO_TEST = "Hello, world!";
}

void tearDown(void) {
    // clean stuff up here
    STR_TO_TEST = "";
}
void test_ota_existance(){
    TEST_ASSERT_TRUE(STR_TO_TEST.equalsIgnoreCase("HELLO, WORLD!"));
}

void setup()
{
    delay(1000); // service delay
    UNITY_BEGIN();
    RUN_TEST(test_ota_existance);
    UNITY_END(); // stop unit testing
}

void loop()
{
}