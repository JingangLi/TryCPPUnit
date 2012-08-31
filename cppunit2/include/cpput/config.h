#ifndef CPPUT_CONFIG_H_INCLUDED
# define CPPUT_CONFIG_H_INCLUDED

# include <cpptl/config.h>

# define CPPUT_BOOST_FRIENDLY
//# define CPPUT_DLL_SUPPORT
//# define CPPUT_USE_WIN32_DLL     // LoadLibrary
//# define CPPUT_USE_DL_DLL      // dlopen
//# define CPPUT_USE_SHL_DLL     // shl_open
//# define CPPUT_USE_BEOS_DLL

# define CPPUT_USE_RTTI_TO_NAME_SUITE 1

// compiler specific stuffs...
///////////////////////////////////////////////////////////////////////////

# if _MSC_VER <= 1200    // VC++ 6 or before
#  pragma warning( disable : 4786 ) // ident trunc to '255' chars in debug info
#  define CPPUT_STD_VECTOR_ITERATOR_IS_POINTER 1
#  define CPPUT_HAS_FUNCTION_TEMPLATE_ORDERING 0
#  define CPPUT_HAS_TEMPLATE_PARTIAL_SPECIALIZATION 0
# endif

# if _MSC_VER <= 1300    // VC++ 7.0 or before
// VC++ 7.0 does have deduced typename, but their behavior is not consistent with
// VC++ 7.1. Since it is not required to compile, we just pretend it's not available.
#  define CPPUT_NO_DEDUCED_TYPENAME
# endif

# if _MSC_VER >= 1310 // VC++ 7.1
#  define CPPUT_HAS_FUNCTION_TEMPLATE_ORDERING 1
#  define CPPUT_HAS_TEMPLATE_PARTIAL_SPECIALIZATION 1
#  pragma warning( disable : 4800 ) // forcing value to bool performance warning
#  pragma warning( disable : 4018 ) // '<' signed/unsigned mismatch
# endif

// compiler suffix...
///////////////////////////////////////////

// Set CPPUT_DEDUCED_TYPENAME depending on the compiler support for deduced typename.
# ifdef CPPUT_NO_DEDUCED_TYPENAME
#  define CPPUT_DEDUCED_TYPENAME
# else
#  define CPPUT_DEDUCED_TYPENAME typename
# endif

# ifdef CPPUT_NO_FUNCTION_TEMPLATE_ORDERING
#  define CPPUT_HAS_FUNCTION_TEMPLATE_ORDERING 0
# endif

# ifdef CPPUT_NO_TEMPLATE_PARTIAL_SPECIALIZATION
#  define CPPUT_HAS_TEMPLATE_PARTIAL_SPECIALIZATION 0
# endif

# ifdef CPPUT_USE_BOOST_SHARED_PTR
#  undef CPPUT_BOOST_FRIENDLY
#  define CPPUT_BOOST_FRIENDLY 1
# endif

# if !defined(CPPUT_NO_DLL_SUPPORT)
# define CPPUT_DLL_SUPPORT 1
# endif

// STL specific stuffs...
///////////////////////////////////////////////////////////////////////////


// define CPPUT_DLL_BUILD when building CppUnit dll.
# ifdef CPPUT_DLL_BUILD
#  define CPPUT_API __declspec(dllexport)
# endif

// define CPPUT_DLL when linking to CppUnit dll.
# if defined( CPPUT_DLL )
#  define CPPUT_API __declspec(dllimport)
# endif

// generating or linking to a static library
#if !defined( CPPUT_API )
#  define CPPUT_API
#endif

// Auto-link feature, define symbol CPPUT_NO_AUTO_LINK to disable
# if !defined(CPPUT_NO_AUTO_LINK)
#  define CPPUT_LIB_NAME "cpput"
//#  include <cpput/autolink.h>
# endif

#endif // CPPUT_CONFIG_H_INCLUDED
