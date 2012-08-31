#ifndef CPPTLTEST_SMALLMAPTEST_H_INCLUDED
# define CPPTLTEST_SMALLMAPTEST_H_INCLUDED

# include <cpptl/smallmap.h>
# include <cpput/testing.h>

class SmallMapTest : public CppUT::TestCase
{
   CPPUT_TESTSUITE_BEGIN( SmallMapTest )
   CPPUT_TESTSUITE_TEST( testDefaultConstructor );
   CPPUT_TESTSUITE_TEST( testOneInsert );
   CPPUT_TESTSUITE_TEST( testOrderedInserts );
   CPPUT_TESTSUITE_TEST( testUnorderedInserts );
   CPPUT_TESTSUITE_TEST( testDuplicateInserts );
   CPPUT_TESTSUITE_TEST( testErase );
   CPPUT_TESTSUITE_TEST( testInsertWithHint );
   CPPUT_TESTSUITE_END()

public:
   typedef CppTL::SmallMap<int,int> SmallMapIntInt;
   typedef SmallMapIntInt::value_type IntInt;

   static int keyset1[10];
   static int keyset2[10];
   static int keyset3[4];

   void testDefaultConstructor();
   void testOneInsert();
   void testOrderedInserts();
   void testUnorderedInserts();
   void testDuplicateInserts();
   void testInsertWithHint();
   void testErase();

private:
   CppUT::CheckerResult checkEraseRange( int minSeed, int maxSeed, int beginRange, int endRange);
   CppUT::CheckerResult checkEraseIterator( int minSeed, int maxSeed, int value );
   CppUT::CheckerResult checkKeyExists( SmallMapIntInt &sm, int key );
};


#endif // CPPTLTEST_SMALLMAPTEST_H_INCLUDED
