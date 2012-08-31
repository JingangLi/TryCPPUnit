#include "testing.h"
#include <cpput/testing.h>
#include <stdio.h>
//#include "assertenumtest.h"
//#include "assertstringtest.h"
//#include "enumeratortest.h"
//#include "reflectiontest.h"
//#include "registrytest.h"
//#include "testfixturetest.h"
//#include "smallmaptest.h"
//#include "valuetest.h"
//#include "formattest.h"

#include <cpput/lighttestrunner.h>



bool testBasicAssertions();
bool testExceptionGuard();
//bool testTestRunResult();
bool testTestSuite();
bool testTestCase();
bool testFunctors();
bool testTestInfo();
bool testRegistry();


static CppUT::SuiteMeta cpputSuite = CPPUT_CURRENT_SUITE();


static bool bootStrapTest()
{
   return testBasicAssertions()  &&
          testFunctors()  &&
          testTestInfo()  &&
          testExceptionGuard()  &&
//          testTestContext() &&
//          testTestSuite() &&
          testTestCase()  &&
          testRegistry();
}


static void displayConfiguration()
{
   CppTL::StringBuffer config;
// Thread API
#if CPPTL_HAS_THREAD
   config += "Thread: Yes, API = ";
# if CPPTL_USE_WIN32_THREAD
   config += "win32";
# elif CPPTL_USE_PTHREAD_THREAD
   config += "pthread";
# else
   config += "???";
# endif
   config += "\n";
#else
   config += "Thread: No\n";
#endif
// Atomic counter implementation
   config += "Atomic counter API: ";
#if CPPTL_HAS_THREAD_SAFE_ATOMIC_COUNTER
# if CPPTL_USE_LINUX_ATOMIC
   config += "linux atomic operations (limited to 24 bits)";
# elif CPPTL_USE_WIN32_ATOMIC
   config += "win32";
# elif CPPTL_USE_PTHREAD_ATOMIC
   config += "pthread mutex";
# else
   config += "???";
# endif
#else
   config += "none (not thread-safe)";
#endif
   config += "\n";
// Int64
   config += "Int64: ";
#ifndef CPPTL_NO_INT64
   config += "Yes\n";
#else
   config += "No\n";
#endif
// RTTI
   config += "RTTI: ";
#ifndef CPPTL_NO_RTTI
   config += "Yes\n";
#else
   config += "No\n";
#endif

   printf( "=> CPPUT Configuration:\n%s\n", config.c_str() );
}


int main( int argc, const char *argv[] )
{
   displayConfiguration();

   bool bootStrapSuccess = bootStrapTest();

   if ( !bootStrapSuccess )
   {
      printf( "bootstrap tests failed...\n" );
      return 1;
   }

   printf( "All bootstrap tests passed successfuly...\n" );
   printf( "Suite registry content:\n%s\nTest runner output:\n", 
           CppUT::Registry::dump().c_str() );

/* // using opentest
   CppUT::TestRunner runner;
   CppUT::AbstractTestSuitePtr rootSuite = 
      CppTL::staticPointerCast<CppUT::AbstractTestSuite>( allSuite );
   runner.setRootSuite( rootSuite );

   OpenTest::TextTestDriver driver( runner );
   bool sucessful = driver.run();
*/

   CppUT::LightTestRunner runner;
   runner.addSuite( cpputSuite );
   bool sucessful = runner.runTests();
   return sucessful ? 0 : 1;
}
