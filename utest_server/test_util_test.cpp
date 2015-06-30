// db_imp_test.cpp
#include <cstdlib>

#include <cppunit/config/SourcePrefix.h>
#include "test_util_test.h"

#include "test_def.h"
#include "test_util.h"

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( TestUtilTest );


void TestUtilTest::test_test_str()
{
    // when len <= prefixLen
    //   test_str(adfadf, 3) == adf
    {
        CPPUNIT_ASSERT(test_str("asdf", 3) == "asd");
        CPPUNIT_ASSERT(test_str("asdf", 4) == "asdf");
        CPPUNIT_ASSERT(test_str("asdf", 5) == "asdf:");
    }

    // when len < 20
    //   test_str(adf, 13) == adfa:67890123
    {
        CPPUNIT_ASSERT(test_str("asdf", 7) == "asdf:67");
        CPPUNIT_ASSERT(test_str("asdf", 9) == "asdf:6789");
        CPPUNIT_ASSERT(test_str("asdf", 10) == "asdf:67890");
        CPPUNIT_ASSERT(test_str("asdf", 11) == "asdf:678901");
        CPPUNIT_ASSERT(test_str("asdf", 19) == "asdf:67890123456789");
        CPPUNIT_ASSERT(test_str("1234567890123", 19) == "1234567890123:56789");
        CPPUNIT_ASSERT(test_str("123456789012345678", 19) == "123456789012345678:");
    }

    // when len >= 20
    //   test_str(asdf, 45) = asdf:=====(20_56789)(30_56789)12345
    {
        // 20
        {
            CPPUNIT_ASSERT(test_str("123", 20) == "123:======(20_56789)");
            CPPUNIT_ASSERT(test_str("123456789", 20) == "123456789:(20_56789)");
            CPPUNIT_ASSERT(test_str("1234567890", 20) == "1234567890(20_56789)");
            CPPUNIT_ASSERT(test_str("123456789012345", 20) == "1234567890(20_56789)");
        }

        // < 30
        {
            CPPUNIT_ASSERT(test_str("123", 21) == "123:======(20_56789)1");
            CPPUNIT_ASSERT(test_str("123", 23) == "123:======(20_56789)123");
            CPPUNIT_ASSERT(test_str("123", 29) == "123:======(20_56789)123456789");
        }

        // >= 30
        {
            CPPUNIT_ASSERT(test_str("123", 30) == "123:======(20_56789)(30_56789)");
            CPPUNIT_ASSERT(test_str("123", 31) == "123:======(20_56789)(30_56789)1");
            CPPUNIT_ASSERT(test_str("123", 45) == "123:======(20_56789)(30_56789)(40_56789)12345");
            CPPUNIT_ASSERT(test_str("123", 129) == "123:======(20_56789)(30_56789)(40_56789)(50_56789)(60_56789)(70_56789)(80_56789)(90_56789)(100_6789)(110_6789)(120_6789)123456789");
            CPPUNIT_ASSERT(test_str("123", 129).size() == 129);
            CPPUNIT_ASSERT(test_str("123", 1239).size() == 1239);
            CPPUNIT_ASSERT(test_str("123", 12390).size() == 12390);
            //CPPUNIT_ASSERT(test_str("123", 123903).size() == 123903); // too slow
        }
    }
}
