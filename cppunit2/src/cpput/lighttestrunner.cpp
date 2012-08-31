#include <cpput/lighttestrunner.h>
#include <cpput/testing.h>
#include <cpptl/sharedptr.h>
#include <cpptl/stringtools.h>
#include <stdio.h>

namespace {
   class Reindenter
   {
   public:
      Reindenter( CppTL::StringBuffer &text )
         : text_( text )
         , pos_( text.length() )
      {
      }

      void apply( const std::string &indent )
      {
         CppTL::ConstString textToIndex = text_.substr( pos_ );
         if ( !textToIndex.empty() )
         {
            text_.truncate( pos_ );
            const char *it = textToIndex.c_str();
            const char *itEnd = textToIndex.end_c_str();
            const char *sliceBegin = it;
            for ( ++it; it <= itEnd; ++it )
            {
               if ( it[-1] == '\n' )
               {
                  text_ += indent;
                  text_ += CppTL::ConstCharView( sliceBegin, it );
                  sliceBegin = it;
               }
            }
         }
      }

   private:
      CppTL::StringBuffer &text_;
      CppTL::StringBuffer::size_type pos_;
   };
} // end anonymous namespace

namespace CppUT {

LightTestRunner::LightTestRunner()
   : testRun_( 0 )
   , testFailed_( 0 )
   , testSkipped_( 0 )
   , ignoredFailureCount_( 0 )
   , totalAssertionCount_( 0 )
{
}


LightTestRunner::~LightTestRunner()
{
}


void 
LightTestRunner::addSuite( const Suite &suite )
{
   suitesToRun_.push_back( suite );
}


bool 
LightTestRunner::runTests()
{
   TestInfo::threadInstance().setTestResultUpdater( *this );
   for ( SuitesToRun::iterator it = suitesToRun_.begin(); it != suitesToRun_.end(); ++it )
      runTestSuite( *it );
   if ( testFailed_ > 0 )
   {
      fprintf( stdout, "Failure report:\n%s", report_.c_str() );
      fflush( stdout );
      fprintf( stdout, "%d/%d tests passed, %d tests failed", 
              testRun_ - testFailed_,
              testRun_,
              testFailed_ );
   }
   else
   {
      fprintf( stdout, "All %d tests passed", testRun_ );
      fflush( stdout );
   }

   if ( ignoredFailureCount_ > 0 )
      fprintf( stdout, ", %d ignored failures", ignoredFailureCount_ );
   fprintf( stdout, " (%d %s).\n", 
            totalAssertionCount_, 
            totalAssertionCount_ > 1 ? "assertions" : "assertion" );
   fflush( stdout );

   return testFailed_ == 0;
}


void 
LightTestRunner::runTestSuite( const Suite &suite )
{
   if ( !suite.isValid() )
   {
      return;
   }

   testPath_.push_back( suite.name() );
   for ( int suiteIndex =0; suiteIndex < suite.nestedSuiteCount(); ++suiteIndex )
   {
      runTestSuite( suite.nestedSuiteAt( suiteIndex ) );
   }
   for ( int index =0; index < suite.testCaseCount(); ++index )
   {
      const TestMeta *test = suite.testCaseAt( index );
      if ( test != 0 )
      {
         testPath_.push_back( test->name() );
         runTestCase( *test );
         testPath_.pop_back();
      }
   }
   testPath_.pop_back();
}


void 
LightTestRunner::runTestCase( const TestMeta &testCase )
{
   fprintf( stdout, "Testing %s : ", getTestPath().c_str() );
   fflush( stdout );
   assertions_.clear();
   logs_.clear();
   results_.clear();
   ++testRun_;
   testCase.runTest();
   
   TestStatus &testStatus = TestInfo::threadInstance().testStatus();

   std::string status;
   switch ( testStatus.status() )
   {
   case TestStatus::passed: 
      status = "OK"; 
      break;
   case TestStatus::skipped: 
      status = "SKIP"; 
      ++testSkipped_;
      break;
   case TestStatus::failed: 
      status = "FAIL"; 
      ++testFailed_;
      break;
   default: status = "?"; break;
   }

   int assertionCount = testStatus.assertionCount();
   status += " (";
   unsigned int failedAssertionCount = testStatus.failedAssertionCount();
   if ( failedAssertionCount > 0 )
   {
      std::string count = CppTL::toString( failedAssertionCount ).c_str();
      status += count + (failedAssertionCount > 1 ? " assertions failed/" 
                                                    : " assertion failed/");
   }

   status += CppTL::toString( assertionCount ).c_str();
   status += (assertionCount > 1 ? " assertions" : " assertion" );
   totalAssertionCount_ += assertionCount;

   if ( testStatus.ignoredFailureCount() > 0 )
   {
      std::string count = CppTL::toString( testStatus.ignoredFailureCount() ).c_str();
      status += ", " + count + " ignored failures";
   }
   status += ")";

   fprintf( stdout, "%s\n", status.c_str() );
   fflush( stdout );
   if ( !assertions_.empty() )
   {

      CppTL::ConstString resultType = assertions_.back().kind() == Assertion::fault ? "fault" 
                                                                                    : "assertion";
      report_ += "-> " + getTestPath() + " : " + resultType + "\n";
      //Assertions::const_iterator it = assertions_.begin();
      //for ( ; it != assertions_.end(); ++it )
      //   reportFailure( *it );
      ResultElements::const_iterator it = results_.begin();
      for ( ; it != results_.end(); ++it )
      {
         const ResultElement &result = *it;
         if ( result.isLog_ )
            reportLog( logs_[result.index_] );
         else
            reportFailure( assertions_[ result.index_ ] );
      }
      report_ += "\n";
   }
}

void 
LightTestRunner::addResultLog( const Json::Value &log )
{
   ResultElement element;
   element.isLog_ = true;
   element.index_ = int(logs_.size());
   results_.push_back( element );
   logs_.push_back( log );
}


void 
LightTestRunner::addResultAssertion( const Assertion &assertion )
{
   ResultElement element;
   element.isLog_ = false;
   element.index_ = int(assertions_.size());
   results_.push_back( element );
   assertions_.push_back( assertion );
}


CppTL::ConstString
LightTestRunner::getTestPath() const
{
   CppTL::StringBuffer buffer;
   for ( TestPath::const_iterator it = testPath_.begin(); it != testPath_.end(); ++it )
      buffer += "/" + *it;
   return buffer;
}


void 
LightTestRunner::reportFailure( const Assertion &failure )
{ // @todo duplicated in Assertion::toString().
   if ( failure.location().isValid() )
   {
      report_ += failure.location().file_;
      report_ += "(" + CppTL::toString( failure.location().line_ ) + ") : ";
   }
   else
      report_ += "unknwon failure location : ";

   CppTL::ConstString failureType = failure.kind() == Assertion::fault ? "fault" 
                                                                       : "assertion";
   if ( failure.isIgnoredFailure() )
   {
      failureType = "*ignored* " + failureType;
      ++ignoredFailureCount_;
   }
   report_ += "[failure type: " + failureType + "]\n";
   reportFailureDetail( failure.detail() );
   report_ += "\n";
}


void 
LightTestRunner::reportFailureDetail( const Json::Value &detail, 
                                      int nestingLevel )
{
   Reindenter reindenter( report_ );
   // Predicate name if any
   if ( !detail["name"].isNull() )
   {
      report_ += "predicate: " + detail["name"].asString() + "\n";
   }
   // Get assertion messages
   const Json::Value &messages = detail["message"];
   const int nbMessage = messages.size();
   if ( nbMessage > 0 )
   {
      report_ += nbMessage > 1 ? "Messages:\n"
                               : "Message: ";
      for ( int index = 0; index < nbMessage; ++index )
         report_ += messages[index].asString() + "\n";
   }
   // Compute max data name length
   const Json::Value &data = detail["data"];
   int nbDetailData = data.size();
   int maxNameLength = 0;
   for ( int indexLength = 0; indexLength < nbDetailData; ++indexLength )
   {
      const int length = data[indexLength]["name"].asString().size();
      maxNameLength = CPPTL_MAX( maxNameLength, length );
   }
   // Aligned result data values
   for ( int index = 0; index < nbDetailData; ++index )
   {
      std::string name = data[index]["name"].asString();
      report_ += name;
      report_ += std::string( maxNameLength - name.size(), ' ');
      report_ += ": ";
      // @todo nice conversion to string
      report_ += data[index]["value"].toStyledString();
      if ( report_[report_.length()-1] != '\n' )
         report_ += "\n";
   }
   // Reports composite assertions
   const Json::Value &composites = detail["composite"];
   const Json::Value::UInt nbComposite = composites.size();
   for ( Json::Value::UInt indexComposite = 0; indexComposite < nbComposite; ++indexComposite )
   {
      const Json::Value &composite = composites[indexComposite];
      CPPTL_ASSERT_MESSAGE( composite.size() == 1, "Composite must have a name" );
      const char *compositeName = composite.begin().memberName();
      const Json::Value &compositeDetail = *(composite.begin());
      report_ += "context: ";
      report_ += compositeName;
      report_ += "\n";
      reportFailureDetail( compositeDetail, nestingLevel + 1 );
   }
   if ( nestingLevel > 0 )
   {
      reindenter.apply( std::string( 2, ' ' ) );
   }
}


void 
LightTestRunner::reportLog( const Json::Value &log )
{
   report_ += "Log:\n";
   if ( log.isConvertibleTo( Json::stringValue ) )
      report_ += log.asString();
   else
      report_ += log.toStyledString();
   if ( report_[ report_.length() -1 ] != '\n' )
      report_ += "\n\n";
}



} // namespace CppUT
