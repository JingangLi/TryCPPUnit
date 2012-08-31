#include <cpput/testinfo.h>
#include <cpptl/stringtools.h>
#include <cpptl/thread.h>

namespace CppUT {


// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// Class AssertionTrigger
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
namespace Impl {
   AssertionTrigger AssertionTrigger::aborting;
   AssertionTrigger AssertionTrigger::checking;

   AssertionTrigger &
   AssertionTrigger::operator +=( const CheckerFileLocation &fileLocation )
   {
      const char *file = fileLocation.file_;
      unsigned int line = fileLocation.lineData_->line_;
      const CheckerResult &result = *(fileLocation.lineData_->result_);
      TestInfo::threadInstance().handleAssertion( file, line, result, this == &aborting );
      return *this;
   }
} // end namespace Impl

// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// Class Assertion
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////

Assertion::Assertion( Kind kind,
                      const SourceLocation &sourceLocation )
   : kind_( kind )
   , location_( sourceLocation )
   , isIgnoredFailure_( false )
{
}


void 
Assertion::setLocation( const SourceLocation &location )
{
   location_ = location;
}


const SourceLocation &
Assertion::location() const
{
   return location_;
}


void 
Assertion::setKind( Kind kind )
{
   kind_ = kind;
}


Assertion::Kind
Assertion::kind() const
{
   return kind_;
}


void 
Assertion::setIgnoredFailure()
{
   isIgnoredFailure_ = true;
}

bool 
Assertion::isIgnoredFailure() const
{
   return isIgnoredFailure_;
}


void 
Assertion::setDetail( const CheckerResult &detail )
{
   detail_ = detail.result_;
}


const Json::Value &
Assertion::detail() const
{
   return detail_;
}

Json::Value &
Assertion::detail()
{
   return detail_;
}


std::string
Assertion::toString() const
{
   CppTL::StringBuffer buffer;
   if ( location().isValid() )
   {
      buffer += location().file_;
      buffer += "(" + CppTL::toString( location().line_ ) + ") : ";
   }
   else
      buffer += "unknwon failure location : ";

   CppTL::ConstString failureType = kind() == fault ? "fault" 
                                                    : "assertion";
   buffer += "[failure type: " + failureType + "]\n";
   buffer += detail_.toStyledString();
   return buffer.c_str();
}



// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// Class TestStatus
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
TestStatus::TestStatus( Status status )
   : status_( status )
   , assertionCount_( 0 )
   , failedAssertionCount_( 0 )
   , ignoredFailureCount_( 0 )
{
}

void 
TestStatus::setStatus( Status status )
{
   status_ = status;
}

TestStatus::Status 
TestStatus::status() const
{
   return status_;
}

bool 
TestStatus::hasFailed() const
{
   return status_ == failed;
}

bool 
TestStatus::hasPassed() const
{
   return status_ == passed;
}

bool 
TestStatus::wasSkipped() const
{
   return status_ == skipped;
}

void 
TestStatus::setStatistics( const std::string &name,
                           const Json::Value &value )
{
   statistics_[name] = value;
}


//Json::Value 
//TestStatus::getStatistics( const CppTL::ConstString &name )
//{
//}

void 
TestStatus::addSpecific( const std::string &type,
                         const Json::Value &value )
{
   specifics_[type] = value;
}

int 
TestStatus::assertionCount() const
{
   return assertionCount_;
}

int 
TestStatus::failedAssertionCount() const
{
   return failedAssertionCount_;
}

int 
TestStatus::ignoredFailureCount() const
{
   return ignoredFailureCount_;
}

void 
TestStatus::increaseAssertionCount( int delta )
{
   assertionCount_ += delta;
}

void 
TestStatus::increaseFailedAssertionCount( int delta )
{
   failedAssertionCount_ += delta;
   if ( failedAssertionCount_ > 0  &&  status_ != failed )
      status_ = failed;
}

void 
TestStatus::increaseIgnoredFailureCount( int delta )
{
   ignoredFailureCount_ += delta;
}

// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// Class TestInfo
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////

static CppTL::ThreadLocalStorage<TestInfoPtr> perThreadStaticData;

TestInfo &
TestInfo::threadInstance()
{
   TestInfoPtr &data = perThreadStaticData.get();
   if ( !data )
      data.reset( new TestInfo() );
   return *data;
}

   
TestInfo::TestInfo()
   : assertionType_( abortingAssertion )
   , updater_( 0 )
{
}


void 
TestInfo::setTestResultUpdater( TestResultUpdater &updater )
{
   updater_ = &updater;
}


void 
TestInfo::removeTestResultUpdater()
{
   updater_ = 0;
}

void 
TestInfo::startNewTest()
{
   testStatus_ = TestStatus( TestStatus::passed );
   assertionType_ = abortingAssertion;
}


TestStatus &
TestInfo::testStatus()
{
   return testStatus_;
}


void 
TestInfo::handleAssertion( const char *file, 
                           unsigned int line, 
                           const CheckerResult &result,
                           bool isAbortingAssertion )
{
   switch ( result.status_ )
   {
   case TestStatus::passed:   // do nothing (assertion count increased)
      testStatus_.increaseAssertionCount();
      break;
   case TestStatus::failed:
      {
         testStatus_.increaseAssertionCount();
         testStatus_.increaseFailedAssertionCount();

         Assertion assertion( Assertion::assertion, SourceLocation( file, line ) );
         assertion.setDetail( result );
         if ( updater_ )
            updater_->addResultAssertion( assertion );

         if ( isAbortingAssertion )
         {
            std::string message;
            if ( abortingAssertionMode_ != fastAbortingAssertion )
               message = assertion.toString().c_str();
            throw AbortingAssertionException( message );
         }
      }
      break;
   case TestStatus::skipped:
      // @todo handle this correctly.
      break;
   default:
      CPPTL_DEBUG_ASSERT_UNREACHABLE;
   }
}


void 
TestInfo::handleUnexpectedException( const CheckerResult &detail )
{
   if ( updater_ )
   {
      Assertion assertion( Assertion::fault );
      assertion.setDetail( detail );
      updater_->addResultAssertion( assertion );
   }
   testStatus_.setStatus( TestStatus::failed );
}


void 
TestInfo::setAbortingAssertionMode( AbortingAssertionMode mode )
{
   abortingAssertionMode_ = mode;
}


void 
TestInfo::log( const Json::Value &log )
{
   if ( updater_ )
      updater_->addResultLog( log );
}


// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// Class ScopedContextOverride
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////

TestInfo::ScopedContextOverride::ScopedContextOverride()
   : context_( perThreadStaticData.get() )
{
   perThreadStaticData.get().reset( new TestInfo() );
}

TestInfo::ScopedContextOverride::~ScopedContextOverride()
{
   perThreadStaticData = context_;
}


// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// Class IgnoreFailureScopedContextOverride
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////

TestInfo::IgnoreFailureScopedContextOverride::IgnoreFailureScopedContextOverride( 
                                                   bool &assertionFailed )
   : assertionFailed_( assertionFailed )
   , assertion_( 0 )
{
   assertionFailed_ = false;
   TestInfo::threadInstance().setTestResultUpdater( *this );
}

TestInfo::IgnoreFailureScopedContextOverride::~IgnoreFailureScopedContextOverride()
{
   TestStatus &testStatus = TestInfo::threadInstance().testStatus();
   assertionFailed_ = !testStatus.hasPassed();
   if ( assertionFailed_ )
   {
      context_->testStatus().increaseIgnoredFailureCount( testStatus.failedAssertionCount() );
      if ( assertion_  &&  context_->updater_ )
      {
         perThreadStaticData = context_;     // restore context before calling callback
         assertion_->setIgnoredFailure();
         context_->updater_->addResultAssertion( *assertion_ );
      }
   }
   delete assertion_;
}

void 
TestInfo::IgnoreFailureScopedContextOverride::addResultLog( const Json::Value &log )
{
}


void 
TestInfo::IgnoreFailureScopedContextOverride::addResultAssertion( const Assertion &assertion )
{
   if ( !assertion_ )
      assertion_ = new Assertion( assertion );
}


// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// CheckerResult
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
CheckerResult::CheckerResult()
   : status_( TestStatus::passed )
{
}


CheckerResult::CheckerResult( bool succeeded )
   : status_( succeeded ? TestStatus::passed
                        : TestStatus::failed )
{
}


void 
CheckerResult::setFailed()
{
   status_ = TestStatus::failed;
}


void 
CheckerResult::appendMessages( const CppUT::Message &message )
{
   if ( !message.empty() )
   {
      Json::Value &messages = result_["message"];
      for ( int index = 0; index < message.count(); ++index )
      {
         messages.append( message.at( index ) );
      }
   }
}


void 
CheckerResult::appendMessage( const std::string &message )
{
   result_["message"].append( message );
}


void 
CheckerResult::compose( const std::string &name, 
                        const CheckerResult &other )
{
   // Only change current status if has the status "passed";
   switch ( status_ )
   {
   case TestStatus::passed:
      status_ = other.status_;
      break;
   case TestStatus::failed:
      break;
   case TestStatus::skipped:
      break;
   default:
      CPPTL_DEBUG_ASSERT_UNREACHABLE;
   }
   // Merge other data if it did not succeed
   if ( other.status_ != TestStatus::passed )
   {
      // Appends a new entry in "composite" list
      Json::Value entry;
      entry[name] = other.result_;
      result_["composite"].append( entry );
   }
}


void 
CheckerResult::setName( const std::string &name )
{
   result_["name"] = name;
}


Json::Value &
CheckerResult::predicate( const char *name )
{
   Json::Value &entry = result_["data"].append( Json::Value() );
   entry["type"] = "predicate";
   entry["name"] = name;
   return entry["value"];
}


Json::Value &
CheckerResult::diagnostic( const char *name )
{
   Json::Value &entry = result_["data"].append( Json::Value() );
   entry["type"] = "diagnostic";
   entry["name"] = name;
   return entry["value"];
}


CheckerResult 
CheckerResult::setPredicateExpression( const char *expression ) const
{
   CheckerResult newResult( *this );
   Json::Value &newData = newResult.result_["data"];
   Json::Value &entry = newData.prepend( Json::Value() );
   entry["type"] = "predicate";
   entry["name"] = "expression";
   entry["value"] = expression;
   return newResult;
}



// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// Short-hand functions in namespace CppUT
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////


void log( const Json::Value &log )
{
   TestInfo::threadInstance().log( log );
}

void log( const std::string &log )
{
   TestInfo::threadInstance().log( log );
}

void log( const char *log )
{
   TestInfo::threadInstance().log( log );
}

void log( const CppTL::ConstString &log )
{
   TestInfo::threadInstance().log( log.c_str() );
}



} // namespace CppUT
