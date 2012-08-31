#include <cpput/testrunresult.h>
#include <cpput/assertcommon.h>
#include <cpput/resultexception.h>
#include <cpput/testing.h>
#include <cpput/testcontext.h>
#include <iostream>

namespace {
   class DummyTest : public CppUT::AbstractTest
   {
   public:

      void run( CppUT::TestContext &context )
      {
      }

      void accept( CppUT::TestVisitor &visitor )
      {
      }

      int totalTestCount() const
      {
         return 1;
      }
   };
}


bool testTestRunResult()
{
   try
   {
      CppUT::TestInfo::startNewTest();
      CppUT::TestRunResult result;
      CppUT::TestContext context;
      CPPUT_ASSERT( result.successful() );
      CPPUT_ASSERT_EQUAL( 0, result.failureCount() );

      DummyTest dummyTest1;
      CppUT::Message message1( "message1" );
      message1.setStatus( false );
      message1.setResultType( "fault" );
      CppUT::TestResult failure1( dummyTest1, message1 );

      result.processTestResult( failure1, context );
      CPPUT_ASSERT_FALSE( result.successful() );
      CPPUT_ASSERT_EQUAL( 1, result.failureCount() );
      const CppUT::TestResult &failure1b = result.failureAt(0);
      CPPUT_ASSERT_FALSE( failure1b.isAssertionFailure() );
      CPPUT_ASSERT_EQUAL( 1, failure1b.message().count() );
      CPPUT_ASSERT_EQUAL( "message1", failure1b.message().at(0) );

      DummyTest dummyTest2;
      CppUT::Message message2( "message2" );
      message2.setStatus( false );
      message2.setResultType( "assertion" );
      CppUT::TestResult failure2( dummyTest2, message2, 
                                  CppUT::Location( "somefile.cpp", 123 ) );
      result.processTestResult( failure2, context );
      CPPUT_ASSERT_FALSE( result.successful() );
      CPPUT_ASSERT_EQUAL( 2, result.failureCount() );
      const CppUT::TestResult &failure2b = result.failureAt(1);
      CPPUT_ASSERT( failure2b.isAssertionFailure() );
      CPPUT_ASSERT_EQUAL( 1, failure2b.message().count() );
      CPPUT_ASSERT_EQUAL( "message2", failure2b.message().at(0) );
      CPPUT_ASSERT_EQUAL( "somefile.cpp", failure2b.location().fileName() );
      CPPUT_ASSERT_EQUAL( 123, failure2b.location().lineNumber() );


      DummyTest dummyTest3;
      CppUT::Message message3( "message3" );
      message3.setStatus( false );
      message3.setResultType( "assertion" );
      CppUT::TestResult failure3( dummyTest3, message3, 
                                  CppUT::Location( "somefile3.cpp", 23 ) );
      result.processTestResult( failure3, context );
      CPPUT_ASSERT_EQUAL( 3, result.failureCount() );
      CPPUT_ASSERT( result.failureAt(2).isAssertionFailure() );

      DummyTest dummyTest4;
      CppUT::Message message4( "message4" );
      message3.setStatus( false );
      message3.setResultType( "assertion" );
      CppUT::TestResult failure4( dummyTest4, message4 );
      result.processTestResult( failure4, context );
      CPPUT_ASSERT_EQUAL( 4, result.failureCount() );
      CPPUT_ASSERT_FALSE( result.failureAt(3).isAssertionFailure() );
   }
   catch ( CppUT::AbortingAssertionException &e )
   {
      std::cout << "testResult() failed: "  <<  e.what()  <<  std::endl;
      return false;
   }

   return true;
}
