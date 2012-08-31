#ifndef CPPUT_TRANSLATE_H_INCLUDED
# define CPPUT_TRANSLATE_H_INCLUDED

# include <cpput/config.h>
# include <cpptl/conststring.h>


namespace CppUT {

/// Translate the specified message
inline std::string CPPUT_API translate( const char *message )
{
   return message;
}


} // namespace CppUT


#endif // CPPUT_TRANSLATE_H_INCLUDED
