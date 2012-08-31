#include <cpput/testing.h>
#include <cpput/assertcommon.h>
//#include "minitestrunner.h"

// @todo test in the presence of a default suite

// Checks that by default test are registered in the root suite
CPPUT_TEST_FUNCTION( testDefaultParentSuite ) 
{
}


// Makes a specific root suite
CPPUT_SUITE( "Root1" ) {

   // Adds two test functions inside it
   CPPUT_TEST_FUNCTION( testRoot1Test1 )
   {
   }

   CPPUT_TEST_FUNCTION_WITH_META( testRoot1Test2, (describe("it is test2"), timeOut(5.0)) )
   {
   }
} // Suite Root1

// Another empty root suite
CPPUT_SUITE( "Root2" ) {
} // Suite Root2


// Need to test that the structure of the root tree match the expected one.

static bool
checkContainedNestedSuite( CppUT::Suite &suite, 
                           const std::string &childSuiteName )
{
   int nestedSuiteCount = suite.nestedSuiteCount();
   for ( int index = 0; index < nestedSuiteCount; ++index )
   {
      CppUT::Suite childSuite = suite.nestedSuiteAt(index);
      CPPUT_ASSERT_EXPR( childSuite.isValid() );
      if ( childSuite.name() == childSuiteName )
         return true;
   }
   return false;
}

static bool
checkContainedTestCase( CppUT::Suite &suite, 
                        const std::string &testCaseName )
{
   int nestedTestCaseCount = suite.testCaseCount();
   for ( int index = 0; index < nestedTestCaseCount; ++index )
   {
      const CppUT::TestMeta *testCase = suite.testCaseAt(index);
      CPPUT_ASSERT( testCase != 0 );
      if ( testCase->name() == testCaseName )
         return true;
   }
   return false;
}



static void
testRegistryRootSuiteContent()
{
   CppUT::Suite rootSuite = CppUT::Registry::getRootSuite();
   CPPUT_ASSERT_EXPR( checkContainedNestedSuite( rootSuite, "Root1" ) );
   CPPUT_ASSERT_EXPR( checkContainedNestedSuite( rootSuite, "Root2" ) );
   CppUT::Suite root1Suite = rootSuite.nestedSuiteByName( "Root1" );
   CppUT::Suite root2Suite = rootSuite.nestedSuiteByName( "Root2" );
   CPPUT_ASSERT_EXPR( root1Suite.isValid() );
   CPPUT_ASSERT_EXPR( root2Suite.isValid() );
   CPPUT_ASSERT_EXPR( root1Suite.parent().isValid() );
   CPPUT_ASSERT_EXPR( root2Suite.parent().isValid() );
   CPPUT_ASSERT_EXPR( root1Suite.parent() == rootSuite );
   CPPUT_ASSERT_EXPR( root2Suite.parent() == rootSuite );
   CPPUT_ASSERT( checkContainedTestCase( root1Suite, "testRoot1Test1" ) );
   CPPUT_ASSERT( checkContainedTestCase( root1Suite, "testRoot1Test2" ) );
   const CppUT::TestMeta *test = root1Suite.testCaseAt( 1 );
   CPPUT_ASSERT_EXPR( test != 0 );
   CPPUT_ASSERT_EQUAL( "testRoot1Test2", test->name() );

   CPPUT_ASSERT_EQUAL( "it is test2", test->description() );
   CPPUT_ASSERT_EQUAL( 5.0, test->timeOut() );
   CPPUT_ASSERT_EQUAL( 0, test->groupCount() );

   // @todo effective test of strict ordering & hashing
   root1Suite < rootSuite;
   rootSuite.hash();
}


bool testRegistry()
{
   printf( "Running bootstrap test: testRegistry()...\n" );
   try
   {     
      CppUT::TestInfo::threadInstance().startNewTest();
      testRegistryRootSuiteContent();
   }
   catch ( const CppUT::AbortingAssertionException &e )
   {
      printf( "testRegistry() failed: %s\n", e.what() );
      return false;
   }
   catch ( ... )
   {
      printf( "testRegistry() failed (uncaught exception)." );
      return false;
   }

   return true;
}



//namespace {
//
//   class MockFixture
//   {
//   public:
//      static CppUT::TestPtr suite()
//      {
//         return CppUT::makeFailingTestCase( "MockFixture", 
//                                            "MockFixture::suite" );
//      }
//   };
//
//   class MockNamableFixture
//   {
//   public:
//      static CppUT::TestPtr suite( const std::string &name = "" )
//      {
//         std::string testName = name;
//         if ( testName.empty() )
//            testName = "MockNamableFixture";
//         return CppUT::makeFailingTestCase( testName, 
//                                            "MockFixture::suite" );
//      }
//   };
//
//CPPUT_REGISTER_SUITE_TO_DEFAULT( MockFixture )
//CPPUT_REGISTER_SUITE_TO_DEFAULT( MockNamableFixture )
//CPPUT_REGISTER_NAMED_SUITE_TO_DEFAULT( MockNamableFixture, "NamedMockNamableFixture" )
//CPPUT_REGISTER_SUITE_IN( MockFixture, "DummyParentSuite" )
//CPPUT_REGISTER_SUITE_IN( MockNamableFixture, "DummyParentSuite" )
//CPPUT_REGISTER_NAMED_SUITE_IN( MockNamableFixture, "DummyParentSuite", 
//                               "NamedMockNamableFixture" )
//} // end anonymous namespace
//
//
//void 
//RegistryTest::setUp()
//{
//   registry_.reset( new CppUT::Registry() );
//   testNames_.clear();
//}
//
//
//void 
//RegistryTest::tearDown()
//{
//   registry_.reset();
//   testNames_.clear();
//}
//
//
//void 
//RegistryTest::testCreateDefaultTests()
//{
//   CppUT::TestSuitePtr suite = registry_->createDefaultTests();
//   CPPUT_ASSERT( testNames_.empty() );
//}
//
//
//void 
//RegistryTest::testAddTestToDefault()
//{
//   registry_->addToDefault( makeTestFactory( "Test1" ) );
//   registry_->addToDefault( makeTestFactory( "Test2" ) );
//   registry_->addToDefault( makeTestFactory( "Test3" ) );
//   CppUT::TestSuitePtr suite = registry_->createDefaultTests();
//
//   runTestSuite( suite );
//
//   CPPUT_ASSERT_EQUAL( 1, testNames_.count( "Test1" ) );
//   CPPUT_ASSERT_EQUAL( 1, testNames_.count( "Test2" ) );
//   CPPUT_ASSERT_EQUAL( 1, testNames_.count( "Test3" ) );
//}
//
//
//void 
//RegistryTest::testAddTestToNamed()
//{
//   registry_->add( "Name", makeTestFactory( "Test1" ) );
//   registry_->add( "Name", makeTestFactory( "Test2" ) );
//   registry_->add( "Name2", makeTestFactory( "Test3" ) );
//   CppUT::TestSuitePtr suite = registry_->createTests( "Name" );
//   CppUT::TestSuitePtr suite2 = registry_->createTests( "Name2" );
//
//   runTestSuite( suite );
//   CPPUT_ASSERT_EQUAL( 1, testNames_.count( "Test1" ) );
//   CPPUT_ASSERT_EQUAL( 1, testNames_.count( "Test2" ) );
//   CPPUT_ASSERT_EQUAL( 0, testNames_.count( "Test3" ) );
//
//   runTestSuite( suite2 );
//   CPPUT_ASSERT_EQUAL( 1, testNames_.count( "Test3" ) );
//}
//
//
//void 
//RegistryTest::testAddAndRemoveChild()
//{
//   registry_->add( "Name", makeTestFactory( "Test1" ) );
//   registry_->add( "Name", makeTestFactory( "Test2" ) );
//   registry_->add( "Name2", makeTestFactory( "Test3" ) );
//   registry_->add( "Name3", makeTestFactory( "Test4" ) );
//   registry_->addChild( "Name", "Name2" );
//   registry_->addChild( "Name", "Name3" );
//
//   // Checks that all tests are correctly executed
//   CppUT::TestSuitePtr suite = registry_->createTests( "Name" );
//   runTestSuite( suite );
//   CPPUT_ASSERT_EQUAL( 1, testNames_.count( "Test1" ) );
//   CPPUT_ASSERT_EQUAL( 1, testNames_.count( "Test2" ) );
//   CPPUT_ASSERT_EQUAL( 1, testNames_.count( "Test3" ) );
//   CPPUT_ASSERT_EQUAL( 1, testNames_.count( "Test4" ) );
//
//   // Checks that dynamic removal of child suite works
//   CPPUT_ASSERT_FALSE( registry_->removeChild( "Name", "ChildSuiteDoesNotExist" ) );
//   CPPUT_ASSERT( registry_->removeChild( "Name", "Name3" ) );
//   CPPUT_ASSERT( registry_->removeChild( "Name", "Name2" ) );
//   CPPUT_ASSERT_FALSE( registry_->removeChild( "Name", "ChildSuiteDoesNotExist" ) );
//
//   // Checks that child suite were actual removed
//   testNames_.clear();
//   CppUT::TestSuitePtr newSuite = registry_->createTests( "Name" );
//   runTestSuite( newSuite );
//   CPPUT_ASSERT_EQUAL( 1, testNames_.count( "Test1" ) );
//   CPPUT_ASSERT_EQUAL( 1, testNames_.count( "Test2" ) );
//   CPPUT_ASSERT_EQUAL( 0, testNames_.count( "Test3" ) );
//   CPPUT_ASSERT_EQUAL( 0, testNames_.count( "Test4" ) );
//}
//
//
//void 
//RegistryTest::testAutoRemoveByTestFactoryId()
//{
//   // @todo
//   // Register test in the static registry and checks that they are correctly removed
//   typedef CppUT::SuiteRegisterer<MockNamableFixture> SuiteAutoRegisterer;
//   {
//      SuiteAutoRegisterer r1( "TestRegister1" );
//      SuiteAutoRegisterer r2( "TestRegister2" );
//      SuiteAutoRegisterer r3( CppUT::Impl::RegisterToNamedSuiteTag(),
//                              "ParentTestRegister1", 
//                              "TestRegister2" );
//      SuiteAutoRegisterer r4( CppUT::Impl::RegisterToNamedSuiteTag(),
//                              "ParentTestRegister1", 
//                              "TestRegister3" );
//      SuiteAutoRegisterer r5( CppUT::Impl::RegisterToNamedSuiteTag(),
//                              "ParentTestRegister2", 
//                              "TestRegister4" );
//
//      CppUT::SuiteRelationshipRegisterer rs1( "ParentTestRegister1" );
//      CppUT::SuiteRelationshipRegisterer rs2( "ParentTestRegister1", "ParentTestRegister2" );
//      // We have the following tree:
//      // Default Suite
//      // |- TestRegister1
//      // |- TestRegister2
//      // |- ParentTestRegister1
//      // |  |- TestRegister2
//      // |  |- TestRegister3
//      // |  |- ParentTestRegister2
//      // |     |- TestRegister4
//      // |- other statically registered test for testStaticRegistration()
//
//      CppUT::TestSuitePtr defaultSuite = CppUT::Registry::instance().createDefaultTests();
//      CPPUT_CHECK_EXPR( containsTestNamed( defaultSuite, "TestRegister1" ) );
//      CPPUT_CHECK_EXPR( containsTestNamed( defaultSuite, "TestRegister2" ) );
//      CPPUT_CHECK_EXPR( containsTestNamed( defaultSuite, "ParentTestRegister1" ) );
//
//      CppUT::TestSuitePtr parentSuite1 = CppUT::Registry::instance().createTests("ParentTestRegister1");
//      CPPUT_CHECK_EXPR( containsTestNamed( parentSuite1, "TestRegister2" ) );
//      CPPUT_CHECK_EXPR( containsTestNamed( parentSuite1, "TestRegister3" ) );
//      CPPUT_CHECK_EXPR( containsTestNamed( parentSuite1, "ParentTestRegister2" ) );
//
//      CppUT::TestSuitePtr parentSuite2 = CppUT::Registry::instance().createTests("ParentTestRegister2");
//      CPPUT_CHECK_EXPR( containsTestNamed( parentSuite2, "TestRegister4" ) );
//
//   }
//   // Checks that the registered suite have been removed
//   CppUT::TestSuitePtr defaultSuite = CppUT::Registry::instance().createDefaultTests();
//   CPPUT_CHECK_EXPR_FALSE( containsTestNamed( defaultSuite, "TestRegister1" ) );
//   CPPUT_CHECK_EXPR_FALSE( containsTestNamed( defaultSuite, "TestRegister2" ) );
//   CPPUT_CHECK_EXPR_FALSE( containsTestNamed( defaultSuite, "ParentTestRegister1" ) );
//
//   // Notes: calling createTests will automatically create an unparented suite of the
//   // specified name if it does not exist.
//   CppUT::TestSuitePtr parentSuite1 = CppUT::Registry::instance().createTests("ParentTestRegister1");
//   CPPUT_CHECK_EXPR_FALSE( containsTestNamed( parentSuite1, "TestRegister2" ) );
//   CPPUT_CHECK_EXPR_FALSE( containsTestNamed( parentSuite1, "TestRegister3" ) );
//   CPPUT_CHECK_EXPR_FALSE( containsTestNamed( parentSuite1, "ParentTestRegister2" ) );
//
//   CppUT::TestSuitePtr parentSuite2 = CppUT::Registry::instance().createTests("ParentTestRegister2");
//   CPPUT_CHECK_EXPR_FALSE( containsTestNamed( parentSuite2, "TestRegister4" ) );
//}
//
//
//void 
//RegistryTest::testStaticRegistration()
//{
//   CppUT::TestSuitePtr defaultSuite = CppUT::Registry::instance().createDefaultTests();
//   CPPUT_CHECK_EXPR( containsTestNamed( defaultSuite, "MockFixture" ) );
//   CPPUT_CHECK_EXPR( containsTestNamed( defaultSuite, "MockNamableFixture" ) );
//   CPPUT_CHECK_EXPR( containsTestNamed( defaultSuite, "NamedMockNamableFixture" ) );
//
//   CppUT::TestSuitePtr parentSuite = CppUT::Registry::instance().createTests("DummyParentSuite");
//   CPPUT_CHECK_EXPR( containsTestNamed( parentSuite, "MockFixture" ) );
//   CPPUT_CHECK_EXPR( containsTestNamed( parentSuite, "MockNamableFixture" ) );
//   CPPUT_CHECK_EXPR( containsTestNamed( parentSuite, "NamedMockNamableFixture" ) );
//}
//
//
//bool 
//RegistryTest::containsTestNamed( const CppUT::TestSuitePtr &suite,
//                                 const std::string &name )
//{
//   for ( int index = 0; index < suite->testCount(); ++index )
//   {
//      if ( suite->testAt(index)->name() == name )
//         return true;
//   }
//   return false;
//}
//
//
//CppUT::TestPtr 
//RegistryTest::makeTest( const std::string &testName )
//{
//   return CppUT::makeTestCase( CppTL::bind_memfn( this, 
//                                                  &RegistryTest::runTest,
//                                                  testName ),
//                               testName );
//}
//
//
//void 
//RegistryTest::runTestSuite( CppUT::TestSuitePtr suite )
//{
//   MiniTestRunner runner;
//   runner.run( suite );
//}
//
//
//void 
//RegistryTest::runTest( const std::string &testName )
//{
//   testNames_.insert( testName );
//}
//
//
//CppUT::TestFactory 
//RegistryTest::makeTestFactory( const std::string &testName )
//{
//   return CppTL::bind_memfnr( this, &RegistryTest::makeTest, testName );
//}
