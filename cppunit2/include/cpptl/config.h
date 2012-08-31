#ifndef CPPTL_CONFIG_H_INCLUDED
# define CPPTL_CONFIG_H_INCLUDED

// Need to include an STL header to detect STL type & version
#include <utility>

// compiler specific stuffs...
///////////////////////////////////////////////////////////////////////////

// Microsoft Visual C++
# if defined(_MSC_VER)
#  if _MSC_VER <= 1200    // VC++ 6 or before
#   pragma warning( disable : 4786 ) // ident trunc to '255' chars in debug info
#   pragma warning( disable : 4800 ) // forcing value to bool performance warning
#   pragma warning( disable : 4018 ) // '<' signed/unsigned mismatch
#   define CPPTL_NO_VECTOR_STD_ITERATOR
#   define CPPTL_NO_FUNCTION_TEMPLATE_ORDERING
#   define CPPTL_NO_TEMPLATE_PARTIAL_SPECIALIZATION
#   define CPPTL_HAS_INT64 1
#   define CPPTL_NO_INT64_TO_DOUBLE // no conversion from int64 to double
#   define CPPTL_NO_ADL // No argument dependent look-up
#  endif

#  if _MSC_VER >= 1200    // VC++ 6 and above
#   undef CPPTL_NO_ENUM_STATIC_CONSTANT
#  endif

#  if _MSC_VER <= 1300    // VC++ 7.0 or before
// VC++ 7.0 does have deduced typename, but their behavior is not consistent with
// VC++ 7.1. Since it is not required to compile, we just pretend it's not available.
#   define CPPTL_NO_DEDUCED_TYPENAME
#   define CPPTL_NO_TEMPLATE_PARTIAL_SPECIALIZATION
# endif

#  if _MSC_VER >= 1310 // VC++ 7.1
#   define CPPTL_HAS_INT64 1
//#  define CPPTL_HAS_LONGLONG 1    // supported but not by default.
#   pragma warning( disable : 4800 ) // forcing value to bool performance warning
#   pragma warning( disable : 4018 ) // '<' signed/unsigned mismatch
#   pragma warning( disable : 4180 ) // qualifier applied to function type has no meaning
#  endif

# ifndef _CPPRTTI // RTTI is not enabled
#  define CPPTL_NO_RTTI
# endif

#  define CPPTL_HAS__SNPRINTF 1

# endif


// STL specifics
///////////////////////////////////////////////////////////////////////////
# if defined(__GLIBCPP__) || defined(__GLIBCXX__) // GNU libstdc++ 3
#  if !defined(_GLIBCPP_USE_WCHAR_T) && !defined(_GLIBCXX_USE_WCHAR_T)
#   define CPPTL_NO_STD_WSTRING 1
#  endif
# endif

# if defined(__GNUC__) && (__GNUC__ < 3) && !defined(__STL_USE_NEW_IOSTREAMS)
#  define CPPTL_NO_SSTREAM 1  // No #include <sstream>
#  define CPPTL_NO_STL_SEQUENCE_AT 1 // No std::vector<>::at std::deque<>::at
#  define CPPTL_NO_STD_WSTRING 1
# endif

// OS specifics
///////////////////////////////////////////////////////////////////////////

# if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
#  define CPPTL_USE_WIN32_ATOMIC 1
#  define CPPTL_USE_WIN32_THREAD 1
# elif defined(linux) || defined(__linux) || defined(__linux__)
#  define CPPTL_USE_PTHREAD_THREAD 1
# elif defined(sun) || defined(__sun)
#  define CPPTL_USE_PTHREAD_THREAD 1
# elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
#  define CPPTL_USE_PTHREAD_THREAD 1
# elif defined(__IBMCPP__)
#  define CPPTL_USE_PTHREAD_THREAD 1
# endif

// Common to all compilers
///////////////////////////////////////////////////////////////////////////

/// CPPTL_TYPENAME is defined as 'typename' if the compiler requires deduced
/// typename in template.
# if defined(CPPTL_NO_DEDUCED_TYPENAME)
#  define CPPTL_TYPENAME
#else
#  define CPPTL_TYPENAME typename
# endif

/// CPPTL_STATIC_CONSTANT is defined to declare a static constant in a class
/// struct A { CPPTL_STATIC_CONSTANT( x, 1 ) }; // A::x = 1
# if defined(CPPTL_NO_ENUM_STATIC_CONSTANT)
#  define CPPTL_STATIC_CONSTANT( type, assignment ) \
              static const int assignment
# else
#  define CPPTL_STATIC_CONSTANT( type, assignment ) \
              enum { assignment }
# endif

/// CPPTL_NO_THROW is used for std::exception subclass destructor throw() specification.
# if defined(CPPTL_NO_NO_THROW_SPECIFICATION)
#  define CPPTL_NO_THROW
# else
#  define CPPTL_NO_THROW throw()
#endif

/// CPPTL_NO_VECTOR_ITERATOR_VALUE_TYPE_DEDUCTION is defined if it not possible to
/// deduced the value_type of std::vector<>::iterator (it is a pointer and there is
/// no partial specialization support).
# if defined(CPPTL_NO_VECTOR_STD_ITERATOR)  && defined(CPPTL_NO_TEMPLATE_PARTIAL_SPECIALIZATION)
#  define CPPTL_NO_VECTOR_ITERATOR_VALUE_TYPE_DEDUCTION 1 // might works with STLPort
# endif

/// CPPTL_NO_TYPE_DEFAULT_ARG is defined if the compiler does not support the following construct:
/// template<class ValueType> void doSomething( CppTL::Type<ValueType> type = CppTL::Type<ValueType>() );
/// With allow for the following usage:
/// doSomething<int>();
/// Some compiler can not deduce type correctly or may cause silent linker error (all function
/// instantion for different type being recognized as a single one).
/// If this construct is not supported, the portable call syntax should be use:
/// doSomething( CppTL::Type<int>() );
/// @todo Should this be kept. It is not used at the time...
# if defined(CPPTL_NO_TYPE_DEFAULT_ARG)
#  define CPPTL_TYPE_DEFAULT_ARG(aType)
# else
#  define CPPTL_TYPE_DEFAULT_ARG(aType) = ::CppTL::Type<aType>()
# endif


// CPPTL_NO_STL_SEQUENCE_AT is defined if the STL does not provide std::vector<>::at
# if defined(CPPTL_NO_STL_SEQUENCE_AT)
#  define CPPTL_AT( container, index ) container[index]
# else
#  define CPPTL_AT( container, index ) (container).at(index)
# endif

# if !defined(CPPTL_HAS_THREAD)
#  if CPPTL_USE_WIN32_THREAD || CPPTL_USE_PTHREAD_THREAD
#   define CPPTL_HAS_THREAD 1
#  endif
# endif

# if !CPPTL_HAS_THREAD_SAFE_ATOMIC_COUNTER
#  if CPPTL_USE_PTHREAD_THREAD && !CPPTL_USE_WIN32_ATOMIC
#   define CPPTL_USE_PTHREAD_ATOMIC 1
#  endif
#  if CPPTL_USE_PTHREAD_ATOMIC || CPPTL_USE_WIN32_ATOMIC
#   define CPPTL_HAS_THREAD_SAFE_ATOMIC_COUNTER 1
#  endif
# endif


// auto-link specification
///////////////////////////////////////////////////////////////////////////

# define CPPTL_API




// Macro tools...
///////////////////////////////////////////////////////////////////////////


/*! Joins to symbol after expanding them into string.
 *
 * Use this macro to join two symbols. Example of usage:
 *
 * \code
 * #define MAKE_UNIQUE_NAME(prefix) CPPTL_JOIN( prefix, __LINE__ )
 * \endcode
 *
 * The macro defined in the example concatenate a given prefix with the line number
 * to obtain a 'unique' identifier.
 *
 * \internal From boost documentation:
 * The following piece of macro magic joins the two 
 * arguments together, even when one of the arguments is
 * itself a macro (see 16.3.1 in C++ standard).  The key
 * is that macro expansion of macro arguments does not
 * occur in CPPUT_JOIN2 but does in CPPTL_JOIN.
 */
#define CPPTL_JOIN( symbol1, symbol2 ) _CPPTL_DO_JOIN( symbol1, symbol2 )

/// \internal
#define _CPPTL_DO_JOIN( symbol1, symbol2 ) _CPPTL_DO_JOIN2( symbol1, symbol2 )

/// \internal
#define _CPPTL_DO_JOIN2( symbol1, symbol2 ) symbol1##symbol2

/*! Adds the line number to the specified string to create a unique identifier.
 * \param prefix Prefix added to the line number to create a unique identifier.
 * \see CPPUT_TEST_SUITE_REGISTRATION for an example of usage.
 */
#define CPPTL_MAKE_UNIQUE_NAME( prefix ) CPPTL_JOIN( prefix, __LINE__ )

#define CPPTL_MIN( a, b ) ((a) < (b) ? a : b)
#define CPPTL_MAX( a, b ) ((a) > (b) ? a : b)

/*! Cause a compilation error if a type is not complete.
 * Typical use if before call to delete operator (ScopedPtr, SharedPtr...).
 * (based on boost.checked_delete)
 */
#define CPPTL_CHECK_TYPE_COMPLETE( Type )                        \
    {                                                            \
       typedef char typeMustBeComplete[ sizeof(Type) ? 1 : -1 ]; \
       (void)sizeof(typeMustBeComplete);                         \
    }

/// Evaluates to the number of elements in the array
#define CPPTL_ARRAY_SIZE( array ) (sizeof(array)/sizeof((array)[0]))

///////////////////////////////////////////////////////////////////////////

#ifdef NDEBUG
# define CPPTL_ASSERT_MESSAGE( cond, message ) \
   (void)(0)

# define CPPTL_DEBUG_ASSERT_UNREACHABLE \
   (void)(0)
#else
# include <assert.h>
# define CPPTL_ASSERT_MESSAGE( cond, message ) \
   assert( (cond)  &&  message )

# define CPPTL_DEBUG_ASSERT_UNREACHABLE \
   assert( false  &&  "unreachable code" )
#endif

namespace CppTL {

// defines portable int64_t  &&  uint64_t
# ifndef CPPTL_NO_INT64
#  if CPPTL_HAS_LONGLONG
      typedef long long int64_t;
      typedef unsigned long long uint64_t;
#  elif CPPTL_HAS_INT64
      typedef __int64 int64_t;
      typedef unsigned __int64 uint64_t;
#  else
#   define CPPTL_NO_INT64 1
#  endif
# endif


#ifndef CPPTL_NO_INT64
   typedef CppTL::int64_t LargestInt;
   typedef CppTL::uint64_t LargestUnsignedInt;
#else
   typedef int LargestInt;
   typedef unsigned int LargestUnsignedInt;
#endif


   typedef unsigned int size_type;

   template<class T>
   struct Type
   {
      typedef T type;
   };

   /// Base class for non copyable class.
   class CPPTL_API NonCopyable
   {
   public:
      NonCopyable()
      {
      }

      ~NonCopyable()
      {
      }

   private:
      NonCopyable( const NonCopyable &other );
      void operator =( const NonCopyable &other );
   };

   template<class T>
   inline void
   trivialSwap( T &left, T &right )
   {
      T temp( left );
      left = right;
      right = temp;
   }

   template<class T>
   void checkedDelete( T *p )
   {
      CPPTL_CHECK_TYPE_COMPLETE( T )
      delete p;
   }

   template<class T>
   void checkedArrayDelete( T *p )
   {
      CPPTL_CHECK_TYPE_COMPLETE( T )
      delete[] p;
   }

   template<class TargetType, class SourceType>
   TargetType checkedCast( SourceType *pointer, Type<TargetType> )
   {
      TargetType casted = dynamic_cast<TargetType>( pointer );
      CPPTL_ASSERT_MESSAGE( casted != 0, "CPPTL_CHECKED_CAST: wrong TargetType." );
      return casted;
   }

#if defined(NDEBUG)  ||  defined(CPPTL_NO_RTTI)
# define CPPTL_CHECKED_CAST( TargetType, pointer ) \
   static_cast<TargetType>( pointer )
#else
# define CPPTL_CHECKED_CAST( TargetType, pointer ) \
   ::CppTL::checkedCast( pointer, ::CppTL::Type<TargetType>() )
#endif



} // namespace CppTL


#endif // CPPTL_CONFIG_H_INCLUDED
