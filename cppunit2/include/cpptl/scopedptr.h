#ifndef CPPTL_SCOPEDPTR_H_INCLUDED
# define CPPTL_SCOPEDPTR_H_INCLUDED

# include <cpptl/config.h>

namespace CppTL {

template<class PointeeType>
class ScopedPtr : public NonCopyable
{
public:
   typedef ScopedPtr<PointeeType> SelfType;

   ScopedPtr( PointeeType *p = 0 )
      : p_( p )
   {
   }

   ~ScopedPtr()
   {
      checkedDelete( p_ );
   }

   void swap( SelfType &other )
   {
      CppTL::trivialSwap( p_, other.p_ );
   }

   void reset( PointeeType *p = 0 )
   {
      SelfType temp(p);
      swap( temp );
   }

   PointeeType *get() const
   {
      return p_;
   }

   PointeeType &operator *() const
   {
      CPPTL_ASSERT_MESSAGE( p_ != 0, "Attempted to use null pointer" );
      return *p_;
   }

   PointeeType *operator ->() const
   {
      CPPTL_ASSERT_MESSAGE( p_ != 0, "Attempted to use null pointer" );
      return p_;
   }

   operator bool() const
   {
      return p_ != 0;
   }

   bool operator !() const
   {
      return p_ == 0;
   }

private:
   PointeeType *p_;
};


template<class PointeeType>
class ScopedArray : public NonCopyable
{
public:
   typedef ScopedArray<PointeeType> SelfType;

   ScopedArray( PointeeType *p = 0 )
      : p_( p )
   {
   }

   ~ScopedArray()
   {
      checkedArrayDelete( p_ );
   }

   void swap( SelfType &other )
   {
      CppTL::trivialSwap( p_, other.p_ );
   }

   void reset( PointeeType *p = 0 )
   {
      SelfType temp(p);
      CppTL::trivialSwap( temp );
   }

   PointeeType *get() const
   {
      return p_;
   }

   PointeeType &operator *() const
   {
      CPPTL_ASSERT_MESSAGE( p_ != 0, "Attempted to use null pointer" );
      return *p_;
   }

   PointeeType *operator ->() const
   {
      CPPTL_ASSERT_MESSAGE( p_ != 0, "Attempted to use null pointer" );
      return p_;
   }

   operator bool() const
   {
      return p_ != 0;
   }

   bool operator !() const
   {
      return p_ == 0;
   }

private:
   PointeeType *p_;
};


} // namespace CppTL



#endif // CPPTL_SCOPEDPTR_H_INCLUDED

