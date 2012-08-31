#ifndef CPPUTTEST_MOCKLISTENER_H_INCLUDED
#define CPPUTTEST_MOCKLISTENER_H_INCLUDED

# include <cpput/testlistener.h>


class MockTestListener : public CppUT::TestListener
{
public:
   MockTestListener()
      : testFailedCount_( 0 )
      , testSucceededCount_( 0 )
      , testResultCount_( 0 )
      , enterTestCount_( 0 )
      , exitTestCount_( 0 )
      , enterSuiteCount_( 0 )
      , exitSuiteCount_( 0 )
   {
   }

// overridden from CppUT::TestListener

   void processTestResult( CppUT::TestResult &result,
                           CppUT::TestContext &context )
   {
      ++testResultCount_;
      if ( result.setFailed() )
         ++testFailedCount_;
      else
         ++testSucceededCount_;
   }

   void enterTest( CppUT::Test &test, CppUT::TestContext &context )
   {
      ++enterTestCount_;
   }

   void exitTest( CppUT::Test &test, CppUT::TestContext &context )
   {
      ++exitTestCount_;
   }

   void enterSuite( CppUT::AbstractTestSuite &suite, 
                    CppUT::TestContext &context )
   {
      ++enterSuiteCount_;
   }

   void exitSuite( CppUT::AbstractTestSuite &suite, 
                   CppUT::TestContext &context )
   {
      ++exitSuiteCount_;
   }

   int testFailedCount_;
   int testSucceededCount_;
   int testResultCount_;
   int enterTestCount_;
   int exitTestCount_;
   int enterSuiteCount_;
   int exitSuiteCount_;
};


#endif // CPPUTTEST_MOCKLISTENER_H_INCLUDED
