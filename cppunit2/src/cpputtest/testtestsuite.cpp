#include <cpput/assertcommon.h>
#include <cpput/testing.h>
#include <cpput/testsuite.h>
#include <stdio.h>

/// @todo Simplify this test some more...

namespace {

   class CalledTest : public CppUT::TestCase
   {
   public:
      CalledTest( int &callCount )
         : CppUT::TestCase( "CalledTest" )
         , callCount_( callCount )
      {
      }

      void run()
      {
         ++callCount_;
      }

      int &callCount_;
   };
}


bool testTestSuite()
{
   printf( "Running bootstrap test: testTestSuite()...\n" );
   try
   {     
      CppUT::TestInfo::threadInstance().startNewTest();
      MockTestVisitor visitor;
      CppUT::TestSuitePtr suite1 = CppUT::makeTestSuite( "Suite 1" );
      suite1->accept( visitor );
      CPPUT_ASSERT_EQUAL( 0, visitor.visitTestCaseCount_ );
      CPPUT_ASSERT_EQUAL( 1, visitor.visitTestSuiteCount_ );

      CPPUT_ASSERT_EQUAL( 0, suite1->testCount() );
      CPPUT_ASSERT_EQUAL( "Suite 1", suite1->name() );

      int callCount1 = 0;
      CppUT::TestPtr test1( new CalledTest( callCount1 ) );
      suite1->add( test1 );
      CPPUT_ASSERT_EQUAL( 1, suite1->testCount() );
      CPPUT_ASSERT( test1 == suite1->testAt(0), "returned test[0] is not added test" );

      int callCount2 = 0;
      CppUT::TestPtr test2( new CalledTest( callCount2 ) );
      suite1->add( test2 );
      CPPUT_ASSERT_EQUAL( 2, suite1->testCount() );
      CPPUT_ASSERT( test1 == suite1->testAt(0), "returned test[0] is not added test" );
      CPPUT_ASSERT( test2 == suite1->testAt(1), "returned test[1] is not added test" );
   }
   catch ( const CppUT::AbortingAssertionException &e )
   {
      printf( "testTestsuite() failed: %s\n", e.what() );
      return false;
   }
   catch ( ... )
   {
      printf( "testTestsuite() failed (uncaught exception)." );
      return false;
   }

   return true;
}
