#ifndef CPPTL_ATOMICCOUNTER_H_INCLUDED
# define CPPTL_ATOMICCOUNTER_H_INCLUDED

# include <cpptl/config.h>
# if CPPTL_USE_PTHREAD_ATOMIC
#  include <pthread.h>
# endif

# if CPPTL_USE_WIN32_ATOMIC // Forwards declaration for WIN32 (avoid including windows.h)

#  if _MSC_VER <= 1200    // VC++ 6 platform SDK as not volatile
extern "C" __declspec(dllimport) long __stdcall InterlockedIncrement(long *);
extern "C" __declspec(dllimport) long __stdcall InterlockedDecrement(long *);
#  else
extern "C" __declspec(dllimport) long __stdcall InterlockedIncrement(long volatile *);
extern "C" __declspec(dllimport) long __stdcall InterlockedDecrement(long volatile *);
#  endif
 
# endif // ifdef CPPTL_USE_WIN32_ATOMIC



namespace CppTL {

// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// AtomicCounter implementation for WIN32
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
# if CPPTL_USE_WIN32_ATOMIC

class CPPTL_API AtomicCounter
{
public:
   AtomicCounter( long count = 0 )
      : count_( count )
   {
   }

   void increment()
   {
      InterlockedIncrement( const_cast< long * >( &count_ ) ); 
   }

   /// Decrements the count
   /// @returns true if count is non null.
   bool decrement()
   {
      return InterlockedDecrement( const_cast< long * >( &count_ ) ) != 0; 
   }

   /// Should returns the current value of the count.
   /// \warning this method is only defined for debugging and testing
   /// purpose, you should not rely on it in production code path.
   long count() const
   {
      return count_;
   }

private:
   volatile long count_;
};


// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// AtomicCounter implementation using pthread mutex
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
#elif CPPTL_USE_PTHREAD_ATOMIC

class CPPTL_API AtomicCounter
{
public:
   AtomicCounter( long count = 0 )
      : count_( count )
   {
      pthread_mutex_init( &lock_, 0 );
   }

   AtomicCounter( const AtomicCounter &other )
   {
      pthread_mutex_init( &lock_, 0 );
   }

   ~AtomicCounter()
   {
      pthread_mutex_destroy( &lock_ );
   }

   AtomicCounter &operator =( const AtomicCounter &other )
   {
      return *this;
   }

   void increment()
   {
      pthread_mutex_lock( &lock_ );
      ++count_;
      pthread_mutex_unlock( &lock_ );
   }

   /// Decrements the count
   /// @returns true if count is non null.
   bool decrement()
   {
      pthread_mutex_lock( &lock_ );
      bool isNotNull = --count_ != 0;
      pthread_mutex_unlock( &lock_ );
      return isNotNull;
   }

   /// Should returns the current value of the count.
   /// \warning this method is only defined for debugging and testing
   /// purpose, you should not rely on it in production code path.
   long count() const
   {
      pthread_mutex_lock( &lock_ );
      long value = count_;
      pthread_mutex_unlock( &lock_ );
      return value;
   }

private:
   mutable pthread_mutex_t lock_;
   volatile long count_;
};



// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// NON thread-safe implementation of AtomicCounter
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
#else

class CPPTL_API AtomicCounter
{
public:
   AtomicCounter( long count = 0 );
      : count_( count )
   {
   }

   void increment()
   {
      return ++count_;
   }

   bool decrement()
   {
      return --count_ != 0;
   }

   long count() const
   {
      return count_;
   }

private:
   long count_;
};

# endif // ifdef CPPTL_USE_WIN32_ATOMIC


} // namespace CppTL


#endif // CPPTL_ATOMICCOUNTER_H_INCLUDED

