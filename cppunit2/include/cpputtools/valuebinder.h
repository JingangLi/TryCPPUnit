#ifndef CPPUTTOOLS_VALUEBINDER_H_INCLUDED
# define CPPUTTOOLS_VALUEBINDER_H_INCLUDED

# include "value.h"


namespace CppUTTools {

namespace Impl {

   class ValueBinderImpl
   {
   public:
      virtual ~ValueBinderImpl() {}

      virtual void set( void *storage, const Value &value ) const = 0;

      virtual Value get( const void *storage ) const = 0;

      virtual TypeId type() const = 0;
   };

   template<class ValueType>
   class SpecificValueBinderImpl : public ValueBinderImpl
   {
   public:
      static const ValueBinderImpl &instance()
      {
         static SpecificValueBinderImpl dummy;
         return dummy;
      }

   public: // overridden from ValueBinderImpl
      void set( void *storage, const Value &value ) const
      {
         if ( !value.isEmpty() )
            *( static_cast<ValueType *>( storage ) )= ::get<ValueType>( value );
      }

      Value get( const void *storage ) const
      {
         return value( *( static_cast<const ValueType *>( storage ) ) );
      }

      TypeId type() const
      {
         return typeId<ValueType>();
      }
   };

   class VoidValueBinderImpl : public ValueBinderImpl
   {
   public:
      static const ValueBinderImpl &instance()
      {
         static VoidValueBinderImpl dummy;
         return dummy;
      }

   public: // overridden from ValueBinderImpl
      void set( void *storage, const Value &value ) const
      {
      }

      Value get( const void *storage ) const
      {
         return Value();
      }

      TypeId type() const
      {
         return typeId<void>();
      }
   };

} // namespace Impl

class ValueBinder
{
public:
   ValueBinder();

   void set( const Value &value ) const;

   Value get() const;

   TypeId type() const;

   bool isEmpty() const;

public: // private, but friendly template function
   ValueBinder( void *storage,
                const Impl::ValueBinderImpl &impl );

private:
   void *storage_;
   const Impl::ValueBinderImpl *impl_;
};


template<class ValueType>
ValueBinder valueBinder( ValueType &storage )
{
   return ValueBinder( &storage, Impl::SpecificValueBinderImpl<ValueType>::instance() );
}


// Implementation
// ////////////////////////////////////////////////////////////

inline
ValueBinder::ValueBinder()
   : storage_( 0 )
   , impl_( &Impl::VoidValueBinderImpl::instance() )
{
}


inline
ValueBinder::ValueBinder( void *storage,
                          const Impl::ValueBinderImpl &impl )
   : storage_( storage )
   , impl_( &impl )
{
}

inline void 
ValueBinder::set( const Value &value ) const
{
   if ( value.type() != type() )
      throw ValueBadCast();

   impl_->set( storage_, value );
}

inline Value 
ValueBinder::get() const
{
   if ( isEmpty() )
      return Value();
   return impl_->get( storage_ );
}


inline TypeId 
ValueBinder::type() const
{
   return impl_->type();
}


inline bool 
ValueBinder::isEmpty() const
{
   return storage_ == 0;
}

} // namespace CppUTTools


#endif // CPPUTTOOLS_VALUEBINDER_H_INCLUDED
