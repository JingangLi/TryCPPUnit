#ifndef CPPUT_VALUETEST_H
# define CPPUT_VALUETEST_H

# include <cpput/assertcommon.h>
# include <cpput/testing.h>
# include <cpptl/value.h>

struct ValueTest : public CppUT::TestCase
{
   ValueTest();

   const int maxInt_;
   const int minInt_;
};


#endif  // CPPUT_VALUETEST_H

