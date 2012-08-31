#include <cpput/testrunner.h>
#include <cpput/message.h>
#include <cpput/testing.h>
#include <cpput/testinfo.h>
#include <cpput/testsuite.h>
#include <cpput/testvisitor.h>
#include <cpptl/scopedptr.h>
#include <opentest/properties.h>
#include <opentest/testplan.h>


namespace CppUT {


// Class TestRunner::TestDeclaratorVisitor
// //////////////////////////////////////////////////////////////////

class TestRunner::TestDeclaratorVisitor : public TestVisitor
{
public:
   TestDeclaratorVisitor( OpenTest::TestDeclarator &declarator,
                          TestRunner::DeclaredTests &tests )
      : declarator_( declarator )
      , tests_( tests )
   {
   }

public: // overridden from TestVisitor
   void visitTestCase( Test &test )
   {
      OpenTest::TestId id = declarator_.addTest( test.name(), 
                                                 test.info() );
      tests_.insert( TestRunner::DeclaredTests::value_type( id, &test ) );
   }

   void visitTestSuite( AbstractTestSuite &suite )
   {
      OpenTest::TestId id = declarator_.beginSuite( suite.name(), 
                                                    suite.info() );
      tests_.insert( TestRunner::DeclaredTests::value_type( id, &suite ) );
      for ( int index = 0; index < suite.testCount(); ++index )
         suite.testAt( index )->accept( *this );
      declarator_.endSuite();
   }

private:
   OpenTest::TestDeclarator &declarator_;
   TestRunner::DeclaredTests &tests_;
};


class TestRunner::TestResultUpdaterImpl : public TestResultUpdater
{
public:
   TestResultUpdaterImpl( OpenTest::TestRunTracker &tracker )
      : tracker_( tracker )
      , testEntry_( 0 )
   {
   }

   void setTestPlanEntry( const OpenTest::TestPlanEntry &entry )
   {
      testEntry_ = &entry;
   }

   // overridden from TestResultUpdater
   void mergeInResult( const OpenTest::Properties &result )
   {
      tracker_.mergeInTestResult( *testEntry_, result );
   }

   void mergeInResult( const OpenTest::PropertyPath &path,
                       const OpenTest::Value &value )
   {
      tracker_.mergeInTestResult( *testEntry_, path, value );
   }

   void appendToResult( const OpenTest::PropertyPath &path,
                        const OpenTest::Value &value )
   {
      tracker_.appendToTestResult( *testEntry_, path, value );
   }

private:
   OpenTest::TestRunTracker &tracker_;
   const OpenTest::TestPlanEntry *testEntry_;
};
   

// Class TestRunner
// //////////////////////////////////////////////////////////////////


TestRunner::TestRunner()
   : suite_( new TestSuite( "All Tests" ) )
{
}


void 
TestRunner::setRootSuite( const AbstractTestSuitePtr &suite )
{
   suite_ = suite;
}


AbstractTestSuitePtr 
TestRunner::rootSuite() const
{
   return suite_;
}


void 
TestRunner::declareTests( OpenTest::TestDeclarator &declarator )
{
   TestDeclaratorVisitor visitor( declarator, tests_ );
   suite_->accept( visitor );
}


void 
TestRunner::runTests( const OpenTest::TestPlan &testPlan,
                      OpenTest::TestRunTracker &tracker,
                      const OpenTest::Properties &configuration )
{
   tracker.startTestRun();
   CppTL::ScopedPtr<TestResultUpdaterImpl> resultUpdater( 
      new TestResultUpdaterImpl( tracker ) );
   TestInfo::setTestResultUpdater( *resultUpdater );

   OpenTest::TestPlanEntryPtrEnum enumEntries = testPlan.entries();
   while ( enumEntries.hasNext() )
   {
      const OpenTest::TestPlanEntry &entry = *enumEntries.next();
      resultUpdater->setTestPlanEntry( entry );
      OpenTest::TestId id = entry.test();
      DeclaredTests::const_iterator itTest = tests_.find( id );
      tracker.startTest( entry );
      TestInfo::mergeInResult( configuration );
      TestInfo::mergeInResult( entry.configuration() );
      if ( itTest == tests_.end() ) // protocol error => unknown test
      {
         OpenTest::Properties openResult;
         openResult["result"]["status"]["success"] = false;
         openResult["result"]["status"]["type"] = "skip";
         tracker.mergeInTestResult( entry, openResult );
      }
      else
      {
         Test &test = *( itTest->second );
         if ( !test.isTestCase() )
         {
            OpenTest::Properties openResult;
            openResult["result"]["status"]["success"] = false;
            openResult["result"]["status"]["type"] = "skip";
            openResult["result"]["status"]["message"] = "Can not run test suite.";
            tracker.mergeInTestResult( entry, openResult );
         }
         else
         {
            TestCase &testCase = static_cast<TestCase &>( test );
            testCase.runTest();
         }
      }
      sendTestStatus( entry, tracker );
      tracker.endTest( entry );
   }

   tracker.endTestRun();
}


void 
TestRunner::sendTestStatus( const OpenTest::TestPlanEntry &entry,
                            OpenTest::TestRunTracker &tracker )
{
   TestInfo::updateTestStatistics();
   OpenTest::Properties result;
   bool success = !TestInfo::testHasFailed();
   result["result/status/success"] = success;
   if ( TestInfo::testHasFailed() )
      result["result/status/type"] = TestInfo::faultCount() ? "fault" : "assertion";
   tracker.mergeInTestResult( entry, result );
}


} // namespace CppUT

