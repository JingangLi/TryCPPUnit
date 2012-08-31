#ifndef CPPTL_TYPENAME_H_INCLUDED
# define CPPTL_TYPENAME_H_INCLUDED

# include <cpptl/forwards.h>
# include <cpptl/conststring.h>
# ifndef CPPTL_NO_RTTI
#  include <cpptl/stringtools.h>
#  include <typeinfo>
# endif

// AIX: demangle library
// http://publib.boulder.ibm.com/infocenter/pseries/index.jsp?topic=/com.ibm.vacpp7a.doc/proguide/ref/demangle.htm

namespace CppTL {

# ifndef CPPTL_NO_RTTI
   inline std::string CPPUT_API 
   demangleTypeInfoName( const std::type_info &type )
   {
      CppTL::ConstString typeName( type.name() );
      const int classKeywordLength = 6;
      size_type startIndex = 0;
      if ( typeName.substr( 0, classKeywordLength ) == "class " )
         startIndex = classKeywordLength;
      while ( startIndex < typeName.length()  &&  isDigit( typeName[startIndex] ) )
         ++startIndex;
      return typeName.substr(startIndex).str();
   }
#endif

   template<class AType>
   std::string getTypeName( CppTL::Type<AType>, 
                            const char *hint )
   {
# ifndef CPPTL_NO_RTTI
      return demangleTypeInfoName( typeid( AType ) );
# else
      return hint;
# endif
   }


   template<typename Object>
   std::string getObjectTypeName( const Object &object,
                                  const char *hint )
   {
# ifndef CPPTL_NO_RTTI
      return demangleTypeInfoName( typeid(object) );
# else
      return hint;
# endif
   }

} // namespace CppUT


#endif // CPPTL_TYPENAME_H_INCLUDED
