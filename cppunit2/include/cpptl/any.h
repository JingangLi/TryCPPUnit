#ifndef CPPTL_ANY_H_INCLUDED
# define CPPTL_ANY_H_INCLUDED

//# include "ctti.h"
# include <typeinfo>  // std::bad_cast
# include <cpptl/typeinfo.h>
# include <string>

namespace CppTL {


   class AnyBadCast : public std::bad_cast
   {
   public:
      const char * what() const throw()
      {
         return "AnyBadCast: "
                "Any can not be converted to the specified type.";
      }
   };


/// \cond implementation_detail
   namespace Impl {
      class AnyHolder
      {
      public:
         virtual ~AnyHolder() {}

         virtual TypeId type() const = 0;

         virtual AnyHolder *clone() const = 0;

         virtual void *pointer() = 0;
      };


      template<class AnyType>
      class AnyHolderImpl : public AnyHolder
      {
      public:
         AnyHolderImpl( const AnyType &value )
            : value_( value )
         {
         }

         const AnyType &value() const
         {
            return value_;
         }

      public: // overridden from AnyHolder
         TypeId type() const
         {
            return typeId( Type<AnyType>() );
         }

         void *pointer()
         {
            return &value_;
         }

         AnyHolder *clone() const
         {
            return new AnyHolderImpl<AnyType>( value_ );
         }

      private:
         AnyType value_;
      };

   } // namespace Impl
/// \endcond


   class Any
   {
   public:
      Any();
      Any( char value );
      Any( bool value );
      Any( short value );
      Any( int value );
      Any( long value );
      Any( unsigned short value );
      Any( unsigned int value );
      Any( unsigned long value );
      Any( float value );
      Any( double value );
      Any( long double value );
      Any( const std::string &value );
#if !defined(CPPTL_NO_STD_WSTRING)
      Any( const std::wstring &value );
#endif
      Any( const Any &other );

      Any &operator =( const Any &other );

      TypeId type() const;

      void swap( Any &other );

      bool hasSameType( const Any &other ) const;

      bool isEmpty() const;

      const void *valuePointer() const;

      void *valuePointer();

      void copyValueTo( void *storage );

   //private:     // not private for access by get/set/cast function
      Any( Impl::AnyHolder *holder );

      Impl::AnyHolder *holder_;
   };


   template<class ValueType>
   Any makeAny( const ValueType &newValue )
   {
      return Any( new Impl::AnyHolderImpl<ValueType>( newValue ) );
   }


   // class Any
   // ///////////////////////////////////////////////////////////////

   inline Any::Any()
      : holder_( 0 )
   {
   }

   inline Any::Any( bool value )
      : holder_( new Impl::AnyHolderImpl<bool>( value ) )
   {
   }

   inline Any::Any( char value )
      : holder_( new Impl::AnyHolderImpl<char>( value ) )
   {
   }

   inline Any::Any( short value )
      : holder_( new Impl::AnyHolderImpl<short>( value ) )
   {
   }

   inline Any::Any( int value )
      : holder_( new Impl::AnyHolderImpl<int>( value ) )
   {
   }

   inline Any::Any( long value )
      : holder_( new Impl::AnyHolderImpl<long>( value ) )
   {
   }

   inline Any::Any( unsigned short value )
      : holder_( new Impl::AnyHolderImpl<unsigned short>( value ) )
   {
   }

   inline Any::Any( unsigned int value )
      : holder_( new Impl::AnyHolderImpl<unsigned int>( value ) )
   {
   }

   inline Any::Any( unsigned long value )
      : holder_( new Impl::AnyHolderImpl<unsigned long>( value ) )
   {
   }

   inline Any::Any( float value )
      : holder_( new Impl::AnyHolderImpl<float>( value ) )
   {
   }

   inline Any::Any( double value )
      : holder_( new Impl::AnyHolderImpl<double>( value ) )
   {
   }

   inline Any::Any( long double value )
      : holder_( new Impl::AnyHolderImpl<long double>( value ) )
   {
   }

   inline Any::Any( const std::string &value )
      : holder_( new Impl::AnyHolderImpl<std::string>( value ) )
   {
   }

#if !defined(CPPTL_NO_STD_WSTRING)
   inline Any::Any( const std::wstring &value )
      : holder_( new Impl::AnyHolderImpl<std::wstring>( value ) )
   {
   }
#endif


   inline Any::Any( const Any &other )
      : holder_( other.holder_ ? other.holder_->clone() : 0 )
   {
   }

   inline Any &
   Any::operator =( const Any &other )
   {
      Any tmp( other );
      swap( tmp );
      return *this;
   }

   inline TypeId 
   Any::type() const
   {
      return holder_ ? holder_->type() : typeId( Type<void>() );
   }

   inline void 
   Any::swap( Any &other )
   {
      Impl::AnyHolder *tmp = holder_;
      holder_ = other.holder_;
      other.holder_ = tmp;
   }

   inline bool 
   Any::hasSameType( const Any &other ) const
   {
      return type() == other.type();
   }

   inline bool 
   Any::isEmpty() const
   {
      return holder_ == 0;
   }

   inline Any::Any( Impl::AnyHolder *holder )
      : holder_( holder )
   {
   }

   inline const void *
   Any::valuePointer() const
   {
      // this const cast avoid extra virtual function
      Impl::AnyHolder *holder = const_cast<Impl::AnyHolder *>( holder_ );
      return holder ? holder->pointer() : 0;
   }

   inline void *
   Any::valuePointer()
   {
      return holder_ ? holder_->pointer() : 0;
   }


} // namespace CppTL



// Those function are imported in the global namespace since all compiler don't have
// argument dependent look-up. They all take type 'Any' in parameter which is in a namespace.

template<class ValueType>
CppTL::Any &set( CppTL::Any &value, 
                 const ValueType &newValue )
{
   value = CppTL::Any( new CppTL::Impl::AnyHolderImpl<ValueType>( newValue ) );
   return value;
}

template<class ValueType>
const ValueType &any_cast( const CppTL::Any &value, CppTL::Type<ValueType> )
{
   if ( value.type() != CppTL::typeId( CppTL::Type<ValueType>() ) )
      throw CppTL::AnyBadCast();

   return static_cast<CppTL::Impl::AnyHolderImpl<ValueType> *>( value.holder_ )->value();
}

template<class ValueType>
const ValueType *get( const CppTL::Any *value, CppTL::Type<ValueType> )
{
   if ( value->type() != CppTL::typeId( CppTL::Type<ValueType>() ) )
      return 0;
   return &( static_cast<CppTL::Impl::AnyHolderImpl<ValueType> *>( value->holder_ )->value() );
}


#endif // CPPTL_ANY_H_INCLUDED
