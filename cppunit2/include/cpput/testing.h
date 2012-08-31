#ifndef CPPUT_TESTING_INCLUDED_H
# define CPPUT_TESTING_INCLUDED_H

# include <cpput/forwards.h>
// The implementation headers are only allowed for includes at this place
// The macro is used to enfore that.
# define CPPUT_TESTING_H_PROLOG_IMPL_INCLUDES
# include <cpput/impl/assertions.h>
# include <cpput/impl/stringize.h>
# include <cpput/impl/testcase.h>
# include <cpput/impl/traits.h>
# undef CPPUT_TESTING_H_PROLOG_IMPL_INCLUDES
# include <cpptl/conststring.h> // for ResourceNames
# include <cpptl/functor.h>
# include <json/value.h> // for MetaData
# include <deque>
# include <set>
# include <string>

# ifndef CPPUT_NO_DEFAULT_STRINGIZE
#  ifndef CPPTL_NO_SSTREAM
#   include <sstream>
#  else // standard stream are not available, use the deprecated one
#   include <strstream>
#  endif // ifndef CPPTL_NO_SSTREAM
# endif // ifndef CPPUT_NO_DEFAULT_STRINGIZE

namespace CppUT {

    /** Minimized and simple interface that expose all user oriented features.
     * All implementation details are hidden behind pimpl idiom.
     * Tests discovery:
     * - list all tests
     * - greping by test name, with stars matching
     * Adding a test factory:
     * - C function
     * - Macro fixture
     * - Polymorphic fixture
     * Executing a list of test
     * - events on test execution
     * - list of all results
     * Writing tests:
     * - assertions
     * - string converter
     * - equality comparison
     *   => prefer predicates instead of operator customization (but may still be useful).
     * - using resources
     * Resources factory:
     * - declaring resources
     *
     * - Major refactoring:
     *   Takes down the registry and replace it with a test case registry
     *   => we next to register factory for test case, not for test suite
     *   => test suite becomes only organizational element likes group
     *   => test suite just have a name & a parent suite name

1) Split TestCase into
- TestMeta
  => all description stuff:
     - Test base class interface
     - requireResource()
     - factory function to create TestCase instance
- TestCase
  => all behavior stuffs:
     - virtual setUp/runTest/tearDown
2) LightTestRunner
- accept TestMeta as input, not Test

How to proceed:
Step 1: introducing the test case factory
1) Rename TestCase to TestMeta
2) Change fixture factory to accept a factory for the fixture instead of an instance
3) Introduce new TestCase class with setUp/run/tearDown
4) Change parent class of subclass of TestMeta to TestCase and
   create the TestMeta using makeFixtureTestCase

How to handle abstract test case:
- test may be defined in abstract base class
=> fixture factory is only available if fixture was instantiated.
conclusion:
Subclassing need to be reworked to:
- instantiate the test case for real when a concrete derived class exist
- store an abstract test meta in suite
- materialize on concrete class


Need to separate Suite from TestMeta.
=> break the inheritance, only test cases are runnable

Go for minimal test suite for now:
- has a name
- may have a parent test suite
- may have child test suites
- may have TestCaseMeta

How and where are suite stored ?
=> want to make path separator configurable
=> this means that static parent/child relationship must be stored
=> but interpretation occurs at run-time
=> need to keep a list of TestMeta associated to a suite



*/


/// Parent suite macro below can be implemented as follow:
/// Alls tests between BEGIN / END are registered in the specified suite.
/// @param name use '/' to separate parent and child suite
///             Starts the name with '/' to make the suite a root suite visible to all.
/// Notes: this can be implemented as a set of nested anonymous namespace:
/// using CppUT::parentSuite;
/// namespace { // CPPUT_BEGIN_SUITE( "Trading" )
///    const char *currentSuite = "Trading";
///    namespace { // CPPUT_BEGIN_SUITE( "Forex" )
///        const char *currentSuite = "Forex";
///    } // CPPUT_END_SUITE
/// } // CPPUT_END_SUITE

#if 0
// Parent Suite Declaration
#define CPPUT_SET_DEFAULT_SUITE( name )
#define CPPUT_SUITE( name )
#define CPPUT_NESTED_SUITE( name )
#define CPPUT_END_NESTED_SUITE
#define CPPUT_END_SUITE

// Parenting suite
#define CPPUT_PARENT_SUITE( parentName, childName )

// Plain C function
#define CPPUT_REGISTER_TEST_FUNCTION( testFunction )
#define CPPUT_TEST( testFunctionName )

// Lightweight fixture
#define CPPUT_TEST_LIGHT_FIXTURE( FixtureType, testName )
#define CPPUT_TEST_LIGHT_FIXTURE_WITH_SPECIFICS( FixtureType, testName, specifics )
#endif




// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// Free function based test cases
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////

/*! \brief Declare and register a simple test case in the current suite.
 * The function is declared as a static void testFunction().
 * \code
 * CPPUT_TEST_FUNCTION( myTest1 ) {
 *   CPPUT_ASSERT_TRUE( false );
 * }
 * \endcode
 */
#define CPPUT_TEST_FUNCTION( testFunctionName )             \
   static void testFunctionName();                          \
   _CPPUT_REGISTER_TEST_FUNCTION( testFunctionName );       \
   static void testFunctionName()

/*! \brief Declare and register a simple test case in the current suite.
 * The function is declared as a static void testFunction().
 * \code
 * CPPUT_TEST_FUNCTION( myTest1 ) {
 *   CPPUT_ASSERT_TRUE( false );
 * }
 * \endcode
 */
#define CPPUT_TEST_FUNCTION_WITH_META( testFunctionName, metaData )                                   \
   namespace {                                                                                        \
      class CppUT##testFunctionName##Meta : public ::CppUT::TestExtendedDataFactory                   \
      {                                                                                               \
      public:                                                                                         \
         static ::CppUT::MetaData cppUTRegisterTestCase()                                             \
         {                                                                                            \
            return ::CppUT::MetaData( #testFunctionName, metaData );                                  \
         }                                                                                            \
      };                                                                                              \
   }                                                                                                  \
   static void testFunctionName();                                                                    \
   _CPPUT_REGISTER_TEST_FUNCTION_WITH_META( testFunctionName,                                         \
                                            CppUT##testFunctionName##Meta::cppUTRegisterTestCase() ); \
   static void testFunctionName()


// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// Lightweight test fixture
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////

/*! \brief Declare and register a light fixture test case.
 * The test case is named after the function name and register in the suite named after 
 * FixtureType.
 * \code
 * struct A    // The fixture
 * {
 *     A() 
 *         : text_( "hello" )
 *     {
 *     }
 *     std::string text_;
 * };
 * CPPUT_FIXTURE_TEST( A, testInit )     // Defines a test case for the fixture.
 * {
 *     CPPUT_CHECK_TRUE( text_ == "hello" );   // Directly access fixture members.
 * }
 * \endCode
 */
#define CPPUT_FIXTURE_TEST( FixtureType, testFunction )                 \
   _CPPUT_FIXTURE_TEST_COMMON( FixtureType, testFunction )              \
                              CPPUT_CURRENT_SUITE() ) );                \
                                                                        \
   void FixtureType##testFunction::run()


/*! \brief Declare and register a light fixture test case with extended data.
 * The test case is named after the function name and register in the suite named after 
 * FixtureType.
 * \code
 * struct A    // The fixture
 * {
 *     A() 
 *         : text_( "hello" )
 *     {
 *     }
 *     std::string text_;
 * };
 * CPPUT_TEST_LIGHT_FIXTURE_WITH_SPECIFICS( A, testInit, (timeout(0.2), group("init")) )
 * {
 *     CPPUT_CHECK_TRUE( text_ == "hello" );   // Directly access fixture members.
 * }
 * \endCode
 */
#define CPPUT_FIXTURE_TEST_WITH_META( FixtureType, testFunction, metaData ) \
   _CPPUT_FIXTURE_TEST_COMMON( FixtureType, testFunction )              \
                              metaData,                                 \
                              CPPUT_CURRENT_SUITE() ) );                \
                                                                        \
   void FixtureType##testFunction::run()

#define CPPUT_EXPORTED_FIXTURE_SUITE( FixtureType, exportedVariableName )  \
   extern ::CppUT::SuiteMeta exportedVariableName(                         \
   CPPUT_CURRENT_SUITE().makeNestedSuite( #FixtureType ) )



// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// Abstract/Polymorphic test fixture implementation macros
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////

# define CPPUT_TESTSUITE_BEGIN( FixtureType )                              \
   _CPPUT_TESTSUITE_COMMON( FixtureType )                                  \
   _CPPUT_TESTSUITE_BASE_RUN( FixtureType )                                \
   _CPPUT_TESTSUITE_COMMON_CONCRETE( FixtureType )                         \

# define CPPUT_TESTSUITE_ABSTRACT_BEGIN( FixtureType )                     \
   _CPPUT_TESTSUITE_COMMON( FixtureType )                                  \
   _CPPUT_TESTSUITE_BASE_RUN( FixtureType )                                \
   _CPPUT_TESTSUITE_COMMON_ABSTRACT( FixtureType )

# define CPPUT_TESTSUITE_ABSTRACT_EXTEND( FixtureType, ParentFixtureType ) \
   _CPPUT_TESTSUITE_COMMON( FixtureType )                                  \
   _CPPUT_TESTSUITE_DERIVED_RUN( FixtureType, ParentFixtureType )          \
   _CPPUT_TESTSUITE_COMMON_ABSTRACT( FixtureType )                         \
   _CPPUT_TESTSUITE_COMMON_EXTEND( FixtureType, ParentFixtureType );

# define CPPUT_TESTSUITE_EXTEND( FixtureType, ParentFixtureType )          \
   _CPPUT_TESTSUITE_COMMON( FixtureType )                                  \
   _CPPUT_TESTSUITE_DERIVED_RUN( FixtureType, ParentFixtureType )          \
   _CPPUT_TESTSUITE_COMMON_CONCRETE( FixtureType )                         \
   _CPPUT_TESTSUITE_COMMON_EXTEND( FixtureType, ParentFixtureType );

# define CPPUT_TESTSUITE_TEST( testMethod )                                \
      FixtureTestFactory::addTest( suite, factory, #testMethod,            \
                                   &CppUTSelfType :: testMethod )

# define CPPUT_TESTSUITE_TEST_WITH_META( testMethod, theMetaData )         \
      FixtureTestFactory::addTest( suite, factory, #testMethod,            \
                                   &CppUTSelfType :: testMethod,           \
                                   theMetaData )

# define CPPUT_TESTSUITE_END()                                             \
   }                                                                       \
   private:



// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// Test Suite declarations
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////

///*! \ingroup group_testregistry
class CPPUT_API Suite
{
   friend class Impl::RegistryImpl;
   friend class SuiteMeta;
public:
   /// Creates an invalid suite.
   Suite();

   /// Creates a new orphaned suite
   Suite( const std::string &name );

   //Suite( const Suite &other );
   ~Suite();

   //Suite &operator =( const Suite &other );

   Suite parent() const;

   bool isValid() const;

   std::string name() const;

   int nestedSuiteCount() const;
   Suite nestedSuiteAt( int index ) const;
   Suite nestedSuiteByName( const std::string &name ) const;
   Suite makeNestedSuite( const std::string &name );

   int testCaseCount() const;
   const TestMeta *testCaseAt( int index ) const;

   void add( const TestMeta &testCase );

   bool operator <( const Suite &other ) const;
   bool operator ==( const Suite &other ) const;
   unsigned int hash() const;

private:
   Suite( Impl::SuiteImpl *suite );
   Impl::SuiteImpl *impl_;
};


/*! \ingroup group_testregistry
 * \brief Static Suite description handle.
 */
class CPPUT_API SuiteMeta : public Suite
{
   friend class Impl::RegistryImpl;
public:
   ~SuiteMeta();

private:
   explicit SuiteMeta( Impl::SuiteImpl *impl );
};


/*! \ingroup group_testregistry
 * Static registry for all tests.
 * Any suite or test added to the registry by a dynamic library that contains test should
 * be removed using remove(). Helper macros take carre of this.
 */
class CPPUT_API Registry
{
public:
   static Suite getRootSuite();

   static std::string dump();
};


/*! \ingroup group_testregistry
 * \brief Set the default test suite.
 * This is a convenience function to be used in an header. It defined the default
 * parent suite for all tests including that headers. This is typically used to
 * group all tests of a given library under the same parent suite.
 *
 * \code
 * // MyLib/testing.h
 * #ifndef MYLIB_TESTING_H_INCLUDED
 * # define MYLIB_TESTING_H_INCLUDED
 * # include <cpput/testing.h>
 * CPPUT_SET_DEFAULT_SUITE( "MyLib" );
 * #endif
 * // MyLib/test1.cpp
 * #include "testing.h"
 * // TestCase "test1" will be placed into the suite "MyLib"
 * CPPUT_TEST( test1 ) { // ...
 * }
 * // MyLib/test2.cpp
 * #include "testing.h"
 * // TestCase "test1" will be placed into the suite "MyLib"
 * CPPUT_TEST( test1 ) { // ...
 * }
 * \endcode
 * \warning You must use CPPUT_NESTED_SUITE if you want to put test in a child suite of
 *          MyLib.
 */
#define CPPUT_SET_DEFAULT_SUITE( name )                              \
   namespace {                                                       \
      _CPPUT_DECLARE_ROOT_SUITE( name, makeAndSetDefaultRootSuite ); \
   }

/*! \ingroup group_testregistry
 * \brief Returns the suite test will register in the context were the macro occurs.
 * Notes: this is mostly an implementation detail. Should not need this unless you are
 * creating  some new kind of test case factor like CPPUT_TEST, CPPUT_FIXTURE_TEST...
 * @return This macro expand into an expression of type CppUT::SuiteMeta.
 */
#define CPPUT_CURRENT_SUITE() \
   ::CppUT::Impl::currentSuite()


/*! \ingroup group_testregistry
 * \brief Declares the root parent suite of tests until CPPUT_END_SUITE.
 */
#define CPPUT_SUITE( name )                                                \
   _CPPUT_DECLARE_ROOT_SUITE( name, makeNestedSuiteInDefaultRootSuite );   \
   namespace

#define CPPUT_SUITE_REGISTER( FixtureType )                    \
   static ::CppUT::StaticSuite *                               \
      CPPTL_MAKE_UNIQUE_NAME(cpputRegisterFixtureSuite) =      \
      ::CppUT::Impl::reparentSuite( CPPUT_CURRENT_SUITE(),     \
                                    FixtureType :: suite() )


// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// Custom test cases declarations
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////

/*! This class represents the data about a test.
 * \ingroup group_testcases
 *
 * It is the base class of TestMeta.
 */
class CPPUT_API MetaData
{
public:
   explicit MetaData( const std::string &name );

   MetaData( const std::string &name,
             const TestExtendedData &metaDataFactory );

   virtual ~MetaData();

   void setDescription( const std::string &description );

   std::string description() const;

   std::string name() const;

   void setTimeOut( double timeOutInSeconds );

   double timeOut() const;

   void addToGroup( const std::string &groupName );

   int groupCount() const;

   std::string groupAt( unsigned int index ) const;

   Json::Value &input();

   const Json::Value &input() const;

private:
   Json::Value info_;
};



/*! \brief Helper to set test extended data (description, time-out, dependencies...)
 * \ingroup group_testfixture
 * 
 * Example of usage:
 * \code
 * class SomeClass : public TestExtendedDataFactory {
 * public:
 *    static void enrichTest( MetaData &test ) {
 *       CppUT::TestExtendedDataHelper( test )( describe( "Test conversion" ),
 *                                              timeOut( 30.0 ),
 *                                              depends( "testInit" ) );
 *    }
 * };
 * \endcode
 *
 * The above code sample is equivalent to:
 *
 * \code
 * static void enrichTest( MetaData &test ) {
 *    test.setDescription( "Test conversion" );
 *    test.setTimeOut( 30.0 );
 *    test.setDependenciesFromPackedString( "testInit" );
 * }
 * \endcode
 *
 * The only purpose of this class is to make it easy to set test properties without
 * direct access to the test object. It is used to implement the macro
 * \link CPPUT_TEST_WITH_SPECIFICS() \endlink.
 * \ingroup group_testfixture
 */
class TestExtendedDataHelper
{
public:
   TestExtendedDataHelper( MetaData &test );
   virtual ~TestExtendedDataHelper();

   MetaData &operator()( const TestExtendedData &data ) const;

private:
   MetaData &test_;
};



/*! \ingroup group_testfixture
 */
class CPPUT_API TestExtendedData
{
public:
   virtual ~TestExtendedData();

   virtual void apply( MetaData &test ) const = 0;

   Impl::TestExtendedDataList operator ,( const TestExtendedData &other ) const;
};



/*! \ingroup group_testfixture
 */
class CPPUT_API TestExtendedDataFactory
{
public:
   virtual ~TestExtendedDataFactory();

   static Impl::DescriptionData describe( const std::string &description );

   static Impl::TimeOutData timeOut( double timeOutInSeconds );

   static Impl::DependenciesData depends( const std::string &dependency );

   static Impl::GroupData group( const std::string &groupName );
};


typedef CppTL::Functor0R<TestCase *> TestCaseFactory;


/*! \brief An abstract test case that can be run.
 * \ingroup group_testcases
 */
class CPPUT_API TestCase : public TestExtendedDataFactory
{
   friend class TestMeta;
public:
   TestCase();

   virtual ~TestCase();

   virtual void setUp();

   virtual void run() = 0;

   virtual void tearDown();

   //void setUpTestResources();

   //void tearDownTestResources();
};


/// @todo figure out how to move this to impl/testing.h
namespace Impl {

   template<class FunctorType>
   class FunctorTestCase : public TestCase
   {
   public:
      static TestCase *factory( FunctorType run )
      {
         return new FunctorTestCase<FunctorType>( run );
      }

      FunctorTestCase( FunctorType run )
         : run_( run )
      {
      }

   public: // overriden from TestCase
      virtual void run()
      {
         run_();
      }

   private:
      FunctorType run_;
   };

} // end namespace Impl {


/*! \brief A test case that can be run.
 * \ingroup group_testcases
 */
class CPPUT_API TestMeta : public MetaData
{
public:
   TestMeta( const TestCaseFactory &factory,
             const std::string &name );

   TestMeta( TestCaseFactoryFn factory,
             const std::string &name );

   TestMeta( const TestCaseFactory &factory,
             const MetaData &metaData );

   TestMeta( TestCaseFactoryFn factory,
             const MetaData &metaData );

   /*! \brief Run the test case using the default ExceptionGuard.
    *
    * The default exception guard only detect 
    * 
    */
   bool runTest() const;

   /*! \brief Run the test case using the specified ExceptionGuard.
    *
    * Call TestInfo::startNewTest() before starting the test.
    * Then, setUp(), run() and finally tearDown() are called. run() is only called
    * if setUp() did not failed (no assertion or exception failed during setUp()
    * call).
    *
    * \param guardsChain Call to setUp(), run() and tearDown() are made through the
    *                    specified exception guard. This allow capturing and extracting
    *                    information for failure report from exception 
    *                    that do not subclass std::exception.
    */
   bool runTest( const ExceptionGuard &guardsChain ) const;

private:
   TestCaseFactory factory_;
};


// Implementation Notes:
// Be wary of makeTestCase overloading. There is an optimizer bug in MSVC6 that cause it
// to recursively call makeTestCase(CppTL::Functor0) when such an overloading exist
// and makeTestCase is called with a functor. This has been worked around by removing that 
// overloading.


/*! \brief Creates a TestMeta with the specified name and run function.
 * \ingroup group_testcases
 */
TestMeta CPPUT_API makeTestCase( void (*run)(),
                                 const std::string &name );


/*! \brief Creates a TestMeta with the specified name, run function and metaData.
 * \ingroup group_testcases
 */
TestMeta CPPUT_API makeTestCase( void (*run)(),
                                 const MetaData &metaData );


/*! \brief Creates a TestMeta with the specified name and run functor.
 * \ingroup group_testcases
 * FunctorType must be copyable and callable with no parameter.
 */
template<class FunctorType>
TestMeta makeTestCase( FunctorType run,
                       const std::string &name )
{
   return TestMeta( CppTL::bind_cfnr( &Impl::FunctorTestCase<FunctorType>::factory, run ), name );
}


/*! \brief Creates a TestMeta with the specified name and run functor.
 * \ingroup group_testcases
 * FunctorType must be copyable and callable with no parameter.
 */
template<class FunctorType>
TestMeta makeTestCase( FunctorType run,
                       const MetaData &metaData )
{
   return TestMeta( CppTL::bind_cfnr( &Impl::FunctorTestCase<FunctorType>::factory, run ), metaData );
}


/*! \brief Creates a TestMeta with the specified name that will always fail.
 * \ingroup group_testcases
 *
 * The test case run body simply call CPPUT_FAIL() with the specified message.
 *
 * This is useful for example when a Suite is created from input data 
 * and those an invalid or inaccessible. A failing test case can be added to
 * the suite to report the error.
 */
TestMeta CPPUT_API makeFailingTestCase( const std::string &name,
                                        const Message &message );




/** Exception handler context.
 * Used to report exception details and chain call to the next exception handler,
 * the protected function or rethrow a previously caught exception.
 * \see ExceptionHandlerFn, ExceptionGuard.
 */
class ExceptionGuardContext
{
public:
   virtual ~ExceptionGuardContext();

   /// Call either the next guard in the chain, or the protected function.
   /// Notes: Assertions that check if an expression does not throw
   /// actually use the generic catch handler idiom. That is the exception
   /// is caught by the assertion macro using catch ( ... ) and will be
   /// rethrown by chainCall().
   virtual void chainCall() = 0;

   /// Indicates that a std::exception was raised and captures the fault detail.
   /// @param e Fault exception type and message are captured from this exception object.
   /// @param typeHint Hint about the exception type used if RTTI is disabled.
   void setStdExceptionFault( const std::exception &e, 
                              const char *typeHint = "std:exception" );

   /// Indicates that an exception was raised and captures the fault details.
   /// @param faultType Type of the exception that was raised.
   /// @param message Message associated to the exception.
   virtual void setExceptionFault( const char *faultType, 
                                   const std::string &message ) = 0;

   /// Indicates that a fault occurred but detail have already been added to result.
   /// Typically called when an AbortingAssertionException is caught. Assertions
   /// are reported as soon as they are thrown.
   virtual void setFaultAlreadyHandled() = 0;
};

/*! \brief Protects exception leaking from a function call.
 * The function is expected to call context.chainCall() while surrounding it with
 * try/catch statement to handle specific exception types.
 * Notes that context.chainCall() may either call a function that may throw an exception,
 * or rethrow an exception that was previously caught to extract fault details.
 *
 * A typical implementation would be:
 * \code
 * try 
 * {
 *    context.chainCall();
 * }
 * catch ( const MyException &e )
 * {
 *    context.setExceptionFault( "MyException", e.getMessage() );  
 * }
 * \endcode
 * @see Context.
 */
typedef void (*ExceptionHandlerFn)( ExceptionGuardContext &context );


/*! Tests setUp(), tearDown(), run() call are protected by a ExceptionGuard.
 *
 * This guard chain ensures that any exceptions thrown by the call is caught and properly handled.
 * The default guard chain handles AbortingAssertionException (thrown when an assertion fails),
 * std::exception, and any other exception.
 * 
 * An ExceptionHandlerFn can be added to the guard chain to handle exception type unknown
 * to the test framework which are not derived from std::exception (MFC CException * for example).
 * \ingroup group_testcases
 * \see ExceptionHandlerFn, ExceptionGuardContext.
 * \code
 * static void mfcExceptionHandler( ExceptionGuardContext &context )
 * {
 *    try
 *    {
 *       context.chainCall();
 *    }
 *    catch ( CException *e )
 *    {
 *       // Extracts the exception detail
 *       TCHAR cause[512];
 *       e.GetErrorMessage( cause, sizeof(cause) );
 *       CString message( cause );
 *       // Give the exception to the handler context
 *       // This will add a fault event to the current test with the specified detail.
 *       context.setExceptionFault( "CException", message );
 *       // MFC requires the Delete method to be called for clean-up
 *       e->Delete();
 *    }
 * }
 * \endcode
 */
class CPPUT_API ExceptionGuard
{
public:
   typedef std::deque<ExceptionHandlerFn> Guards;

   ExceptionGuard();

   void add( ExceptionHandlerFn exceptionHandler );

   void removeLastAdded();

   /// Returns \c true if the test did not fail (skipped or passed TestStatus).
   bool protect( CppTL::Functor0 test ) const;

   /// Rethrows the last exception caught and handle it.
//   void rethrowExceptionAndHandle() const;

private:
   Guards guards_;
};




// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// Equality test customization for testing
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////

# ifndef CPPUT_NO_DEFAULT_EQUALITY

/*! \ingroup group_assertions_support
    */
   template<class AType, class BType>
   struct EqualityTraits
   {
      static bool equals( const AType &a, const BType &b )
      {
         return a == b;
      }
   };

   /*! \ingroup group_assertions_support
    * @todo change implementation to match stringize.
    * @todo delegate to compareTest() => <0, == 0, > 0. Would allow comparison assertions.
    */
   template<typename AType, typename BType>
   bool equalityTest( const AType &a, const BType &b )
   {
      return Impl::testIsEqual( a, b, isEqual( &a, &b ) );
   }
  

# endif // CPPUT_NO_DEFAULT_EQUALITY









// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// String conversion
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////

// Defines the convertToString() & stringize() function use by assertions to output a value.


/* How to:
 * - define a conversion function for a string type:
 *   overload the function std::string getStdString( MyStringType )
 *
 *   namespace CppUT { 
 *   inline std::string getStdString( const MyStringType &s ) {
 *      return s.str();
 *   } }
 *
 */



// 1) Is it a string ?
// std::string convertToString( ptr ) => Yes, convert to string & add quotes.
// NoToStringConversion convertToString( .. )
// stringize( impl::stringize( value, convertToString( ptr ) ) );
// 2) Is there an overloaded toString() function for that type
// std::string toString( ptr ); => Yes, convert to string using the function
// NoToStringConversion toString( ... );
// 3) Fall back DefaultStringizer.

   /*! \brief A generic functor that can be used to convert a value into a string.
    * \ingroup group_assertions_support
    * It is used as default argument by function template that allow the user to
    * pass a specific functor to convert data to string.
    * \warning This class template should not be specialized. See stringize() for detail.
    */
   template<class ValueType>
   struct DefaultStringizer
   {
      std::string operator()( const ValueType &value ) const
      {
         return stringize( value );
      }
   };

   /*! \ingroup group_assertions_support
    */
   template<class ValueType>
   struct RefStringizer
   {
      std::string operator()( ValueType value ) const
      {
         return stringize( value );
      }
   };

   /*! \ingroup group_assertions_support
    */
   template<class ValueType>
   struct DerefStringizer
   {
      std::string operator()( ValueType value ) const
      {
         return stringize( *value );
      }
   };

// ------------------- convertToString -------------------------------
// User should overload getStdString() to support their own string types.
//
// IMPORTANT: to handle compiler that do not support function template ordering
// (CPPTL_NO_FUNCTION_TEMPLATE_ORDERING), such as vc++ 6.0.
// getStdString() must never be called with a 'by value' parameter.
// Passing by value would result in undefined behavior for non string type parameter
// 'eaten' by NotConvertibleToStdString convertToString( ... ).
// In practice, this means that a templated function accepting const ref parameters
// should be the caller. For example:
//
// template<class StringType1, class StringType2>
// void checkRegExMatched( const StringType1 &pattern,   // <= the const ref here ensure that
//                         const StringType2 &str )      // convertToString will only be passed reference.
// {
//    RegEx regex( CppUT::convertToString(&pattern ) );
//    CppUT::checkTrue( regex.matched( CppUT::convertToString( &str ) ) );
// }

   /*! \ingroup group_assertions_support
    */
   inline std::string getStdString( const char *cstr )
   {
      return std::string( cstr );
   }

   /*! \ingroup group_assertions_support
    */
   inline std::string getStdString( const std::string &s )
   {
      return s;
   }

   /*! \ingroup group_assertions_support
    */
   inline std::string getStdString( const CppTL::ConstString &s )
   {
      return s.c_str();
   }

#ifdef CPPTL_NO_FUNCTION_TEMPLATE_ORDERING
   /*! \ingroup group_assertions_support
    */
   inline Impl::NotConvertibleToStdString getStdString( ... )
   {
      return Impl::NotConvertibleToStdString();
   }
#else
   /*! \ingroup group_assertions_support
    */
   template<class T>
   inline Impl::NotConvertibleToStdString getStdString( const T & )
   {
      return Impl::NotConvertibleToStdString();
   }
#endif

   /*! \ingroup group_assertions_support
    */
   template <class StringType>
   inline std::string convertToString( const StringType &s )
   {
      return getStdString( s );  // if you get a compilation error on this call, then getStdString() has not been overloaded for your string type.
   }


// ------------------- stringize -------------------------------
// If CPPUT_NO_DEFAULT_STRINGIZE is defined, the user is expected 
// to provide a function which accept the following types:
// template<class T>
// std::string defaultStringize( const T &value );

#ifdef CPPTL_NO_FUNCTION_TEMPLATE_ORDERING
   inline Impl::NoToStringOverload toString( ... )
   {
      return Impl::NoToStringOverload();
   }
#else
   template<class T>
   inline Impl::NoToStringOverload toString( const T & )
   {
      return Impl::NoToStringOverload();
   }
#endif

   /*! \ingroup group_assertions_support
    */
   template<typename ValueType>
   std::string stringize( const ValueType &value )
   {
      return Impl::stringize( value, getStdString(&value) );
   }


# ifndef CPPUT_NO_DEFAULT_STRINGIZE

   /*! \ingroup group_assertions_support
    */
   template<typename ValueType>
   std::string defaultStringize( const ValueType &value )
   {
#  ifndef CPPTL_NO_SSTREAM
         std::ostringstream os;
         os  <<  value;
         return os.str();
#  else // standard stream are not available, use the deprecated one
         std::ostrstream os;
         os  <<  value;
         return std::string( os.str(), os.pcount() );
#  endif
   }

#  endif






} // namespace CppUT

// The following header must only be included in this place.
# define CPPUT_TESTING_H_EPILOG_IMPL_INCLUDES
# include <cpput/impl/testcaseepilog.h>
# undef CPPUT_TESTING_H_EPILOG_IMPL_INCLUDES

#endif // CPPUT_TESTING_INCLUDED_H
