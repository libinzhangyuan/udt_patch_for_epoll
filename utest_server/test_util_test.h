// DataFileTest.h
#ifndef TEST_UTIL_TEST_H
#define TEST_UTIL_TEST_H

#include <cppunit/extensions/HelperMacros.h>

class TestUtilTest : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE( TestUtilTest );
  CPPUNIT_TEST( test_test_str );
  CPPUNIT_TEST_SUITE_END();

public:
  void test_test_str();
};

#endif  // TEST_UTIL_TEST_H
