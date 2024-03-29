#ifndef CPPTL__STLIMPL_H_INCLUDED
# define CPPTL__STLIMPL_H_INCLUDED

# include "config.h"
# include <stdlib.h>

/*
 Notes: we need to prefix some (all) functions with cpptl_ because some STL implementations
 are bugged:
 namespace A { 
    void copy();

    struct X { 
       X( int x ) : x_( x ) {} 
       int x_; 
    };
 }
 std::vector<X> a;   // failed to compile because std::vector template find A::copy()
                     // with ADL instead of std::copy().
 */



// Make exception related code optional
# ifndef CPPTL_NO_EXCEPTION_SUPPORT
#  define CPPTL_TRY_BEGIN
#  define CPPTL_TRY_END_CLEANUP( expression )
# else
#  define CPPTL_TRY_BEGIN try
#  define CPPTL_TRY_END_CLEANUP( expression ) catch ( ... ) { expression; throw; }
# endif


namespace CppTL { namespace Impl {

// iterator stuffs

   template<class T>
   struct ConstIteratorTraits
   {
      typedef T &reference;
      typedef T *pointer;
   };

   template<class T>
   struct NonConstIteratorTraits
   {
      typedef T &reference;
      typedef T *pointer;
   };

// algorithm for use when implementing containers
// algorithms

template<class T>
inline void construct( T &object, const T &value )
{
   new (&object) T(value);
}

template<class T>
inline void destruct( T &object )
{
   object.~T();
}

template<class T,class OutputIt>
inline void destruct_range( OutputIt first, OutputIt last )
{
   for ( ; first != last; ++first )
      ::CppTL::Impl::destruct( *first );
}

template<class InputIt,class OutputIt>
void
cpptl_copy( InputIt first, InputIt last, OutputIt firstDestIt )
{
   for ( ; first != last; ++first, ++firstDestIt )
      *first = *firstDestIt;
}

template<class InputIt,class OutputIt>
void
copy_backwards( InputIt first, InputIt last, OutputIt endDestIt )
{
   if ( first != last )
   {
      do
      {
         --last;
         --endDestIt;
         *endDestIt = *last;
      }
      while ( first != last );
   }
}

template<class InputIt,class OutputIt>
void
copy_with_construct( InputIt first, InputIt last, OutputIt firstDestIt )
{
   for ( ; first != last; ++first, ++firstDestIt )
   {
      CPPTL_TYPENAME InputIt::reference old = *first;
      ::CppTL::Impl::construct(*firstDestIt, *first );
   }
}

template<class OutputIt, class ValueType>
void
copy_with_construct_value( OutputIt firstDestIt, unsigned int count, const ValueType &value )
{
   while ( count-- > 0 )
   {
      construct( *firstDestIt, value );
      ++firstDestIt;
   }
}


template<class InputIt,class OutputIt>
void
copy_and_destroy( InputIt first, InputIt last, OutputIt firstDestIt )
{
   for ( ; first != last; ++first, ++firstDestIt )
   {
      CPPTL_TYPENAME InputIt::reference old = *first;
      ::CppTL::Impl::construct(*firstDestIt, old );
      ::CppTL::Impl::destruct( old );
   }
}

template<class InputIt,class OutputIt>
void
copy_backwards_and_destroy( InputIt first, InputIt last, OutputIt endDestIt )
{
   if ( first != last )
   {
      do
      {
         --last;
         --endDestIt;
         CPPTL_TYPENAME InputIt::reference old = *last;
         ::CppTL::Impl::construct( *endDestIt, old );
         ::CppTL::Impl::destruct( old );
      }
      while ( first != last );
   }
}


} } // namespace CppTL { namespace Impl {



namespace CppTL { 

// Common predicates
template<class T>
struct LessPred
{
   bool operator()( const T &a, const T &b ) const
   {
      return a < b;
   }
};

// Pair
template<class A, class B>
class Pair
{
public:
   typedef A first_type;
   typedef B second_type;

   Pair()
   {
   }

   Pair( const first_type &a, const second_type &b )
      : first( a )
      , second( b )
   {
   }

   first_type first;
   second_type second;
};


// Common allocators

template<class T>
class MallocAllocator
{
public:
   typedef unsigned int size_t;

   T *allocate()
   {
      return static_cast<T*>( malloc( sizeof(T) ) );
   }

   void release( T *object )
   {
      free( object );
   }

   T *allocateArray( size_t count )
   {
      return static_cast<T*>( malloc( sizeof(T) * count ) );
   }

   void releaseArray( T *array, size_t allocatedSize )
   {
      free( array );
   }

   void swap( MallocAllocator &other )
   {
   }
};


} // namespace CppTL


#endif // CPPTL__STLIMPL_H_INCLUDED
