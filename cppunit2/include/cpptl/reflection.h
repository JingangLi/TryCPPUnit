#ifndef CPPTL_REFLECTION_H_INCLUDED
# define CPPTL_REFLECTION_H_INCLUDED

# include <cpptl/conststring.h>
# include <cpptl/enumerator.h>
# include <cpptl/sharedptr.h>
# include <cpptl/any.h>
# include <map>
# include <vector>



namespace CppTL {

typedef std::vector<Any> MethodParameters;

/// \cond implementation_detail
namespace Impl {
   class AttributAccessor;
   class InvokableBase;

   typedef SharedPtr<AttributAccessor> AttributAccessorPtr;
   typedef SharedPtr<InvokableBase> InvokableBasePtr;

   class MethodCall
   {
   public:
      MethodCall( const Method &method,
                  const Any &holder,
                  const MethodParameters &args,
                  Any &result )
         : method_( method )
         , holder_( holder )
         , args_( args )
         , result_( result )
      {
      }

      const Method &method_;
      const Any &holder_;
      const MethodParameters &args_;
      Any &result_;
   };


   class InvokableBase
   {
   public:
      virtual ~InvokableBase()
      {
      }

      virtual void invoke( MethodCall &call ) const = 0;
   };


   class Invokable
   {
   public:
      Invokable( const Impl::InvokableBasePtr &invokable );

      virtual ~Invokable()
      {
      }

      void invoke( Impl::MethodCall &call ) const;

      TypeId returnType_;
      std::vector<TypeId> argTypes_;
      InvokableBasePtr invokable_;
   };


   class AttributAccessor
   {
   public:
      virtual ~AttributAccessor()
      {
      }

      virtual TypeId getType() const =0;

      virtual Any get( const Any &holder) const = 0;

      virtual void set( const Any &holder,
                        const Any &value ) const = 0;
   };


   template<class Object, class AttributType>
   class AttributAccessorImpl : public AttributAccessor
   {
   public:
      typedef AttributType Object::*Attribut;

      AttributAccessorImpl( Attribut attribut )
         : attribut_( attribut )
      {
      }

      TypeId getType() const
      {
         return typeId( Type<AttributType>() );
      }

      Any get( const Any &holder ) const
      {
         Object &object = *any_cast( holder, Type<Object *>() );
         return makeAny( object.*attribut_ );
      }

      void set( const Any &holder,
                const Any &value ) const
      {
         Object &object = *any_cast( holder, Type<Object *>() );
         object.*attribut_ = any_cast( value, Type<AttributType>() );
      }

   private:
      Attribut attribut_;
   };


   template<class Object, class TheAttributType>
   AttributAccessorPtr makeAccessor( TheAttributType Object::*attribut )
   {
      return AttributAccessorPtr( 
         new AttributAccessorImpl<Object
                                 ,TheAttributType>( attribut ) );
   }

} // namespace Impl
/// \endcond


class Attribut
{
public:
   Attribut( const ConstString &name,
             const Impl::AttributAccessorPtr &accessor );

   ConstString getName() const;

   TypeId getType() const;

   void set( const Any &object, 
             const Any &value ) const;

   Any get( const Any &object ) const;

private:
   Class *class_;
   ConstString name_;
   Impl::AttributAccessorPtr accessor_;
};


class Method
{
public:
   Method( const ConstString &name,
           const Impl::Invokable &invokable );

   Any invoke( const Any &object,
               const MethodParameters &args ) const;

   ConstString getName() const;

   size_type getArgCount() const;

   TypeId getArgType( size_type index ) const;

   TypeId getReturnType() const;

   AnyEnumerator<TypeId> getArgTypes() const;

private:
   Class *class_;
   ConstString name_;
   Impl::Invokable invokable_;
};


class Class
{
public:
   static Class &create( const ConstString &name,
                         TypeId type );

   static const Class *findClass( const ConstString &name );

   ConstString getName() const;

   const Method *findMethod( const ConstString &name ) const;

   void addMethod( const ConstString &name, 
                   const Impl::Invokable &invokable );

   const Attribut *findAttribut( const ConstString &name ) const;

   void addAttribut( const ConstString &name, 
                     const Impl::AttributAccessorPtr &accessor );

private:
   typedef SharedPtr<Class> ClassPtr;
   typedef SharedPtr<Method> MethodPtr;
   typedef SharedPtr<Attribut> AttributPtr;
   typedef std::map<ConstString,ClassPtr> ClassesByName;
   typedef std::map<ConstString,MethodPtr> MethodsByName;
   typedef std::map<ConstString,AttributPtr> AttributsByName;

   static ClassesByName &registry();

   Class( const ConstString &name,
          TypeId type );

   AttributsByName attributsByName_;
   MethodsByName methodsByName_;
   ConstString name_;
   TypeId type_;
};


template<class ClassType>
class AutoRegisterClassReflection
{
public:
   AutoRegisterClassReflection()
   {
      ClassType::registerClassReflection();
   }
};


} // namespace CppTL




# define CPPTL_REFLECT_CLASS_BEGIN( ClassType )                             \
   typedef ClassType _Reflection_SelfType;                                  \
   virtual const ::CppTL::Class *getClass() const                           \
   {                                                                        \
      return ::CppTL::Class::findClass( #ClassType );                       \
   }                                                                        \
   static void registerClassReflection()                                    \
   {                                                                        \
      ::CppTL::Class &class_ = ::CppTL::Class::create( #ClassType,          \
                         ::CppTL::typeId( ::CppTL::Type<ClassType>() ) );

//# define CPPTL_REFLECT_CLASS_EXTEND( ClassType )
# define CPPTL_REFLECT_METHOD( method )                                           \
      {                                                                           \
         ::CppTL::Impl::Invokable invokable = ::CppTL::Impl::makeInvokable(       \
                                              &_Reflection_SelfType::method );    \
         class_.addMethod( #method, invokable );                                  \
      }

# define CPPTL_REFLECT_METHOD_WITH_RETURN( method )                               \
      {                                                                           \
         ::CppTL::Impl::Invokable invokable = ::CppTL::Impl::makeInvokableR(      \
                                              &_Reflection_SelfType::method );    \
         class_.addMethod( #method, invokable );                                  \
      }

# define CPPTL_REFLECT_RENAMED_ATTRIBUT( attribut, name )                           \
      {                                                                             \
         ::CppTL::Impl::AttributAccessorPtr accessor = ::CppTL::Impl::makeAccessor( \
                                              &_Reflection_SelfType::attribut );    \
         class_.addAttribut( name, accessor );                                      \
      }

# define CPPTL_REFLECT_ATTRIBUT( attribut )                 \
   CPPTL_REFLECT_RENAMED_ATTRIBUT( attribut, #attribut )

# define CPPTL_REFLECT_CLASS_END() \
   }

# define CPPTL_REFLECT_REGISTER_CLASS( ClassType )          \
   static ::CppTL::AutoRegisterClassReflection<ClassType>   \
      CPPTL_MAKE_UNIQUE_NAME(cpptlReflectRegisterClass);


# include <cpptl/reflection.inl>
# include <cpptl/reflectionimpl10.h>

#endif // CPPTL_REFLECTION_H_INCLUDED

