#include <cpput/assertcommon.h>
#include <cpput/testing.h>
#include <stdio.h>
#include <stdexcept>


namespace {

   static void callbackNoThrow()
   {
   }

   static void callbackThrow()
   {
      throw std::runtime_error( "callbackNoThrow exception" );
   }

   struct MayThrowFunctor
   {
      MayThrowFunctor( bool shouldThrow )
         : shouldThrow_( shouldThrow )
      {
      }

      void operator()() const
      {
         if ( shouldThrow_ )
         {
            throw std::runtime_error( "MayThrowFunctor exception" );
         }
      }

      bool shouldThrow_;
   };


   class MockTestCase : public CppUT::TestCase
   {
   public:
      static int setUpCall_;
      static int tearDownCall_;
      static int runCall_;

      typedef void (*RunFn)();

      RunFn setUp_;
      RunFn run_;
      RunFn tearDown_;

      static CppUT::TestCase *factory( RunFn setUp, RunFn run, RunFn tearDown )
      {
         MockTestCase *test = new MockTestCase();
         test->setUp_ = setUp;
         test->run_ = run;
         test->tearDown_ = tearDown;
         setUpCall_ = 0;
         tearDownCall_ = 0;
         runCall_ = 0;
         return test;
      }

   public: // overridden from CppUT::TestCase
      virtual void setUp()
      {
         ++setUpCall_;
         setUp_();
      }

      virtual void run()
      {
         ++tearDownCall_;
         run_();
      }

      virtual void tearDown()
      {
         ++runCall_;
         tearDown_();
      }
   };
   
   int MockTestCase::setUpCall_ = -1;
   int MockTestCase::tearDownCall_ = -1;
   int MockTestCase::runCall_ = -1;

   class MockTestCaseDtorThrow : public MockTestCase
   {
   public:
      virtual ~MockTestCaseDtorThrow()
      {
         // In theory exception should never be thrown from destructor as it may cause memory leaks,
         // or crash if another exception was already present.
         // In practice, the framework needs to detect such case.
         throw std::runtime_error( "Invalid state" );
      }

      static CppUT::TestCase *factory()
      {
         MockTestCaseDtorThrow *test = new MockTestCaseDtorThrow();
         test->setUp_ = callbackNoThrow;
         test->run_ = callbackNoThrow;
         test->tearDown_ = callbackNoThrow;
         setUpCall_ = 0;
         tearDownCall_ = 0;
         runCall_ = 0;
         return test;
      }
   };

} // end anonymous namespace



//class TestTestCaseFixture : public CppTL::IntrusiveCount
//{
//public:
//   TestTestCaseFixture()
//      : setUp_(0)
//      , tearDown_(0)
//      , run_(0)
//   {
//   }
//
//   void runWithParameter( int runIncrement )
//   {
//      run_ += runIncrement;
//   }
//
//   void setUp()
//   {
//      ++setUp_;
//   }
//
//   void tearDown()
//   {
//      ++tearDown_;
//   }
//
//   void run()
//   {
//      ++run_;
//   }
//
//   int setUp_;
//   int tearDown_;
//   int run_;
//};
//
//typedef CppTL::IntrusivePtr<TestTestCaseFixture> TestTestCaseFixturePtr;
//
//
//struct SetUpCaller
//{
//   SetUpCaller( const TestTestCaseFixturePtr &fixture )
//      : fixture_( fixture )
//   {
//   }
//
//   void operator()() const
//   {
//      fixture_->setUp();
//   }
//
//   TestTestCaseFixturePtr fixture_;
//};
//




static bool runTestCase( CppUT::TestMeta &test )
{
   bool success = false;
   {
      CppUT::TestInfo::ScopedContextOverride contextOverride;
      success = test.runTest();
   }
   return success;
}

static CppUT::TestCase *testCaseNoFailureFactory()
{
   return MockTestCase::factory( callbackNoThrow, callbackNoThrow, callbackNoThrow );
}

static void testCaseSetUpRunTearDown()
{
   CppUT::TestMeta test( testCaseNoFailureFactory, "Test 1" );
   CPPUT_ASSERT_EQUAL( "Test 1", test.name() );

   CPPUT_ASSERT( runTestCase( test ) && "testCaseSetUpRunTearDown" );
   CPPUT_ASSERT_EQUAL( 1, MockTestCase::setUpCall_ );
   CPPUT_ASSERT_EQUAL( 1, MockTestCase::runCall_ );
   CPPUT_ASSERT_EQUAL( 1, MockTestCase::tearDownCall_ );
}


// @todo Examine if we should preserve this complex interface.
//static CppUT::TestPtr makeTest( void (*setUpCallback)( int *),
//                                int *testSetUpCall,
//                                void (*runCallback)( int *),
//                                int *testRunCall,
//                                void (*tearDownCallback)( int *),
//                                int *testTearDownCall,
//                                const std::string &name )
//{
//   return CppUT::makeTestCase( 
//      CppTL::bind_cfn(setUpCallback, testSetUpCall ),
//      CppTL::bind_cfn(runCallback, testRunCall ),
//      CppTL::bind_cfn(tearDownCallback, testTearDownCall ),
//      name );
//}

static CppUT::TestCase *testCaseSetUpThrowFactory()
{
   return MockTestCase::factory( callbackThrow, callbackNoThrow, callbackNoThrow );
}


static void testCaseSetUpThrow()
{
   CppUT::TestMeta test( testCaseSetUpThrowFactory, "Test Setup Throw" );

   CPPUT_ASSERT_EXPR_FALSE( runTestCase( test ) );
   CPPUT_ASSERT_EQUAL( 1, MockTestCase::setUpCall_ );
   CPPUT_ASSERT_EQUAL( 0, MockTestCase::runCall_ );
   CPPUT_ASSERT_EQUAL( 0, MockTestCase::tearDownCall_ );
}


static CppUT::TestCase *testCaseRunThrowFactory()
{
   return MockTestCase::factory( callbackNoThrow, callbackThrow, callbackNoThrow );
}


static void testCaseRunThrow()
{
   CppUT::TestMeta test( testCaseRunThrowFactory, "Test Run Throw" );

   CPPUT_ASSERT_EXPR_FALSE( runTestCase( test ) );
   CPPUT_ASSERT_EQUAL( 1, MockTestCase::setUpCall_ );
   CPPUT_ASSERT_EQUAL( 1, MockTestCase::runCall_ );
   CPPUT_ASSERT_EQUAL( 1, MockTestCase::tearDownCall_ );
}


static CppUT::TestCase *testCaseTearDownThrowFactory()
{
   return MockTestCase::factory( callbackNoThrow, callbackNoThrow, callbackThrow );
}


static void testCaseTearDownThrow()
{
   CppUT::TestMeta test( testCaseTearDownThrowFactory, "Test Tear Down Throw" );

   CPPUT_ASSERT_EXPR_FALSE( runTestCase( test ) );
   CPPUT_ASSERT_EQUAL( 1, MockTestCase::setUpCall_ );
   CPPUT_ASSERT_EQUAL( 1, MockTestCase::runCall_ );
   CPPUT_ASSERT_EQUAL( 1, MockTestCase::tearDownCall_ );
}

///@todo Adds test for factory return NULL, factory throw.
static CppUT::TestCase *testCaseNullInstanceFactory()
{
   return 0;
}


static void testCaseNullInstance()
{
   CppUT::TestMeta test( testCaseNullInstanceFactory, "Test Null Factory" );

   CPPUT_ASSERT_EXPR_FALSE( runTestCase( test ) );
}


static CppUT::TestCase *testCaseFactoryThrowFactory()
{
   // The test case constructors may throw an exception. Checks that.
   throw std::runtime_error( "Bad Environment" );
}


static void testCaseFactoryThrow()
{
   CppUT::TestMeta test( testCaseFactoryThrowFactory, "Test Factory Throw" );

   CPPUT_ASSERT_EXPR_FALSE( runTestCase( test ) );
}


static void testCaseDestructorThrow()
{
   CppUT::TestMeta test( MockTestCaseDtorThrow::factory, 
                         "Test Destructor Throw" );

   CPPUT_ASSERT_EXPR_FALSE( runTestCase( test ) );
   CPPUT_ASSERT_EQUAL( 1, MockTestCaseDtorThrow::setUpCall_ );
   CPPUT_ASSERT_EQUAL( 1, MockTestCaseDtorThrow::runCall_ );
   CPPUT_ASSERT_EQUAL( 1, MockTestCaseDtorThrow::tearDownCall_ );
}



static void testCaseMakeTestCaseFromFunction()
{
   CppUT::TestMeta test1 = CppUT::makeTestCase( &callbackNoThrow, "function.noThrow" );
   CPPUT_ASSERT_EXPR( runTestCase( test1 ) );

   CppUT::TestMeta test2 = CppUT::makeTestCase( &callbackThrow, "function.throw" );
   CPPUT_ASSERT_EXPR_FALSE( runTestCase( test2 ) );

   CppUT::MetaData meta( "functionWithMeta.throw" );
   meta.setDescription( "Test with description" );
   CppUT::TestMeta test3 = CppUT::makeTestCase( &callbackThrow, meta );
   CPPUT_ASSERT_EXPR_FALSE( runTestCase( test3 ) );
   CPPUT_ASSERT_EQUAL( std::string("Test with description"), test3.description() );
}


static void testCaseMakeTestCaseFromFunctor0()
{
   CppUT::TestMeta test1 = CppUT::makeTestCase( CppTL::fn0(MayThrowFunctor(false)), 
                                                "functor.noThrow" );
   CPPUT_ASSERT_EXPR( runTestCase( test1 ) );

   CppUT::TestMeta test2 = CppUT::makeTestCase( CppTL::fn0(MayThrowFunctor(true)), 
                                                "functor.throw" );
   CPPUT_ASSERT_EXPR_FALSE( runTestCase( test2 ) );

   CppUT::MetaData meta( "functorWithMeta.throw" );
   meta.setDescription( "Test with description" );
   CppUT::TestMeta test3 = CppUT::makeTestCase( CppTL::fn0(MayThrowFunctor(true)), meta );
   CPPUT_ASSERT_EXPR_FALSE( runTestCase( test3 ) );
   CPPUT_ASSERT_EQUAL( std::string("Test with description"), test3.description() );
}


static void testCaseMakeTestCaseFromFunctor()
{
   CppUT::TestMeta test1 = CppUT::makeTestCase( MayThrowFunctor(false), 
                                                "functor.noThrow" );
   CPPUT_ASSERT_EXPR( runTestCase( test1 ) );

   CppUT::TestMeta test2 = CppUT::makeTestCase( MayThrowFunctor(true), 
                                                "functor.throw" );
   CPPUT_ASSERT_EXPR_FALSE( runTestCase( test2 ) );

   CppUT::MetaData meta( "functorWithMeta.throw" );
   meta.setDescription( "Test with description" );
   CppUT::TestMeta test3 = CppUT::makeTestCase( MayThrowFunctor(true), meta );
   CPPUT_ASSERT_EXPR_FALSE( runTestCase( test3 ) );
   CPPUT_ASSERT_EQUAL( std::string("Test with description"), test3.description() );
}


static void testRunFixture()
{
   ///@todo fix this
   //TestTestCaseFixturePtr fixture = TestTestCaseFixturePtr( new TestTestCaseFixture() );

   //CppUT::TestPtr test = CppUT::makeFixtureTestCase( ::CppTL::Type<TestTestCaseFixture>(),
   //                             CppTL::memfn0( fixture, &TestTestCaseFixture::run ),
   //                             "Test1" );
   //CPPUT_ASSERT( test->isTestCase() );
   //CPPUT_ASSERT_EQUAL( "Test1", test->name() );

   //CPPUT_ASSERT_EXPR( runTestCase( *test ) );
   //CPPUT_ASSERT_EQUAL( 1, fixture->setUp_ );
   //CPPUT_ASSERT_EQUAL( 1, fixture->run_ );
   //CPPUT_ASSERT_EQUAL( 1, fixture->tearDown_ );
}


static void testParametrizedFixture()
{
   ///@todo fix this
   //TestTestCaseFixturePtr fixture = TestTestCaseFixturePtr( new TestTestCaseFixture() );
   //CppUT::TestPtr test = CppUT::makeTestCase(
   //   CppTL::bind_memfn( fixture, &TestTestCaseFixture::runWithParameter, 1 ),
   //   CppTL::bind_memfn( fixture, &TestTestCaseFixture::runWithParameter, 10 ),
   //   CppTL::bind_memfn( fixture, &TestTestCaseFixture::runWithParameter, 100 ),
   //   "Test1" );
   //CPPUT_ASSERT( test->isTestCase() );
   //CPPUT_ASSERT_EQUAL( "Test1", test->name() );

   //CPPUT_ASSERT_EXPR( runTestCase( *test ) );
   //CPPUT_ASSERT_EQUAL( 0, fixture->setUp_ );
   //CPPUT_ASSERT_EQUAL( 111, fixture->run_ );
   //CPPUT_ASSERT_EQUAL( 0, fixture->tearDown_ );
}


bool testTestCase()
{
   printf( "Running bootstrap test: testTestCase()...\n" );
   try
   {     
      CppUT::TestInfo::threadInstance().startNewTest();
      testCaseSetUpRunTearDown();
      testCaseSetUpThrow();
      testCaseRunThrow();
      testCaseTearDownThrow();
      testCaseNullInstance();
      testCaseFactoryThrow();
      testCaseDestructorThrow();
      // ? Should we also test if we correctly handle a throwing destructor ?
      // This would requires the test to throw from the destructor which is
      // a corner edge of C++
      testCaseMakeTestCaseFromFunction();
      testCaseMakeTestCaseFromFunctor0();
      testCaseMakeTestCaseFromFunctor();
/// @todo fix this
//      testRunFixture();
//      testParametrizedFixture();
   }
   catch ( const CppUT::AbortingAssertionException &e )
   {
      printf( "testTestCase() failed: %s\n", e.what() );
      return false;
   }
   catch ( ... )
   {
      printf( "testTestCase() failed (uncaught exception)." );
      return false;
   }

   return true;
}
