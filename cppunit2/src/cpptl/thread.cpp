#include <cpptl/thread.h>
#include <vector>
#if CPPTL_HAS_THREAD
# ifdef CPPTL_USE_WIN32_THREAD
#  if !defined(APIENTRY)
#   undef NOMINMAX
#   define WIN32_LEAN_AND_MEAN 
#   define NOGDI
#   define NOUSER
#   define NOKERNEL
#   define NOSOUND
#   define NOMINMAX
#   define BLENDFUNCTION void    // for mingw & gcc
#   include <windows.h>
#  endif
# elif defined(CPPTL_USE_PTHREAD_THREAD)
#  include <pthread.h>
# endif // # elif defined(CPPTL_USE_PTHREAD_THREAD)
#endif

// Notes concerning threading implementation:
// CppUnit need has very few thread-safety, hence the implementation of the threading
// API need not to be highly performant.
//
// But, the ThreadLocalStorage should not use lock for each get/set (locking when creating
// the variable in a new thread is ok though). ThreadLocalStorage is use by assertions
// and this would therefore decrease the actually concurrency of the threads using assertions.

namespace CppTL {


// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// No Thread API
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
#if !(CPPTL_HAS_THREAD)

Mutex::Mutex()
{
}

Mutex::~Mutex()
{
}

void Mutex::lock()
{
}

void Mutex::unlock()
{
}


#else



// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// Common Thread API implementation
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////

// class ThreadExitHandler
// //////////////////////////////////////////////////////////////////////

namespace Impl {

   class ThreadExitHandler : public NonCopyable
   {
   public:
      static ThreadExitHandler &instance()
      {
         // This static is instantiated at static construction time
         // so it's thread-safe
         static ThreadExitHandler handler;
         return handler;
      }

      ThreadExitHandler()
         : magic1_( 0x1a9cd67f ) // magic number are used to protect against
         , magic2_( 0x3e4a3c9d ) // possible race condition on shutdown.
      {
      }

      ~ThreadExitHandler()
      {
         Mutex::ScopedLockGuard guard( lock_ );
         while ( !handlers_.empty() )
         {
            handlers_.begin()->first();
            handlers_.erase( handlers_.begin() );
         }
         magic1_ = 0xdeadbeef;
         magic2_ = 0xdeadbeef;
      }

      void add( const Functor0 &handler, 
                const void *tag )
      {
         if ( !isValid() )
            return;
         Mutex::ScopedLockGuard guard( lock_ );
         Handlers::iterator it = find( tag );
         if ( it == handlers_.end() )
            handlers_.push_back( HandlerInfo( handler, tag ) );
         else
            *it = HandlerInfo( handler, tag );
      }

      void remove( const void *tag )
      {
         if ( !isValid() )
            return;
         Mutex::ScopedLockGuard guard( lock_ );
         Handlers::iterator it = find( tag );
         if ( it != handlers_.end() )
         {
            it->first();
            handlers_.erase( it );
         }
      }

      void process()
      {
         if ( !isValid() )
            return;
         Mutex::ScopedLockGuard guard( lock_ );
         Handlers::iterator it = handlers_.begin();
         for ( ; it != handlers_.end(); ++it )
            it->first();
      }

   private:
      typedef std::pair<Functor0,const void *> HandlerInfo;
      typedef std::vector<HandlerInfo> Handlers;

      Handlers::iterator find( const void *tag )
      {
         Handlers::iterator it = handlers_.begin();
         for ( ; it != handlers_.end(); ++it )
            if ( it->second == tag )
               return it;
         return handlers_.end();
      }

      bool isValid() const
      {
         return magic1_ == 0x1a9cd67f && magic2_ == 0x3e4a3c9d;
      }

      Handlers handlers_;
      Mutex lock_;
      unsigned int magic1_;
      unsigned int magic2_;
   };

} // namespace Impl


class ThreadExitHandlerInitializer
{
public:
   ThreadExitHandlerInitializer()
   {
      Impl::ThreadExitHandler::instance();
   }
};

// This force a call to ThreadExitHandler::instance(), 
// and ensure it is properly initialized.
static ThreadExitHandlerInitializer threadExitInitializer;

void addThreadExitHandler( const Functor0 &handler, 
                           const void *tag )
{
   Impl::ThreadExitHandler::instance().add( handler, tag );
}

void removeThreadExitHandler( const void *tag )
{
   Impl::ThreadExitHandler::instance().remove( tag );
}

/// This fonction must be called at the end of each thread
/// to ensure ThreadLocalStorage are properly cleaned-up.
void processThreadExitHandlers()
{
   Impl::ThreadExitHandler::instance().process();
}



# if CPPTL_USE_WIN32_THREAD

// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// Thread API for WIN32
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////

// class Mutex (win32)
// //////////////////////////////////////////////////////////////////////

Mutex::Mutex()
   : data_( 0 )
{
   CRITICAL_SECTION *cs = new CRITICAL_SECTION();
   ::InitializeCriticalSection( cs );
   data_ = cs;
}


Mutex::~Mutex()
{
   CRITICAL_SECTION *cs = static_cast<CRITICAL_SECTION *>( data_ );
   if ( cs )
      ::DeleteCriticalSection( cs );
   delete cs;
}

void 
Mutex::lock()
{
   CRITICAL_SECTION *cs = static_cast<CRITICAL_SECTION *>( data_ );
   ::EnterCriticalSection( cs );
}


void 
Mutex::unlock()
{
   CRITICAL_SECTION *cs = static_cast<CRITICAL_SECTION *>( data_ );
   ::LeaveCriticalSection( cs );
}


// class Impl::RawThreadStorage (win32)
// //////////////////////////////////////////////////////////////////////

namespace Impl {

   class RawThreadStorage
   {
   public:
      RawThreadStorage( const CppTL::Functor1<void *> &deallocator )
         : deallocator_( deallocator )
      {
         tlsIndex_ = ::TlsAlloc();
         //      if ( tlsIndex_ == TLS_OUT_OF_INDEXES ) // failed. 
         // Can't throw exception => during static initialization...

         addThreadExitHandler( memfn0( this, &RawThreadStorage::onThreadExit ),
                               this );
      }

      ~RawThreadStorage()
      {
         removeThreadExitHandler( this );
         ::TlsFree( tlsIndex_ );
      }

      void onThreadExit()
      {
         deallocator_( getRawThreadStorage( this ) );
         setRawThreadStorage( this, 0 );
      }

      CppTL::Functor1<void *> deallocator_;
      DWORD tlsIndex_;
   };

   RawThreadStorage *createRawThreadStorage( const CppTL::Functor1<void *> &deallocator )
   {
      return new RawThreadStorage( deallocator );
   }

   void freeRawThreadStorage( RawThreadStorage *storage )
   {
      delete storage;
   }

   void *getRawThreadStorage( RawThreadStorage *storage )
   {
      // @todo assert if NULL
      return ::TlsGetValue( storage->tlsIndex_ );
   }

   void setRawThreadStorage( RawThreadStorage *storage, 
                             void *value )
   {
      if ( ::TlsSetValue( storage->tlsIndex_, value ) == 0 )
      {
         // @todo handle failure
      }
   }

} // namespace Impl


# elif defined(CPPTL_USE_PTHREAD_THREAD)

// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// Thread API for PTHREAD
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////

// class Mutex (pthread)
// //////////////////////////////////////////////////////////////////////

Mutex::Mutex()
   : data_( 0 )
{
   pthread_mutex_t *mutex = new pthread_mutex_t();
   if ( pthread_mutex_init( mutex, 0 ) != 0 )
   {  // initialization error
   }
   data_ = mutex;
}


Mutex::~Mutex()
{
   pthread_mutex_t *mutex = static_cast<pthread_mutex_t *>( data_ );
   if ( mutex )
      pthread_mutex_destroy( mutex );
   delete mutex;
}

void 
Mutex::lock()
{
   pthread_mutex_t *mutex = static_cast<pthread_mutex_t *>( data_ );
   if ( pthread_mutex_lock( mutex ) != 0 )
   {  // @todo error
   }
}


void 
Mutex::unlock()
{
   pthread_mutex_t *mutex = static_cast<pthread_mutex_t *>( data_ );
   if ( pthread_mutex_unlock( mutex ) != 0 )
   {  // @todo error
   }
}


// class Impl::ThreadLocalStorageImpl (pthread)
// //////////////////////////////////////////////////////////////////////

namespace Impl {

   class RawThreadStorage
   {
   public:
      RawThreadStorage( const CppTL::Functor1<void *> &deallocator )
         : deallocator_( deallocator )
      {
         if ( pthread_key_create( &key_, 0 ) != 0 )
         { // error: Can't throw exception => during static initialization
         }

         addThreadExitHandler( memfn0( this, &RawThreadStorage::onThreadExit ),
                               this );
      }

      ~RawThreadStorage()
      {
         removeThreadExitHandler( this );
         if ( pthread_key_delete( key_ ) != 0 )
         { // error: can't throw exception, during static uninitialization
         }
      }

      void onThreadExit()
      {
         deallocator_( getRawThreadStorage( this ) );
         setRawThreadStorage( this, 0 );
      }

      CppTL::Functor1<void *> deallocator_;
      pthread_key_t key_;
   };

   RawThreadStorage *createRawThreadStorage( const CppTL::Functor1<void *> &deallocator )
   {
      return new RawThreadStorage( deallocator );
   }

   void freeRawThreadStorage( RawThreadStorage *storage )
   {
      delete storage;
   }

   void *getRawThreadStorage( RawThreadStorage *storage )
   {
      // @todo assert if NULL
      return pthread_getspecific( storage->key_ );
   }

   void setRawThreadStorage( RawThreadStorage *storage, 
                             void *value )
   {
      if ( pthread_setspecific( storage->key_, value ) != 0 )
      { // @todo handle failure
      }
   }

} // namespace Impl


# endif // # elif defined(CPPTL_USE_PTHREAD_THREAD)

#endif // #ifdef CPPTL_HAS_THREAD


} // namespace CppTL
