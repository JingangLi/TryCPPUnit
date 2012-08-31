#ifndef CPPUTTOOLS_REFCOUNTPTR_H_INCLUDED
# define CPPUTTOOLS_REFCOUNTPTR_H_INCLUDED

# include <cpput/config.h>

namespace CppUTTools
{
   class ReferenceCounted
   {
   public:
      virtual ~ReferenceCounted() {}

      virtual void incrementReferenceCount() = 0;

      virtual bool decrementReferenceCount() = 0;

      virtual unsigned int getReferenceCount() const = 0;
   };

   // A ReferenceCount which does not provide thread-safety.
   class ReferenceCountedNoThread : public ReferenceCounted
   {
   public:
      ReferenceCountedNoThread()
         : count_( 0 )
      {
      }

   public:  // overridden from ReferenceCounted
      void incrementReferenceCount()
      {
         ++count_;
      }

      bool decrementReferenceCount()
      {
         return --count_ == 0;
      }

      unsigned int getReferenceCount() const
      {
         return count_;
      }
   private:
      unsigned int count_;
   };


   namespace Impl {

      /// ReferenceCountPtr base class, implementation common to all pointers...
      class ReferenceCountPtrBase
      {
      public:
         long use_count() const
         {
            if ( p_ )
               return p_->getReferenceCount();
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

         void reset()
         {
            ReferenceCountPtrBase tmp;
            tmp.swap( *this );
         }
      protected:
         ReferenceCountPtrBase()
            : p_( 0 )
         {
         }
         
         ReferenceCountPtrBase( ReferenceCounted *p )
            : p_( p )
         {
            if ( p_ )
               p_->incrementReferenceCount();
         }

         ReferenceCountPtrBase( const ReferenceCountPtrBase &other )
            : p_( other.p_ )
         {
            if ( p_ )
               p_->incrementReferenceCount();
         }

         ~ReferenceCountPtrBase()
         {
            if ( p_  &&  p_->decrementReferenceCount() )
               delete p_;
         }

         void swap( ReferenceCountPtrBase &other )
         {
            ReferenceCounted *tmpP = p_;
            p_ = other.p_;
            other.p_ = tmpP;
         }

         ReferenceCounted *get() const
         {
            return p_;
         }

      private:
         ReferenceCounted *p_;
      };

   } // namespace Impl
      
   template<class PointeeType>
   class RefCountPtr : public Impl::ReferenceCountPtrBase
   {
   public:
      typedef RefCountPtr<PointeeType> ThisType; 
      typedef Impl::ReferenceCountPtrBase SuperClass;

      RefCountPtr()
      {
      }

      RefCountPtr( PointeeType *p )
         : Impl::ReferenceCountPtrBase( p )
      {
      }

      RefCountPtr( const ThisType &other )
         : Impl::ReferenceCountPtrBase( other )
      {
      }

      void reset( PointeeType *p )
      {
         RefCountPtr tmp( p );
         tmp.swap( *this );
      }

      PointeeType *get() const
      {
         return static_cast<PointeeType *>( SuperClass::get() );
      }

      void swap( RefCountPtr &other )
      {
         SuperClass::swap( other );
      }

      ThisType &operator =( const RefCountPtr &other )
      {
         ThisType tmp( other );
         swap( tmp );
         return *this;
      }

      PointeeType &operator *() const
      {
         // assert( SuperClass::get() != 0 )
         return *( static_cast<PointeeType *>( SuperClass::get() ) );
      }

      PointeeType *operator ->() const
      {
         return static_cast<PointeeType *>( SuperClass::get() );
      }
   };


   template<class TargetType, class SourceType>
   RefCountPtr<TargetType>
   staticPointerCast( const RefCountPtr<SourceType> &p )
   {
      TargetType *target = static_cast<TargetType *>( p.get() );
      return RefCountPtr<TargetType>( target );
   }

} // namespace CppUT


#endif // CPPUTTOOLS_REFCOUNTPTR_H_INCLUDED
