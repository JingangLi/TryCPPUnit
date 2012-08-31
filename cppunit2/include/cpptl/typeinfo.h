#ifndef CPPTL_TYPEINFO_H_INCUDED
# define CPPTL_TYPEINFO_H_INCUDED

# include "config.h"
# include <string>

// /////////////////////////////////////////////////////
// /////////////////////////////////////////////////////
// CTTI using std::type_info & typeid
// /////////////////////////////////////////////////////
// /////////////////////////////////////////////////////
# ifndef CPPTL_NO_RTTI

#  include <typeinfo>

namespace CppTL {

class TypeId
{
public:
   TypeId( const std::type_info &typeId )
      : type_( &typeId )
   {
   }

   bool operator ==( const TypeId &other ) const
   {
      return *type_ == *(other.type_);
   }

   bool operator !=( const TypeId &other ) const
   {
      return !( *this == other );
   }

   bool operator <( const TypeId &other ) const
   {
      // Notes: experience has proven this to be bugged:
      // in a staticaly linked programm on AIX, 
      // two distinct instances of std::type_info for the same type.
      // They compared to false. Can be worked-around by comparing name though.
      return type_->before( *(other.type_) );
   }
private:
   const std::type_info *type_;
};


// Compile Time Type Information functions
// //////////////////////////////////////////////////////////////////

template<class AType>
TypeId typeId( Type<AType> )
{
   return typeid( AType );
}


} // namespace CppTL


#define CPPTL_DECLARE_TYPEINFO( AType )
#define CPPTL_DECLARE_TYPE_AND_PTR_INFO( AType )

// /////////////////////////////////////////////////////
// /////////////////////////////////////////////////////
// CTTI using hande-made mecanism (template overload)
// /////////////////////////////////////////////////////
// /////////////////////////////////////////////////////
# else

namespace CppTL {
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

#ifdef CPPTL_NO_FUNCTION_TEMPLATE_ORDERING
inline TypeId typeId( ... )
#else
template<class AType>
inline TypeId typeId( Type<AType> )
#endif
{
   return __error__typeId_function_not_overloaded;
}

} // namespace CppTL

#define CPPTL_DECLARE_TYPEINFO( AType )       \
   namespace CppTL {                          \
      inline TypeId typeId( Type<AType> )     \
      {                                       \
         return TypeId( #AType );             \
      }                                       \
   }

#define CPPTL_DECLARE_TYPE_AND_PTR_INFO( AType )       \
   CPPTL_DECLARE_TYPEINFO( AType );                    \
   CPPTL_DECLARE_TYPEINFO( AType * )

CPPTL_DECLARE_TYPEINFO( void );
CPPTL_DECLARE_TYPEINFO( bool );
CPPTL_DECLARE_TYPEINFO( char );
CPPTL_DECLARE_TYPEINFO( signed char );
CPPTL_DECLARE_TYPEINFO( unsigned char );
CPPTL_DECLARE_TYPEINFO( short );
CPPTL_DECLARE_TYPEINFO( unsigned short );
CPPTL_DECLARE_TYPEINFO( int );
CPPTL_DECLARE_TYPEINFO( unsigned int );
CPPTL_DECLARE_TYPEINFO( long );
CPPTL_DECLARE_TYPEINFO( unsigned long );
CPPTL_DECLARE_TYPEINFO( float );
CPPTL_DECLARE_TYPEINFO( double );
CPPTL_DECLARE_TYPEINFO( long double );
CPPTL_DECLARE_TYPEINFO( const char * );
CPPTL_DECLARE_TYPEINFO( const wchar_t * );
CPPTL_DECLARE_TYPEINFO( std::string );
CPPTL_DECLARE_TYPEINFO( std::wstring );

# endif

#endif // CPPTL_TYPEINFO_H_INCUDED
