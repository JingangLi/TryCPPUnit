#include <cpput/assertstring.h>
#include <algorithm>

namespace CppUT {

CheckerResult 
checkStdStringStartsWith( const std::string &string,
                          const std::string &pattern,
                          const Message &message )
{
   CheckerResult result;
   if ( string.substr( 0, pattern.length() ) != pattern )
   {
      result.setFailed();
      result.appendMessages( message );
      result.appendMessage( translate( "String does not start with the expected pattern." ) );
      result.setName( "actual.startsWith( pattern )" );
      result.predicate("pattern") = CppTL::quoteMultiLineString( pattern ).c_str();
      result.diagnostic("actual") = CppTL::quoteMultiLineString( string ).c_str();
   }
   return result;
}


CheckerResult 
checkStdStringEndsWith( const std::string &string,
                        const std::string &pattern,
                        const Message &message )
{
   CheckerResult result;
   if ( string.length() < pattern.length()  ||
        string.substr( string.length() - pattern.length() ) != pattern )
   {
      result.setFailed();
      result.appendMessages( message );
      result.appendMessage( translate( "String does not end with the expected pattern." ) );
      result.setName( "actual.endsWith( pattern )" );
      result.predicate("pattern" ) = CppTL::quoteMultiLineString( pattern ).c_str();
      result.diagnostic("actual") = CppTL::quoteMultiLineString( string ).c_str();
   }
   return result;
}


CheckerResult 
checkStdStringContains( const std::string &string,
                        const std::string &pattern,
                        const Message &message )
{
   CheckerResult result;
   if ( string.find( pattern ) == std::string::npos )
   {
      result.setFailed();
      result.appendMessages( message );
      result.appendMessage( translate( "String does not contain the expected pattern." ) );
      result.setName( "actual.contains( pattern )" );
      result.predicate( "pattern" ) = CppTL::quoteMultiLineString( pattern ).c_str();
      result.diagnostic( "actual" ) = CppTL::quoteMultiLineString( string ).c_str();
   }
   return result;
}


CheckerResult 
checkStdStringEquals( const std::string &expected,
                      const std::string &actual,
                      const Message &message )
{
   CheckerResult result;
   if ( actual != expected )
   {
      unsigned int differenceIndex = 0;
      while ( differenceIndex < actual.length()  &&  
              differenceIndex < expected.length()  &&
              actual[differenceIndex] == expected[differenceIndex] )
      {
         ++differenceIndex;
      }

      std::string common = expected.substr( 0, differenceIndex );
      std::string actualTail = actual.substr( differenceIndex );
      std::string expectedTail = expected.substr( differenceIndex );

      result.setFailed();
      result.appendMessages( message );
      result.appendMessage( translate( "Strings are not equal." ) );
      result.setName( "actual == expected" );
      // @todo make it so that expected/actual are not displayed
      result.predicate( "expected" ) = CppTL::quoteMultiLineString( expected ).c_str();
      result.diagnostic( "actual" ) = CppTL::quoteMultiLineString( actual ).c_str();
      if ( !common.empty() )
         result.diagnostic( "common") = CppTL::quoteMultiLineString( common ).c_str();
      result.diagnostic("divergence index") = stringize( differenceIndex ).c_str();
      result.diagnostic("expected suffix") = CppTL::quoteMultiLineString( expectedTail ).c_str();
      result.diagnostic("actual suffix") = CppTL::quoteMultiLineString( actualTail ).c_str();
   }
   return result;
}

} // namespace CppUT
