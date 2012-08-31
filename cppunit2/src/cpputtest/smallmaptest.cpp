#include "smallmaptest.h"
#include "testing.h"
#include <cpput/assertcommon.h>

CPPUT_SUITE_REGISTER( SmallMapTest );


int SmallMapTest::keyset1[10] = { 0, 1, 3, 7, 8, 
                                  12, 18, 20, 21, 23 };
int SmallMapTest::keyset2[10] = { 16, 2, 15, 1, 0, 
                                  18, 97, 5, 6, 7 };
int SmallMapTest::keyset3[4] =  {        15, 1, 
                                  18,           7 };


#define LOCAL_CHECK_ERASE_RANGE \
   CPPUT_BEGIN_CHECKING_MACRO() checkEraseRange

#define LOCAL_CHECK_ERASE_ITERATOR \
   CPPUT_BEGIN_CHECKING_MACRO() checkEraseIterator



CppUT::CheckerResult
SmallMapTest::checkEraseRange( int minSeed, int maxSeed, int beginRange, int endRange)
{
   CppUT::CheckerResult result;
   result.compose( CppUT::checkTrue( beginRange >= minSeed  &&  beginRange <= maxSeed ) );
   result.compose( CppUT::checkTrue( endRange >= minSeed  &&  endRange <= maxSeed ) );
   result.compose( CppUT::checkTrue( beginRange <= endRange ) );
   result.compose( CppUT::checkTrue( minSeed <= maxSeed ) );
   // @todo fix this and skip following if result has failed.
   SmallMapIntInt sm;
   for ( int generator = minSeed; generator < maxSeed; ++generator )
      sm.insert( IntInt( generator, generator * 101 ) );
   sm.erase( sm.begin() + (beginRange - minSeed), 
             sm.begin() + (endRange - minSeed));
   for ( int checker = minSeed; checker < maxSeed; ++checker )
   {
      CppTL::ConstString msg = "Checking key: " + CppTL::toString( checker );
      int expected = (checker >= beginRange  &&  checker < endRange) ? 0 : 1;
      result.compose( CppUT::checkEquals( expected, sm.count(checker), msg.c_str() ) );
   }
   result.compose( CppUT::checkEquals( maxSeed-minSeed - (endRange-beginRange), sm.size() ) );
   return result;
}


CppUT::CheckerResult  
SmallMapTest::checkEraseIterator( int minSeed, int maxSeed, int value )
{
   CppUT::CheckerResult result;
   CppUT::checkTrue( minSeed <= maxSeed &&  value >= minSeed  &&  value < maxSeed );
   SmallMapIntInt sm;
   for ( int generator = minSeed; generator < maxSeed; ++generator )
      sm.insert( IntInt( generator, generator * 101 ) );
   sm.erase( sm.begin() + (value - minSeed) );
   for ( int checker = minSeed; checker < maxSeed; ++checker )
   {
      CppTL::ConstString msg = "Checking key: " + CppTL::toString( checker );
      int expected = (checker == value) ? 0 : 1;
      result.compose( CppUT::checkEquals( expected, sm.count(checker), msg.c_str() ) );
   }
   result.compose( CppUT::checkEquals( maxSeed-minSeed - 1, sm.size() ) );
   return result;
}


void 
SmallMapTest::testDefaultConstructor()
{
   SmallMapIntInt sm;
   CPPUT_CHECK_EXPR( sm.empty() );
   CPPUT_CHECK_COMPARE( 0, ==, sm.size() );
   CPPUT_CHECK_COMPARE( 0, ==, sm.count(1234) );
}


void 
SmallMapTest::testOneInsert()
{
   SmallMapIntInt sm;
   CppTL::Pair<SmallMapIntInt::iterator,bool> result = sm.insert( IntInt( 3,33 ) );
   CPPUT_CHECK_EXPR( result.second );
   CPPUT_CHECK_EXPR( result.first == sm.begin() );
   CPPUT_CHECK_EXPR_FALSE( sm.empty() );
   CPPUT_CHECK_COMPARE( 1, ==, sm.size() );
   CPPUT_CHECK_COMPARE( 1, ==, sm.count(3) );
   CPPUT_ASSERT_EXPR( sm.begin() == sm.find(3) );
   CPPUT_CHECK_COMPARE( 33, ==, sm.find(3)->second );
}


CppUT::CheckerResult
SmallMapTest::checkKeyExists( SmallMapIntInt &sm, int key )
{
   CppUT::CheckerResult result;
   
   result.predicate("key") = key;
   result.compose( CppUT::checkEquals( 1, sm.count(key), "key not counted" ) );
   result.compose( CppUT::checkTrue( sm.find(key) != sm.end(), "key not found" ) );
   result.compose( CppUT::checkEquals( key, sm.find(key)->first, "key not stored" ) );
   // unlike std::map, operator[] preserve existing value.
   result.compose( CppUT::checkEquals( key*11, sm[key], "[] look-up" ) );
   return result;
}


void 
SmallMapTest::testOrderedInserts()
{
   SmallMapIntInt sm;
   // Checks multiple inserts in order
   const int factor = 11;
   for ( int index1 =0; index1 < CPPTL_ARRAY_SIZE(keyset1); ++index1 )
   {
      CppTL::ConstString msg = "Checking key: " + CppTL::toString( keyset1[index1] );
      CppTL::Pair<SmallMapIntInt::iterator,bool> result = 
         sm.insert( IntInt( keyset1[index1], keyset1[index1] * factor ) );
      CPPUT_CHECK_EXPR( result.second );
      CPPUT_CHECK( result.first == sm.begin() + index1, msg );
   }
   CPPUT_CHECK_EXPR_FALSE( sm.empty() );
   CPPUT_CHECK_COMPARE( CPPTL_ARRAY_SIZE(keyset1), ==, sm.size() );
   // Checks look-up of inserted values, and the 'ordered' property of the map
   for ( int index2 =0; index2 < CPPTL_ARRAY_SIZE(keyset1); ++index2 )
   {
      CPPUT_CHECK_PREDICATE( checkKeyExists( sm, keyset1[index2] ) );
      // Checks that insertion order is preserved (ordered insert)
      CPPUT_CHECK_COMPARE( keyset1[index2], ==, sm.begin()[index2].first );
   }
}


void 
SmallMapTest::testUnorderedInserts()
{
   SmallMapIntInt sm;
   // Checks multiple inserts in order
   const int factor = 11;
   for ( int index1 =0; index1 < CPPTL_ARRAY_SIZE(keyset2); ++index1 )
      sm.insert( IntInt( keyset2[index1], keyset2[index1] * factor ) );
   CPPUT_CHECK_EXPR_FALSE( sm.empty() );
   CPPUT_ASSERT_EQUAL( CPPTL_ARRAY_SIZE(keyset2), sm.size() );
   // Checks look-up of inserted values, and the 'ordered' property of the map
   for ( int index2 =0; index2 < CPPTL_ARRAY_SIZE(keyset2); ++index2 )
   {
      CPPUT_CHECK_PREDICATE( checkKeyExists( sm, keyset2[index2] ) );
   }
   // Checks that the map is ordered
   SmallMapIntInt::iterator it = sm.begin();
   SmallMapIntInt::iterator itEnd = sm.end() - 1;
   for ( ; it != itEnd; ++it )
      CPPUT_CHECK( it->first < it[1].first );
}


void 
SmallMapTest::testDuplicateInserts()
{
   SmallMapIntInt sm;
   CppTL::Pair<SmallMapIntInt::iterator,bool> result;
   result = sm.insert( IntInt(1,11) );
   CPPUT_ASSERT( result.second == true );
   CPPUT_ASSERT( result.first == sm.begin() );

   result = sm.insert( IntInt(3,33) );
   CPPUT_ASSERT( result.second == true );
   CPPUT_ASSERT( result.first == sm.begin()+1 );

   result = sm.insert( IntInt(2,22) );
   CPPUT_ASSERT( result.second == true );
   CPPUT_ASSERT( result.first == sm.begin()+1 );

   result = sm.insert( IntInt(1,777) );
   CPPUT_CHECK( result.second == false );
   CPPUT_CHECK( result.first == sm.begin() );
   CPPUT_CHECK_COMPARE( 1, ==, result.first->first );
   CPPUT_CHECK_COMPARE( 11, ==, result.first->second );

   result = sm.insert( IntInt(2,777) );
   CPPUT_CHECK( result.second == false );
   CPPUT_CHECK( result.first == sm.begin()+1 );
   CPPUT_CHECK_COMPARE( 2, ==, result.first->first );
   CPPUT_CHECK_COMPARE( 22, ==, result.first->second );

   result = sm.insert( IntInt(3,777) );
   CPPUT_CHECK( result.second == false );
   CPPUT_CHECK( result.first == sm.begin()+2 );
   CPPUT_CHECK_COMPARE( 3, ==, result.first->first );
   CPPUT_CHECK_COMPARE( 33, ==, result.first->second );
}


void 
SmallMapTest::testInsertWithHint()
{
   SmallMapIntInt smBegin;
   SmallMapIntInt smMid;
   SmallMapIntInt smLast;
   SmallMapIntInt smEnd;
   // Checks multiple inserts in order
   const int factor = 11;
   for ( int index1 =0; index1 < CPPTL_ARRAY_SIZE(keyset1); ++index1 )
   {
      CppTL::ConstString msg = "Checking key: " + CppTL::toString( keyset1[index1] );
      IntInt item( keyset1[index1], keyset1[index1] * factor );

      SmallMapIntInt::iterator itBegin = smBegin.begin();
      SmallMapIntInt::iterator resultItBegin = smBegin.insert( itBegin, item );
      CPPUT_CHECK( resultItBegin == smBegin.begin() + index1, msg );
      CPPUT_ASSERT_PREDICATE( CppUT::checkEquals(item.first, resultItBegin->first, msg.c_str() ) );

      SmallMapIntInt::iterator itMid = smMid.begin() + (index1 / 2);
      SmallMapIntInt::iterator resultItMid = smMid.insert( itMid, item );

      SmallMapIntInt::iterator itLast = smLast.begin() + CPPTL_MAX(0, index1-1);
      SmallMapIntInt::iterator resultItLast = smLast.insert( itLast, item );

      SmallMapIntInt::iterator itEnd = smEnd.end();
      SmallMapIntInt::iterator resultItEnd = smEnd.insert( itEnd, item );
   }
   CPPUT_ASSERT_EQUAL( CPPTL_ARRAY_SIZE(keyset1), smBegin.size() );
   CPPUT_ASSERT_EQUAL( CPPTL_ARRAY_SIZE(keyset1), smMid.size() );
   CPPUT_ASSERT_EQUAL( CPPTL_ARRAY_SIZE(keyset1), smLast.size() );
   CPPUT_ASSERT_EQUAL( CPPTL_ARRAY_SIZE(keyset1), smEnd.size() );

   // Checks look-up of inserted values, and the 'ordered' property of the map
   for ( int index2 =0; index2 < CPPTL_ARRAY_SIZE(keyset1); ++index2 )
   {
      CppTL::ConstString msg = "Checking key: " + CppTL::toString( keyset1[index2] );
      CPPUT_CHECK_COMPARE( keyset1[index2], ==, smBegin.begin()[index2].first );
      CPPUT_CHECK_COMPARE( keyset1[index2], ==, smMid.begin()[index2].first );
      CPPUT_CHECK_COMPARE( keyset1[index2], ==, smLast.begin()[index2].first );
      CPPUT_CHECK_COMPARE( keyset1[index2], ==, smEnd.begin()[index2].first );
   }
}


void 
SmallMapTest::testErase()
{
   SmallMapIntInt sm;
   // Checks multiple inserts in order
   const int factor = 11;
   for ( int index1 =0; index1 < CPPTL_ARRAY_SIZE(keyset2); ++index1 )
      sm.insert( IntInt( keyset2[index1], keyset2[index1] * factor ) );
   CPPUT_CHECK_EXPR_FALSE( sm.empty() );
   CPPUT_ASSERT_EQUAL( CPPTL_ARRAY_SIZE(keyset2), sm.size() );

   // erase some items [single: begin, last, middle]; range[middle, middle]
   sm.erase( 97 );
   sm.erase( 0 );
   CPPUT_ASSERT( sm.find(16) != sm.end() );
   sm.erase( sm.find(16) );
   sm.erase( sm.find(5), sm.find(5) + 1 );
   sm.erase( sm.begin() + 1, sm.begin() + 3 );

   // Checks look-up of inserted values, and the 'ordered' property of the map
   for ( int index2 =0; index2 < CPPTL_ARRAY_SIZE(keyset3); ++index2 )
   {
      CppTL::ConstString msg = "Checking key: " + CppTL::toString( keyset3[index2] );
      CPPUT_CHECK_PREDICATE( checkKeyExists( sm, keyset3[index2] ) );
   }
   // Checks that the map is ordered
   SmallMapIntInt::iterator it = sm.begin();
   SmallMapIntInt::iterator itEnd = sm.end() - 1;
   for ( ; it != itEnd; ++it )
      CPPUT_CHECK( it->first < it[1].first );

   // Most case involving range erase
   LOCAL_CHECK_ERASE_RANGE( 1,10, 1, 1 ); // empty range at beginning
   LOCAL_CHECK_ERASE_RANGE( 1,10, 5, 5 ); // empty range in the middle
   LOCAL_CHECK_ERASE_RANGE( 1,10, 10, 10 ); // empty range at end
   LOCAL_CHECK_ERASE_ITERATOR( 1, 7, 1 ); // erase first item using iterator
   LOCAL_CHECK_ERASE_ITERATOR( 1, 7, 3 ); // erase middle item using iterator
   LOCAL_CHECK_ERASE_ITERATOR( 1, 7, 6 ); // erase last item using iterator
}

