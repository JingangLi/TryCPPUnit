#ifndef CPPUTTOOLS_ENUMERATOR_H_INCLUDED
# define CPPUTTOOLS_ENUMERATOR_H_INCLUDED

# include <cpptl/typetraits.h>
# include <stdexcept>


#define CPPTL_ENUMERATOR_CHECK_CURRENT \
      CPPTL_ASSERT_MESSAGE( is_open(), \
                            "Attempt to access current element on closed enumerator." )

#define CPPTL_ENUMERATOR_CHECK_ADVANCE \
      CPPTL_ASSERT_MESSAGE( is_open(), \
                            "Attempt to advance enumerator past its end." )

namespace CppTL {

   template<class IteratorType>
   struct STLIteratorTraits
   {
      typedef CPPTL_TYPENAME IteratorType::value_type value_type;
   };

# ifndef CPPTL_NO_TEMPLATE_PARTIAL_SPECIALIZATION
   template<class ValueType>
   struct STLIteratorTraits<ValueType *> // broken stl vector iterator
   {
      typedef ValueType value_type;
   };
# endif

} // namespace CppTL


namespace CppTL { namespace Enum {

/// \cond implementation_detail
namespace Impl
{
   template<class Fn1, class Fn2, class ValueType>
   struct ComposeFn
   {
      typedef CPPTL_TYPENAME Fn1::result_type result_type;

      ComposeFn( Fn1 fn1= Fn1(), Fn2 fn2= Fn2() )
         : fn1_( fn1 )
         , fn2_( fn2 )
      {
      }

      CPPTL_TYPENAME Fn1::result_type operator()( const ValueType &value ) const
      {
         return fn1_( fn2_( value ) );
      }

   private:
      Fn1 fn1_;
      Fn2 fn2_;
   };

} // namespace Impl
/// \endcond

} // namespace Enum

// Class StopEnumerationError
// /////////////////////////////////////////////////////////////////////////

class StopEnumerationError : public std::out_of_range
{
public:
   StopEnumerationError()
      : std::out_of_range( "Enumerator::next() called with no next item." )
   {
   }

   ~StopEnumerationError() throw()
   {
   }
};


class EnumeratorBase
{
public:
   static void stopEnumeration()
   {
      throw StopEnumerationError();
   }
};

template<class EnumeratorType>
void assertEnumeratorHasNext( const EnumeratorType &enumerator )
{
# ifndef NDEBUG
   if ( !enumerator.hasNext() )
      enumerator.stopEnumeration();
# endif
}

template<class EnumValueType>
class TypeEnumeratorBase : public EnumeratorBase
{
public:
   typedef EnumValueType value_type;
};


/* Enumerator concept:
 bool is_open() const;
 T current() const;
 class_type advance();

 // derived functions:
 bool hasNext() const;
 T next();
 T operator *() const;
 class_type &operator ++();
 class_type operator++(int);
 */


/** Implements all sugar functions for enumerator.
 *
 * The following functions must be implemented in DerivedType:
 * \code
   bool is_open() const;
   EnumValueType current() const;
   DerivedType advance();
 * \endcode
 */

#if 0
/*
// removed strangely recurring template and using macro instead. Stressed compiler too much
template<class DerivedType
        ,class EnumValueType>
class EnumerationSugar : public TypeEnumeratorBase<EnumValueType>
{
public:
   typedef EnumValueType value_type;

   bool hasNext() const
   {
      return derived().is_open();
   }

   value_type next()
   {
      value_type value = derived().current();
      derived().advance();
      return value;
   }

   value_type operator *() const
   {
      return derived().current();
   }

   DerivedType &operator ++()
   {
      return derived().advance();
   }

   DerivedType operator ++( int )
   {
      DerivedType old = derived();
      derived().advance();
      return old;
   }

private:
   DerivedType &derived()
   {
      return *static_cast<DerivedType *>(this);
   }

   const DerivedType &derived() const
   {
      return *static_cast<const DerivedType *>(this);
   }
};
*/
#endif

// Enumerator Sugar.
// Enumerattor class should defines:
// typedef ... value_type;
// typedef ... self_type; // class type
// bool is_open() const;
// value_type current() const;
// self_type &advance();
# define CPPTL_IMPL_ENUMERATOR                  \
   bool hasNext() const                         \
   {                                            \
      return is_open();                         \
   }                                            \
                                                \
   value_type next()                            \
   {                                            \
      value_type value = current();             \
      advance();                                \
      return value;                             \
   }                                            \
                                                \
   value_type operator *() const                \
   {                                            \
      return current();                         \
   }                                            \
                                                \
   self_type &operator ++()                     \
   {                                            \
      return advance();                         \
   }                                            \
                                                \
   self_type operator ++( int )                 \
   {                                            \
      self_type old = *this;                    \
      advance();                                \
      return old;                               \
   }



template<class ValueType>
class EmptyEnumerator
{
public:
   typedef ValueType value_type;
   typedef EmptyEnumerator<ValueType> self_type;

   CPPTL_IMPL_ENUMERATOR

   bool is_open() const
   {
      return false;
   }

   value_type current() const
   {
      CPPTL_ENUMERATOR_CHECK_CURRENT;
      throw StopEnumerationError();
   }

   EmptyEnumerator &advance()
   {
      CPPTL_ENUMERATOR_CHECK_ADVANCE;
      return *this;
   }
};



template< class ForwardItType
        , class ForwardItValueType = 
        CPPTL_TYPENAME RemoveConst<CPPTL_TYPENAME STLIteratorTraits<ForwardItType>::value_type>::type
        >
class ForwardItEnumeratorBase
{
public:
   ForwardItEnumeratorBase()
      : current_()
      , end_()
   {
   }

   ForwardItEnumeratorBase( const ForwardItType &begin,
                            const ForwardItType &end )
      : current_( begin )
      , end_( end )
   {
   }

protected:
   ForwardItType current_;
   ForwardItType end_;
};


template< class ForwardItType
        , class ForwardItValueType = 
        CPPTL_TYPENAME RemoveConst<CPPTL_TYPENAME STLIteratorTraits<ForwardItType>::value_type>::type
        >
class ForwardItEnumerator 
   : public ForwardItEnumeratorBase< ForwardItType
                                   , ForwardItValueType >
{
public:
   typedef ForwardItValueType value_type;
   typedef ForwardItEnumerator<ForwardItType,ForwardItValueType> self_type;
   typedef ForwardItEnumeratorBase< ForwardItType
                                   , ForwardItValueType > Super;
   CPPTL_IMPL_ENUMERATOR

   ForwardItEnumerator()
   {
   }

   ForwardItEnumerator( const ForwardItType &begin,
                        const ForwardItType &end )
      : Super( begin, end )
   {
   }

   bool is_open() const
   {
      return Super::current_ != Super::end_;
   }

   value_type current() const
   {
      CPPTL_ENUMERATOR_CHECK_CURRENT;
      return *Super::current_;
   }

   ForwardItEnumerator &advance()
   {
      CPPTL_ENUMERATOR_CHECK_ADVANCE;
      ++(Super::current_);
      return *this;
   }
};


template< class ForwardItType
        , class TransformFn >
class TransformForwardItEnumerator
   : public ForwardItEnumeratorBase< ForwardItType
                                   , CPPTL_TYPENAME TransformFn::result_type >
{
public:
   typedef CPPTL_TYPENAME TransformFn::result_type value_type;
   typedef TransformForwardItEnumerator<ForwardItType,TransformFn> self_type;
   typedef ForwardItEnumeratorBase< ForwardItType
                                   , CPPTL_TYPENAME TransformFn::result_type > Super;

   CPPTL_IMPL_ENUMERATOR

   TransformForwardItEnumerator()
   {
   }

   TransformForwardItEnumerator( const ForwardItType &begin,
                                 const ForwardItType &end )
      : Super( begin, end )
   {
   }

   TransformForwardItEnumerator( const ForwardItType &begin,
                                 const ForwardItType &end,
                                 TransformFn transform )
      : Super( begin, end )
      , transform_( transform )
   {
   }

   bool is_open() const
   {
      return Super::current_ != Super::end_;
   }

   value_type current() const
   {
      CPPTL_ENUMERATOR_CHECK_CURRENT;
      return transform_( *Super::current_ );
   }

   TransformForwardItEnumerator &advance()
   {
      CPPTL_ENUMERATOR_CHECK_ADVANCE;
      ++(Super::current_);
      return *this;
   }

protected:
   TransformFn transform_;
};



template< class STLContainerType
        , class ValueType = CPPTL_TYPENAME RemoveConst<CPPTL_TYPENAME STLContainerType::value_type>::type  >
class ContainerEnumerator : 
   public ForwardItEnumerator< CPPTL_TYPENAME STLContainerType::const_iterator
                             , ValueType>
{
public:
   typedef ValueType value_type;
   typedef ContainerEnumerator<STLContainerType,ValueType> self_type;
   typedef ForwardItEnumerator< CPPTL_TYPENAME STLContainerType::const_iterator
                             , ValueType> Super;
   typedef CPPTL_TYPENAME STLContainerType::const_iterator ContainerConstIterator;

   ContainerEnumerator()
   {
   }

   ContainerEnumerator( const STLContainerType &sequence )
      : Super( sequence.begin(), sequence.end() )
   {
   }

   ContainerEnumerator( ContainerConstIterator begin, ContainerConstIterator end )
      : Super( begin, end )
   {
   }

   ContainerEnumerator( const Super &other )
      : Super( other )
   {
   }

   ContainerEnumerator &advance()
   {
      Super::advance();
      return *this;
   }

   ContainerEnumerator &operator ++()
   {
      Super::operator ++();
      return *this;
   }

   ContainerEnumerator<STLContainerType,ValueType> operator ++( int )
   {
      return ContainerEnumerator<STLContainerType,ValueType>( Super::operator ++( 1 ) );
   }
};


template<class Adaptor
        ,class AdaptedEnumeratorType >
class TransformEnumerator
{
public:
   typedef CPPTL_TYPENAME Adaptor::result_type value_type;
   typedef TransformEnumerator<Adaptor,AdaptedEnumeratorType> self_type;

   CPPTL_IMPL_ENUMERATOR

   TransformEnumerator()
   {
   }

   TransformEnumerator( const AdaptedEnumeratorType &other )
      : enumerator_( other )
   {
   }

   TransformEnumerator( const AdaptedEnumeratorType &other,
                        Adaptor adaptor )
      : enumerator_( other )
      , adaptor_( adaptor )
   {
   }

   bool is_open() const
   {
      return enumerator_.is_open();
   }

   value_type current() const
   {
      return adaptor_( enumerator_.current() );
   }

   TransformEnumerator &advance()
   {
      enumerator_.advance();
      return *this;
   }

protected:
   AdaptedEnumeratorType enumerator_;
   Adaptor adaptor_;
};


template< class MapIteratorType
        , class ValueType = 
            CPPTL_TYPENAME RemoveConst<CPPTL_TYPENAME MapIteratorType::value_type::first_type>::type >
class MapKeysEnumerator : public ForwardItEnumeratorBase< MapIteratorType, ValueType >
{
public:
   typedef ValueType value_type;
   typedef MapKeysEnumerator<MapIteratorType,ValueType> self_type;
   typedef ForwardItEnumeratorBase< MapIteratorType, ValueType > Super;

   CPPTL_IMPL_ENUMERATOR

   MapKeysEnumerator()
   {
   }

   MapKeysEnumerator( MapIteratorType begin, MapIteratorType end )
      : Super( begin, end )
   {
   }

   MapKeysEnumerator( const Super &other )
      : Super( other )
   {
   }

   MapKeysEnumerator &operator =( const Super &other )
   {
      Super::operator =( other );
      return *this;
   }

   bool is_open() const
   {
      return Super::current_ != Super::end_;
   }

   value_type current() const
   {
      CPPTL_ENUMERATOR_CHECK_CURRENT;
      return Super::current_->first;
   }

   MapKeysEnumerator &advance()
   {
      CPPTL_ENUMERATOR_CHECK_ADVANCE;
      ++(Super::current_);
      return *this;
   }
};


template< class MapIteratorType
        , class ValueType = 
            CPPTL_TYPENAME RemoveConst<CPPTL_TYPENAME MapIteratorType::value_type::second_type>::type >
class MapValuesEnumerator : public ForwardItEnumeratorBase< MapIteratorType, ValueType >
{
public:
   typedef ValueType value_type;
   typedef MapValuesEnumerator<MapIteratorType,ValueType> self_type;
   typedef ForwardItEnumeratorBase< MapIteratorType, ValueType > Super;

   CPPTL_IMPL_ENUMERATOR

   MapValuesEnumerator()
   {
   }

   MapValuesEnumerator( MapIteratorType begin, MapIteratorType end )
      : Super( begin, end )
   {
   }

   MapValuesEnumerator( const Super &other )
      : Super( other )
   {
   }

   MapValuesEnumerator &operator =( const Super &other )
   {
      Super::operator =( other );
      return *this;
   }

   bool is_open() const
   {
      return Super::current_ != Super::end_;
   }

   value_type current() const
   {
      CPPTL_ENUMERATOR_CHECK_CURRENT;
      return Super::current_->second;
   }

   MapValuesEnumerator &advance()
   {
      CPPTL_ENUMERATOR_CHECK_ADVANCE;
      ++(Super::current_);
      return *this;
   }
};


template<class FilteredEnumerator
        ,class FilterPredicate>
class FilterEnumerator 
{
public:
   typedef CPPTL_TYPENAME FilteredEnumerator::value_type value_type;
   typedef FilterEnumerator<FilteredEnumerator,FilterPredicate> self_type;

   CPPTL_IMPL_ENUMERATOR

   FilterEnumerator()
   {
   }

   FilterEnumerator( const FilteredEnumerator &enumerator,
                     FilterPredicate filter = FilterPredicate() )
      : enumerator_( enumerator )
      , filter_( filter )
   {
      while ( enumerator_.is_open()  &&  !filter_( enumerator_.current() ) )
         enumerator_.advance();
   }

   bool is_open() const
   {
      return enumerator_.is_open();
   }

   value_type current() const
   {
      CPPTL_ENUMERATOR_CHECK_CURRENT;
      return enumerator_.current();
   }

   FilterEnumerator &advance()
   {
      CPPTL_ENUMERATOR_CHECK_ADVANCE;
      enumerator_.advance();
      while ( enumerator_.is_open()  &&  !filter_( enumerator_.current() ) )
         enumerator_.advance();
      return *this;
   }

private:
   FilteredEnumerator enumerator_;
   FilterPredicate filter_;
};


template<class EnumeratorType>
class SliceEnumerator
{
public:
   typedef CPPTL_TYPENAME EnumeratorType::value_type value_type;
   typedef SliceEnumerator<EnumeratorType> self_type;

   CPPTL_IMPL_ENUMERATOR

   typedef unsigned int size_t;

   SliceEnumerator()
      : length_(0)
   {
   }

   SliceEnumerator( const EnumeratorType &enumerator, 
                    size_t begin,
                    size_t end )
      : length_( end >= begin ? end-begin : 0 )
      , enumerator_( enumerator )
   {
      if ( length_ > 0 )
         while ( begin-- > 0  &&  enumerator_.is_open() )
            enumerator_.advance();
   }

   bool is_open() const
   {
      return length_ > 0  &&  enumerator_.is_open();
   }

   value_type current() const
   {
      CPPTL_ENUMERATOR_CHECK_CURRENT;
      return enumerator_.current();
   }

   SliceEnumerator &advance()
   {
      CPPTL_ENUMERATOR_CHECK_ADVANCE;
      enumerator_.advance();
      --length_;
      return *this;
   }

private:
   EnumeratorType enumerator_;
   size_t length_;
};


template<class ValueType>
class AnyEnumeratorBase
{
public:
   typedef ValueType value_type;

   virtual ~AnyEnumeratorBase()
   {
   }

   virtual bool is_open() const = 0;
 
   virtual ValueType current() const = 0;
 
   virtual void advance() = 0;

   virtual AnyEnumeratorBase *clone() const = 0;
};


template< class EnumeratorType
        , class ValueType = CPPTL_TYPENAME EnumeratorType::value_type >
class AnyEnumeratorImpl : public AnyEnumeratorBase<ValueType>
{
public:
   typedef AnyEnumeratorImpl<EnumeratorType,ValueType> ClassType;
   typedef AnyEnumeratorBase<ValueType> Super;

   AnyEnumeratorImpl()
   {
   }

   AnyEnumeratorImpl( const EnumeratorType &enumerator )
      : enumerator_( enumerator )
   {
   }

public:  // overridden from ProxyEnumeratorBase
   bool is_open() const
   {
      return enumerator_.is_open();
   }
 
   ValueType current() const
   {
      return enumerator_.current();
   }
 
   void advance()
   {
      enumerator_.advance();
   }

   Super *clone() const
   {
      return new ClassType( enumerator_ );
   }

protected:
   EnumeratorType enumerator_;
};



template<class ValueType>
class AnyEnumerator
{
public:
   typedef ValueType value_type;
   typedef AnyEnumerator<ValueType> self_type;
   typedef AnyEnumeratorBase<ValueType> *Ptr;

   CPPTL_IMPL_ENUMERATOR

   AnyEnumerator()
      : enumerator_( new AnyEnumeratorImpl< EmptyEnumerator<ValueType> >() )
   {
   }
   
   explicit AnyEnumerator( Ptr enumerator )
      : enumerator_( enumerator )
   {
   }

   AnyEnumerator( const AnyEnumerator &other )
      : enumerator_( other.enumerator_->clone() )
   {
   }

   AnyEnumerator &operator =( const AnyEnumerator &other )
   {
      AnyEnumerator temp( *this );
      swap( temp );
      return *this;
   }

   ~AnyEnumerator()
   {
      delete enumerator_;
   }

   void swap( AnyEnumerator &other )
   {
      Ptr temp = enumerator_;
      enumerator_ = other.enumerator_;
      other.enumerator_ = temp;
   }

   bool is_open() const
   {
      return enumerator_->is_open();
   }
 
   ValueType current() const
   {
      return enumerator_->current();
   }
 
   void advance()
   {
      enumerator_->advance();
   }

protected:
   Ptr enumerator_;
};


template<class ThinEnumeratorType
        ,class ValueType = CPPTL_TYPENAME ThinEnumeratorType::value_type >
class SugarEnumerator
{
public:
   typedef ValueType value_type;
   typedef SugarEnumerator<ThinEnumeratorType> self_type;

   CPPTL_IMPL_ENUMERATOR

   SugarEnumerator()
   {
   }

   SugarEnumerator( const ThinEnumeratorType &enumerator )
      : enumerator_( enumerator )
   {
   }

   bool is_open() const
   {
      return enumerator_.is_open();
   }

   value_type current() const
   {
      CPPTL_ENUMERATOR_CHECK_CURRENT;
      return enumerator_.current();
   }

   self_type &advance()
   {
      CPPTL_ENUMERATOR_CHECK_ADVANCE;
      enumerator_.advance();
      return *this;
   }

private:
   ThinEnumeratorType enumerator_;
};


template<class ValueType>
class AddressOfAdaptor
{
public:
   typedef ValueType *value_type;

   value_type operator()( const ValueType &value ) const
   {
      return &value;
   }
};


namespace Enum {

// Notes: on broken STL implementation which use value_type * as vector iterator,
// you need to pass explictly ValueType => range<int>( vectorInt.begin(), vectorInt.end() );
template<class ForwardItType >
ForwardItEnumerator<ForwardItType>
range( ForwardItType begin, ForwardItType end )
{
   typedef ForwardItEnumerator<ForwardItType> EnumType;
   return EnumType( begin, end );
}
template<class ForwardItType
        ,class ValueType>
ForwardItEnumerator<ForwardItType,CPPTL_TYPENAME ValueType::type>
range( ForwardItType begin, ForwardItType end, ValueType )
{
   typedef ForwardItEnumerator<ForwardItType,CPPTL_TYPENAME ValueType::type> EnumType;
   return EnumType( begin, end );
}


template<class ForwardItType
        ,class Adaptor >
TransformForwardItEnumerator<ForwardItType,Adaptor>
rangeTransform( ForwardItType begin, ForwardItType end, Adaptor adaptor )
{
   typedef TransformForwardItEnumerator<ForwardItType,Adaptor> EnumType;
   return EnumType( begin, end, adaptor );
}


template<class ContainerType>
ContainerEnumerator<ContainerType>
container( const ContainerType &enumeratedContainer )
{
   typedef ContainerEnumerator<ContainerType> EnumType;
   return EnumType( enumeratedContainer );
}


template<class ContainerType
        ,class ValueType>
ContainerEnumerator<ContainerType,CPPTL_TYPENAME ValueType::type>
container( const ContainerType &enumeratedContainer, ValueType )
{
   typedef ContainerEnumerator<ContainerType,CPPTL_TYPENAME ValueType::type> EnumType;
   return EnumType( enumeratedContainer );
}

template<class ForwardItType>
MapKeysEnumerator<ForwardItType>
keysRange( ForwardItType begin, ForwardItType end )
{
   typedef MapKeysEnumerator<ForwardItType> EnumType;
   return EnumType( begin, end );
}

template<class ForwardItType
        ,class ValueType>
MapKeysEnumerator<ForwardItType,CPPTL_TYPENAME ValueType::type>
keysRange( ForwardItType begin, ForwardItType end, ValueType )
{
   typedef MapKeysEnumerator<ForwardItType,CPPTL_TYPENAME ValueType::type> EnumType;
   return EnumType( begin, end );
}

template<class ContainerType>
MapKeysEnumerator<
     CPPTL_TYPENAME ContainerType::const_iterator
   , CPPTL_TYPENAME RemoveConst<CPPTL_TYPENAME ContainerType::value_type::first_type>::type 
   > // Notes: deducing second template param cause ICE on VC++ 7.1
keys( const ContainerType &container )
{
   return keysRange( container.begin(), container.end() );
}

template<class ContainerType
        ,class ValueType>
MapKeysEnumerator< CPPTL_TYPENAME ContainerType::const_iterator
                 , CPPTL_TYPENAME ValueType::type >
keys( const ContainerType &container, ValueType )
{
   return keysRange( container.begin(), container.end(), ValueType() );
}

template<class ContainerType>
MapValuesEnumerator<
     CPPTL_TYPENAME ContainerType::const_iterator
   , CPPTL_TYPENAME RemoveConst<CPPTL_TYPENAME ContainerType::value_type::second_type>::type 
   > // Notes: deducing second template param cause ICE on VC++ 7.1
values( const ContainerType &container )
{
   typedef MapValuesEnumerator<CPPTL_TYPENAME ContainerType::const_iterator> EnumType;
   return EnumType( container.begin(), container.end() );
}

template<class ContainerType
        ,class ValueType>
MapValuesEnumerator< CPPTL_TYPENAME ContainerType::const_iterator
                   , CPPTL_TYPENAME ValueType::type >
values( const ContainerType &container, ValueType )
{
   typedef MapValuesEnumerator< CPPTL_TYPENAME ContainerType::const_iterator
                              , CPPTL_TYPENAME ValueType::type > EnumType;
   return EnumType( container.begin(), container.end() );
}

template<class ForwardItType>
MapValuesEnumerator<ForwardItType>
valuesRange( ForwardItType begin, ForwardItType end )
{
   typedef MapValuesEnumerator<ForwardItType> EnumType;
   return EnumType( begin, end );
}

template<class ForwardItType
        ,class ValueType>
MapValuesEnumerator< ForwardItType, CPPTL_TYPENAME ValueType::type >
valuesRange( ForwardItType begin, ForwardItType end, ValueType )
{
   typedef MapValuesEnumerator< ForwardItType, CPPTL_TYPENAME ValueType::type > EnumType;
   return EnumType( begin, end );
}


template<class EnumeratorType>
AnyEnumerator<CPPTL_TYPENAME EnumeratorType::value_type>
any( const EnumeratorType &enumerator )
{
   typedef AnyEnumerator<CPPTL_TYPENAME EnumeratorType::value_type> EnumType;
   return EnumType( new AnyEnumeratorImpl<EnumeratorType>( enumerator ) );
}


template<class EnumeratorType
        ,class Adaptor>
TransformEnumerator<Adaptor, EnumeratorType>
transform( const EnumeratorType &enumerator, Adaptor adaptor )
{
   typedef TransformEnumerator<Adaptor, EnumeratorType> EnumType;
   return EnumType( enumerator, adaptor );
}


template<class EnumeratorType>
TransformEnumerator< AddressOfAdaptor<CPPTL_TYPENAME EnumeratorType::value_type>, 
                     EnumeratorType >
addressOfTransform( const EnumeratorType &enumerator )
{
   typedef AddressOfAdaptor<CPPTL_TYPENAME EnumeratorType::value_type> Adaptor;
   typedef TransformEnumerator< Adaptor, EnumeratorType > EnumType;
   Adaptor adaptor;
   return EnumType( enumerator, adaptor );
}


template<class EnumeratorType
        ,class Predicate>
FilterEnumerator<EnumeratorType,Predicate>
filter( const EnumeratorType &enumerator, Predicate predicate )
{
   typedef FilterEnumerator<EnumeratorType,Predicate> EnumType;
   return EnumType( enumerator, predicate );
}


template<class EnumeratorType>
SliceEnumerator<EnumeratorType>
slice( const EnumeratorType &enumerator, 
       unsigned int beginIndex, 
       unsigned int endIndex = -1 )
{
   typedef SliceEnumerator<EnumeratorType> EnumType;
   return EnumType( enumerator, beginIndex, endIndex );
}


/// Implements full Enumerator interface for an enumerator providing
/// only is_open(), current() and advance().
template<class ThinEnumeratorType>
SugarEnumerator<ThinEnumeratorType>
sugar( const ThinEnumeratorType &e )
{
   typedef SugarEnumerator<ThinEnumeratorType> EnumType;
   return EnumType( e );
}



// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// AnyEnumerator helpers...
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////


// Notes: on broken STL implementation which use value_type * as vector iterator,
// you need to pass explictly ValueType => range<int>( vectorInt.begin(), vectorInt.end() );
template<class ForwardItType>
AnyEnumerator<CPPTL_TYPENAME RemoveConst<CPPTL_TYPENAME ForwardItType::value_type>::type>
anyRange( ForwardItType begin, ForwardItType end )
{
   return any( range( begin, end ) );
}


template<class ForwardItType
        ,class ValueType>
AnyEnumerator<CPPTL_TYPENAME ValueType::type>
anyRange( ForwardItType begin, ForwardItType end, ValueType type )
{
   return any( range( begin, end, type ) );
}


template<class ForwardItType
        ,class Adaptor >
AnyEnumerator<CPPTL_TYPENAME Adaptor::result_type>
anyRangeTransform( ForwardItType begin, ForwardItType end, Adaptor adaptor )
{
   return any( rangeTransform( begin, end, adaptor ) );
}

template<class ContainerType>
AnyEnumerator<CPPTL_TYPENAME RemoveConst<CPPTL_TYPENAME ContainerType::value_type>::type>
anyContainer( const ContainerType &enumeratedContainer )
{
   return any( container( enumeratedContainer ) );
}

template<class ContainerType
        ,class ValueType>
AnyEnumerator<CPPTL_TYPENAME ValueType::type>
anyContainer( const ContainerType &enumeratedContainer, ValueType type )
{
   return any( container( enumeratedContainer, type ) );
}

template<class ForwardItType>
AnyEnumerator<CPPTL_TYPENAME RemoveConst<
                  CPPTL_TYPENAME ForwardItType::value_type::first_type>::type>
anyKeysRange( ForwardItType begin, ForwardItType end )
{
   return any( keysRange( begin, end ) );
}

template<class ForwardItType
        ,class ValueType>
AnyEnumerator<CPPTL_TYPENAME ValueType::type>
anyKeysRange( ForwardItType begin, ForwardItType end, ValueType type )
{
   return any( keysRange( begin, end, type ) );
}

template<class ContainerType
        ,class ValueType>
AnyEnumerator<CPPTL_TYPENAME ValueType::type>
anyKeys( const ContainerType &container, ValueType type )
{
   return any( keys( container, type ) );
}

template<class ContainerType>
AnyEnumerator<CPPTL_TYPENAME RemoveConst<
                  CPPTL_TYPENAME ContainerType::value_type::first_type>::type>
anyKeys( const ContainerType &container )
{
   return any( keys( container ) );
}

template<class ForwardItType>
AnyEnumerator<CPPTL_TYPENAME RemoveConst<
                  CPPTL_TYPENAME ForwardItType::value_type::second_type>::type>
anyValuesRange( ForwardItType begin, ForwardItType end )
{
   return any( valuesRange( begin, end ) );
}

template<class ForwardItType
        ,class ValueType>
AnyEnumerator<CPPTL_TYPENAME ValueType::type>
anyValuesRange( ForwardItType begin, ForwardItType end, ValueType type )
{
   return any( valuesRange( begin, end, type ) );
}

template<class ContainerType>
AnyEnumerator<CPPTL_TYPENAME RemoveConst<
                  CPPTL_TYPENAME ContainerType::value_type::second_type>::type>
anyValues( const ContainerType &container )
{
   return any( values( container ) );
}

template<class ContainerType
        ,class ValueType>
AnyEnumerator<CPPTL_TYPENAME ValueType::type>
anyValues( const ContainerType &container, ValueType type )
{
   return any( values( container, type ) );
}


template<class EnumeratorType
        ,class Adaptor>
AnyEnumerator<CPPTL_TYPENAME Adaptor::result_type>
anyTransform( const EnumeratorType &enumerator, Adaptor adaptor )
{
   return any( transform( enumerator, adaptor ) );
}


template<class EnumeratorType>
AnyEnumerator<CPPTL_TYPENAME EnumeratorType::value_type *>
anyAddressOfTransform( const EnumeratorType &enumerator )
{
   return any( addressOfTransform( enumerator ) );
}


template<class EnumeratorType
        ,class Predicate>
AnyEnumerator<CPPTL_TYPENAME EnumeratorType::value_type>
anyFilter( const EnumeratorType &enumerator, Predicate predicate )
{
   return any( filter( enumerator, predicate ) );
}


template<class EnumeratorType>
AnyEnumerator<CPPTL_TYPENAME EnumeratorType::value_type>
anySlice( const EnumeratorType &enumerator, 
          unsigned int beginIndex, 
          unsigned int endIndex = -1 )
{
   return any( slice( enumerator, beginIndex, endIndex ) );
}


template<class ThinEnumeratorType>
AnyEnumerator<CPPTL_TYPENAME ThinEnumeratorType::value_type>
anySugar( const ThinEnumeratorType &e )
{
   return any( sugar( e ) );
}



} // namespace Enum
} // namespace CppTL


#endif // CPPUTTOOLS_ENUMERATOR_H_INCLUDED
