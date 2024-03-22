#include "pch.h"

#ifdef BUILD_UNITTEST

#include <warning_push.h>
#include <gtest/gtest.h>
#include <warning_pop.h>

using namespace newlang;

TEST(Logger, Logger) {

    LOG_DUMP("LOG_DEBUG");

    LOG_DEBUG("LOG_DEBUG");
    LOG_DEBUG("LOG_DEBUG %s", "sssssssssss");
    LOG_DEBUG("LOG_DEBUG %i", 11111111);

    LOG_ERROR("LOG_ERROR");
    LOG_ERROR("LOG_ERROR %f", 333.3333);

    LOG_INFO("LOG_INFO %s %s %s\n\n", "a", "bb", "ccc");

}

TEST(Logger, HexToByte) {
    EXPECT_EQ(HexToByte('0'), 0);
    EXPECT_EQ(HexToByte('1'), 1);
    EXPECT_EQ(HexToByte('2'), 2);
    EXPECT_EQ(HexToByte('3'), 3);
    EXPECT_EQ(HexToByte('4'), 4);
    EXPECT_EQ(HexToByte('5'), 5);
    EXPECT_EQ(HexToByte('6'), 6);
    EXPECT_EQ(HexToByte('7'), 7);
    EXPECT_EQ(HexToByte('8'), 8);
    EXPECT_EQ(HexToByte('9'), 9);

    EXPECT_EQ(HexToByte('A'), 0xA);
    EXPECT_EQ(HexToByte('B'), 0xB);
    EXPECT_EQ(HexToByte('C'), 0xC);
    EXPECT_EQ(HexToByte('D'), 0xD);
    EXPECT_EQ(HexToByte('E'), 0xE);
    EXPECT_EQ(HexToByte('F'), 0xF);

    EXPECT_EQ(HexToByte('a'), 0xA);
    EXPECT_EQ(HexToByte('b'), 0xB);
    EXPECT_EQ(HexToByte('c'), 0xC);
    EXPECT_EQ(HexToByte('d'), 0xD);
    EXPECT_EQ(HexToByte('e'), 0xE);
    EXPECT_EQ(HexToByte('f'), 0xF);

    EXPECT_THROW(HexToByte('w'), std::invalid_argument);
    EXPECT_THROW(HexToByte('+'), std::invalid_argument);
    EXPECT_THROW(HexToByte('S'), std::invalid_argument);
}

TEST(Logger, HexToBin) {
    std::array<uint8_t, 100> buffer;

    EXPECT_EQ(0, HexToBin("", buffer.data(), buffer.size()));
    EXPECT_EQ(0, HexToBin("12", buffer.data(), 0));
    EXPECT_EQ(1, HexToBin("01", buffer.data(), buffer.size()));
    EXPECT_EQ(2, HexToBin("2345", buffer.data(), buffer.size()));
    EXPECT_EQ(3, HexToBin("678910", buffer.data(), buffer.size()));

    EXPECT_EQ(2, HexToBin("ABCD", buffer.data(), 2));

    EXPECT_EQ(11, HexToBin("0123456789ABCDEFabcdef", buffer.data(), buffer.size()));
    EXPECT_EQ(buffer[0], 0x01);
    EXPECT_EQ(buffer[1], 0x23);
    EXPECT_EQ(buffer[2], 0x45);
    EXPECT_EQ(buffer[3], 0x67);
    EXPECT_EQ(buffer[4], 0x89);
    EXPECT_EQ(buffer[5], 0xAB);
    EXPECT_EQ(buffer[6], 0xCD);
    EXPECT_EQ(buffer[7], 0xEF);
    EXPECT_EQ(buffer[8], 0xAB);
    EXPECT_EQ(buffer[9], 0xCD);
    EXPECT_EQ(buffer[10], 0xEF);

    EXPECT_TRUE(HexToBinEq("", buffer.data(), 0));
    EXPECT_TRUE(HexToBinEq("01", buffer.data(), 1));
    EXPECT_TRUE(HexToBinEq("0123", buffer.data(), 2));
    EXPECT_TRUE(HexToBinEq("012345", buffer.data(), 3));
    EXPECT_TRUE(HexToBinEq("0123456789ABCDEFabcdef", buffer.data(), 11));

    EXPECT_FALSE(HexToBinEq("11", buffer.data(), 1));
    EXPECT_FALSE(HexToBinEq("0123456789ABCDEFabcdef", buffer.data(), 12));
    EXPECT_FALSE(HexToBinEq("0123456789ABCDEFabcdefFF", buffer.data(), 11));


    EXPECT_THROW(HexToBin(nullptr, buffer.data(), buffer.size()), std::invalid_argument);
    EXPECT_THROW(HexToBin(nullptr, nullptr, buffer.size()), std::invalid_argument);
    EXPECT_THROW(HexToBin("", nullptr, buffer.size()), std::invalid_argument);

    uint8_t buf[17] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF, 0xF, 0};
    EXPECT_STREQ(BinToHex(buf, sizeof (buf)).c_str(), "0102030405060708090A0B0C0D0E0F0F00");
}


std::string test_log_out;

void test_log_callback(void *, Logger::LogLevelType, const char * str, bool) {
    test_log_out.append(str);
}

TEST(Logger, Duplicate) {

    Logger::Instance()->SetLogLevel(LOG_LEVEL_DEBUG);
    Logger::Instance()->SetCallback(&test_log_callback, nullptr);

    LOG_DEBUG("Test 1");
    EXPECT_TRUE(test_log_out.find("D:Test 1 (logger_test.cpp:") == 0);
    test_log_out.clear();
    LOG_DEBUG("Test");
    EXPECT_TRUE(test_log_out.find("D:Test (logger_test.cpp:") == 0);
    test_log_out.clear();

    Logger::Instance()->SetLogLevel(LOG_LEVEL_NORMAL);
    LOG_INFO("123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 ");
    EXPECT_STREQ(test_log_out.c_str(), "123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 \n");
    test_log_out.clear();

    Logger::Instance()->SetLogLevel(LOG_LEVEL_MAX);
    LOG_INFO("123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 ");
    EXPECT_TRUE(test_log_out.find("123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789 123456789  (logger_test.cpp:") == 0);
    test_log_out.clear();

    Logger::Instance()->SetCallback(nullptr, nullptr);

}

#endif
