# include <cpput/assertcommon.h>
# include <cpput/message.h>
# include <cpput/translate.h>


namespace CppUT
{


CheckerResult 
pass()
{
   return CheckerResult();
}


CheckerResult 
fail()
{
   return fail( translate("Assertion Failed") );
}


CheckerResult 
fail( const LazyMessage &message )
{
   CheckerResult result;
   result.setFailed();
   result.appendMessages( message.makeMessage() );
   return result;
}


CheckerResult 
checkTrue( bool shouldBeTrue,
           const LazyMessage &message )
{
   CheckerResult result;
   if ( !shouldBeTrue )
   {
      result.setFailed();
      result.setName( "is true" );
      result.appendMessage( translate( "expression did not evaluate to true." ) );
      result.appendMessages( message.makeMessage() );
   }
   return result;
}


CheckerResult 
checkFalse( bool shouldBeFalse,
            const LazyMessage &message )
{
   CheckerResult result;
   if ( shouldBeFalse )
   {
      result.setName( "is false" );
      result.appendMessage( translate( "expression did not evaluate to false." ) );
      result.appendMessages( message.makeMessage() );
      result.setFailed();
   }
   return result;
}


CheckerResult 
checkExprTrue( bool shouldBeTrue,
               const char *expressionCode )
{
   CheckerResult result;
   if ( !shouldBeTrue )
   {
      result.setFailed();
      result.setName( "is true" );
      result.appendMessage( translate( "expression did not evaluate to true." ) );
      result.predicate( "expression" ) = expressionCode;
   }
   return result;
}


CheckerResult 
checkExprFalse( bool shouldBeFalse,
                const char *expressionCode )
{
   CheckerResult result;
   if ( shouldBeFalse )
   {
      result.setName( "is false" );
      result.appendMessage( translate( "expression did not evaluate to false." ) );
      result.predicate( "expression" ) = expressionCode;
      result.setFailed();
   }
   return result;
}


CheckerResult 
checkAssertionFail( bool assertionFailed,
                    const LazyMessage &message )
{
   CheckerResult result;
   if ( !assertionFailed )
   {
      // @todo capture assertion expression code
      result.setName( "assertion fail" );
      result.appendMessage( translate( "Assertion expression did not fail as expected." ) );
      result.appendMessages( message.makeMessage() );
      result.setFailed();
   }
   return result;
}


CheckerResult 
checkAssertionPass( bool assertionFailed,
                    const LazyMessage &message )
{
   CheckerResult result;
   if ( assertionFailed )
   {
      // @todo capture assertion expression code
      result.setName( "assertion pass" );
      result.appendMessage( translate( "Assertion expression did not pass as expected." ) );
      result.appendMessages( message.makeMessage() );
      result.setFailed();
   }
   return result;
}


void
makeComparisonFailedMessage( CheckerResult &result, 
                             const char *comparisonOperator,
                             const std::string &expected,
                             const std::string &actual )
{
   result.setFailed();
   result.setName( std::string("actual ") + comparisonOperator + " expected" );
   result.diagnostic( "actual" ) = actual;
   result.predicate( "expected") = expected;
}


CheckerResult 
checkDoubleEquals( double expected, 
                   double actual, 
                   double tolerance, 
                   const LazyMessage &message )
{
   CheckerResult result;
   double diff = expected - actual;
   if ( diff < 0 )
      diff = -diff;
   if ( diff > tolerance )
   {
      result.setFailed();
      result.appendMessages( message.makeMessage() );
      result.setName( "(actual == expected) +/- tolerance [floating-point]" );
      result.diagnostic( "actual" ) = actual;
      result.predicate( "expected" ) = expected;
      result.predicate( "tolerance" ) = tolerance;
      result.diagnostic( "difference" ) = diff;
   }
   return result;
}


void 
skipCurrentTest()
{
   TestStatus &status = TestInfo::threadInstance().testStatus();
   if ( status.hasPassed() )
      status.setStatus( TestStatus::skipped );
   throw SkipTestException();
}


} // namespace CppUT

