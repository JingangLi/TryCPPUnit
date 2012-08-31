#include <cpput/assertcommon.h>
#include <cpput/testing.h>
#include <cpptl/functor.h>
#include <stdio.h>

// Need to ensure that:
// Standard exception are caught
// Non standard exception are caught
// User specified exception handler are called.

// @todo adds check to ensure that AbortingAssertionException is caught.

static void throwStdException()
{
   throw std::exception();
}

namespace {
   class CustomException : public std::runtime_error
   {
   public:
      typedef std::runtime_error Super;
      CustomException() 
         : Super( "CustomException" )
      {
      }

      // @todo adds macro for throw()
      virtual ~CustomException() CPPTL_NO_THROW
      {
      }
   };

   struct AnyException
   {
   };
}


static void throwAnyException()
{
   throw AnyException();
}


static void throwCustomException()
{
   throw CustomException();
}



static int protectedCallCount = 0;

static void protectedFunction()
{
   ++protectedCallCount;
}


static void 
testExceptionGuardRunFunctor()
{
   protectedCallCount = 0;

   CppUT::ExceptionGuard guard;
   CppUT::TestInfo::threadInstance().startNewTest();
   bool result = guard.protect( CppTL::cfn0( &protectedFunction ) );
   CPPUT_ASSERT_EQUAL( 1, protectedCallCount );
   CPPUT_ASSERT_EQUAL( true, result );
}


static void 
testExceptionGuardCatchStandardException()
{
   CppUT::ExceptionGuard guard;
   CppUT::TestInfo::threadInstance().startNewTest();
   bool result = guard.protect( CppTL::cfn0( &throwStdException ) );
   CPPUT_ASSERT_EQUAL( false, result );
}


static void 
testExceptionGuardCatchAnyException()
{
   CppUT::ExceptionGuard guard;
   CppUT::TestInfo::threadInstance().startNewTest();
   bool result = guard.protect( CppTL::cfn0( &throwAnyException ) );
   CPPUT_ASSERT_EQUAL( false, result );
}


static int exceptionHandlerCallCount = 0;

static void customExceptionHandler( CppUT::ExceptionGuardContext &context )
{
   try
   {
      ++exceptionHandlerCallCount;
      context.chainCall();
   }
   catch ( const CustomException & )
   {
      context.setExceptionFault( "CustomException", "Custom exception test" );
   }
}


static void
testExceptionGuardAddCustomHandler()
{
   CppUT::ExceptionGuard guard;
   // check that the exception translator is called
   exceptionHandlerCallCount = 0;
   guard.add( &customExceptionHandler );
   CppUT::TestInfo::threadInstance().startNewTest();
   bool result = guard.protect( CppTL::cfn0( &throwCustomException ) );
   CPPUT_ASSERT_EQUAL( false, result );
   CPPUT_ASSERT_EQUAL( 1, exceptionHandlerCallCount );

   // check that standard and ... catch handler are still active.
   CppUT::TestInfo::threadInstance().startNewTest();
   exceptionHandlerCallCount = 0;
   result = guard.protect( CppTL::cfn0( &throwStdException ) );
   CPPUT_ASSERT_EQUAL( false, result );
   CPPUT_ASSERT_EQUAL( 1, exceptionHandlerCallCount );

   CppUT::TestInfo::threadInstance().startNewTest();
   result = guard.protect( CppTL::cfn0( &throwAnyException ) );
   CPPUT_ASSERT_EQUAL( false, result );

   // check that custom exception handler is no longer called after removal
   exceptionHandlerCallCount = 0;
   guard.removeLastAdded();
   CppUT::TestInfo::threadInstance().startNewTest();
   result = guard.protect( CppTL::cfn0( &throwCustomException ) );
   CPPUT_ASSERT_EQUAL( false, result );
   CPPUT_ASSERT_EQUAL( 0, exceptionHandlerCallCount );

   // check that standard and ... catch handler are still active, 
   // but not the custom exception handler.
   CppUT::TestInfo::threadInstance().startNewTest();
   exceptionHandlerCallCount = 0;
   result = guard.protect( CppTL::cfn0( &throwCustomException ) );
   CPPUT_ASSERT_EQUAL( false, result );
   CPPUT_ASSERT_EQUAL( 0, exceptionHandlerCallCount );

   CppUT::TestInfo::threadInstance().startNewTest();
   exceptionHandlerCallCount = 0;
   result = guard.protect( CppTL::cfn0( &throwStdException ) );
   CPPUT_ASSERT_EQUAL( false, result );
}


//static void
//testRethrowException()
//{
//   CppUT::ExceptionGuard guard;
//   guard.add( &customExceptionHandler );
//   exceptionHandlerCallCount = 0;
//   try
//   {
//      throwCustomException();
//   }
//   catch ( ... )
//   {
//      // The final chain call execute "throw;" which rethrow the catched exception.
//      // customExceptionHandler should catch the previously rethrown CustomException
//      guard.rethrowExceptionAndHandle();
//   }
//   CPPUT_ASSERT_EQUAL( 1, exceptionHandlerCallCount );
//}


/* Notes: startNewtest() must be called before each protect because
 * protect() returns value depends on testStatus.
 */

bool testExceptionGuard()
{
   printf( "Running bootstrap test: testExceptionGuard()...\n" );
   try
   {
      CppUT::TestInfo::threadInstance().startNewTest();
      testExceptionGuardRunFunctor();
      testExceptionGuardCatchStandardException();
      testExceptionGuardCatchAnyException();
      testExceptionGuardAddCustomHandler();
      //testRethrowException();
   }
   catch ( const CppUT::AbortingAssertionException &e )
   {
      printf( "testExceptionGuard() failed: %s\n", e.what() );
      return false;
   }
   catch ( ... )
   {
      printf( "testExceptionGuard() failed (uncaught exception).\n" );
      return false;
   }

   return true;
}
