#ifndef CPPTL_THREAD_H_INCLUDED
# define CPPTL_THREAD_H_INCLUDED

# include <cpptl/forwards.h>
# if CPPTL_HAS_THREAD
#  include <cpptl/functor.h>
# endif // # if CPPTL_HAS_THREAD


namespace CppTL {

void CPPTL_API processThreadExitHandlers();


// Non-recursive mutex
class CPPTL_API Mutex : public NonCopyable
{
public:
   class ScopedLockGuard
   {
   public:
      ScopedLockGuard( Mutex &mutex )
         : mutex_( mutex )
      {
         mutex_.lock();
      }

      ~ScopedLockGuard()
      {
         mutex_.unlock();
      }

   private:
      Mutex &mutex_;
   };

   Mutex();
   ~Mutex();

   void lock();
   void unlock();
# if CPPTL_HAS_THREAD
private:
   void *data_;
# endif
};
 
   
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// Implementation of thread API for NON THREAD-SAFE mode
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
# if !(CPPTL_HAS_THREAD)

template<class ValueType>
class ThreadLocalStorage
{
public:
   typedef ThreadLocalStorage<ValueType> SelfType;

   ThreadLocalStorage( const ValueType &defaultValue = ValueType() )
      : value_( defaultValue )
   {
   }

   bool isInitialized() const
   {
      return true;
   }

   ValueType &get()
   {
      return value_;
   }

private:
   ValueType value_;
};


// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// Implementation of thread API for THREAD-SAFE mode
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
# else // # if !(CPPTL_HAS_THREAD)

/// \cond implementation_detail
namespace Impl {

   class RawThreadStorage;

   RawThreadStorage *CPPTL_API createRawThreadStorage( const Functor1<void *> &deallocator );

   void CPPTL_API freeRawThreadStorage( RawThreadStorage *storage );

   void *CPPTL_API getRawThreadStorage( RawThreadStorage *storage );

   void setRawThreadStorage( RawThreadStorage *storage, void *value );

} // namespace Impl
/// \endcond


template<class ValueType>
class ThreadLocalStorage : public NonCopyable
{
public:
   typedef void(*FreeFn)(void*);
   typedef ThreadLocalStorage<ValueType> SelfType;

   ThreadLocalStorage( const ValueType &defaultValue = ValueType() )
      : storage_( Impl::createRawThreadStorage( cfn1( FreeFn(freeValue) ) ) )
      , defaultValue_( defaultValue )
   {
   }

   virtual ~ThreadLocalStorage()
   {
      Impl::freeRawThreadStorage( storage_ );
   }

   bool isInitialized() const
   {
      return Impl::getRawThreadStorage( storage_ ) != 0;
   }

   ValueType &get()
   {
      ValueType * value = static_cast<ValueType *>( 
         Impl::getRawThreadStorage( storage_ ) );
      if ( !value )
      {
         value = new ValueType( defaultValue_ );
         Impl::setRawThreadStorage( storage_, value );
      }
      return *value;
   }

   SelfType &operator =( const ValueType &other )
   {
      get() = other;
      return *this;
   }

private:
   static void freeValue( void *p )
   {
      delete static_cast<ValueType *>( p );
   }

   ValueType defaultValue_;
   Impl::RawThreadStorage *storage_;
};

# endif // #  ifndef CPPTL_THREAD_SAFE

} // namespace CppTL



#endif // CPPTL_THREAD_H_INCLUDED
