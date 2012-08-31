#ifndef CPPTL_AUTOLINK_H_INCLUDED
# define CPPTL_AUTOLINK_H_INCLUDED

# if !defined(CPPTL_NO_AUTOLINK)  &&  !defined(CPPTL_DLL_BUILD)
#  define CPPTL_AUTOLINK_NAME "cpptl"
#  undef CPPTL_AUTOLINK_DLL
#  ifdef CPPTL_DLL
#   define CPPTL_AUTOLINK_DLL
#  endif
#  include "autolink.h"
# endif

#endif // CPPTL_AUTOLINK_H_INCLUDED
