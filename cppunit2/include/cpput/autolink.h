// No gards, this header can be included multiple time

// Generic header to automatically link against a specified library
// The library name prefix must be defined in CPPUT_LIB_NAME.
// CPPUT_LIB_NAME will be undefined after including this header.

// The full library name is build according to the followin rules:
// (0) CPPUT_LIB_NAME: library name prefix (lut,...)
// (a) TOOLSET: vc6, vc7, bcb4, bcb5, bcb6
// (b) THREADING: m(multithreaded), s(single threaded)
// (c) DEBUG MODE: r(release), d(debug)
// (d) LINKAGE: s(static), d(dynamic)
// FULLNAME: 0_a_bcd.lib


#if !defined(CPPUT_LIB_NAME)
# error Macro CPPUT_LIB_NAME should be defined. You should not include this header directly.
#endif


// Select compiler
#if defined(_MSC_VER) && (_MSC_VER == 1200) // VC6
# define CPPUT_LIB_TOOLSET "vc6"
#elif defined(_MSC_VER) && (_MSC_VER >= 1300) //VC7 (.NET 2002)
# define CPPUT_LIB_TOOLSET "vc7"
#elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x560) // CBuilder 6
# define CPPUT_LIB_TOOLSET "bcb6"
#elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x550)
# define CPPUT_LIB_TOOLSET "bcb5"
#elif defined(__BORLANDC__) && (__BORLANDC__ >= 0x540)
# define CPPUT_LIB_TOOLSET "bcb4"
#endif

// Select threading
#if defined(_MT) || defined(__MT__)
# define CPPUT_LIB_THREADING "m"
#else
# define CPPUT_LIB_THREADING "s"
#endif

// Select debug mode
#if defined(_DEBUG)
# define CPPUT_LIB_DEBUG_MODE "d"
#else
# define CPPUT_LIB_DEBUG_MODE "r"
#endif

// Select linkage
#if defined(CPPUT_STATIC_LINK)
# define CPPUT_LIB_LINKAGE "s"
#else
# define CPPUT_LIB_LINKAGE "d"
#endif

// Automatic link
#if defined(CPPUT_LIB_TOOLSET)    && \
    defined(CPPUT_LIB_THREADING)  && \
    defined(CPPUT_LIB_LINKAGE)    && \
    defined(CPPUT_LIB_DEBUG_MODE)
# define CPPUT_LIB_FULL_NAME                                                     \
    CPPUT_LIB_NAME "_" CPPUT_LIB_TOOLSET "_" CPPUT_LIB_THREADING CPPUT_LIB_DEBUG_MODE \
    CPPUT_LIB_LINKAGE ".lib"
# pragma comment(lib,CPPUT_LIB_FULL_NAME)
#endif

#undef CPPUT_LIB_TOOLSET
#undef CPPUT_LIB_THREADING
#undef CPPUT_LIB_LINKAGE
#undef CPPUT_LIB_DEBUG_MODE
#undef CPPUT_LIB_FULL_NAME
#undef CPPUT_LIB_NAME

