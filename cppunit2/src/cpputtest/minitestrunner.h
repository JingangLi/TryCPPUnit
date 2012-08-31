#ifndef CPPUTTEST_MINITESTRUNNER_H_INCLUDED
# define CPPUTTEST_MINITESTRUNNER_H_INCLUDED

# include <cpput/testing.h>

class MiniTestRunner
{
public:
   MiniTestRunner()
   {
      reset();
   }

   void run( const CppUT::TestMeta &test )
   {
      // This override prevent errors occuring during testing to
      // count has failure of the test running that testing.
      CppUT::TestInfo::ScopedContextOverride override;
      if ( test.runTest() )
         ++passed_;
      else
         ++failed_;
      ++tested_;
   }

   void run( const CppUT::Suite & suite )
   {
      if ( !suite.isValid() )
      {
         return;
      }

      for ( int suiteIndex =0; suiteIndex < suite.nestedSuiteCount(); ++suiteIndex )
      {
         run( suite.nestedSuiteAt( suiteIndex ) );
      }

      for ( int index =0; index < suite.testCaseCount(); ++index )
      {
         const CppUT::TestMeta *test = suite.testCaseAt( index );
         if ( test != 0 )
         {
            run( *test );
         }
      }
      ++suite_;
   }

   void reset()
   {
      suite_ = 0;
      tested_ = 0;
      passed_ = 0;
      failed_ = 0;
   }

   int suite_;
   int tested_;
   int passed_;
   int failed_;

private:

};

#endif // CPPUTTEST_MINITESTRUNNER_H_INCLUDED
