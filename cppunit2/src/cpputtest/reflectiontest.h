#ifndef CPPUT_REFLECTIONTEST_H
# define CPPUT_REFLECTIONTEST_H

# include <cpput/assertcommon.h>
# include <cpput/testing.h>

class ReflectionTest : public CppUT::TestCase
{
   CPPUT_TESTSUITE_BEGIN( ReflectionTest )
   CPPUT_TESTSUITE_TEST( testBasicReflection );
   CPPUT_TESTSUITE_END()

public:
   ReflectionTest();

   virtual ~ReflectionTest();

   void setUp();
   void tearDown();

   void testBasicReflection();
private:
};


#endif  // CPPUT_REFLECTIONTEST_H

