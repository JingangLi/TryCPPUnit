#ifndef CPPTL_SHAREDPTR_H_INCLUDED
# define CPPTL_SHAREDPTR_H_INCLUDED

# include <cpptl/forwards.h>

// We use inheritance instead of the typedef in a struct to
// simulate the template typedef because the typedef in a struct
// trick does not support type deduction in template functions.


// If CPPTL_USE_BOOST_SHARED_PTR is defined, then we use boost::shared_ptr
// as our smart_pointer, otherwise we use a simple (partial) implementation
// of our own.
# ifndef CPPTL_USE_BOOST_SHARED_PTR

#  include <cpptl/atomiccounter.h>

namespace CppTL
{

   /// \cond implementation_detail
   namespace Impl {

      struct StaticPointerCastTag {};

      /// Smart-pointer base class to avoid template code bloat
      class SharedPtrBase
      {
      public:
         long use_count() const
         {
            if ( count_ )
               return count_->count();
            return 0;
         }

         bool unique() const
         {
            return use_count() == 1;
         }

         operator bool() const
         {
            return p_ != 0;
         }

         bool operator !() const
         {
            return p_ == 0;
         }
      protected:
         SharedPtrBase()
            : p_( 0 )
            , count_( 0 )
         {
         }
         explicit SharedPtrBase( void *p )
            : p_( p )
            , count_( p ? new AtomicCounter( 1 ) : 0 )
         {
         }

         SharedPtrBase( void *p, 
                       AtomicCounter *count, 
                       StaticPointerCastTag )
            : p_( p )
            , count_( count )
         {
            count_->increment();
         }

         SharedPtrBase( const SharedPtrBase &other )
            : p_( other.p_ )
            , count_( other.count_ )
         {
            if ( p_ )
               count_->increment();
         }

         ~SharedPtrBase()
         {
            // subclass destructor should call releaseCount();
         }

         bool releaseCount()
         {
            if ( count_  &&  count_->decrement() == 0 )
            {
               delete count_;
               return true;
            }
            return false;
         }

         void swap( SharedPtrBase &other )
         {
            void *tmpP = p_;
            AtomicCounter *tmpCount = count_;
            p_ = other.p_;
            count_ = other.count_;
            other.p_ = tmpP;
            other.count_ = tmpCount;
         }

         void *get() const
         {
            return p_;
         }

         void *deref() const
         {
            // assert( p_ != 0 )
            return p_;
         }

      //private:     // Friend template function is not well supported
                     // Private access required by function staticPointerCast<>
      public:
         void *p_;
         AtomicCounter *count_;
      };

   } // namespace Impl
   /// \endcond
      
   template<class PointeeType>
   class SharedPtr : public Impl::SharedPtrBase
   {
   public:
      typedef SharedPtr<PointeeType> ThisType; 
      typedef Impl::SharedPtrBase SuperClass;

      SharedPtr()
      {
      }

      explicit SharedPtr( PointeeType *p )
         : Impl::SharedPtrBase( p )
      {
      }

      SharedPtr( PointeeType *p, 
                 AtomicCounter *count, 
                Impl::StaticPointerCastTag )
         : Impl::SharedPtrBase( p, count, Impl::StaticPointerCastTag() )
      {
      }

      SharedPtr( const ThisType &other )
         : Impl::SharedPtrBase( other )
      {
      }

      ~SharedPtr()
      {
         if ( releaseCount() )
            checkedDelete( static_cast<PointeeType *>( p_ ) );
      }

      void reset()
      {
         SharedPtr tmp;
         tmp.swap( *this );
      }

      void reset( PointeeType *p )
      {
         SharedPtr tmp( p );
         tmp.swap( *this );
      }

      PointeeType *get() const
      {
         return static_cast<PointeeType *>( SharedPtrBase::get() );
      }

      void swap( SharedPtr &other )
      {
         SuperClass::swap( other );
      }

      ThisType &operator =( const SharedPtr &other )
      {
         ThisType tmp( other );
         swap( tmp );
         return *this;
      }

      PointeeType &operator *() const
      {
         // assert( p_ != 0 )
         return *( static_cast<PointeeType *>( p_ ) );
      }

      PointeeType *operator ->() const
      {
         return static_cast<PointeeType *>( SuperClass::deref() );
      }
   };


   template<class TargetType, class SourceType>
   SharedPtr<TargetType>
   staticPointerCast( const SharedPtr<SourceType> &p )
   {
      TargetType *target = static_cast<TargetType *>( p.p_ );
      return SharedPtr<TargetType>( target, p.count_, 
                                   Impl::StaticPointerCastTag() );
   }

} // namespace CppTL

# else // ifndef CPPTL_USE_BOOST_SHARED_PTR

#  include <boost/shared_ptr.hpp>

namespace CppTL
{

   template<class PointeeType>
   class SharedPtr : public ::boost::shared_ptr<PointeeType>
   {
   public:
      typedef SharedPtr<PointeeType> ThisType; 

      SharedPtr()
      {
      }

      explicit SharedPtr( PointeeType *p )
         : ::boost::shared_ptr( p )
      {
      }

      SharedPtr( const ::boost::shared_ptr<PointeeType> &other )
         : ::boost::shared_ptr( other )
      {
      }

      ThisType &operator =( const ::boost::shared_ptr<PointeeType> &other )
      {
         return ::boost::shared_ptr<PointeeType>::operator =( other );
      }
   };


   template<class T, class U> 
   SharedPtr<T>
   staticPointerCast( const ::boost::shared_ptr<U> & r)
   {
	   return SharedPtr<T>( ::boost::static_pointer_cast<T>( r ) );
   }

} // namespace CppTL

# endif // ifndef CPPTL_USE_BOOST_SHARED_PTR



#endif // CPPTL_SHAREDPTR_H_INCLUDED

