#include "pch.h"

#ifdef UNITTEST

#include "pch.h"

#include <warning_push.h>
#include <gtest/gtest.h>
#include <warning_pop.h>

#include <rational.h>

using namespace newlang;

TEST(ObjTest, BigNum) {

    BigNum dec;

    ASSERT_TRUE(dec.value);

    dec.SetFromString("123456789");
    ASSERT_TRUE(dec.value);
    
    ASSERT_STREQ("123456789", dec.GetAsString().c_str());
    ASSERT_EQ(123456789, dec.GetAsInteger());
    ASSERT_EQ(123456789, dec.GetAsNumber());

    BigNum dec2;
    dec2.SetFromString("-123456789");
    ASSERT_STREQ("-123456789", dec2.GetAsString().c_str());
    ASSERT_EQ(-123456789, dec2.GetAsInteger());
    ASSERT_EQ(-123456789, dec2.GetAsNumber());

    dec.add(dec2);
    ASSERT_STREQ("0", dec.GetAsString().c_str());
    ASSERT_EQ(0, dec.GetAsInteger());

    dec.sub(dec2);
    ASSERT_STREQ("123456789", dec.GetAsString().c_str());
    ASSERT_EQ(123456789, dec.GetAsInteger());


    BigNum dec3;
    dec3.SetFromString("123");
    ASSERT_STREQ("123", dec3.GetAsString().c_str());
    ASSERT_EQ(123, dec3.GetAsInteger());

    dec.mul(dec3);
    ASSERT_STREQ("15185185047", dec.GetAsString().c_str());
    ASSERT_EQ(15185185047, dec.GetAsInteger());

    dec.div(dec2, dec3);
    ASSERT_STREQ("-123", dec.GetAsString().c_str());
    ASSERT_EQ(-123, dec.GetAsInteger());

    ASSERT_STREQ("0", dec3.GetAsString().c_str());
    ASSERT_EQ(0, dec3.GetAsInteger());


    dec.SetFromString("123456789123456789123456789123456789");
    ASSERT_STREQ("123456789123456789123456789123456789", dec.GetAsString().c_str());
    ASSERT_ANY_THROW(dec.GetAsInteger());
    ASSERT_DOUBLE_EQ(1.23456789123456789e+35, dec.GetAsNumber());

}

#endif