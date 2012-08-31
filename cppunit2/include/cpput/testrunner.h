#ifndef CPPUT_TESTRUNNER_H_INCLUDED
# define CPPUT_TESTRUNNER_H_INCLUDED

# include <cpput/forwards.h>
# include <opentest/testrunner.h>
# include <cpptl/intrusiveptr.h>
# include <map>


namespace CppUT {

class CPPUT_API TestRunner : public OpenTest::TestRunner
{
public:
   typedef std::map<OpenTest::TestId,Test *> DeclaredTests;

   TestRunner();

   void setRootSuite( const AbstractTestSuitePtr &suite );

   AbstractTestSuitePtr rootSuite() const;

public: // overridden from OpenTest::TestRunner
   void declareTests( OpenTest::TestDeclarator &declarator );

   void runTests( const OpenTest::TestPlan &testPlan,
                  OpenTest::TestRunTracker &tracker,
                  const OpenTest::Properties &configuration );
	
private:
   class TestDeclaratorVisitor;
   class TestResultUpdaterImpl;

   void sendTestStatus( const OpenTest::TestPlanEntry &entry,
                        OpenTest::TestRunTracker &tracker );

   AbstractTestSuitePtr suite_;
   DeclaredTests tests_;
};



} // namespace CppUT


#endif // CPPUT_TESTRUNNER_H_INCLUDED
