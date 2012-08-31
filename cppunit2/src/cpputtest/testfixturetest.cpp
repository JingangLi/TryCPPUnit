#include "testing.h"
#include <cpput/testing.h>
#include <cpput/testing.h>
#include <cpput/testing.h>
#include <cpput/assertcommon.h>
#include "minitestrunner.h"
#include <deque>
#include <set>

CPPUT_SUITE( "HandMadeAbstractTestFixture" ) {

   typedef std::deque<std::string> Logs;
   static Logs logs;

/* How are abstract fixture implemented:
   - each class in the hierarchy declares a static function to create an instance of the
     fixture
   - in the fixture, the run method use the cpputRun_ member function pointer to find out
     which test case to executed. If NULL, then delegate execution to parant class
     that has its own cpputRun_ member function pointer.
   - when populating the test suite, a class specific test case factory is provided.
     It does the following when called:
     - instantiate the fixture using the static function pointer passed at initialization
     - set the cpputRun_ member function pointer on the fixture
 */


   class HMBasicFixture : public CppUT::TestCase
   {
   public:
// Begin code in _CPPUT_TESTSUITE_COMMON macro

      // Member function to run
      void (HMBasicFixture::*cpputRun_)();

      struct FixtureTestFactory
      {
         typedef CppUT::TestCase *result_type;

         void (HMBasicFixture::*run_)();
         CppUT::TestCaseFactoryFn factory_;

         static void addTest( CppUT::Suite &suite,
                              CppUT::TestCaseFactoryFn fixtureFactory,
                              const char *name,
                              void (HMBasicFixture::*run)() )
         {
            CppUT::Impl::DefaultTestExtendedData metaData;
            addTest( suite, fixtureFactory, name, run, metaData );
         }

         static void addTest( CppUT::Suite &suite,
                              CppUT::TestCaseFactoryFn fixtureFactory,
                              const char *name,
                              void (HMBasicFixture::*run)(),
							         const CppUT::TestExtendedData &metaData )
         {
            FixtureTestFactory factory;
            factory.run_ = run;
            factory.factory_ = fixtureFactory;
            suite.add( CppUT::TestMeta( ::CppTL::fn0r( factory ), CppUT::MetaData(name, metaData) ) );
         }


         CppUT::TestCase *operator()() const
         {
            HMBasicFixture *fixture = static_cast<HMBasicFixture *>( factory_() );
            if ( fixture != 0 )
            {
               fixture->cpputClearRunnable();
               fixture->cpputRun_ = run_;
            }
            return fixture;
         }
      };


      virtual void cpputClearRunnable()
      {
         cpputRun_ = 0;
      }

      virtual void run()
      {
         CPPUT_ASSERT_EXPR( cpputRun_ != 0 );
         (this->*cpputRun_)();
      }

// End code in _CPPUT_TESTSUITE_COMMON macro

// Begin code in _CPPUT_TESTSUITE_COMMON_CONCRETE macro
      // Only if not abstract
      static CppUT::TestCase *fixtureFactory()
      {
         return new HMBasicFixture();
      }

      // Only if not abstract
      static CppUT::Suite suite()
      {
         CppUT::Suite thisSuite( "HMBasicFixture" );
         addTests( thisSuite );
         return thisSuite;
      }

      // No default parameter if abstract
      static void addTests( CppUT::Suite &suite,
         CppUT::TestCaseFactoryFn factory = &HMBasicFixture::fixtureFactory )
      {
// End code in _CPPUT_TESTSUITE_COMMON_CONCRETE macro
         FixtureTestFactory::addTest( suite, factory, "test1", 
                                      &HMBasicFixture::test1 );
         FixtureTestFactory::addTest( suite, factory, "test2", 
                                      &HMBasicFixture::test2,
                                      ( describe("test2 has meta data"), timeOut( 4.0 ) ) );
      }

      void test1()
      {
         logs.push_back( "test1" );
      }

      virtual void test2()
      {
         logs.push_back( "test2" );
      }
   };



   class HMDerivedFixture : public HMBasicFixture
   {
   public:
      // Member function to run
      void (HMDerivedFixture::*cpputRun_)();

      static CppUT::TestCase *fixtureFactory()
      {
         return new HMDerivedFixture();
      }


      struct FixtureTestFactory
      {
         typedef CppUT::TestCase *result_type;

         void (HMDerivedFixture::*run_)();
         CppUT::TestCaseFactoryFn factory_;

         static void addTest( CppUT::Suite &suite,
                              CppUT::TestCaseFactoryFn fixtureFactory,
                              const char *name,
                              void (HMDerivedFixture::*run)() )
         {
            FixtureTestFactory factory;
            factory.run_ = run;
            factory.factory_ = fixtureFactory;
            suite.add( CppUT::TestMeta( ::CppTL::fn0r( factory ), name ) );
         }

         CppUT::TestCase *operator()() const
         {
            HMDerivedFixture *fixture = static_cast<HMDerivedFixture *>( factory_() );
            if ( fixture != 0 )
            {
               fixture->cpputRun_ = run_;
            }
            return fixture;
         }
      };


      virtual void cpputClearRunnable()
      {
         HMBasicFixture::cpputClearRunnable();
         cpputRun_ = 0;
      }

      virtual void run()
      {
         if ( cpputRun_ != 0 )
         {
            (this->*cpputRun_)();
         }
         else
         {
            HMBasicFixture::run();
         }
      }

      static CppUT::Suite suite()
      {
         CppUT::Suite thisSuite( "HMDerivedFixture" );
         addTests( thisSuite );
         return thisSuite;
      }

      static void addTests( CppUT::Suite &suite,
         CppUT::TestCaseFactoryFn factory = &HMDerivedFixture::fixtureFactory )
      {
         HMBasicFixture::addTests( suite, factory );
         FixtureTestFactory::addTest( suite, factory, "test3", 
                                      &HMDerivedFixture::test3 );
      }

      virtual void test2()
      {
         logs.push_back( "derived.test2" );
      }

      void test3()
      {
         logs.push_back( "derived.test3" );
      }
   };


   CPPUT_TEST_FUNCTION( testBasicFixture ) {
      logs.clear();
      MiniTestRunner runner;
      runner.run( HMBasicFixture::suite() );
      CPPUT_ASSERT_EQUAL( 2, runner.tested_ );
      CPPUT_ASSERT_EQUAL( 1, runner.suite_ );
      CPPUT_ASSERT_EQUAL( 2, logs.size() );
      CPPUT_ASSERT_EQUAL( "test1", logs[0] );
      CPPUT_ASSERT_EQUAL( "test2", logs[1] );
      CPPUT_ASSERT_EQUAL( 2, HMBasicFixture::suite().testCaseCount() );
      const CppUT::TestMeta *test2 = HMBasicFixture::suite().testCaseAt( 1 );
      CPPUT_ASSERT( test2 != 0 );
      CPPUT_ASSERT_EQUAL( "test2 has meta data", test2->description() );
      CPPUT_ASSERT_EQUAL( 4.0, test2->timeOut() );
   }

   CPPUT_TEST_FUNCTION( testDerivedFixture ) {
      logs.clear();
      MiniTestRunner runner;
      runner.run( HMDerivedFixture::suite() );
      CPPUT_ASSERT_EQUAL( 3, runner.tested_ );
      CPPUT_ASSERT_EQUAL( 1, runner.suite_ );
      CPPUT_ASSERT_EQUAL( 3, logs.size() );
      CPPUT_ASSERT_EQUAL( "test1", logs[0] );
      CPPUT_ASSERT_EQUAL( "derived.test2", logs[1] );
      CPPUT_ASSERT_EQUAL( "derived.test3", logs[2] );
   }

}



CPPUT_SUITE( "AbstractTestFixture" ) {

   class EmptyFixture : public CppUT::TestCase
   {
      CPPUT_TESTSUITE_BEGIN( EmptyFixture )
      CPPUT_TESTSUITE_END()
   };


   class TwoTestFixture : public CppUT::TestCase
   {
      CPPUT_TESTSUITE_BEGIN( TwoTestFixture )
      CPPUT_TESTSUITE_TEST( testPass );
      CPPUT_TESTSUITE_TEST( testFail );
      CPPUT_TESTSUITE_END()
   public:
      void testPass()
      {
      }

      void testFail()
      {
         CPPUT_ASSERT( false, "forced failure" );
      }
   };

   class ExtendTestFixture : public TwoTestFixture
   {
      CPPUT_TESTSUITE_EXTEND( ExtendTestFixture, TwoTestFixture )
      CPPUT_TESTSUITE_TEST( testFail2 );
      CPPUT_TESTSUITE_END()
   public:
      void testFail2()
      {
         CPPUT_ASSERT( false, "forced failure 2" );
      }
   };

   class AbstractTestFixture : public CppUT::TestCase
   {
      CPPUT_TESTSUITE_ABSTRACT_BEGIN( AbstractTestFixture )
      CPPUT_TESTSUITE_TEST( testDerived );
      CPPUT_TESTSUITE_TEST( testFail );
      CPPUT_TESTSUITE_END()
   public:
      virtual void testDerived() = 0;

      void testFail()
      {
         CPPUT_ASSERT( false, "forced failure" );
      }
   };

   class ConcretTestFixture : public AbstractTestFixture
   {
      CPPUT_TESTSUITE_EXTEND( ConcretTestFixture, AbstractTestFixture )
      CPPUT_TESTSUITE_TEST( testFail2 );
      CPPUT_TESTSUITE_END()
   public:
      void testDerived()
      {
         CPPUT_ASSERT( false, "forced failure 3" );
      }

      void testFail2()
      {
         CPPUT_ASSERT( false, "forced failure 2" );
      }
   };



   class SharedFixtureTestBase : public CppUT::TestCase
   {
   public:
      SharedFixtureTestBase()
      {
         created_.push_back( this );
      }

      virtual ~SharedFixtureTestBase()
      {
         destroyed_.push_back( this );
      }

      void test1() { test(); }
      void test2() { test(); }
      void test3() { test(); }

      static std::vector<SharedFixtureTestBase *> created_;
      static std::vector<SharedFixtureTestBase *> runned_;
      static std::vector<SharedFixtureTestBase *> destroyed_;

   private:
      void test()
      {
         runned_.push_back( this );
      }
   };

   std::vector<SharedFixtureTestBase *> SharedFixtureTestBase::created_;
   std::vector<SharedFixtureTestBase *> SharedFixtureTestBase::runned_;
   std::vector<SharedFixtureTestBase *> SharedFixtureTestBase::destroyed_;


   class NoSharedFixtureTest : public SharedFixtureTestBase
   {
   public:
      CPPUT_TESTSUITE_BEGIN( NoSharedFixtureTest )
      CPPUT_TESTSUITE_TEST( test1 );
      CPPUT_TESTSUITE_TEST( test2 );
      CPPUT_TESTSUITE_TEST( test3 );
      CPPUT_TESTSUITE_END()
   };


   class TestFixtureWithSpecific : public CppUT::TestCase
   {
      CPPUT_TESTSUITE_BEGIN( TestFixtureWithSpecific );
      CPPUT_TESTSUITE_TEST_WITH_META( testFail,
                                 ( describe( "Forced test failure" ),
                                   timeOut( 30.0 ),
                                   group("testfixture"),
                                   group("test")
                                 ) );
      CPPUT_TESTSUITE_END()
   public:
      void testFail()
      {
         CPPUT_ASSERT( false, "forced failure" );
      }
   };

   CPPUT_TEST_FUNCTION( testEmptySuite ) {
      logs.clear();
      MiniTestRunner runner;
      runner.run( EmptyFixture::suite() );
      CPPUT_ASSERT_EQUAL( 0, runner.tested_ );
      CPPUT_ASSERT_EQUAL( 1, runner.suite_ );
   }

   CPPUT_TEST_FUNCTION( testTwoTestsSuite )
   {
      MiniTestRunner runner;
      runner.run( TwoTestFixture::suite() );
      CPPUT_ASSERT_EQUAL( 2, runner.tested_ );
      CPPUT_ASSERT_EQUAL( 1, runner.failed_ );
   }


   CPPUT_TEST_FUNCTION( testExtendTestSuite )
   {
      MiniTestRunner runner;
      runner.run( ExtendTestFixture::suite() );
      CPPUT_ASSERT_EQUAL( 3, runner.tested_ );
      CPPUT_ASSERT_EQUAL( 2, runner.failed_ );
   }


   CPPUT_TEST_FUNCTION( testExtendAbstractTestSuite )
   {
      MiniTestRunner runner;
      runner.run( ConcretTestFixture::suite() );
      CPPUT_ASSERT_EQUAL( 3, runner.tested_ );
      CPPUT_ASSERT_EQUAL( 3, runner.failed_ );
   }


   CPPUT_TEST_FUNCTION( testDefaultIsNoSharedFixture )
   {
      MiniTestRunner runner;
      SharedFixtureTestBase::created_.clear();
      SharedFixtureTestBase::runned_.clear();
      SharedFixtureTestBase::destroyed_.clear();
      runner.run( NoSharedFixtureTest::suite() );
      CPPUT_ASSERT_EQUAL( 3, runner.tested_ );
      CPPUT_ASSERT_EQUAL( 0, runner.failed_ );
      CPPUT_ASSERT_EQUAL( 3, int(SharedFixtureTestBase::created_.size() ) );
      CPPUT_ASSERT_EQUAL( 3, int(SharedFixtureTestBase::runned_.size() ) );
      CPPUT_ASSERT_EQUAL( 3, int(SharedFixtureTestBase::destroyed_.size() ) );
      for ( int index = 0; index < 3; ++index )
      {
         SharedFixtureTestBase *instance = SharedFixtureTestBase::created_[index];
         CPPUT_ASSERT( SharedFixtureTestBase::runned_[index] = instance );
         CPPUT_ASSERT( SharedFixtureTestBase::destroyed_[index] = instance );
      }
   }


   CPPUT_TEST_FUNCTION( testFixtureWithSpecifics )
   {
      CppUT::Suite suite = TestFixtureWithSpecific::suite();
      CPPUT_ASSERT_EQUAL( 1, suite.testCaseCount() );
      const CppUT::TestMeta *test = suite.testCaseAt(0);
      CPPUT_ASSERT( test != 0 );
      CPPUT_ASSERT_EQUAL( "Forced test failure", test->description() );
      CPPUT_ASSERT_EQUAL( 30.0, test->timeOut() );
      CPPUT_ASSERT_EQUAL( 2, test->groupCount() );
      CPPUT_ASSERT_EQUAL( "testfixture", test->groupAt(0) );
      CPPUT_ASSERT_EQUAL( "test", test->groupAt(1) );

      MiniTestRunner runner;
      runner.run( suite );
      CPPUT_ASSERT_EQUAL( 1, runner.tested_ );
      CPPUT_ASSERT_EQUAL( 1, runner.failed_ );
   }


}
