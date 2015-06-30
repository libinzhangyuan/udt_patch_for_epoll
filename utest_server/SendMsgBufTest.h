// SendMsgBufTest.h
#ifndef _UDT_SEND_MSG_BUF_TEST_H_
#define _UDT_SEND_MSG_BUF_TEST_H_

#include <cppunit/extensions/HelperMacros.h>

class SendMsgBufTest : public CPPUNIT_NS::TestFixture
{
  CPPUNIT_TEST_SUITE( SendMsgBufTest );
  CPPUNIT_TEST( test_constructor );
  CPPUNIT_TEST( test_AddGetHasMsg );
  CPPUNIT_TEST( test_PeekFrontMsg );
  CPPUNIT_TEST( test_PopMsg );
  CPPUNIT_TEST( test_CleanMsg );
  CPPUNIT_TEST( test_CleanAllMsg );
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void test_constructor();
  void test_AddGetHasMsg();
  void test_PeekFrontMsg();
  void test_PopMsg();
  void test_CleanMsg();
  void test_CleanAllMsg();
};

#endif  // _UDT_SEND_MSG_BUF_TEST_H_
