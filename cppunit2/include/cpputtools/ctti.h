#ifndef CPPUTTOOLS_CONFIGURATION_CTTI_H_INCUDED
# define CPPUTTOOLS_CONFIGURATION_CTTI_H_INCUDED

#include <string>

// /////////////////////////////////////////////////////
// /////////////////////////////////////////////////////
// CTTI using std::type_info & typeid
// /////////////////////////////////////////////////////
// /////////////////////////////////////////////////////
# ifndef CPPUTTOOLS_CTTI_NO_TYPEINFO

#  include <typeinfo>

namespace CppUTTools {

class TypeId
{
public:
   TypeId( const std::type_info &typeId )
      : type_( typeId )
   {
   }

   bool operator ==( const TypeId &other ) const
   {
      return type_ == other.type_;
   }

   bool operator !=( const TypeId &other ) const
   {
      return !( *this == other );
   }

   bool operator <( const TypeId &other ) const
   {
      return type_.before( other.type_ );
   }
private:
   const std::type_info &type_;
};


// Compile Time Type Information functions
// //////////////////////////////////////////////////////////////////

template<class Type>
TypeId typeId()
{
   return typeid( Type );
}


} // namespace CommandLineOptions



// /////////////////////////////////////////////////////
// /////////////////////////////////////////////////////
// CTTI using hande-made mecanism (template overload)
// /////////////////////////////////////////////////////
// /////////////////////////////////////////////////////
# else // ifndef CPPUTTOOLS_CTTI_NO_TYPEINFO

class TypeId
{
public:
   TypeId( const char *type )
      : type_( type )
   {
   }

   bool operator <( const TypeId &other ) const
   {
      return strcmp( type_, other.type_) < 0;
   }

   bool operator ==( const TypeId &other ) const
   {
      return strcmp( type_, other.type_ ) == 0;
   }

   bool operator !=( const TypeId &other ) const
   {
      return !( *this == other );
   }

private:
   const char *type_;
};

template<class Type>
TypeId typeId<Type>()
{
   return __error__typeId_function_not_overloaded;
}

#define CPPUTTOOLS_DECLARE_CTTI( type )   \
   template<>                             \
   TypeId typeId<type>()                  \
   {                                      \
      return TypeId( #type );             \
   }


CPPUTTOOLS_DECLARE_CTTI( char );
CPPUTTOOLS_DECLARE_CTTI( signed char );
CPPUTTOOLS_DECLARE_CTTI( unsigned char );
CPPUTTOOLS_DECLARE_CTTI( short );
CPPUTTOOLS_DECLARE_CTTI( unsigned short );
CPPUTTOOLS_DECLARE_CTTI( int );
CPPUTTOOLS_DECLARE_CTTI( unsigned int );
CPPUTTOOLS_DECLARE_CTTI( long );
CPPUTTOOLS_DECLARE_CTTI( unsigned long );
CPPUTTOOLS_DECLARE_CTTI( float );
CPPUTTOOLS_DECLARE_CTTI( double );
CPPUTTOOLS_DECLARE_CTTI( long double );
CPPUTTOOLS_DECLARE_CTTI( const char * );
CPPUTTOOLS_DECLARE_CTTI( const wchar_t * );
CPPUTTOOLS_DECLARE_CTTI( std::string );
CPPUTTOOLS_DECLARE_CTTI( std::wstring );


# endif




#endif // CPPUTTOOLS_CONFIGURATION_CTTI_H_INCUDED
