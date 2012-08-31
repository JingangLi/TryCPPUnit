#ifndef CPPUT_PARAMETRIZEDSOURCE_H_INCLUDED
# define CPPUT_PARAMETRIZEDSOURCE_H_INCLUDED

# include <cpput/config.h>
# include <cpput/testing.h>
# include <cpptl/sharedptr.h>
# include <map>
# include <stdexcept>
# include <string>
# include <sstream>
# include <vector>


namespace CppUT {

class ParameterNode;
class Test;
class TestParameterSource;
class SuiteParameterSource;


typedef CppTL::SharedPtr<ParameterNode> ParameterNodePtr;


class TestParameterFactory
{
public:
   virtual ~TestParameterFactory()
   {
   }

   virtual int testCaseCount() = 0;

   virtual std::string testCaseNameSuffix( int index ) = 0;

   virtual ParameterNodePtr testCaseParameter( int index ) = 0;
};

class TestParameterFactoryImpl : public TestParameterFactory
{
public:
   int testCaseCount();

   std::string testCaseNameSuffix( int index );

   ParameterNodePtr testCaseParameter( int index );

   void addTestCase( const std::string &nameSuffix,
                     const ParameterNodePtr &parameterNode );

private:
   typedef std::pair<std::string,ParameterNodePtr> TestData;
   typedef std::vector<TestData> Tests;
   Tests tests_;
};

typedef CppTL::SharedPtr<TestParameterFactory> TestParameterFactoryPtr;
typedef CppTL::Functor1R<TestParameterFactoryPtr, std::string> SuiteParameterFactory;


class ParameterSourceRegistry
{
public:
   static ParameterSourceRegistry &instance();

   void addSuiteSource( const std::string &suiteName,
                        const std::string &complementaryName,
                        SuiteParameterFactory suiteParameterFactory );

   SuiteParameterFactory getSuiteParameterFactory( 
                        const std::string &suiteName,
                        const std::string &complementaryName = "" ) const;

private:
   typedef std::pair<std::string,std::string> SuiteKey;

   typedef std::map<SuiteKey, SuiteParameterFactory> SuitePaths;
   SuitePaths suitePath_;
};
/*
class ParametrizedTestCase : public TestCase
{
public:
};
*/



class ParameterNode
{
public:
   virtual ~ParameterNode()
   {
   }

   void throwOperationNotSupported( const std::string &operationName ) const;

   virtual std::string nodeName() const = 0;

   virtual int childCount() const = 0;

   virtual ParameterNodePtr childAt( int index ) const = 0;

   virtual ParameterNodePtr childNamed( const std::string &nodeName ) const;

   virtual std::string text() const = 0;

//   virtual std::wstring unicodeText() const = 0;
};


/*

namespace Impl
{
   struct BindStringStreamsFunctor
   {
      BindStringStreamsFunctor( const std::string &str1,
                                const std::string &str2,
                                ::CppUT::Functor2<std::istringstream &
                                                 ,std::istringstream &> run )
         : str1_( str1 )
         , str2_( str2 )
         , run_( run )
      {
      }

      void operator()()
      {
         std::istringstream is1( str1_ );
         std::istringstream is2( str2_ );
         run_( is1, is2 );
      }

      std::string str1_;
      std::string str2_;
      ::CppUT::Functor2<std::istringstream &,std::istringstream &> run_;
   };

} // namespace Impl


TestCasePtr makeSimpleParametrizedTestCase( const std::string &testName,
                                 Functor0 setUp,
                                 Functor2<std::istringstream &,std::istringstream &> run,
                                 Functor0 teardown )
{
   ParameterSourceRegistry::instance().getTestParameterFactory( testName

   Functor0 runFn = makeFn0( Impl::BindStringStreamsFunctor( inputText, expectedText, run ) );
   return makeTestCase( setUp, runFn, teardown, testName );
}
*/


/*
 *	Parametrized tests
 * Two kinds:
 * Suite => global settings for the fixture indicates how to get parameters
 * Case: => settings are specified for the test case, along with the method name.
 * Dismiss Case, too complex and can be easily implemented using a suite source that
 * do some dispatching.
 *
 * Parameter source is associated using:
 * - the suite name
 * - an additional optional ambiguity riser.
 *
 *
 */




} // namespace CppUT


/*
#define CPPUT_TEST_FLAT_PARAMETER( testMethod )                              \

# define CPPUT_TEST( testMethod )                                            \
      fixture = fixtureFactory();                                            \
      suite->add( ::CppUT::makeFixtureTestCase( fixture,                     \
                   ::CppUT::makeMemFn0<CppUT_ThisType>( fixture,             \
                                       &CppUT_ThisType::testMethod ),        \
                   #testMethod ) )
*/


#endif // CPPUT_PARAMETRIZEDSOURCE_H_INCLUDED
