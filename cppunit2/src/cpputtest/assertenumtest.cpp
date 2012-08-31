#include "assertenumtest.h"
#include "testing.h"
#include <algorithm>
#include <iterator>


CPPUT_SUITE_REGISTER( AssertEnumTest );


void 
AssertEnumTest::setUp()
{
   vEmpty_.clear();
   v123_.clear();
   v123_.push_back( 1 );
   v123_.push_back( 2 );
   v122_ = std::vector<int>( v123_ );
   v122_.push_back( 2 );
   v123_.push_back( 3 );

   v1234_ = std::vector<int>( v123_ );
   v1234_.push_back( 4  );

   v321_ = std::vector<int>( v123_ );
   std::reverse( v321_.begin(), v321_.end() );

   v4321_ = std::vector<int>( v1234_ );
   std::reverse( v4321_.begin(), v4321_.end() );
}


void 
AssertEnumTest::tearDown()
{
   v122_.clear();
   v123_.clear();
   v1234_.clear();
   v321_.clear();
   v4321_.clear();
}


void 
AssertEnumTest::testAssertSequenceEqual()
{
   CPPUT_ASSERT_ASSERTION_FAIL(( CPPUT_ASSERT_SEQUENCE_EQUAL( CppTL::Enum::container( vEmpty_ ),
                                                              CppTL::Enum::container( v123_ ) ) ));
   CPPUT_ASSERT_ASSERTION_FAIL(( CPPUT_ASSERT_SEQUENCE_EQUAL( CppTL::Enum::container( v123_ ), 
                                                              CppTL::Enum::container( v321_ ) ) ));
   CPPUT_ASSERT_ASSERTION_FAIL(( CPPUT_ASSERT_SEQUENCE_EQUAL( CppTL::Enum::container( v123_ ), 
                                                              CppTL::Enum::container( v122_ ) ) ));
   CPPUT_ASSERT_ASSERTION_FAIL(( CPPUT_ASSERT_SEQUENCE_EQUAL( CppTL::Enum::container( v123_ ), 
                                                              CppTL::Enum::container( v4321_ ) ) ));
   CPPUT_ASSERT_ASSERTION_FAIL(( CPPUT_ASSERT_SEQUENCE_EQUAL( CppTL::Enum::container( v123_ ), 
                                                              CppTL::Enum::container( v1234_ ) ) ));
   CPPUT_ASSERT_ASSERTION_FAIL(( CPPUT_ASSERT_SEQUENCE_EQUAL( CppTL::Enum::container( v1234_ ), 
                                                              CppTL::Enum::container( v123_ ) ) ));

   CPPUT_ASSERT_ASSERTION_PASS(( CPPUT_ASSERT_SEQUENCE_EQUAL( CppTL::Enum::container( vEmpty_ ), 
                                                              CppTL::Enum::container( vEmpty_ ) ) ));
   CPPUT_ASSERT_ASSERTION_PASS(( CPPUT_ASSERT_SEQUENCE_EQUAL( CppTL::Enum::container( v122_ ), 
                                                              CppTL::Enum::container( v122_ ) ) ));
   CPPUT_ASSERT_ASSERTION_PASS(( CPPUT_ASSERT_SEQUENCE_EQUAL( CppTL::Enum::container( v1234_ ), 
                                                              CppTL::Enum::container( v1234_ ) ) ));
}


void 
AssertEnumTest::testAssertSetEqual()
{
   CPPUT_ASSERT_ASSERTION_FAIL(( CPPUT_ASSERT_SET_EQUAL( CppTL::Enum::container( vEmpty_ ),
                                                        CppTL::Enum::container( v123_ ) ) ));
   CPPUT_ASSERT_ASSERTION_FAIL(( CPPUT_ASSERT_SET_EQUAL( CppTL::Enum::container( v123_ ),
                                                        CppTL::Enum::container( vEmpty_ ) ) ));
   CPPUT_ASSERT_ASSERTION_FAIL(( CPPUT_ASSERT_SET_EQUAL( CppTL::Enum::container( v123_ ),
                                                        CppTL::Enum::container( v1234_ ) ) ));
   CPPUT_ASSERT_ASSERTION_FAIL(( CPPUT_ASSERT_SET_EQUAL( CppTL::Enum::container( v123_ ),
                                                        CppTL::Enum::container( v122_ ) ) ));

   CPPUT_ASSERT_ASSERTION_PASS(( CPPUT_ASSERT_SET_EQUAL( CppTL::Enum::container( v123_ ),
                                                        CppTL::Enum::container( v321_ ) ) ));
   CPPUT_ASSERT_ASSERTION_PASS(( CPPUT_ASSERT_SET_EQUAL( CppTL::Enum::container( v4321_ ),
                                                        CppTL::Enum::container( v1234_ ) ) ));
}
