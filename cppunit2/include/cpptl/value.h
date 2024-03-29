#ifndef CPPTL_VALUE_H_INCLUDED
# define CPPTL_VALUE_H_INCLUDED
# include "forwards.h"
# include "typetraits.h"


/*
* need a new Any supporting feature required for unit testing:
  - toString (toJson?)
  - type to text (for conversion error message)
  - equality comparison test (input fixture)
  - support for basic types
  - support for string types
  - support for unknown aggregate types.
  - conversion between type (int, uint, const char *, std::string...)
  - type comparison

Implemented:
- framework to expose any service for Value
- framework for comparison

To do:
- type identification, comparison and hashing
- type to string
- conversion

To think:
- pointer and cast support
- smart-pointer ? Notes: requires dynamic allocation of target object

About type identification:
- we already force the creation of a static to identify type.
- it can be used to replace type_info and all its bug & portability issues.
- but still need to be able to identify that two types are similar
Problems:
dll / static linking:
type info are not shared. Solution: use a single registry held in a dll.
thread-safe registration:
static variable stored in getValueType is a pointer on type in the registry.
ValueType is copied by value in the registry ensuring a single pointer is always returned.
Registry is synchronized to ensure type safety.


*/

namespace CppTL {

class Value;
class ValueType;
class ValueTypeRegistry;

// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// class ValueType
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////

class ValueType
{
public:
   static ValueType typeInt;
   static ValueType typeUInt;
   static ValueType typeDouble;
   static ValueType typeString;
   static ValueType typeCString;

   void clear()
   {
      memset( this, 0, sizeof(ValueType) );
   }

   // Notes: clone & initialize seems similar
   void (*initialize_)( const void *data, Value &target );
   /// \param source is uninitialized. data_ must be initialized after call.
   /// \param target is guaranted to be of the same type as \c source.
   void (*clone_)( const Value &source, Value &target );
   void (*destroy_)( Value &target );
   ConstCharView (*typeToString_)();
   ConstCharView (*valueToString_)();
   void (*convertTo_)( const Value &source, Value &target );

   /// \param a, b Guaranted to be of the same ValueType.
   /// \param canCompare [In/Out] Must be set to \c false if value can not be compared.
   ///                            Left unchanged otherwise (\c true).
   int (*compare_)( const Value &a, const Value &b, bool &canCompare );

   /// \param a, b Guaranted to be of the same ValueType.
   /// \param canCompare [In/Out] Must be set to \c false if value can not be compared.
   ///                            Left unchanged otherwise (\c true).
   bool (*less_)( const Value &a, const Value &b, bool &canCompare );

   /// \param a, b Guaranted to be of the same ValueType.
   /// \param canCompare [In/Out] Must be set to \c false if value can not be compared.
   ///                            Left unchanged otherwise (\c true).
   bool (*equal_)( const Value &a, const Value &b, bool &canCompare );

   // ? was used for conversion I think.
   void (*rawData_)( Value &value );
};

// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// class Value
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////


typedef LargestInt ValueInt;
typedef LargestUnsignedInt ValueUInt;

typedef char ValueDataBuffer[24];

union ValueData
{
   //ValueInt int_;
   //ValueUInt uint_;
   //double double_;
   ValueDataBuffer string_;
   void *other_;
};

struct NoneValueTypeTag
{
};

class ValueException : public std::runtime_error
{
public:
   enum Cause
   {
      notSameType = 1
   };
   ValueException( Cause cause, const char *message,
                   const ValueType *lhs, const ValueType *rhs )
      : std::runtime_error( "Can not proceed. ValueType are not compatible" )
      , lhs_( lhs )
      , rhs_( rhs )
   {
   }

   const ValueType * const lhs_;
   const ValueType * const rhs_;
};

/// @todo Makes a predicate to make value strictly ordered (by type, then value). Requires comparable.
/// @todo optimize operator == & operator !=
class Value
{
public:
   typedef ValueInt Int;
   typedef ValueUInt UInt;

   Value();

   Value( const ValueType &type, const void *data );

   Value( const Value &other )
   {
      type_ = other.type_;
      type_->clone_( other, *this );
   }

   ~Value()
   {
      if ( type_->destroy_ )
         type_->destroy_( *this );
   }

   Value &operator =( const Value &other )
   {
      if( &other != this )
      {
         if ( type_->destroy_ )
            type_->destroy_( *this );
         type_ = other.type_;
         type_->clone_( other, *this );
      }
      return *this;
   }

   bool isNone() const;
   bool isInt() const;
   bool isUInt() const;
   bool isDouble() const;
   bool isString() const;

   Int asInt() const;
   UInt asUInt() const;
   double asDouble() const;
   ConstCharView asString() const;

   bool convertTo( const ValueType &targetType, Value &other ) const
   {
      return false;
   }

   /*! \brief Compares with other value.
    * Attempts to compare this value with \c other value. Only value of same type can be
    * compared unless \c allowAutoConvert is \c true.
    * Comparison attempts to use the following ValueType functions:
    * - compare_
    * - less_
    * - equal_ (only used as a possible optimization when using less_).
    *
    * \param other Value this value is compared to. If it is of a different type,
    *              and \c allowAutoConvert is \c true then other will be converted
    *              converted to this type first.
    * \param canCompare [Output] If \c true indicates that the values were
    *                   successfully compared, otherwise comparison was not possible
    *                   and the comparison result is not valid.
    * \param allowAutoConvert Indicates if an automatic conversion should be attempted
    *                         if values are of different types.
    * \return Only valid if \c canCompare is \c true. Basicaly equivalent to *this - other:
    * - < 0  : *this < other
    * - == 0 : *this == other
    * - > 0  : *this > other
    */
   int compare( const Value &other, bool &canCompare, bool allowAutoConvert = true ) const
   {
      if ( type_ != other.type_ )
      {
         Value otherSameType;
         if ( other.convertTo( *type_, otherSameType ) )
            return compare( otherSameType, canCompare );
      }
      else if ( type_->compare_ )
      {
         canCompare = true;
         return type_->compare_( *this, other, canCompare );
      }
      else if ( type_->less_ )
      {
         canCompare = true;
         bool lhsLessThanRhs = type_->less_( *this, other, canCompare );
         if ( lhsLessThanRhs )
            return -1;
         if ( canCompare  &&  type_->equal_ )
            return type_->equal_( *this, other, canCompare ) ? 0 : 1;
         if ( canCompare )
            return type_->less_( other, *this, canCompare ) ? 1 : 0;
         return 0;
      }
      canCompare = false;
      return 0;
   }

   /// \exception If value are of distinct types.
   bool operator <( const Value &other ) const;
   /// \exception If value are of distinct types.
   bool operator <=( const Value &other ) const;
   /// \exception If value are of distinct types.
   bool operator ==( const Value &other ) const;
   /// \exception If value are of distinct types.
   bool operator !=( const Value &other ) const;
   /// \exception If value are of distinct types.
   bool operator >=( const Value &other ) const;
   /// \exception If value are of distinct types.
   bool operator >( const Value &other ) const;

public:
   /// Internal data, should only be used by ValueType function implementation.
   ValueData data_;

private:
   const ValueType *type_;
};


// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// namespace ValueTypeHelper
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////

namespace ValueTypeHelper {


/// Allocate DataType dynamically.
template<typename DataType>
class LargeTypeStorageHelper
{
public:
   static DataType &data( Value &value )
   {
      return *static_cast<DataType *>( value.data_.other_ );
   }

   static const DataType &data( const Value &value )
   {
      return *static_cast<const DataType *>( value.data_.other_ );
   }

   static void initialize( const void *data, Value &target )
   {
      target.data_.other_ = new DataType( *static_cast<const DataType *>( data ) );
   }

   static void clone( const Value &source, Value &target )
   {
      target.data_.other_ = new DataType( data(source) );
   }
   
   static void destroy( Value &target )
   {
      delete target.data_.other_;
   }
};


/// Store DataType in string buffer if it fit in.
template<typename DataType>
class SmallTypeStorageHelper
{
public:
   static DataType &data( Value &value )
   {
      return *(DataType *)( value.data_.string_ );
   }

   static const DataType &data( const Value &value )
   {
      return *(const DataType *)( value.data_.string_ );
   }

   static void initialize( const void *sourceData, Value &target )
   {
      DataType *targetData = &data(target);
      new (targetData) DataType( *static_cast<const DataType *>( sourceData ) ); // placement new
   }

   static void clone( const Value &source, Value &target )
   {
      DataType *targetData = &data(target);
      new (targetData) DataType( data(source) ); // placement new
   }
   
   static void destroy( Value &target )
   {
      data(target).~DataType();
   }
};


/// Select storage strategy depending on size of DataType.
template<typename DataType>
class StoragePolicySelector
{
public:
   typedef CPPTL_TYPENAME IfType<LargeTypeStorageHelper<DataType>
                                ,SmallTypeStorageHelper<DataType>
                                ,(sizeof(DataType) > sizeof(ValueDataBuffer)) 
                                >::type type;
};


template<typename DataType>
class CommonLifeCycle
{
public:
   typedef CommonLifeCycle<DataType> SelfType;
   typedef CPPTL_TYPENAME StoragePolicySelector<DataType>::type StoragePolicy;

   static void registerFunctions( ValueType &type )
   {
      type.initialize_ = &StoragePolicy::initialize;
      type.clone_ = &StoragePolicy::clone;
      type.destroy_ = &StoragePolicy::destroy;
   }
};



/// Comparison helper.
/// \param DataType must support operators < and ==.
template<typename DataType>
class Compare
{
public:
   typedef Compare<DataType> SelfType;
   typedef CPPTL_TYPENAME StoragePolicySelector<DataType>::type StoragePolicy;

   static void registerFunctions( ValueType &type )
   {
      type.compare_ = &SelfType::compare;
      type.less_ = &SelfType::less;
      type.equal_ = &SelfType::equal;
   }

   static int compare( const Value &aData, const Value &bData, bool &canCompare )
   {
      const DataType &a = StoragePolicy::data(aData);
      const DataType &b = StoragePolicy::data(bData);
      if ( a < b )
         return -1;
      if ( a == b )
         return 0;
      return 1;
   }

   static bool less( const Value &aData, const Value &bData, bool &canCompare )
   {
      const DataType &a = StoragePolicy::data(aData);
      const DataType &b = StoragePolicy::data(bData);
      return a < b;
   }

   static bool equal( const Value &aData, const Value &bData, bool &canCompare )
   {
      const DataType &a = StoragePolicy::data(aData);
      const DataType &b = StoragePolicy::data(bData);
      return a == b;
   }
};


/// Comparison helper.
/// \param DataType must support operators <.
template<typename DataType>
class LessCompare
{
public:
   typedef Compare<DataType> SelfType;
   typedef CPPTL_TYPENAME StoragePolicySelector<DataType>::type StoragePolicy;

   static void registerFunctions( ValueType &type )
   {
      type.compare_ = &SelfType::compare;
      type.less_ = &SelfType::less;
      type.equal_ = &SelfType::equal;
   }

   static int compare( const Value &aData, const Value &bData, bool &canCompare )
   {
      const DataType &a = StoragePolicy::data(aData);
      const DataType &b = StoragePolicy::data(bData);
      if ( a < b )
         return -1;
      if ( b < a )
         return 1;
      return 0;
   }

   static bool less( const Value &aData, const Value &bData, bool &canCompare )
   {
      const DataType &a = StoragePolicy::data(aData);
      const DataType &b = StoragePolicy::data(bData);
      return a < b;
   }

   static bool equal( const Value &aData, const Value &bData, bool &canCompare )
   {
      const DataType &a = StoragePolicy::data(aData);
      const DataType &b = StoragePolicy::data(bData);
      return (a < b)  &&  !(b < a);
   }
};


template<class DataType>
class CommonValueType : public ValueType
{
public:
   CommonValueType()
   {
      clear();
      CommonLifeCycle<DataType>::registerFunctions( *this );
   }
};


template<class DataType>
class ComparableValueType : public ValueType
{
public:
   ComparableValueType()
   {
      clear();
      CommonLifeCycle<DataType>::registerFunctions( *this );
      Compare<DataType>::registerFunctions( *this );
   }
};


} // end of namespace ValueTypeHelper



// default implementation of getValueType
# ifdef CPPTL_NO_FUNCTION_TEMPLATE_ORDERING
   struct HasDefaultValueTypeHelper
   {
   };

   template<class DataType>
   inline const ValueType &getValueTypeSelector( const HasDefaultValueTypeHelper &, 
                                                 const Type<DataType> & )
   {
# else
   template<class DataType>
   inline const ValueType &getValueType( const Type<DataType> & )
   {
# endif
      // Notes: if static within a template function is an issue, then
      // it can be moved to a dynamic allocation in a "type" registry. Requires TypeInfo.
      static ValueTypeHelper::CommonValueType<DataType> type;
      return type;
   }


struct NoneValueType : public ValueType
{
   NoneValueType()
   {
      clear();
      clone_ = &NoneValueType::clone;
      compare_ = &NoneValueType::compare;
   }

   static void clone( const Value &, Value & )
   {
   }

   static int compare( const Value &aData, const Value &bData, bool &canCompare )
   {
      return 0;
   }
};

   
inline const ValueType &getValueType( Type<NoneValueTypeTag> )
{
   static NoneValueType type; // @todo thread safety issue => move to .cpp with static init
   return type;
}

   
inline const ValueType &getValueType( Type<int> )
{
   static ValueTypeHelper::ComparableValueType<int> type; // @todo thread safety issue => move to .cpp with static init
   return type;
}




// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// Value, free functions
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////

/*
 * Notes: user can provides a specific implementation by overloding getValueType() for a given
 * type.
 * inline const ValueType &getValueType( const Type<T> & )
 */

#ifdef CPPTL_NO_FUNCTION_TEMPLATE_ORDERING
   inline HasDefaultValueTypeHelper getValueType( ... )
   {
      return HasDefaultValueTypeHelper();
   }

   template<class T>
   inline const ValueType &getValueTypeHelper( const ValueType &valueType, 
                                               const Type<T> &type )
   {
      return valueType;
   }

   template<class T>
   inline const ValueType &getValueTypeSelector( const Type<T> &type )
   {
      return getValueTypeHelper( getValueType(type), type );
   }

#  define CPPTL_GET_VALUE_TYPE( AType ) ::CppTL::getValueTypeSelector( ::CppTL::Type<AType>() )

# else

#  define CPPTL_GET_VALUE_TYPE( AType ) ::CppTL::getValueType( ::CppTL::Type<AType>() )

# endif


template<typename TargetValueType>
void initialize( Value &value, Type<TargetValueType> )
{
}

template<typename TargetValueType>
Value initialize( Type<TargetValueType> )
{
}


template<typename TargetValueType>
Value makeValue( const TargetValueType &value )
{
   const ValueType &type = CPPTL_GET_VALUE_TYPE( TargetValueType );
   return Value( type, &value );
}

template<typename TargetValueType>
void makeValue( Value &value, const TargetValueType &initialValue )
{
}


template<typename TargetValueType>
const TargetValueType &any_cast( const Value &value, Type<TargetValueType> )
{
}

template<typename TargetValueType>
TargetValueType &any_cast( Value &value, Type<TargetValueType> )
{
}

template<typename TargetValueType>
const TargetValueType *any_cast( const Value *value, Type<TargetValueType> )
{
}

template<typename TargetValueType>
TargetValueType *any_cast( Value *value, Type<TargetValueType> )
{
}





// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// class Value, inlines
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////


inline 
Value::Value()
{
   type_ = &CPPTL_GET_VALUE_TYPE( NoneValueTypeTag );
}


inline
Value::Value( const ValueType &type, const void *data )
   : type_( &type )
{
   type.initialize_( data, *this );
}


inline bool 
Value::isNone() const
{
   return type_ == &CPPTL_GET_VALUE_TYPE( NoneValueTypeTag );
}


inline bool 
Value::operator <( const Value &other ) const
{
   bool canCompare;
   bool result = compare( other, canCompare ) < 0;
   if ( !canCompare )
      throw ValueException( ValueException::notSameType, "Value::operator <", type_, other.type_ );
   return result;
}


inline bool 
Value::operator <=( const Value &other ) const
{
   bool canCompare;
   bool result = compare( other, canCompare ) <= 0;
   if ( !canCompare )
      throw ValueException( ValueException::notSameType, "Value::operator <=", type_, other.type_ );
   return result;
}


inline bool 
Value::operator ==( const Value &other ) const
{
   bool canCompare;
   bool result = compare( other, canCompare ) == 0;
   if ( !canCompare )
      throw ValueException( ValueException::notSameType, "Value::operator ==", type_, other.type_ );
   return result;
}


inline bool 
Value::operator !=( const Value &other ) const
{
   bool canCompare;
   bool result = compare( other, canCompare ) != 0;
   if ( !canCompare )
      throw ValueException( ValueException::notSameType, "Value::operator !=", type_, other.type_ );
   return result;
}


inline bool 
Value::operator >=( const Value &other ) const
{
   bool canCompare;
   bool result = compare( other, canCompare ) >= 0;
   if ( !canCompare )
      throw ValueException( ValueException::notSameType, "Value::operator >=", type_, other.type_ );
   return result;
}


inline bool 
Value::operator >( const Value &other ) const
{
   bool canCompare;
   bool result = compare( other, canCompare ) > 0;
   if ( !canCompare )
      throw ValueException( ValueException::notSameType, "Value::operator >", type_, other.type_ );
   return result;
}





} // end namespace CppTL

#endif // CPPTL_VALUE_H_INCLUDED
