#include "valuetest.h"
#include "testing.h"
#include <cpptl/value.h>


ValueTest::ValueTest()
   : maxInt_( ((unsigned int)~0)/2 )
   , minInt_( ~((unsigned int)(~0)/2) )
{
}

CPPUT_FIXTURE_TEST( ValueTest, testNone )
{
   CppTL::Value value;
   CPPUT_CHECK_EXPR( value.isNone() );
   CppTL::Value value2;
   CPPUT_CHECK_EXPR( value2.isNone() );
   CPPUT_CHECK_EXPR( value == value2 );
   CPPUT_CHECK_EXPR_FALSE( value < value2 );
   CPPUT_CHECK_EXPR( value <= value2 );
   CPPUT_CHECK_EXPR_FALSE( value != value2 );
   CPPUT_CHECK_EXPR( value >= value2 );
   CPPUT_CHECK_EXPR_FALSE( value > value2 );
}


CPPUT_FIXTURE_TEST( ValueTest, testLifeCycle )
{
   CppTL::Value value1 = CppTL::makeValue( maxInt_ );
   CppTL::Value value1a = value1;
   CppTL::Value value2 = CppTL::makeValue( minInt_ );
   CppTL::Value value2a = CppTL::makeValue( minInt_ );
   CPPUT_CHECK_EXPR_FALSE( value1a.isNone() );

   CPPUT_CHECK_EXPR( value2 < value1 );
   CPPUT_CHECK_EXPR( value2 <= value1 );
   CPPUT_CHECK_EXPR( value1 != value2 );
   CPPUT_CHECK_EXPR_FALSE( value1 == value2 );
   CPPUT_CHECK_EXPR_FALSE( value2 >= value1 );
   CPPUT_CHECK_EXPR_FALSE( value2 > value1 );

   CPPUT_CHECK_EXPR( value1 == value1a );
   CPPUT_CHECK_EXPR( value2 == value2a );
}
