#ifndef CPPUT_COMMANDLINEOPTIONS_H_INCLUDED
 #define CPPUT_COMMANDLINEOPTIONS_H_INCLUDED

# include <cpput/assertcommon.h>
# include <cpput/testing.h>
# include <cpputtools/configuration.h>


class CommandLineOptionsTest : public CppUT::TestFixture
{
public:
   CPPUT_TESTSUITE_BEGIN( CommandLineOptionsTest );
   CPPUT_TEST( testParseShortOption );
   CPPUT_TEST( testLongOption );
   CPPUT_TEST( testErrorOnBadOptionName );
   CPPUT_TEST( testParseShortOptionAdjascentParameter );
   CPPUT_TEST( testParseShortOptionNextParameter );
   CPPUT_TEST( testParserLongOptionAdjascentParameter );
   CPPUT_TEST( testParserLongOptionNextParameter );
   CPPUT_TEST( testStoreParameterValues );
   CPPUT_TEST( testValuesLookUpAsShortOrLong );
   CPPUT_TEST( testParseRequiredParameter );
   CPPUT_TEST( testCustomValidator );
   CPPUT_TEST( testEnumerationValidator );
   CPPUT_TESTSUITE_END();
   
   void setUp();

   void tearDown();

   void testParseShortOption();
   void testLongOption();
   void testErrorOnBadOptionName();
   
   void testParseShortOptionAdjascentParameter();
   void testParseShortOptionNextParameter();

   void testParserLongOptionAdjascentParameter();
   void testParserLongOptionNextParameter();

   void testValuesLookUpAsShortOrLong();

   void testStoreParameterValues();

   void testParseRequiredParameter();

   void testCustomValidator();

   void testEnumerationValidator();

private:
   CppUT::CheckerResult checkParse( CppUTTools::CommandLineOptions::OptionsDescription &description,
                                    const char *argv[] );
   CppUT::CheckerResult checkParseBadOption( CppUTTools::CommandLineOptions::OptionsDescription &description,
                                             const char *argv[],
                                             const CppTL::ConstString &badOptionName );

   enum LogLevel {
      fatal = 1,
      warning,
      info
   };


   CppUTTools::CommandLineOptions::OptionsValues *values_;
};


#endif // CPPUT_COMMANDLINEOPTIONS_H_INCLUDED
