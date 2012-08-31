#ifndef CPPTL_REFLECTION_INL_INCLUDED
# define CPPTL_REFLECTION_INL_INCLUDED

namespace CppTL {

// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// Class Class
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////

   inline
   Class &
   Class::create( const ConstString &name,
                  TypeId type )
   {
      ClassPtr class_( new Class( name, type ) );
      registry()[ name ] = class_;
      return *class_;
   }

   inline
   const Class *
   Class::findClass( const ConstString &name )
   {
      ClassesByName::const_iterator it = registry().find( name );
      if ( it == registry().end() )
         return 0;
      return it->second.get();
   }

   inline
   Class::Class( const ConstString &name,
                 TypeId type )
      : name_( name )
      , type_( type )
   {
   }

   inline
   Class::ClassesByName &
   Class::registry()
   {
      static ClassesByName classes_;
      return classes_;
   }

   inline
   ConstString 
   Class::getName() const
   {
      return name_;
   }

   inline
   const Method *
   Class::findMethod( const ConstString &name ) const
   {
      MethodsByName::const_iterator it = methodsByName_.find( name );
      if ( it == methodsByName_.end() )
         return 0;
      return it->second.get();
   }

   inline
   void 
   Class::addMethod( const ConstString &name, 
                     const Impl::Invokable &invokable )
   {
      MethodPtr method( new Method( name, invokable ) );
      methodsByName_[name] = method;
   }

   inline
   const Attribut *
   Class::findAttribut( const ConstString &name ) const
   {
      AttributsByName::const_iterator it = attributsByName_.find( name );
      if ( it == attributsByName_.end() )
         return 0;
      return it->second.get();
   }


   inline
   void 
   Class::addAttribut( const ConstString &name, 
                       const Impl::AttributAccessorPtr &accessor )
   {
      AttributPtr attribut( new Attribut( name, accessor ) );
      attributsByName_[name] = attribut;
   }

// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// Class Method
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////

inline
Method::Method( const ConstString &name,
                const Impl::Invokable &invokable )
   : name_( name )
   , invokable_( invokable )
{
}


inline
Any
Method::invoke( const Any &object,
                const MethodParameters &args ) const
{
   Any result;
   Impl::MethodCall call( *this, object, args, result );
   invokable_.invoke( call );
   return result;
}


inline
ConstString 
Method::getName() const
{
   return name_;
}


inline
size_type 
Method::getArgCount() const
{
   return size_type(invokable_.argTypes_.size());
}


inline
TypeId 
Method::getArgType( size_type index ) const
{
   return CPPTL_AT( invokable_.argTypes_, index);
}


inline
TypeId 
Method::getReturnType() const
{
   return typeId( Type<void>() );
}


inline
AnyEnumerator<TypeId> 
Method::getArgTypes() const
{
   return Enum::anyContainer( invokable_.argTypes_, Type<TypeId>() );
}


// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// Class Attribut
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////

inline
Attribut::Attribut( const ConstString &name,
                    const Impl::AttributAccessorPtr &accessor )
   : name_( name )
   , accessor_( accessor )
{
}


inline
ConstString 
Attribut::getName() const
{
   return name_;
}


inline
TypeId 
Attribut::getType() const
{
   return accessor_->getType();
}


inline
void 
Attribut::set( const Any &object, 
               const Any &value ) const
{
   accessor_->set( object, value );
}


inline
Any 
Attribut::get( const Any &object ) const
{
   return accessor_->get( object );
}


// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// Class Invokable
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////

/// \cond implementation_detail
namespace Impl {

   inline
   Invokable::Invokable( const InvokableBasePtr &invokable )
      : invokable_( invokable )
      , returnType_( typeId( Type<void>() ) )
   {
   }

   inline
   void 
   Invokable::invoke( MethodCall &call ) const
   {
      invokable_->invoke( call );
   }

} // namespace Impl
/// \endcond


} // namespace CppTL


#endif // CPPTL_REFLECTION_INL_INCLUDED
