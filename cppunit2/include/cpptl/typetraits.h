#ifndef CPPTL_TYPETRAITS_H_INCLUDED
# define CPPTL_TYPETRAITS_H_INCLUDED

# include <cpptl/forwards.h>

// @todo move this to a header dedicated to basic generic programming technics


namespace CppTL
{

   /** Remove const prefix of a type.
    * \Warning Older compiler such as VC 6.0 do not allow implementation of
    *          this feature. Const will not be removed on such platform.
    */
   template<class T>
   struct RemoveConst
   {
      typedef T type;
   };

   #if !defined( CPPTL_NO_TEMPLATE_PARTIAL_SPECIALIZATION )

   template<class T>
   struct RemoveConst<const T>
   {
      typedef T type;
   };

   #endif






   //@todo BooleanType should be moved to a simple generic programming header.
   template<const bool isTrue>
   struct BooleanType
   {
   };

   typedef BooleanType<false> FalseType;
   typedef BooleanType<true> TrueType;




#if !defined(CPPTL_NO_TEMPLATE_PARTIAL_SPECIALIZATION)
   /** Returns type A if true, type B otherwise.
    * \c true case, returns A
    * \code
    * CppTL::IfType< LargeTypePolicy,SmallTypePolicy,sizeof(T) > 128 >
    * \endcode
    */
   template<typename A, typename B, const bool condition>
   struct IfType
   {
      typedef A type;
   };

   /// Returns type A if true, type B otherwise.
   /// \c false case, returns B
   template<typename A, typename B>
   struct IfType<A,B,false>
   {
      typedef B type;
   };
#else // work-around lack of template partial specialization using template member
   /// Returns type A if true, type B otherwise
   template<typename A, typename B, const bool condition>
   struct IfType
   {
      template<const bool condition>
      struct Selector
      {
         typedef A type;
      };
      template<>
      struct Selector<false>
      {
         typedef B type;
      };

      typedef CPPTL_TYPENAME Selector<condition>::type type;
   };
#endif

} // namespace CppTL

#endif // CPPTL_TYPETRAITS_H_INCLUDED
