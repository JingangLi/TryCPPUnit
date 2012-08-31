#ifndef CPPTL_INTRUSIVEPTR_H_INCLUDED
# define CPPTL_INTRUSIVEPTR_H_INCLUDED

# include <cpptl/forwards.h>
# include <cpptl/atomiccounter.h>

// We use inheritance instead of the typedef in a struct to
// simulate the template typedef because the typedef in a struct
// trick does not support type deduction in template functions.


// If CPPTL_USE_BOOST_INTRUSIVE_PTR is defined, then we use boost::intrusive_ptr
// as our smart_pointer, otherwise we use a simple implementation
// of our own.

namespace CppTL
{
   /// Instrusive counter base class for use with InstrusivePtr.
   class IntrusiveCount
   {
   public:
      virtual ~IntrusiveCount()
      {
      }

      void incrementReferenceCount()
      {
         count_.increment();
      }

      void releaseReferenceCount()
      {
         if ( !count_.decrement() )
            delete this;
      }

   private:
      mutable AtomicCounter count_;
   };

}

# ifndef CPPTL_USE_BOOST_SHARED_PTR
namespace CppTL
{
   inline void instrusivePtrAddRef( IntrusiveCount *p )
   {
      p->incrementReferenceCount();
   }

   inline void intrusivePtrRelease( IntrusiveCount *p )
   {
      p->releaseReferenceCount();
   }

   template<class PointeeType>
   class IntrusivePtr
   {
   public:
      typedef IntrusivePtr<PointeeType> ThisType; 

      IntrusivePtr()
         : p_( 0 )
      {
      }

      IntrusivePtr( PointeeType *p )
         : p_( p )
      {
         if ( p )
            instrusivePtrAddRef( p );
      }

      IntrusivePtr( const ThisType &other )
         : p_( other.p_ )
      {
         if ( p_ )
            instrusivePtrAddRef( p_ );
      }

      ~IntrusivePtr()
      {
         if ( p_ )
            intrusivePtrRelease( p_ );
      }

      void reset()
      {
         IntrusivePtr tmp;
         tmp.swap( *this );
      }

      void reset( PointeeType *p )
      {
         IntrusivePtr tmp( p );
         tmp.swap( *this );
      }

      PointeeType *get() const
      {
         return p_;
      }

      void swap( ThisType &other )
      {
         CppTL::trivialSwap( p_, other.p_ );
      }

      ThisType &operator =( const ThisType &other )
      {
         ThisType tmp( other );
         swap( tmp );
         return *this;
      }

      PointeeType &operator *() const
      {
         CPPTL_ASSERT_MESSAGE( p_ != 0, "Attempting to dereference a null pointer with operator *." );
         return *p_;
      }

      PointeeType *operator ->() const
      {
         CPPTL_ASSERT_MESSAGE( p_ != 0, "Attempting to use a null pointer with operator ->." );
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

      bool operator ==( const ThisType &other ) const
      {
         return p_ == other.p_;
      }

      bool operator !=( const ThisType &other ) const
      {
         return !(*this == other );
      }

      bool operator <( const ThisType &other ) const
      {
         return p_ < other.p_;
      }

      bool operator <=( const ThisType &other ) const
      {
         return p_ <= other.p_;
      }

      bool operator >=( const ThisType &other ) const
      {
         return p_ >= other.p_;
      }

      bool operator >( const ThisType &other ) const
      {
         return p_ > other.p_;
      }

   private:
      PointeeType *p_;
   };


   template<class T, class U> 
   IntrusivePtr<T>
   staticPointerCast( const IntrusivePtr<U> & r)
   {
	   return IntrusivePtr<T>( static_cast<T*>( r.get() ) );
   }

} // namespace CppTL

# else // ifndef CPPTL_USE_BOOST_SHARED_PTR

#  include <boost/shared_ptr.hpp>

namespace CppTL
{

   template<class PointeeType>
   class IntrusivePtr : public ::boost::intrusive_ptr<PointeeType>
   {
   public:
      typedef IntrusivePtr<PointeeType> ThisType; 

      IntrusivePtr()
      {
      }

      IntrusivePtr( PointeeType *p )
         : ::boost::intrusive_ptr( p )
      {
      }

      IntrusivePtr( const ::boost::intrusive_ptr<PointeeType> &other )
         : ::boost::intrusive_ptr( other )
      {
      }

      ThisType &operator =( const ::boost::intrusive_ptr<PointeeType> &other )
      {
         return ::boost::intrusive_ptr<PointeeType>::operator =( other );
      }
   };


   template<class T, class U> 
   IntrusivePtr<T>
   staticPointerCast( const ::boost::intrusive_ptr<U> & r)
   {
	   return IntrusivePtr<T>( ::boost::static_pointer_cast<T>( r ) );
   }

} // namespace CppTL

namespace boost {
   inline void intrusive_ptr_add_ref( CppTL::IntrusiveCount *p )
   {
      p->incrementReferenceCount();
   }

   inline void intrusive_ptr_release( CppTL::IntrusiveCount *p )
   {
      p->releaseReferenceCount();
   }
} // namespace boost

# endif // ifndef CPPTL_USE_BOOST_SHARED_PTR


namespace CppTL {

template<class DataType>
class SharedDataPtr
{
public:
   typedef SharedDataPtr<DataType> ThisType;
   typedef DataType Data;
   
   SharedDataPtr()
   {
   }
   
   SharedDataPtr( const Data &data )
      : shared_( new CountedData( data ) )
   {
   }

   SharedDataPtr( const ThisType &other )
      : shared_( other.shared_ )
   {
   }

   ~SharedDataPtr()
   {
   }

   Data *get() const
   {
      CountedData *data = shared_.get();
      return data ? &(data->data_) : 0;
   }

   void swap( ThisType &other )
   {
      shared_.swap( other.shared_ );
   }

   ThisType &operator =( const ThisType &other )
   {
      ThisType tmp( other );
      swap( tmp );
      return *this;
   }

   DataType &operator *() const
   {
      // assert( p_ != 0 )
      return shared_->data_;
   }

   DataType *operator ->() const
   {
      return &(shared_->data_);
   }

   operator bool() const
   {
      return shared_;
   }

   bool operator !() const
   {
      return !shared_;
   }

private:
   class CountedData : public IntrusiveCount
   {
   public:
      CountedData( const Data &data )
         : data_( data )
      {
      }

      Data data_;
   };

   IntrusivePtr<CountedData> shared_;
};


} // namespace CppTL


#endif // CPPTL_INTRUSIVEPTR_H_INCLUDED

