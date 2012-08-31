#ifndef CPPUT_ENUMERATORTEST_H
# define CPPUT_ENUMERATORTEST_H

# include <cpput/assertcommon.h>
# include <cpput/testing.h>
# include <cpptl/enumerator.h>
# include <deque>
# include <list>
# include <map>
# include <set>
# include <vector>

class EnumeratorTest : public CppUT::TestCase
{
   CPPUT_TESTSUITE_BEGIN( EnumeratorTest )
   CPPUT_TESTSUITE_TEST( testEmptyEnumerator );
   CPPUT_TESTSUITE_TEST( testEmptyAnyEnumerator );
   CPPUT_TESTSUITE_TEST( testForwardItEnumerator );
   CPPUT_TESTSUITE_TEST( testTransformForwardItEnumerator );
   CPPUT_TESTSUITE_TEST( testContainerEnumerator );
   CPPUT_TESTSUITE_TEST( testMapKeysEnumerator );
   CPPUT_TESTSUITE_TEST( testMapValuesEnumerator );
   CPPUT_TESTSUITE_TEST( testTransformEnumerator );
   CPPUT_TESTSUITE_TEST( testFilterEnumerator );
   CPPUT_TESTSUITE_TEST( testSliceEnumerator );
   CPPUT_TESTSUITE_TEST( testSugarEnumerator );
   CPPUT_TESTSUITE_END()

public:
   EnumeratorTest();

   virtual ~EnumeratorTest();

   void setUp();
   void tearDown();

   void testEmptyEnumerator();
   void testEmptyAnyEnumerator();
   void testForwardItEnumerator();
   void testTransformForwardItEnumerator();
   void testContainerEnumerator();
   void testMapKeysEnumerator();
   void testMapValuesEnumerator();
   void testTransformEnumerator();
   void testFilterEnumerator();
   void testSliceEnumerator();
   void testSugarEnumerator();


/*
   void testStlEnumerator();
   void testStlRangeEnumerator();

   void testStlEnumConstructAdapator();

   void testStlMapKeysEnumerator();
   void testStlMapValuesEnumerator();
   void testSliceEnumerator();
*/
private:
   typedef CppTL::AnyEnumerator<int> IntEnum;
   typedef CppTL::AnyEnumerator<char> CharEnum;

   typedef std::deque<int> IntDeque;
   IntDeque deque1_;

   typedef std::vector<int> IntVector;
   IntVector vector1_;

   typedef std::set<int> IntSet;
   IntSet set1_;

   typedef std::list<int> IntList;
   IntList list1_;

   typedef std::map<int,std::string> IntStringMap;
   IntStringMap map1_;

   typedef CppTL::AnyEnumerator<IntDeque> IntDequeEnum;
   typedef std::map<std::string,int> StringIntMap;
   typedef CppTL::AnyEnumerator<std::string> StringEnum;
   typedef CppTL::AnyEnumerator<std::string> StringIntMapFirstEnum;
   typedef CppTL::AnyEnumerator<int> StringIntMapSecondEnum;
};


#endif  // CPPUT_ENUMERATORTEST_H

