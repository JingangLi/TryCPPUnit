#ifndef CPPUTTOOLS_VALUE_H_INCLUDED
# define CPPUTTOOLS_VALUE_H_INCLUDED

# include "ctti.h"
# include <typeinfo>  // std::bad_cast
# include <string>

/// @todo Implement broken on VC++ 6 because of linker bug:
///       => all get<xxx>() are resolved as the same function !



namespace CppUTTools {


   class ValueBadCast : public std::bad_cast
   {
   public:
      const char * what() const throw()
      {
         return "ValueBadCast: "
                "Value can not be converted to the specified type.";
      }
   };


   namespace Impl {
      class ValueHolder
      {
      public:
         virtual ~ValueHolder() {}

         virtual TypeId type() const = 0;

         virtual ValueHolder *clone() const = 0;

         virtual void *pointer() = 0;
      };


      template<class ValueType>
      class ValueHolderImpl : public ValueHolder
      {
      public:
         ValueHolderImpl( const ValueType &value )
            : value_( value )
         {
         }

         const ValueType &value() const
         {
            return value_;
         }

      public: // overridden from ValueHolder
         TypeId type() const
         {
            return typeId<ValueType>();
         }

         void *pointer()
         {
            return &value_;
         }

         ValueHolder *clone() const
         {
            return new ValueHolderImpl<ValueType>( value_ );
         }

      private:
         ValueType value_;
      };

   } // namespace Impl


   class Value
   {
   public:
      Value();
      Value( char value );
      Value( bool value );
      Value( short value );
      Value( int value );
      Value( long value );
      Value( unsigned short value );
      Value( unsigned int value );
      Value( unsigned long value );
      Value( float value );
      Value( double value );
      Value( long double value );
      Value( const std::string &value );
#if !defined(CPPTL_NO_STD_WSTRING)
      Value( const std::wstring &value );
#endif
      Value( const Value &other );

      Value &operator =( const Value &other );

      TypeId type() const;

      void swap( Value &other );

      bool hasSameType( const Value &other ) const;

      bool isEmpty() const;

      const void *valuePointer() const;

      void *valuePointer();

      void copyValueTo( void *storage );

   //private:     // not private for access by get/set/cast function
      Value( Impl::ValueHolder *holder );

      Impl::ValueHolder *holder_;
   };


   template<class ValueType>
   Value value( const ValueType &newValue )
   {
      return Value( new Impl::ValueHolderImpl<ValueType>( newValue ) );
   }


   // class Value
   // ///////////////////////////////////////////////////////////////

   inline Value::Value()
      : holder_( 0 )
   {
   }

   inline Value::Value( bool value )
      : holder_( new Impl::ValueHolderImpl<bool>( value ) )
   {
   }

   inline Value::Value( char value )
      : holder_( new Impl::ValueHolderImpl<char>( value ) )
   {
   }

   inline Value::Value( short value )
      : holder_( new Impl::ValueHolderImpl<short>( value ) )
   {
   }

   inline Value::Value( int value )
      : holder_( new Impl::ValueHolderImpl<int>( value ) )
   {
   }

   inline Value::Value( long value )
      : holder_( new Impl::ValueHolderImpl<long>( value ) )
   {
   }

   inline Value::Value( unsigned short value )
      : holder_( new Impl::ValueHolderImpl<unsigned short>( value ) )
   {
   }

   inline Value::Value( unsigned int value )
      : holder_( new Impl::ValueHolderImpl<unsigned int>( value ) )
   {
   }

   inline Value::Value( unsigned long value )
      : holder_( new Impl::ValueHolderImpl<unsigned long>( value ) )
   {
   }

   inline Value::Value( float value )
      : holder_( new Impl::ValueHolderImpl<float>( value ) )
   {
   }

   inline Value::Value( double value )
      : holder_( new Impl::ValueHolderImpl<double>( value ) )
   {
   }

   inline Value::Value( long double value )
      : holder_( new Impl::ValueHolderImpl<long double>( value ) )
   {
   }

   inline Value::Value( const std::string &value )
      : holder_( new Impl::ValueHolderImpl<std::string>( value ) )
   {
   }

#if !defined(CPPTL_NO_STD_WSTRING)
   inline Value::Value( const std::wstring &value )
      : holder_( new Impl::ValueHolderImpl<std::wstring>( value ) )
   {
   }
#endif


   inline Value::Value( const Value &other )
      : holder_( other.holder_ ? other.holder_->clone() : 0 )
   {
   }

   inline Value &
   Value::operator =( const Value &other )
   {
      Value tmp( other );
      swap( tmp );
      return *this;
   }

   inline TypeId 
   Value::type() const
   {
      return holder_ ? holder_->type() : typeId<void>();
   }

   inline void 
   Value::swap( Value &other )
   {
      Impl::ValueHolder *tmp = holder_;
      holder_ = other.holder_;
      other.holder_ = tmp;
   }

   inline bool 
   Value::hasSameType( const Value &other ) const
   {
      return type() == other.type();
   }

   inline bool 
   Value::isEmpty() const
   {
      return holder_ == 0;
   }

   inline Value::Value( Impl::ValueHolder *holder )
      : holder_( holder )
   {
   }

   inline const void *
   Value::valuePointer() const
   {
      // this const cast avoid extra virtual function
      Impl::ValueHolder *holder = const_cast<Impl::ValueHolder *>( holder_ );
      return holder ? holder->pointer() : 0;
   }

   inline void *
   Value::valuePointer()
   {
      return holder_ ? holder_->pointer() : 0;
   }


} // namespace CppUTTools



// Those function are imported in the global namespace since all compiler don't have
// argument dependent look-up. They all take type 'Value' in parameter which is in a namespace.

template<class ValueType>
CppUTTools::Value &set( CppUTTools::Value &value, 
                        const ValueType &newValue )
{
   value = CppUTTools::Value( new CppUTTools::Impl::ValueHolderImpl<ValueType>( newValue ) );
   return value;
}

template<class ValueType>
const ValueType &get( const CppUTTools::Value &value )
{
   if ( value.type() != CppUTTools::typeId<ValueType>() )
      throw CppUTTools::ValueBadCast();

   return static_cast<CppUTTools::Impl::ValueHolderImpl<ValueType> *>( value.holder_ )->value();
}

template<class ValueType>
const ValueType *get( const CppUTTools::Value *value )
{
   if ( value.type() != CppUTTools::typeId<ValueType>() )
      return 0;
   return static_cast<CppUTTools::Impl::ValueHolderImpl<ValueType> *>( value.holder_ )->value();
}


#endif // CPPUTTOOLS_VALUE_H_INCLUDED
