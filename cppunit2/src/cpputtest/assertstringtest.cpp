#include "testing.h"
#include <cpput/assertcommon.h>
#include <cpput/assertstring.h>
#include <cpput/testing.h>
#include <string.h>     // use string.h for easier portability

#if _MSC_VER >= 1400 // VC++ 8.0
#pragma warning( disable : 4996 )   // disable warning about strdup being deprecated.
#endif

namespace {

   // A small string wrapper to test 'custom' string comparison
   class StringWrapper
   {
   public:
      StringWrapper( const char *str )
         : str_( str )
      {
      }

      std::string str() const
      {
         return str_;
      }

   private:
      const char *str_;
   };
#if defined(CPPTL_NO_ADL)
}

namespace CppUT {
#endif // if defined(CPPTL_NO_ADL)

   // Specialize convertToStringImpl function to handle StringWrapper type.
   // @todo Does this work on VC6 without putting it in CppUT namespace
   inline std::string getStdString( const StringWrapper &str )
   {
      return str.str();
   }

}


CPPUT_SUITE( "StringAssertions" ) {

CPPUT_TEST_FUNCTION( testStringStartsWith )
{
   CPPUT_ASSERT_ASSERTION_FAIL(( CPPUT_ASSERTSTR_START( "abcdef", "bc" ) ));
   CPPUT_ASSERT_ASSERTION_FAIL(( CPPUT_ASSERTSTR_START( "abcdef", "AB" ) ));
   CPPUT_ASSERT_ASSERTION_FAIL(( CPPUT_ASSERTSTR_START( "abcdef", "ef" ) ));
   CPPUT_ASSERT_ASSERTION_PASS(( CPPUT_ASSERTSTR_START( "abcdef", "ab" ) ));
   CPPUT_ASSERT_ASSERTION_PASS(( CPPUT_ASSERTSTR_START( "abcdef", "abcdef" ) ));
   CPPUT_ASSERT_ASSERTION_PASS(( CPPUT_ASSERTSTR_START( "abcdef", "" ) ));

   CPPUT_ASSERT_ASSERTION_PASS(( CPPUT_CHECKSTR_START( "abcdef", "ab" ) ));
}


CPPUT_TEST_FUNCTION( testStringEndsWith )
{
   CPPUT_ASSERT_ASSERTION_FAIL(( CPPUT_ASSERTSTR_END( "abcdef", "de" ) ));
   CPPUT_ASSERT_ASSERTION_FAIL(( CPPUT_ASSERTSTR_END( "abcdef", "EF" ) ));
   CPPUT_ASSERT_ASSERTION_FAIL(( CPPUT_ASSERTSTR_END( "abcdef", "ab" ) ));
   CPPUT_ASSERT_ASSERTION_PASS(( CPPUT_ASSERTSTR_END( "abcdef", "def" ) ));
   CPPUT_ASSERT_ASSERTION_PASS(( CPPUT_ASSERTSTR_END( "abcdef", "abcdef" ) ));
   CPPUT_ASSERT_ASSERTION_PASS(( CPPUT_ASSERTSTR_END( "abcdef", "" ) ));

   CPPUT_ASSERT_ASSERTION_PASS(( CPPUT_CHECKSTR_END( "abcdef", "def" ) ));
}


CPPUT_TEST_FUNCTION( testStringContains )
{
   CPPUT_ASSERT_ASSERTION_FAIL(( CPPUT_ASSERTSTR_CONTAIN( "abcdef", "fe" ) ));
   CPPUT_ASSERT_ASSERTION_FAIL(( CPPUT_ASSERTSTR_CONTAIN( "abcdef", "BC" ) ));
   CPPUT_ASSERT_ASSERTION_FAIL(( CPPUT_ASSERTSTR_CONTAIN( "abcdef", "abcdefg" ) ));
   CPPUT_ASSERT_ASSERTION_FAIL(( CPPUT_ASSERTSTR_CONTAIN( "abcdef", "0abcdef" ) ));
   CPPUT_ASSERT_ASSERTION_PASS(( CPPUT_ASSERTSTR_CONTAIN( "abcdef", "ab" ) ));
   CPPUT_ASSERT_ASSERTION_PASS(( CPPUT_ASSERTSTR_CONTAIN( "abcdef", "ef" ) ));
   CPPUT_ASSERT_ASSERTION_PASS(( CPPUT_ASSERTSTR_CONTAIN( "abcdef", "cd" ) ));
   CPPUT_ASSERT_ASSERTION_PASS(( CPPUT_ASSERTSTR_CONTAIN( "abcdef", "abcdef" ) ));
   CPPUT_ASSERT_ASSERTION_PASS(( CPPUT_ASSERTSTR_CONTAIN( "abcdef", "" ) ));

   CPPUT_ASSERT_ASSERTION_PASS(( CPPUT_CHECKSTR_CONTAIN( "abcdef", "cd" ) ));
}


CPPUT_TEST_FUNCTION( testStringEquals )
{
   std::string abcdef( "abcdef" );
   char duplicate[32];
   strcpy( duplicate, "abcdef" );   
   // we use duplicate of the string to ensure that comparison of
   // two 'char *' string are different pointers.

   CPPUT_ASSERT_ASSERTION_FAIL(( CPPUT_ASSERTSTR_EQUAL( "abcdef", "ABCDEF" ) ));
   CPPUT_ASSERT_ASSERTION_FAIL(( CPPUT_ASSERTSTR_EQUAL( "ABCDEF", "abcdef" ) ));
   CPPUT_ASSERT_ASSERTION_FAIL(( CPPUT_ASSERTSTR_EQUAL( "abcdef", "abcde" ) ));
   CPPUT_ASSERT_ASSERTION_FAIL(( CPPUT_ASSERTSTR_EQUAL( "abcdef", "def" ) ));
   CPPUT_ASSERT_ASSERTION_FAIL(( CPPUT_ASSERTSTR_EQUAL( "abcdef", "" ) ));
   CPPUT_ASSERT_ASSERTION_FAIL(( CPPUT_ASSERTSTR_EQUAL( "", "abcdef" ) ));

   CPPUT_ASSERT_ASSERTION_PASS(( CPPUT_ASSERTSTR_EQUAL( "abcdef", duplicate ) ));
   CPPUT_ASSERT_ASSERTION_PASS(( CPPUT_ASSERTSTR_EQUAL( "abcdef", abcdef ) ));
   CPPUT_ASSERT_ASSERTION_PASS(( CPPUT_ASSERTSTR_EQUAL( abcdef, "abcdef" ) ));
   CPPUT_ASSERT_ASSERTION_PASS(( CPPUT_ASSERTSTR_EQUAL( "", std::string() ) ));

   CPPUT_ASSERT_ASSERTION_PASS(( CPPUT_CHECKSTR_EQUAL( "abcdef", abcdef ) ));
}


CPPUT_TEST_FUNCTION( testCustomStringTypeHandling )
{
   StringWrapper abcdefWrap( "abcdef" );
   std::string abcdef( "abcdef" );

   CPPUT_ASSERT_ASSERTION_FAIL(( CPPUT_ASSERTSTR_EQUAL( abcdefWrap, "def" ) ));
   CPPUT_ASSERT_ASSERTION_PASS(( CPPUT_ASSERTSTR_EQUAL( abcdef, abcdefWrap ) ));

   CPPUT_ASSERT_ASSERTION_FAIL(( CPPUT_ASSERTSTR_START( abcdefWrap, "ef" ) ));
   CPPUT_ASSERT_ASSERTION_PASS(( CPPUT_ASSERTSTR_START( abcdefWrap, "ab" ) ));

   CPPUT_ASSERT_ASSERTION_FAIL(( CPPUT_ASSERTSTR_END( abcdefWrap, "ab" ) ));
   CPPUT_ASSERT_ASSERTION_PASS(( CPPUT_ASSERTSTR_END( abcdefWrap, "def" ) ));

   CPPUT_ASSERT_ASSERTION_FAIL(( CPPUT_ASSERTSTR_CONTAIN( abcdefWrap, "0abcdef" ) ));
   CPPUT_ASSERT_ASSERTION_PASS(( CPPUT_ASSERTSTR_CONTAIN( abcdefWrap, "ab" ) ));
}

} // end suite stringAssertionsSuite
