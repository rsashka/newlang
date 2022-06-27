#include "pch.h"

//#ifdef UNITTEST
//
//#include "core/pch.h"
//
//#include <core/warning_push.h>
//#include <gtest/gtest.h>
//#include <core/warning_pop.h>
//
//#include <core/fraction.h>
//
//
//TEST(ObjTest, BigNum) {
//
//    BigNum dec;
//
//    ASSERT_FALSE(dec.m_is_init);
//    ASSERT_TRUE(dec.m_value);
//
//    dec.SetFromString("123456789");
//    ASSERT_STREQ("123456789", dec.GetAsString().c_str());
//    ASSERT_EQ(123456789, dec.GetAsInteger());
//    ASSERT_EQ(123456789, dec.GetAsNumber());
//
//    BigNum dec2;
//    dec2.SetFromString("-123456789");
//    ASSERT_STREQ("-123456789", dec2.GetAsString().c_str());
//    ASSERT_EQ(-123456789, dec2.GetAsInteger());
//    ASSERT_EQ(-123456789, dec2.GetAsNumber());
//    
//    dec.add(dec2);
//    ASSERT_STREQ("0", dec.GetAsString().c_str());
//    ASSERT_EQ(0, dec.GetAsInteger());
//    
//    dec.sub(dec2);
//    ASSERT_STREQ("123456789", dec.GetAsString().c_str());
//    ASSERT_EQ(123456789, dec.GetAsInteger());
//
//
//    BigNum dec3;
//    dec3.SetFromString("123");
//    ASSERT_STREQ("123", dec3.GetAsString().c_str());
//    ASSERT_EQ(123, dec3.GetAsInteger());
//
//    dec.mul(dec3);
//    ASSERT_STREQ("15185185047", dec.GetAsString().c_str());
//    ASSERT_EQ(15185185047, dec.GetAsInteger());
//    
//    dec.div(dec2, dec3);
//    ASSERT_STREQ("-123", dec.GetAsString().c_str());
//    ASSERT_EQ(-123, dec.GetAsInteger());
//
//    ASSERT_STREQ("0", dec3.GetAsString().c_str());
//    ASSERT_EQ(0, dec3.GetAsInteger());
//}
//
//#endif