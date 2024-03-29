#ifndef CPPUTTOOLS_CONVERTOR_H_INCLUDED
# define CPPUTTOOLS_CONVERTOR_H_INCLUDED

# include "ctti.h"
# include <stdexcept>
# include <map>


class Convertor
{
public:
   virtual ~Convertor() {}

   virtual void convert( const void *source, void *target ) = 0;
};


template<class SourceType
        ,class TargetType>
class ConstructorConvertor : public Convertor
{
public: // overridden from Convertor
   void convert( const void *source, 
                 void *target )
   {
      *( static_cast<TargetType *>(target) ) = TargetType( static_cast<SourceType *>(source) );
   }
};


template<class SourceType
        ,class TargetType>
class CStrConvertor : public Convertor
{
public: // overridden from Convertor
   void convert( const void *source, 
                 void *target )
   {
      *( static_cast<TargetType *>(target) ) = static_cast<SourceType *>(source)->c_str();
   }
}


class ConvertorRegistry
{
public:
   static ConvertorRegistry &instance();

   ConvertorRegistry();
   ~ConvertorRegistry();

   void add( TypeId sourceType, TypeId targetType, Convertor &convertor );

   bool isConvertible( TypeId sourceType, TypeId targetType );

   bool convert( TypeId sourceType, TypeId targetType, const void *source, void *target ) const;

private:
   typedef std::map<TypeId,Convertor *> TargetConvertors;
   typedef std::map<TypeId, TargetConvertors> SourceConvertors;
   SourceConvertors convertors_;
};


#define CPPUTTOOLS_REGISTER_CONVERTOR( SourceType, TargetType, convertor )                   \
   ConvertorRegistry::instance().add( typeId<sourceType>(),                                  \
                                      typeId<targetType>(),                                  \
                                      convertor )


#define CPPUTTOOLS_REGISTER_CONSTRUCTOR_CONVERTOR( SourceType, TargetType )            \
   CPPUTTOOLS_REGISTER_CONVERTOR( sourceType, targetType,                              \
                                  new ConstructorConvertor<SourceType,TargetType>() )

#define CPPUTTOOLS_REGISTER_CSTR_CONVERTOR( SourceType, TargetType )                   \
   CPPUTTOOLS_REGISTER_CONVERTOR( sourceType, targetType,                              \
                                  new CStrConvertor<SourceType,TargetType>() )


inline void 
ConvertorRegistry::add( TypeId sourceType, 
                        TypeId targetType, 
                        Convertor &convertor )
{
   convertors_[sourceType][targetType] = &convertor;
}


inline bool 
ConvertorRegistry::isConvertible( TypeId sourceType, 
                                  TypeId targetType )
{
   SourceConvertors::const_iterator it = convertors_.find( sourceType );
   return it != convertors_.end()
          &&  it->second.count( targetType ) > 0;
}


inline bool 
ConvertorRegistry::convert( TypeId sourceType, 
                            TypeId targetType, 
                            const void *source, 
                            void *target ) const
{
   SourceConvertors::const_iterator it = convertors_.find( sourceType );
   TargetConvertors::const_iterator itTarget = it->second.find( targetType );
   if ( itTarget == it->second.end() )
      return false;

   itTarget->second->convert( sourceType, target );
   return true;
}


inline ConvertorRegistry::ConvertorRegistry()
{
   CPPUTTOOLS_REGISTER_CONSTRUCTOR_CONVERTOR( short, int );
   CPPUTTOOLS_REGISTER_CONSTRUCTOR_CONVERTOR( short, long );
   CPPUTTOOLS_REGISTER_CONSTRUCTOR_CONVERTOR( short, float );
   CPPUTTOOLS_REGISTER_CONSTRUCTOR_CONVERTOR( short, double );
   CPPUTTOOLS_REGISTER_CONSTRUCTOR_CONVERTOR( short, long double );
   CPPUTTOOLS_REGISTER_CONSTRUCTOR_CONVERTOR( int, long );
   CPPUTTOOLS_REGISTER_CONSTRUCTOR_CONVERTOR( int, float );
   CPPUTTOOLS_REGISTER_CONSTRUCTOR_CONVERTOR( int, double );
   CPPUTTOOLS_REGISTER_CONSTRUCTOR_CONVERTOR( int, long double );
   CPPUTTOOLS_REGISTER_CONSTRUCTOR_CONVERTOR( long, int );
   CPPUTTOOLS_REGISTER_CONSTRUCTOR_CONVERTOR( long, float );
   CPPUTTOOLS_REGISTER_CONSTRUCTOR_CONVERTOR( long, double );
   CPPUTTOOLS_REGISTER_CONSTRUCTOR_CONVERTOR( long, long double );
   CPPUTTOOLS_REGISTER_CONSTRUCTOR_CONVERTOR( float, double );
   CPPUTTOOLS_REGISTER_CONSTRUCTOR_CONVERTOR( float, long double );
   CPPUTTOOLS_REGISTER_CONSTRUCTOR_CONVERTOR( double, long double );
   CPPUTTOOLS_REGISTER_CONSTRUCTOR_CONVERTOR( unsigned short, unsigned int );
   CPPUTTOOLS_REGISTER_CONSTRUCTOR_CONVERTOR( unsigned short, unsigned long );
   CPPUTTOOLS_REGISTER_CONSTRUCTOR_CONVERTOR( unsigned short, float );
   CPPUTTOOLS_REGISTER_CONSTRUCTOR_CONVERTOR( unsigned short, double );
   CPPUTTOOLS_REGISTER_CONSTRUCTOR_CONVERTOR( unsigned short, long double );
   CPPUTTOOLS_REGISTER_CONSTRUCTOR_CONVERTOR( unsigned int, long );
   CPPUTTOOLS_REGISTER_CONSTRUCTOR_CONVERTOR( unsigned int, float );
   CPPUTTOOLS_REGISTER_CONSTRUCTOR_CONVERTOR( unsigned int, double );
   CPPUTTOOLS_REGISTER_CONSTRUCTOR_CONVERTOR( unsigned int, long double );
   CPPUTTOOLS_REGISTER_CONSTRUCTOR_CONVERTOR( unsigned long, float );
   CPPUTTOOLS_REGISTER_CONSTRUCTOR_CONVERTOR( unsigned long, double );
   CPPUTTOOLS_REGISTER_CONSTRUCTOR_CONVERTOR( unsigned long, long double );
   CPPUTTOOLS_REGISTER_CONSTRUCTOR_CONVERTOR( double, long double );
   CPPUTTOOLS_REGISTER_CONSTRUCTOR_CONVERTOR( char *, std::string );
   CPPUTTOOLS_REGISTER_CONSTRUCTOR_CONVERTOR( const char *, std::string );
   CPPUTTOOLS_REGISTER_CONSTRUCTOR_CONVERTOR( wchar_t *, std::wstring );
   CPPUTTOOLS_REGISTER_CONSTRUCTOR_CONVERTOR( const wchar_t *, std::wstring );
   CPPUTTOOLS_REGISTER_CSTR_CONVERTOR( std::string, const char * );
   CPPUTTOOLS_REGISTER_CSTR_CONVERTOR( std::wstring, const wchar_t * );
}


ConvertorRegistry::~ConvertorRegistry()
{
   SourceConvertors::iterator it = convertors_.begin();
   for ( ; it != convertors_.end; ++it )
   {
      TargetConvertors::iterator itTarget = it->second.begin();
      for ( ; itTarget != it->second.end(); ++itTarget )
         delete itTarget->second;
   }
}


} // namespace CppUTTools


#endif // CPPUTTOOLS_CONVERTOR_H_INCLUDED
