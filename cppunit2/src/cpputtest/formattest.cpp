#include "formattest.h"
#include <cpput/assert.h>
#include <cpptl/format.h>

void 
FormatTest::setUp()
{
   double x = 1, y = 1, z = 0;
   for ( int i = 1; i < 200; ++i ) {
      z += x + y;
      y *= 10;
      z /= 10;
   }
}


void 
FormatTest::tearDown()
{
}


void 
FormatTest::testLengthNoFormat()
{
   CPPUT_ASSERT_EQUAL( 9, CppTL::Formatter( "123456789", CppTL::FormatArgs() ).length() );
}


void 
FormatTest::testLengthIntFormat()
{
   CPPUT_ASSERT_EQUAL( 4+9, 
                       CppTL::Formatter( "ab%dcd", CppTL::args(123456789) ).length() );
}

