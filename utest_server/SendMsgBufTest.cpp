// db_imp_test.cpp
#include <cstdlib>

#include <cppunit/config/SourcePrefix.h>
#include "SendMsgBufTest.h"
#include "send_msg_buf.h"
#include "test_def.h"
#include "test_util.h"

#define private public
#define protected public
#undef private
#undef protected


// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( SendMsgBufTest );

void SendMsgBufTest::setUp()
{
}


void SendMsgBufTest::tearDown()
{
}


void SendMsgBufTest::test_constructor()
{
}

void SendMsgBufTest::test_AddGetHasMsg()
{
    {
        SendMsgBuf buf;
        CPPUNIT_ASSERT(buf.GetMsgQueue(1).size() == 0);
        CPPUNIT_ASSERT(buf.HasMsg(1) == false);
    }
    {
        SendMsgBuf buf;
        buf.AddMsg(1, std::make_shared<std::string>("111"));
        CPPUNIT_ASSERT(buf.GetMsgQueue(2).size() == 0);
        CPPUNIT_ASSERT(buf.HasMsg(2) == false);

        CPPUNIT_ASSERT(buf.GetMsgQueue(1).size() == 1);
        CPPUNIT_ASSERT(buf.HasMsg(1) == true);
        CPPUNIT_ASSERT(*buf.GetMsgQueue(1).front() == "111");
    }
    {
        SendMsgBuf buf;
        buf.AddMsg(1, std::make_shared<std::string>("111"));
        buf.AddMsg(1, std::make_shared<std::string>("222"));
        buf.AddMsg(3, std::make_shared<std::string>("333"));
        CPPUNIT_ASSERT(buf.GetMsgQueue(2).size() == 0);
        CPPUNIT_ASSERT(buf.HasMsg(2) == false);

        CPPUNIT_ASSERT(buf.HasMsg(1) == true);
        std::queue<msg_ptr_t> queue1 = buf.GetMsgQueue(1);
        CPPUNIT_ASSERT(queue1.size() == 2);
        CPPUNIT_ASSERT(*queue1.front() == "111");
        queue1.pop();
        CPPUNIT_ASSERT(*queue1.front() == "222");
    }
}

void SendMsgBufTest::test_PeekFrontMsg()
{
    {
        SendMsgBuf buf;
        CPPUNIT_ASSERT(buf.PeekFrontMsg(1) == false);
    }
    {
        SendMsgBuf buf;
        buf.AddMsg(1, std::make_shared<std::string>("111"));
        CPPUNIT_ASSERT(bool(buf.PeekFrontMsg(2)) == false);
        CPPUNIT_ASSERT(*buf.PeekFrontMsg(1) == "111");
        CPPUNIT_ASSERT(buf.GetMsgQueue(1).size() == 1);
        CPPUNIT_ASSERT(*buf.PeekFrontMsg(1) == "111");
    }
    {
        SendMsgBuf buf;
        buf.AddMsg(1, std::make_shared<std::string>("111"));
        buf.AddMsg(1, std::make_shared<std::string>("222"));
        buf.AddMsg(3, std::make_shared<std::string>("333"));
        CPPUNIT_ASSERT(bool(buf.PeekFrontMsg(2)) == false);

        CPPUNIT_ASSERT(*buf.PeekFrontMsg(1) == "111");
    }
}

void SendMsgBufTest::test_PopMsg()
{
    // empty buf
    {
        SendMsgBuf buf;
        buf.PopMsg(1);
    }

    // buf with one msg.
    {
        SendMsgBuf buf;
        buf.AddMsg(1, std::make_shared<std::string>("111"));

        // pop a not exist sock.
        buf.PopMsg(2);
        CPPUNIT_ASSERT(*buf.PeekFrontMsg(1) == "111");

        // pop one
        buf.PopMsg(1);
        CPPUNIT_ASSERT(bool(buf.PeekFrontMsg(1)) == false);
        CPPUNIT_ASSERT(buf.GetMsgQueue(1).size() == 0);

        // pop when empty
        buf.PopMsg(1);
    }

    // buf with multi msg.
    {
        SendMsgBuf buf;
        buf.AddMsg(1, std::make_shared<std::string>("111"));
        buf.AddMsg(1, std::make_shared<std::string>("112"));
        buf.AddMsg(3, std::make_shared<std::string>("333"));
        CPPUNIT_ASSERT(bool(buf.PeekFrontMsg(2)) == false);

        CPPUNIT_ASSERT(*buf.PeekFrontMsg(1) == "111");
    }
}

void SendMsgBufTest::test_CleanMsg()
{
    // buf with multi msg.
    {
        SendMsgBuf buf;
        buf.AddMsg(1, std::make_shared<std::string>("111"));
        buf.AddMsg(1, std::make_shared<std::string>("112"));
        buf.AddMsg(3, std::make_shared<std::string>("333"));

        buf.CleanMsg(2);
        CPPUNIT_ASSERT(buf.GetMsgQueue(1).size() == 2);
        CPPUNIT_ASSERT(buf.GetMsgQueue(3).size() == 1);

        buf.CleanMsg(3);
        CPPUNIT_ASSERT(buf.GetMsgQueue(1).size() == 2);
        CPPUNIT_ASSERT(buf.GetMsgQueue(3).size() == 0);

        buf.CleanMsg(1);
        CPPUNIT_ASSERT(buf.GetMsgQueue(1).size() == 0);
        CPPUNIT_ASSERT(buf.GetMsgQueue(3).size() == 0);

        buf.AddMsg(3, std::make_shared<std::string>("333"));
        CPPUNIT_ASSERT(buf.GetMsgQueue(1).size() == 0);
        CPPUNIT_ASSERT(buf.GetMsgQueue(3).size() == 1);
    }
}

void SendMsgBufTest::test_CleanAllMsg()
{
    SendMsgBuf buf;
    buf.AddMsg(1, std::make_shared<std::string>("111"));
    buf.AddMsg(1, std::make_shared<std::string>("112"));
    buf.AddMsg(3, std::make_shared<std::string>("333"));
    buf.CleanAllMsg();
    CPPUNIT_ASSERT(buf.GetMsgQueue(1).size() == 0);
}
