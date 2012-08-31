#ifndef CPPUT_ASSERTENUMTEST_H_INCLUDED
# define CPPUT_ASSERTENUMTEST_H_INCLUDED

# include <cpput/assertenum.h>
# include <cpput/testing.h>
# include <cpptl/enumerator.h>
# include <deque>
# include <map>
# include <set>
# include <vector>

class AssertEnumTest : public CppUT::TestCase
{
   CPPUT_TESTSUITE_BEGIN( AssertEnumTest )
   CPPUT_TESTSUITE_TEST( testAssertSequenceEqual );
   CPPUT_TESTSUITE_TEST( testAssertSetEqual );
   CPPUT_TESTSUITE_END()

public:
   void setUp();
   void tearDown();

   void testAssertSequenceEqual();
   void testAssertSetEqual();

private:
   typedef CppTL::AnyEnumerator<int> IntEnum;
   typedef CppTL::AnyEnumerator<char> CharEnum;

   std::vector<int> vEmpty_;
   std::vector<int> v122_;
   std::vector<int> v123_;
   std::vector<int> v321_;
   std::vector<int> v1234_;
   std::vector<int> v4321_;
};


#endif  // CPPUT_ASSERTENUMTEST_H_INCLUDED

