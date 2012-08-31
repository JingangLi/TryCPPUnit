#include "commandlineoptionstest.h"
#include <cpput/assertstring.h>
//#include <cpputtools/value.h>

using namespace CppUTTools;


#define LOCAL_CHECK_PARSE \
   CPPUT_BEGIN_ASSERTION_MACRO() \
   checkParse

#define LOCAL_CHECK_PARSE_BAD_OPTION \
   CPPUT_BEGIN_ASSERTION_MACRO() \
   checkParseBadOption

void 
CommandLineOptionsTest::setUp()
{
   values_ = 0;
}


void 
CommandLineOptionsTest::tearDown()
{
   delete values_;
}


CppUT::CheckerResult
CommandLineOptionsTest::checkParse( CommandLineOptions::OptionsDescription &description,
                                    const char *argv[] )
{
   CppUT::CheckerResult result; // What can make the test fails ???
   delete values_;
   values_ = new CommandLineOptions::OptionsValues();
   std::vector<const char *> commandLine;
   commandLine.push_back( "test_app.exe" );
   while ( *argv )
      commandLine.push_back( *argv++ );

   CommandLineOptions::parseCommandLine( commandLine.size(), &commandLine[0], description, *values_ );
   values_->validate( description );
   values_->store( description );
   return result;
}


CppUT::CheckerResult
CommandLineOptionsTest::checkParseBadOption( CommandLineOptions::OptionsDescription &description,
                                             const char *argv[],
                                             const CppTL::ConstString &badOptionName )
{
   CppUT::CheckerResult result;
   try 
   {
      result = checkParse( description, argv );
      result.setFailed();
      result.message_.add( "Bad option '" + badOptionName + "' not detected" );
   }
   catch ( const CommandLineOptions::CommandLineParseError &e )
   {
      result.compose( CppUT::checkEquals( CommandLineOptions::CommandLineParseError::invalidOptionName,
                                          e.cause() ) );
      result.compose( CppUT::checkEquals( badOptionName, e.optionName() ) );
   }
   return result;
}


void 
CommandLineOptionsTest::testParseShortOption()
{
   CommandLineOptions::OptionsDescription options( "test" );
   options.add( ",f", "" );
   options.add( ",x", "" );
   
   const char *args[] = { "-f", "-x", 0 };
   LOCAL_CHECK_PARSE( options, args );
   CPPUT_ASSERT_EXPR( values_->hasOption("-f") );
   CPPUT_ASSERT_EXPR( values_->hasOption("-x") );
}


void 
CommandLineOptionsTest::testLongOption()
{
   CommandLineOptions::OptionsDescription options( "test" );
   options.add( "flag", "" );
   options.add( "exclude", "" );
   
   const char *args[] = { "--flag", "--exclude", 0 };
   LOCAL_CHECK_PARSE( options, args );
   CPPUT_ASSERT_EXPR( values_->hasOption("flag") );
   CPPUT_ASSERT_EXPR( values_->hasOption("exclude") );
}


void 
CommandLineOptionsTest::testErrorOnBadOptionName()
{
   CommandLineOptions::OptionsDescription options( "test" );
   options.add( "time", "" );
   options.add( "exclude,x", "" );
   options.add( ",I", "" );
   options.add( "duration,d", "" );
   options.add( "fast,f", "" );

   const char *args1[] = { "--time", "--exclude", "-I", "--duration", "--fast", 0 };
   LOCAL_CHECK_PARSE( options, args1 );

   const char *args1b[] = { "-x", "-I", "-d", "-f", 0 };
   LOCAL_CHECK_PARSE( options, args1b );

   const char *args2[] = { "-t", "--exclude", 0 };
   LOCAL_CHECK_PARSE_BAD_OPTION( options, args2, "-t" );

   const char *args3[] = { "--exclude", "--super", 0 };
   LOCAL_CHECK_PARSE_BAD_OPTION( options, args3, "--super" );

   const char *args4[] = { "--x", "--super", 0 };
   LOCAL_CHECK_PARSE_BAD_OPTION( options, args4, "--x" );

   // Notes: we might want to allow this
   const char *args5[] = { "-exclude", "--super", 0 };
   LOCAL_CHECK_PARSE_BAD_OPTION( options, args5, "-exclude" );
}

/*
void 
CommandLineOptionsTest::testParseOptionParameter()
{
   CommandLineOptions::OptionsDescription options( "test" );
   int time;
   CppTL::ConstString excluded;
   double duration;
   options.add( "time,t", time, "" );
   options.add( "exclude,x", excluded, "" );
   options.add( "include,i", "" );
   options.add( "duration,d", duration, "" );
   options.add( "fast,f", "" );
   
   const char *args[] = { "--time", "1234", 
                          "--fast",
                          "--exclude", "arg", 
                          "--include", "arg2", 
                          "--duration", "12.34", 
                          0 };
   LOCAL_CHECK_PARSE( options, args );
   CPPUT_ASSERT_EXPR( values_->hasOption("fast") );
   CPPUT_ASSERT_EXPR( values_->hasOption("time") );
   CPPUT_ASSERT_EXPR( values_->hasOption("exclude") );
   CPPUT_ASSERT_EXPR( values_->hasOption("include") );
   CPPUT_ASSERT_EXPR( values_->hasOption("duration") );
}
*/

void 
CommandLineOptionsTest::testParseShortOptionAdjascentParameter()
{
   CommandLineOptions::OptionsDescription options( "test" );
   options.add( CommandLineOptions::option<std::string>( ",c:", "" ) );
   options.add( CommandLineOptions::optionDefault<std::string>( ",f", ".config", "" ) );
 
   const char *args[] = { "-cabcdef", 0 };
   LOCAL_CHECK_PARSE( options, args );
   CPPUT_ASSERT_EXPR( values_->hasOption("-c") );
   CPPUT_ASSERTSTR_EQUAL( "abcdef", 
                          get<std::string>( values_->optionValue("-c") ) );
   
   const char *args2[] = { "-f.ini", 0 };
   LOCAL_CHECK_PARSE( options, args2 );
   CPPUT_ASSERT_EXPR( values_->hasOption("-f") );
   CPPUT_ASSERTSTR_EQUAL( ".ini", 
                          get<std::string>( values_->optionValue("-f") ) );
   
   const char *args3[] = { "-f", 0 };
   LOCAL_CHECK_PARSE( options, args3 );
   CPPUT_ASSERT_EXPR( values_->hasOption("-f") );
   CPPUT_ASSERTSTR_EQUAL( ".config", 
                          get<std::string>( values_->optionValue("-f") ) );
}


void 
CommandLineOptionsTest::testParseShortOptionNextParameter()
{
   CommandLineOptions::OptionsDescription options( "test" );
   options.add( CommandLineOptions::option<std::string>( ",c:", "" ) );
   options.add( CommandLineOptions::optionDefault<std::string>(",f", ".config", "" ) );
   options.add( ",t", "" );
 
   const char *args[] = { "-c", "abcdef", 0 };
   LOCAL_CHECK_PARSE( options, args );
   CPPUT_ASSERT_EXPR( values_->hasOption("-c") );
   CPPUT_ASSERTSTR_EQUAL( "abcdef", 
                          get<std::string>( values_->optionValue("-c") ) );
   
   const char *args2[] = { "-f", ".ini", 0 };
   LOCAL_CHECK_PARSE( options, args2 );
   CPPUT_ASSERT_EXPR( values_->hasOption("-f") );
   CPPUT_ASSERTSTR_EQUAL( ".ini", 
                          get<std::string>( values_->optionValue("-f") ) );
   
   const char *args3[] = { "-f", 0 };
   LOCAL_CHECK_PARSE( options, args3 );
   CPPUT_ASSERT_EXPR( values_->hasOption("-f") );
   CPPUT_ASSERTSTR_EQUAL( ".config", 
                          get<std::string>( values_->optionValue("-f") ) );
 
   const char *args4[] = { "-c", "abcdef", "-t", 0 };
   LOCAL_CHECK_PARSE( options, args4 );
   CPPUT_ASSERT_EXPR( values_->hasOption("-c") );
   CPPUT_ASSERTSTR_EQUAL( "abcdef", 
                          get<std::string>( values_->optionValue("-c") ) );
   
   const char *args5[] = { "-f", ".ini", "-t", 0 };
   LOCAL_CHECK_PARSE( options, args5 );
   CPPUT_ASSERT_EXPR( values_->hasOption("-f") );
   CPPUT_ASSERTSTR_EQUAL( ".ini", 
                          get<std::string>( values_->optionValue("-f") ) );
   
   const char *args6[] = { "-f", "-t", 0 };
   LOCAL_CHECK_PARSE( options, args6 );
   CPPUT_ASSERT_EXPR( values_->hasOption("-f") );
   CPPUT_ASSERTSTR_EQUAL( ".config", 
                          get<std::string>( values_->optionValue("-f") ) );
}


void 
CommandLineOptionsTest::testParserLongOptionAdjascentParameter()
{
   CommandLineOptions::OptionsDescription options( "test" );
   options.add( "column:", "" );
   options.add( CommandLineOptions::optionDefault<std::string>( "file", ".config", "" ) );
 
   const char *args[] = { "--column=abcdef", 0 };
   LOCAL_CHECK_PARSE( options, args );
   CPPUT_ASSERT_EXPR( values_->hasOption("column") );
   CPPUT_ASSERTSTR_EQUAL( "abcdef", 
                          get<std::string>( values_->optionValue("column") ) );
   
   const char *args2[] = { "--file=.ini", 0 };
   LOCAL_CHECK_PARSE( options, args2 );
   CPPUT_ASSERT_EXPR( values_->hasOption("file") );
   CPPUT_ASSERTSTR_EQUAL( ".ini", 
                          get<std::string>( values_->optionValue("file") ) );
   
   const char *args3[] = { "--file", 0 };
   LOCAL_CHECK_PARSE( options, args3 );
   CPPUT_ASSERT_EXPR( values_->hasOption("file") );
   CPPUT_ASSERTSTR_EQUAL( ".config", 
                          get<std::string>( values_->optionValue("file") ) );
}


void 
CommandLineOptionsTest::testParserLongOptionNextParameter()
{
   CommandLineOptions::OptionsDescription options( "test" );
   options.add( "column:", "" );
   options.add( CommandLineOptions::optionDefault<std::string>( "file", ".config", "" ) );
   options.add( ",t", "" );
 
   const char *args[] = { "--column", "abcdef", 0 };
   LOCAL_CHECK_PARSE( options, args );
   CPPUT_ASSERT_EXPR( values_->hasOption("column") );
   CPPUT_ASSERTSTR_EQUAL( "abcdef", 
                          get<std::string>( values_->optionValue("column") ) );
   
   const char *args2[] = { "--file", ".ini", 0 };
   LOCAL_CHECK_PARSE( options, args2 );
   CPPUT_ASSERT_EXPR( values_->hasOption("file") );
   CPPUT_ASSERTSTR_EQUAL( ".ini", 
                          get<std::string>( values_->optionValue("file") ) );
   
   const char *args3[] = { "--file", 0 };
   LOCAL_CHECK_PARSE( options, args3 );
   CPPUT_ASSERT_EXPR( values_->hasOption("file") );
   CPPUT_ASSERTSTR_EQUAL( ".config", 
                          get<std::string>( values_->optionValue("file") ) );
 
   const char *args4[] = { "--column", "abcdef", "-t", 0 };
   LOCAL_CHECK_PARSE( options, args4 );
   CPPUT_ASSERT_EXPR( values_->hasOption("column") );
   CPPUT_ASSERTSTR_EQUAL( "abcdef", 
                          get<std::string>( values_->optionValue("column") ) );
   
   const char *args5[] = { "--file", ".ini", "-t", 0 };
   LOCAL_CHECK_PARSE( options, args5 );
   CPPUT_ASSERT_EXPR( values_->hasOption("file") );
   CPPUT_ASSERTSTR_EQUAL( ".ini", 
                          get<std::string>( values_->optionValue("file") ) );
   
   const char *args6[] = { "--file", "-t", 0 };
   LOCAL_CHECK_PARSE( options, args6 );
   CPPUT_ASSERT_EXPR( values_->hasOption("file") );
   CPPUT_ASSERTSTR_EQUAL( ".config", 
                          get<std::string>( values_->optionValue("file") ) );
}


void 
CommandLineOptionsTest::testValuesLookUpAsShortOrLong()
{
   CommandLineOptions::OptionsDescription options( "test" );
   options.add( "column,c:", "" );

   const char *args[] = { "-c8", 0 };
   LOCAL_CHECK_PARSE( options, args );
   CPPUT_ASSERT_EXPR( values_->hasOption("-c") );
   CPPUT_ASSERT_EXPR( values_->hasOption("column") );

   const char *args2[] = { "--column", "8", 0 };
   LOCAL_CHECK_PARSE( options, args2 );
   CPPUT_ASSERT_EXPR( values_->hasOption("-c") );
   CPPUT_ASSERT_EXPR( values_->hasOption("column") );
}


void 
CommandLineOptionsTest::testStoreParameterValues()
{
   CommandLineOptions::OptionsDescription options( "test" );
   int column = -1;
   std::string file;
   options.add( CommandLineOptions::option( "column,c:", column, "" ) );
   options.add( CommandLineOptions::option( "file", file, ".config", "" ) );
 
   const char *args[] = { "-c8", "--file", "test", 0 };
   LOCAL_CHECK_PARSE( options, args );
   CPPUT_ASSERT_EXPR( values_->hasOption("column") );
   CPPUT_ASSERT_EXPR( values_->hasOption("file") );
   CPPUT_ASSERTSTR_EQUAL( "test", file );
   CPPUT_ASSERT_EQUAL( 8, column );
 
   const char *args2[] = { "--file", "-c", "8", 0 };
   LOCAL_CHECK_PARSE( options, args2 );
   CPPUT_ASSERT_EXPR( values_->hasOption("column") );
   CPPUT_ASSERT_EXPR( values_->hasOption("file") );
   CPPUT_ASSERTSTR_EQUAL( ".config", file );
   CPPUT_ASSERT_EQUAL( 8, column );
}


void 
CommandLineOptionsTest::testParseRequiredParameter()
{
   CommandLineOptions::OptionsDescription options( "test" );
   options.add( "column,c:", "" );
   options.add( "file:", "" );
 
   const char *args[] = { "--file", "--column", "abcdef", 0 };
   try
   {
      checkParse( options, args );
      CPPUT_FAIL( "Command line parsing should have failed because of missing parameter of --file." );
   }
   catch ( const CommandLineOptions::ValidatorError &e )
   {
      CPPUT_ASSERT_EXPR( e.cause() == CommandLineOptions::ValidatorError::missingParameter );
   }
}


static Value
validateTraceOrTime( const CommandLineOptions::Parameters &parameters )
{
   std::string text = parameters.at(0);
   if ( text != "trace"  &&  text != "time" )
      throw CommandLineOptions::ValidatorError( CommandLineOptions::ValidatorError::badParameterValue,
                                                "trace or time expected." );
   return value( text );
}


void 
CommandLineOptionsTest::testCustomValidator()
{
   CommandLineOptions::OptionsDescription options( "test" );
   std::string file;
   options.add( CommandLineOptions::option( "debug,d:", file, "" ).setValidator( 
      CommandLineOptions::validator( &validateTraceOrTime ) ) );

   const char *args[] = { "-d", "trace", 0 };
   LOCAL_CHECK_PARSE( options, args );
   CPPUT_ASSERT_EXPR( values_->hasOption("debug") );
   CPPUT_ASSERTSTR_EQUAL( "trace", file );

   const char *args2[] = { "-d", "time", 0 };
   LOCAL_CHECK_PARSE( options, args2 );
   CPPUT_ASSERT_EXPR( values_->hasOption("debug") );
   CPPUT_ASSERTSTR_EQUAL( "time", file );

   const char *args3[] = { "-d", "bad", 0 };
   try
   {
      checkParse( options, args3 );
      CPPUT_FAIL( "Validation should have failed because of bad parameter for -d." );
   }
   catch ( const CommandLineOptions::ValidatorError &e )
   {
      CPPUT_ASSERTSTR_EQUAL( "debug", e.optionName() );
      CPPUT_ASSERT_EXPR( e.cause() == CommandLineOptions::ValidatorError::badParameterValue );
   }
}


void 
CommandLineOptionsTest::testEnumerationValidator()
{
   CommandLineOptions::OptionsDescription options( "test" );
   LogLevel level;
   CommandLineOptions::EnumerationValidator<LogLevel> logLevelValidator;
   logLevelValidator.add( "fatal", fatal, "Log only fatal events." );
   logLevelValidator.add( "warning", warning, "Log only fatal and warning events." );
   logLevelValidator.add( "info", info, "Log all events." );
   options.add( CommandLineOptions::validatedOption(
                  "level,l", 
                  level, 
                  fatal,
                  "Set log level",
                  logLevelValidator.clone() ) );
   
   const char *args[] = { "-l", "info", 0 };
   LOCAL_CHECK_PARSE( options, args );
   CPPUT_ASSERT_EXPR( values_->hasOption("level") );
   CPPUT_ASSERT_EQUAL( info, level );
   
   const char *args2[] = { "-l", 0 };
   LOCAL_CHECK_PARSE( options, args2 );
   CPPUT_ASSERT_EXPR( values_->hasOption("level") );
   CPPUT_ASSERT_EQUAL( fatal, level );


}
