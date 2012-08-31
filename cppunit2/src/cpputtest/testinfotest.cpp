#include <cpput/assertcommon.h>
#include <cpput/testing.h>
#include <cpput/assertenum.h>
#include <stdio.h>
#include <stdexcept>

namespace {
   // Capture the current TestStatus statistics
   struct AssertionCount
   {
      AssertionCount()
      {
         captureStatus();
      }

      void resetStatistics()
      {
         CppUT::TestInfo::threadInstance().startNewTest();
      }

      void captureStatus()
      {
         status_ = CppUT::TestInfo::threadInstance().testStatus();
      }

      CppUT::CheckerResult check( int assertion, 
                  int failed, 
                  int ignored, 
                  CppUT::TestStatus::Status status = CppUT::TestStatus::passed )
      {
         CppUT::CheckerResult result;
         result.compose( "assertion count", CppUT::checkEquals( assertion, status_.assertionCount() ) );
         result.compose( "failed assertion count", CppUT::checkEquals( failed, status_.failedAssertionCount() ) );
         result.compose( "ignored failure count", CppUT::checkEquals( ignored, status_.ignoredFailureCount() ) );
         result.compose( "status", CppUT::checkEquals( status, status_.status() ) );
         return result;
      }

      CppUT::TestStatus status_;
   };

   struct MockTestResultEvent
   {
      MockTestResultEvent( const std::string &type,
                           const std::string &detail )
         : type_( type )
         , detail_( detail )
      {
      }

      bool operator ==( const MockTestResultEvent &other ) const
      {
         return type_ == other.type_  &&
                ( detail_.find( other.detail_ ) != std::string::npos
                  ||  other.detail_.find( detail_ ) != std::string::npos );
      }

      bool operator !=( const MockTestResultEvent &other ) const
      {
         return !( *this == other );
      }

      std::string type_;
      std::string detail_;
   };

   class MockTestResultUpdater : public CppUT::TestResultUpdater
   {
   public:
   public: // overridden from CppUT::TestResultUpdater
      virtual void addResultLog( const Json::Value &log )
      {
         MockTestResultEvent event( "log_event", log.toStyledString() );
         actualEvents_.push_back( event );
      }

      virtual void addResultAssertion( const CppUT::Assertion &assertion )
      {
         MockTestResultEvent event( "assertion_event", assertion.toString() );
         if ( assertion.isIgnoredFailure() )
            event.detail_ += " [ignored_failure]";
         actualEvents_.push_back( event );
      }

      void expectLogEvent( const Json::Value &log )
      {
         MockTestResultEvent event( "log_event", log.toStyledString() );
         expectedEvents_.push_back( event );
      }

      void expectAssertionEvent( const std::string &detail, bool isIgnored = false )
      {
         MockTestResultEvent event( "assertion_event", detail );
         if ( isIgnored )
            event.detail_ += " [ignored_failure]";
         expectedEvents_.push_back( event );
      }

      void reset()
      {
         actualEvents_.clear();
         expectedEvents_.clear();
      }

      CppUT::CheckerResult verify()
      {
         CppUT::CheckerResult result;
         std::string expected;
         std::string actual;
         unsigned int minCount = CPPTL_MIN( expectedEvents_.size(), actualEvents_.size() );
         unsigned int index = 0;
         for ( ; index < minCount; ++index )
         {
            if ( expectedEvents_[index] != actualEvents_[index] )
               break;
         }

         if ( index < minCount  ||  expectedEvents_.size() != actualEvents_.size() )
         {
            result.setFailed();
            result.setName( "expected test event sequence == actual test event sequence " );
            result.appendMessage( "Expected and actual test event sequences did not match" );
            result.diagnostic("divergence index") = index;
            result.diagnostic("actual") = toString( actualEvents_ );
            result.predicate("expected") = toString( expectedEvents_ );
         }

         reset();
         return result;
      }

      typedef std::deque<MockTestResultEvent> TestEvents;

      std::string toString( const TestEvents &events ) const
      {
         std::string buffer;
         for ( unsigned int index =0; index < events.size(); ++index )
         {
            std::string indexStr = CppTL::toString(index).c_str();
            buffer += "[" + indexStr + "]='" + CppUT::stringize(events[index]) + "'\n";
         }
         return buffer;
      }

      TestEvents expectedEvents_;
      TestEvents actualEvents_;
   };
} // end anonymous namespace

namespace CppUT {
   /// We override this function so that CppUnit know how to convert
   /// MockTestResultEvent instance into a string.
   inline std::string toString( const MockTestResultEvent &v )
   {
      return "TestEvent( type: " + v.type_ + "; detail: " + v.detail_ + " )";
   }
} 



// Assumes a local variable named 'count of type AssertionCount is declared
#define TESTINFO_ASSERT_STATISTICS_ARE( assertion, failed, ignore )        \
   count.captureStatus(),           /* capture test statistics */          \
   CPPUT_BEGIN_ASSERTION_MACRO()    /* warning: change test statistics */  \
   count.check( assertion, failed, ignore );                               \
   CppUT::TestInfo::threadInstance().testStatus() = count.status_;

#define TESTINFO_ASSERT_STATUS_IS( assertion, failed, ignore, status )     \
   count.captureStatus(),           /* capture test statistics */          \
   CPPUT_BEGIN_ASSERTION_MACRO()    /* warning: change test statistics */  \
   count.check( assertion, failed, ignore, status );                       \
   CppUT::TestInfo::threadInstance().testStatus() = count.status_;

#define TESTINFO_IGNORE( assertion ) \
   try { assertion; } catch ( CppUT::AbortingAssertionException & ) {}

#define TESTINFO_VERIFY_TESTEVENTS( updater ) \
   CPPUT_BEGIN_ASSERTION_MACRO()              \
   (updater).verify()

static void testTestInfoStatus()
{
   CppUT::TestInfo &testInfo = CppUT::TestInfo::threadInstance();
   AssertionCount count;

   // successful aborting assertion
   count.resetStatistics();
   CPPUT_ASSERT( true );
   TESTINFO_ASSERT_STATISTICS_ARE( 1, 0, 0 );

   // successful checking assertion
   count.resetStatistics();
   CPPUT_CHECK( true );
   TESTINFO_ASSERT_STATISTICS_ARE( 1, 0, 0 );

   // failing aborting assertion
   count.resetStatistics();
   TESTINFO_IGNORE(( CPPUT_ASSERT( false ) ))
   TESTINFO_ASSERT_STATUS_IS( 1, 1, 0, CppUT::TestStatus::failed );

   // failing checking assertion
   count.resetStatistics();
   CPPUT_CHECK( false );
   TESTINFO_ASSERT_STATUS_IS( 1, 1, 0, CppUT::TestStatus::failed );

   // ignored successful aborting assertion
   count.resetStatistics();
   CPPUT_IGNORE_FAILURE(( CPPUT_ASSERT( true ) ));
   TESTINFO_ASSERT_STATUS_IS( 1, 1, 0, CppUT::TestStatus::failed );

   // ignored successful checking assertion
   count.resetStatistics();
   CPPUT_IGNORE_FAILURE(( CPPUT_CHECK( true ) ));
   TESTINFO_ASSERT_STATUS_IS( 1, 1, 0, CppUT::TestStatus::failed );

   // ignored failing aborting assertion
   count.resetStatistics();
   CPPUT_IGNORE_FAILURE(( CPPUT_ASSERT( false ) ));
   TESTINFO_ASSERT_STATISTICS_ARE( 1, 0, 1 );

   // ignored failing checking assertion
   count.resetStatistics();
   CPPUT_IGNORE_FAILURE(( CPPUT_CHECK( false ) ));
   TESTINFO_ASSERT_STATISTICS_ARE( 1, 0, 1 );

   // skipped test
   count.resetStatistics();
   try 
   {
      CPPUT_SKIP_TEST();
   }
   catch ( CppUT::SkipTestException & ) 
   {
   }
   TESTINFO_ASSERT_STATUS_IS( 0, 0, 0, CppUT::TestStatus::skipped );
}

// Test status transition logic
static void testTestInfoStatusStatus()
{
   CppUT::TestInfo &testInfo = CppUT::TestInfo::threadInstance();
   AssertionCount count;

// passed => passed
   count.resetStatistics();
   CPPUT_CHECK( true );
   TESTINFO_ASSERT_STATUS_IS( 1, 0, 0, CppUT::TestStatus::passed );

// passed => skipped
   try 
   {
      CPPUT_SKIP_TEST();
   }
   catch ( CppUT::SkipTestException & ) 
   {
   }
   TESTINFO_ASSERT_STATUS_IS( 1, 0, 0, CppUT::TestStatus::skipped );

// skipped => failed
   TESTINFO_IGNORE(( CPPUT_ASSERT( false ) ))
   TESTINFO_ASSERT_STATUS_IS( 2, 1, 0, CppUT::TestStatus::failed );

// failed => failed
   CPPUT_IGNORE_FAILURE(( CPPUT_CHECK( false ) ));
   TESTINFO_ASSERT_STATUS_IS( 3, 1, 1, CppUT::TestStatus::failed );

// passed => failed
   count.resetStatistics();
   CPPUT_IGNORE_FAILURE(( CPPUT_CHECK( true ) ));
   TESTINFO_ASSERT_STATUS_IS( 1, 1, 0, CppUT::TestStatus::failed );
}

static CppUT::CheckerResult
composedPredicate( int x, int y )
{
   CppUT::CheckerResult result;
   result.predicate( "value" ) = 2*x;
   result.compose( "x==y", CppUT::checkEquals( x, y ) );
   result.compose( "y==x", CppUT::checkEquals( y, x ) );
   return result;
}

static void testTestInfoListener()
{
   // we override the context to ensure our result updater is discarded in case of
   // assertion.
   CppUT::TestInfo::ScopedContextOverride context;
   MockTestResultUpdater updater;
   CppUT::TestInfo::threadInstance().setTestResultUpdater( updater );

   // no events
   CPPUT_CHECK( true );
   TESTINFO_VERIFY_TESTEVENTS( updater );

   // one aborting assertion and one non aborting assertion events
   updater.expectAssertionEvent( "msg1" );
   updater.expectAssertionEvent( "msg2" );
   CPPUT_CHECK( false, "msg1" );
   CPPUT_CHECK( true, "msg3" );
   TESTINFO_IGNORE(( CPPUT_ASSERT( false, "msg2" ) ))
   TESTINFO_VERIFY_TESTEVENTS( updater );

   // one aborting assertion and one non aborting assertion events
   updater.expectAssertionEvent( "msg1" );
   updater.expectAssertionEvent( "msg2" );
   CPPUT_CHECK( false, "msg1" );
   TESTINFO_IGNORE(( CPPUT_ASSERT( false, "msg2" ) ))
   TESTINFO_VERIFY_TESTEVENTS( updater );

   // two ignored assertions and one aborting assertion events
   updater.expectAssertionEvent( "CPPUT_CHECK( true, \"ignore1\" )" );
   updater.expectAssertionEvent( "", true ); // ignored failure
   updater.expectAssertionEvent( "abort3" );
   CPPUT_IGNORE_FAILURE(( CPPUT_CHECK( true, "ignore1" ) ));
   CPPUT_IGNORE_FAILURE(( CPPUT_CHECK( false, "ignore2" ) ));
   TESTINFO_IGNORE(( CPPUT_ASSERT( false, "abort3" ) ))
   TESTINFO_VERIFY_TESTEVENTS( updater );

   // two ignored assertions and one aborting assertion events
   updater.expectAssertionEvent( "CPPUT_CHECK( true, \"ignore1\" )" );
   updater.expectLogEvent( "log_1" );
   updater.expectAssertionEvent( "", true ); // ignored failure
   updater.expectLogEvent( "log_2" );
   updater.expectLogEvent( "log_3" );
   updater.expectAssertionEvent( "abort3" );
   updater.expectLogEvent( "log_4" );
   CPPUT_IGNORE_FAILURE(( CPPUT_CHECK( true, "ignore1" ) ));
   CppUT::log( "log_1" );
   CPPUT_IGNORE_FAILURE(( CPPUT_CHECK( false, "ignore2" ) ));
   CppUT::log( "log_2" );
   CppUT::log( "log_3" );
   TESTINFO_IGNORE(( CPPUT_ASSERT( false, "abort3" ) ))
   CppUT::log( "log_4" );
   TESTINFO_VERIFY_TESTEVENTS( updater );

   // two composed predicates failure
   updater.expectAssertionEvent( "1234" );
   updater.expectAssertionEvent( "2468" );
   updater.expectAssertionEvent( "5678" );
   CPPUT_CHECK_PREDICATE( composedPredicate(1234, 5678) );
   CPPUT_CHECK_PREDICATE( composedPredicate(1234, 5678) );
   CPPUT_CHECK_PREDICATE( composedPredicate(1234, 5678) );
   TESTINFO_VERIFY_TESTEVENTS( updater );
}


bool testTestInfo()
{
   printf( "Running bootstrap test: testTestInfo()...\n" );
   try
   {     
      CppUT::TestInfo::threadInstance().startNewTest();
      testTestInfoStatus();
      testTestInfoStatusStatus();
      testTestInfoListener();
   }
   catch ( const CppUT::AbortingAssertionException &e )
   {
      printf( "testTestInfo() failed: %s\n", e.what() );
      return false;
   }
   catch ( ... )
   {
      printf( "testTestInfo() failed (uncaught exception)." );
      return false;
   }

   return true;
}
