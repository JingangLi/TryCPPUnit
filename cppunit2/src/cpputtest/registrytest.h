#ifndef CPPUT_REGISTRYTEST_H_INCLUDED
 #define CPPUT_REGISTRYTEST_H_INCLUDED

# include <cpptl/sharedptr.h>
# include <cpput/assertcommon.h>
# include <cpput/testing.h>
# include <set>


class RegistryTest : public CppUT::TestFixture
{
public:
   CPPUT_TESTSUITE_BEGIN( RegistryTest );
   CPPUT_TEST( testCreateDefaultTests );
   CPPUT_TEST( testAddTestToDefault );
   CPPUT_TEST( testAddTestToNamed );
   CPPUT_TEST( testAddAndRemoveChild );
   CPPUT_TEST( testAutoRemoveByTestFactoryId );
   CPPUT_TEST( testStaticRegistration );
   CPPUT_TESTSUITE_END();
   
   void setUp();

   void tearDown();

   void testCreateDefaultTests();

   void testAddTestToDefault();

   void testAddTestToNamed();

   void testAddAndRemoveChild();

   void testAutoRemoveByTestFactoryId();

   void testStaticRegistration();

private:
   CppUT::TestPtr makeTest( const std::string &testName );

   void runTest( const std::string &testName );

   void runTestSuite( CppUT::TestSuitePtr suite );

   CppUT::TestFactory makeTestFactory( const std::string &testName );

   static bool containsTestNamed( const CppUT::TestSuitePtr &suite,
                                  const std::string &name );

   std::set<std::string> testNames_;

   CppTL::SharedPtr<CppUT::Registry> registry_;
};


#endif // CPPUT_REGISTRYTEST_H_INCLUDED
