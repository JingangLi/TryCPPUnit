#ifndef CPPUT_LIGHTTESTRUNNER_H_INCLUDED
# define CPPUT_LIGHTTESTRUNNER_H_INCLUDED

# include <cpput/forwards.h>
# include <cpput/testinfo.h>
# include <cpput/testing.h>
# include <cpptl/intrusiveptr.h>
# include <deque>

namespace CppUT {

   /* Lightweight test runner intended for unit testing CppUnit itself 
    * and the open test framework.
    */
   class LightTestRunner : private TestResultUpdater
   {
   public:
      LightTestRunner();

      virtual ~LightTestRunner();

      void addSuite( const Suite &suite );

      bool runTests();

   private: // overridden from TestResultUpdater
      virtual void addResultLog( const Json::Value &log );

      virtual void addResultAssertion( const Assertion &assertion );

   private:
      void runTestSuite( const Suite &suite );
      void runTestCase( const TestMeta &testCase );
      CppTL::ConstString getTestPath() const;
      void reportFailure( const Assertion &failure );
      void reportFailureDetail( const Json::Value &detail, 
                                int nestingLevel = 0);
      void reportLog( const Json::Value &log );

      struct ResultElement
      {
         int index_;
         bool isLog_;
      };

      typedef std::deque<Suite> SuitesToRun;
      SuitesToRun suitesToRun_;
      typedef std::deque<CppTL::ConstString> TestPath;
      TestPath testPath_;
      CppTL::StringBuffer report_;
      typedef std::deque<Json::Value> Logs;
      Logs logs_;
      typedef std::deque<Assertion> Assertions;
      Assertions assertions_;
      typedef std::deque<ResultElement> ResultElements;
      ResultElements results_;
      unsigned int testRun_;
      unsigned int testFailed_;
      unsigned int testSkipped_;
      unsigned int ignoredFailureCount_;
      unsigned int totalAssertionCount_;
   }; 

} // namespace CppUT


#endif // CPPUT_LIGHTTESTRUNNER_H_INCLUDED
