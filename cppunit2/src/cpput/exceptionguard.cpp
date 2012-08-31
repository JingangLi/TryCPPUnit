#include <cpput/testing.h>
#include <cpput/testinfo.h>
#include <cpptl/typename.h>
#include <cpptl/functor.h>
#include <cpptl/conststring.h>

namespace CppUT {


// class ExceptionHandlerContext
// //////////////////////////////////////////////////////////////////

namespace Impl {

   class ExceptionHandlerContext : public ExceptionGuardContext
   {
   public:
      bool faultOccurred_;

      ExceptionHandlerContext( const ExceptionGuard::Guards &guards )
         : guards_( guards )
         , rethrow_( true )
         , index_( 0 )
         , faultOccurred_( false )
      {
      }

      ExceptionHandlerContext( const ExceptionGuard::Guards &guards, 
                               const CppTL::Functor0 &functor )
         : guards_( guards )
         , functor_( functor )
         , rethrow_( false )
         , index_( 0 )
         , faultOccurred_( false )
      {
      }


   public: // overridden from ExceptionGuardContext
         virtual void chainCall()
         {
            if ( index_ == guards_.size() )
            {
               if ( rethrow_ )   // rethrow a previously caught exception
               {
                  throw;
               }
               else              // protect a function call that may throw
               {
                  functor_();
               }
            }
            else
            {
               ExceptionHandlerFn guard( guards_.at(index_) );
               ++index_;
               guard( *this );
            }
         }

         virtual void setExceptionFault( const char *faultType, 
                                         const std::string &message )
         {
            CheckerResult result;
            result.setFailed();
//            Assertion fault( Assertion::fault );
            result.setName( "test case does not throw any exception" );
            result.appendMessage( "test threw an unexpected exception" );
            result.diagnostic("exception type") = faultType;
            result.diagnostic("exception message") = message;
            TestInfo::threadInstance().handleUnexpectedException( result );

            faultOccurred_ = true;
         }

         virtual void setFaultAlreadyHandled()
         {
            faultOccurred_ = true;
         }

   private:
      const ExceptionGuard::Guards &guards_;
      CppTL::Functor0 functor_;
      int index_;
      bool rethrow_;
   };


} // namespace Impl


// class ExceptionGuardContext
// //////////////////////////////////////////////////////////////////


ExceptionGuardContext::~ExceptionGuardContext()
{
}


void 
ExceptionGuardContext::chainCall()
{
}


void 
ExceptionGuardContext::setStdExceptionFault( const std::exception &e, 
                                             const char *typeHint )
{
   setExceptionFault( CppTL::getObjectTypeName( e, typeHint ).c_str(),
                      e.what() );
}

// baseExceptionHandler
// //////////////////////////////////////////////////////////////////

// Handles CppUT exceptions, std::exception hierarchy and any unknown exception.
static void baseExceptionHandler( ExceptionGuardContext &context )
{
   try
   {
      context.chainCall();
   }
   catch ( const AbortingAssertionException & )
   {
      // Already injected into result
      context.setFaultAlreadyHandled();
   }
// @todo Need to add catch for all standard derived classes. 
// Warning: => they don't always exist on all platforms.
   catch ( const std::runtime_error &e )
   {
      context.setStdExceptionFault( e, "std::runtime_error" );
   }
   catch ( const std::exception &e )
   {
      context.setStdExceptionFault( e, "std::exception" );
   }
   catch ( ... ) 
   {
      context.setExceptionFault( "unknown", "" );
   }
}


// class ExceptionGuard
// //////////////////////////////////////////////////////////////////

ExceptionGuard::ExceptionGuard()
{
   guards_.push_back( &baseExceptionHandler );
}


void 
ExceptionGuard::add( ExceptionHandlerFn exceptionHandler )
{
   //@todo should check if test are in progress.
   guards_.push_back( exceptionHandler );
}


void 
ExceptionGuard::removeLastAdded()
{
   //@todo should check if test are in progress.
   if ( guards_.size() == 1 ) // @todo should we throw an exception ?
      return;

   guards_.pop_back();
}


bool
ExceptionGuard::protect( CppTL::Functor0 test ) const
{
   Impl::ExceptionHandlerContext context( guards_, test );
   context.chainCall();
   return !context.faultOccurred_;
}


// @todo fix this: they assertion is always added. We
// should probably only fill up a CheckerResult.
//void 
//ExceptionGuard::rethrowExceptionAndHandle() const
//{
//   Impl::ExceptionHandlerContext context( guards_ );
//   context.chainCall();
//   // @todo should check that the fault was handled
//}


} // namespace CppUT
