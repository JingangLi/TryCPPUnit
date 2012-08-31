#include <cpput/assertcommon.h>
#include <stdio.h>
#include <stdexcept>


static bool testAssertThrow()
{
   CppUT::TestInfo::threadInstance().startNewTest();
   try
   {
      CPPUT_ASSERT_THROW( throw std::runtime_error( "dummy" ), std::runtime_error );
   }
   catch ( ... )
   {
      printf( "Test 1: CPPUT_ASSERT_THROW should not have thrown an exception\n" );
      return false;
   }

   CppUT::TestInfo::threadInstance().startNewTest();
   try
   {
      CPPUT_ASSERT_THROW( 1234, std::exception );
      printf( "Test 2: expected AbortingAssertionException not thrown by CPPUT_ASSERT_THROW.\n" );
      return false;
   }
   catch ( CppUT::AbortingAssertionException & )
   {
   }

   return true;
}


static bool testAssertNoThrow()
{
   CppUT::TestInfo::threadInstance().startNewTest();
   try
   {
      CPPUT_ASSERT_NO_THROW( 1234 );
   }
   catch ( CppUT::AbortingAssertionException &e )
   {
      printf( "Test 3: CPPUT_ASSERT_NO_THROW should not "
              "have thrown an exception.\n%s\n", e.what() );
      return false;
   }

   CppUT::TestInfo::threadInstance().startNewTest();
   try
   {
      CPPUT_ASSERT_NO_THROW( throw std::runtime_error( "dummy" ) );
      printf( "Test 4: expected AbortingAssertionException "
              "not thrown by CPPUT_ASSERT_THROW.\n" );
      return false;
   }
   catch ( CppUT::AbortingAssertionException & )
   {
   }

   return true;
}

// A test predicate
static CppUT::CheckerResult
returnStatus( bool succeed )
{
   CppUT::CheckerResult result;
   if ( !succeed )
   {
      result.setFailed();
      result.diagnostic( "flag" ) = succeed;
   }
   return result;
}


namespace
{
   // This unnamed enum is a type with no linkage. Used to check that stringize() can
   // correctly stringize type without linkage.
   enum { noLinkageTypeValue = 1 };
}

bool testBasicAssertions()
{
   printf( "Running bootstrap test: testBasicAssertions()...\n" );
   if ( !testAssertThrow() )
      return false;
   if ( !testAssertNoThrow() )
      return false;

   CppUT::TestInfo::threadInstance().startNewTest();
   try
   {
      // @todo blep Needs to add test to ensure that expression are evaluated only once by macro

      // CPPUT_ASSERT
      CPPUT_ASSERT_ASSERTION_PASS(( CPPUT_ASSERT( true ) ));
      CPPUT_ASSERT_ASSERTION_FAIL(( CPPUT_ASSERT( false ) ));

      CPPUT_ASSERT_ASSERTION_PASS(( CPPUT_ASSERT( true, "should pass" ) ));
      CPPUT_ASSERT_ASSERTION_FAIL(( CPPUT_ASSERT( false, 
         "should throw AbortingAssertionException"  ) ));

      CPPUT_ASSERT_ASSERTION_PASS(( CPPUT_ASSERT_EXPR( true ) ));
      CPPUT_ASSERT_ASSERTION_FAIL(( CPPUT_ASSERT_EXPR( false ) ));

      // CPPUT_ASSERT_FALSE
      CPPUT_ASSERT_ASSERTION_PASS(( CPPUT_ASSERT_FALSE( false ) ));
      CPPUT_ASSERT_ASSERTION_FAIL(( CPPUT_ASSERT_FALSE( true ) ));

      CPPUT_ASSERT_ASSERTION_PASS(( CPPUT_ASSERT_FALSE( false, "should pass" ) ));
      CPPUT_ASSERT_ASSERTION_FAIL(( CPPUT_ASSERT_FALSE( true, 
         "should throw AbortingAssertionException"  ) ));

      CPPUT_ASSERT_ASSERTION_PASS(( CPPUT_ASSERT_EXPR_FALSE( false ) ));
      CPPUT_ASSERT_ASSERTION_FAIL(( CPPUT_ASSERT_EXPR_FALSE( true ) ));

      // CPPUT_ASSERT_EQUAL (compatibility macro)
      CPPUT_ASSERT_ASSERTION_PASS(( CPPUT_ASSERT_EQUAL( 1234567, 1234567 ) ));
      CPPUT_ASSERT_ASSERTION_FAIL(( CPPUT_ASSERT_EQUAL( 1234567, 456 ) ));

      CPPUT_ASSERT_ASSERTION_PASS(( CPPUT_ASSERT_EQUAL( std::string("abc"), "abc" ) ));
      CPPUT_ASSERT_ASSERTION_FAIL(( CPPUT_ASSERT_EQUAL( "def", std::string("abc") ) ));

      CPPUT_ASSERT_ASSERTION_PASS(( CPPUT_ASSERT_PREDICATE( true ) ));
      CPPUT_ASSERT_ASSERTION_FAIL(( CPPUT_ASSERT_PREDICATE( false ) ));

      CPPUT_ASSERT_ASSERTION_PASS(( CPPUT_ASSERT_PREDICATE( returnStatus(true) ) ));
      CPPUT_ASSERT_ASSERTION_FAIL(( CPPUT_ASSERT_PREDICATE( returnStatus(false) ) ));

      // @todo: should we support this? Stringize a type without linkage
//      CPPUT_ASSERT_ASSERTION_PASS( CPPUT_ASSERT_EQUAL( 1, noLinkageTypeValue ) );
//      CPPUT_ASSERT_ASSERTION_FAIL( CPPUT_ASSERT_EQUAL( 2, noLinkageTypeValue ) );

      // CPPUT_ASSERT_COMPARE
      CPPUT_ASSERT_ASSERTION_PASS(( CPPUT_ASSERT_COMPARE( 1, <, 2 ) ));
      CPPUT_ASSERT_ASSERTION_FAIL(( CPPUT_ASSERT_COMPARE( 2, <, 1 ) ));
      CPPUT_ASSERT_ASSERTION_PASS(( CPPUT_ASSERT_COMPARE( std::string("a"), <, "b" ) ));
      CPPUT_ASSERT_ASSERTION_FAIL(( CPPUT_ASSERT_COMPARE( "b", <, std::string("a") ) ));

      CPPUT_ASSERT_ASSERTION_PASS(( CPPUT_ASSERT_COMPARE( 1, <=, 2 ) ));
      CPPUT_ASSERT_ASSERTION_FAIL(( CPPUT_ASSERT_COMPARE( 2, <=, 1 ) ));
      CPPUT_ASSERT_ASSERTION_PASS(( CPPUT_ASSERT_COMPARE( std::string("a"), <=, "b" ) ));
      CPPUT_ASSERT_ASSERTION_FAIL(( CPPUT_ASSERT_COMPARE( "b", <=, std::string("a") ) ));
      CPPUT_ASSERT_ASSERTION_PASS(( CPPUT_ASSERT_COMPARE( 1, <=, 1) ));
      CPPUT_ASSERT_ASSERTION_PASS(( CPPUT_ASSERT_COMPARE( std::string("a"), <=, "a" ) ));

      CPPUT_ASSERT_ASSERTION_PASS(( CPPUT_ASSERT_COMPARE( 1234567, ==, 1234567 ) ));
      CPPUT_ASSERT_ASSERTION_FAIL(( CPPUT_ASSERT_COMPARE( 1234567, ==, 456 ) ));
      CPPUT_ASSERT_ASSERTION_PASS(( CPPUT_ASSERT_COMPARE( std::string("abc"), ==, "abc" ) ));
      CPPUT_ASSERT_ASSERTION_FAIL(( CPPUT_ASSERT_COMPARE( "def", ==, std::string("abc") ) ));

      CPPUT_ASSERT_ASSERTION_PASS(( CPPUT_ASSERT_COMPARE( 1234567, !=, 456 ) ));
      CPPUT_ASSERT_ASSERTION_FAIL(( CPPUT_ASSERT_COMPARE( 1234567, !=, 1234567 ) ));
      CPPUT_ASSERT_ASSERTION_PASS(( CPPUT_ASSERT_COMPARE( std::string("abc"), !=, "def" ) ));
      CPPUT_ASSERT_ASSERTION_FAIL(( CPPUT_ASSERT_COMPARE( "abc", !=, std::string("abc") ) ));

      CPPUT_ASSERT_ASSERTION_PASS(( CPPUT_ASSERT_COMPARE( 2, >=, 1 ) ));
      CPPUT_ASSERT_ASSERTION_FAIL(( CPPUT_ASSERT_COMPARE( 1, >=, 2 ) ));
      CPPUT_ASSERT_ASSERTION_PASS(( CPPUT_ASSERT_COMPARE( std::string("b"), >=, "a" ) ));
      CPPUT_ASSERT_ASSERTION_FAIL(( CPPUT_ASSERT_COMPARE( "a", >=, std::string("b") ) ));
      CPPUT_ASSERT_ASSERTION_PASS(( CPPUT_ASSERT_COMPARE( 2, >=, 2 ) ));
      CPPUT_ASSERT_ASSERTION_PASS(( CPPUT_ASSERT_COMPARE( std::string("b"), >=, "b" ) ));

      CPPUT_ASSERT_ASSERTION_PASS(( CPPUT_ASSERT_COMPARE( 2, >, 1 ) ));
      CPPUT_ASSERT_ASSERTION_FAIL(( CPPUT_ASSERT_COMPARE( 1, >, 2 ) ));
      CPPUT_ASSERT_ASSERTION_PASS(( CPPUT_ASSERT_COMPARE( std::string("b"), >, "a" ) ));
      CPPUT_ASSERT_ASSERTION_FAIL(( CPPUT_ASSERT_COMPARE( "a", >, std::string("b") ) ));

      // Test that actual and expected are evaluated only once
      int x = 0;
      int y = 0;
      CPPUT_ASSERT_ASSERTION_PASS(( CPPUT_ASSERT_COMPARE( x++, ==, y++ ) ));
      CPPUT_ASSERT_COMPARE( x, ==, 1 );
      CPPUT_ASSERT_COMPARE( y, ==, 1 );
   }
   catch ( const CppUT::AbortingAssertionException &e )
   {
      printf( "testBasicAssertions() failed: %s\n", e.what() );
      return false;
   }
   catch ( ... )
   {
      printf( "testBasicAssertions() failed (uncaught exception)." );
      return false;
   }

   return true;
}
