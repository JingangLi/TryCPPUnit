#ifndef CPPUT_FORMATTEST_H_INCLUDED
# define CPPUT_FORMATTEST_H_INCLUDED

# include <cpput/testfixture.h>

class FormatTest : public CppUT::TestFixture
{
   CPPUT_TESTSUITE_BEGIN( FormatTest );
   CPPUT_TEST( testLengthNoFormat );
   CPPUT_TEST( testLengthIntFormat );
   CPPUT_TESTSUITE_END();

public:
   void setUp();
   void tearDown();

   void testLengthNoFormat();
   void testLengthIntFormat();

private:
};


#endif  // CPPUT_FORMATTEST_H_INCLUDED

