#ifndef CPPUT_INPUTTEST_H_INCLUDED
# define CPPUT_INPUTTEST_H_INCLUDED

# include "forwards.h"
# include <json/value.h>
# include <cpptl/any.h>
# include <cpptl/reflection.h>


namespace CppUT {

// input based test are factory based.
// Input data is provided to the factory
// test is executed based on the input and assertion are done.
// This is a single test case.
// Fixture is stand-alone and not related to test case in anyway.

class CPPUT_API TableDataSource
{
public:
   virtual ~TableDataSource()
   {
   }

   virtual CppTL::Any getCellValue( int column, int row ) const = 0;
   virtual int rowCount() const = 0;
   virtual int columnCount() const = 0;
};

class CPPUT_API JsonTableDataSource : public TableDataSource
{
public:
   JsonTableDataSource( const Json::Value &data )
      : data_( data )
   {
   }

public: // overridden from TableDataSource
   virtual CppTL::Any getCellValue( int column, int row ) const
   {
      const Json::Value &value = data_[row][column];
      switch ( value.type() )
      {
      case Json::nullValue: 
         return CppTL::Any();
      case Json::intValue:
         return CppTL::Any( value.asInt() );
      case Json::uintValue:
         return CppTL::Any( value.asUInt() );
      case Json::realValue:
         return CppTL::Any( value.asDouble() );
      case Json::stringValue:
         return CppTL::Any( value.asString() );
      case Json::booleanValue:
         return CppTL::Any( value.asBool() );
      default:
         return CppTL::Any(); // unsupported conversion, will blow on compare or get/set.
      }
   }

   virtual int rowCount() const
   {
      return data_.size();
   }

   virtual int columnCount() const
   {
      int column = 0;
      return data_[column].size();
   }
   Json::Value data_;
};



class CPPUT_API InputTest
{
public:
   virtual ~InputTest()
   {
   }

   virtual void operator()( const TableDataSource &table ) = 0;
};



template<class Functor>
struct Bind234
{
   typedef CPPTL_TYPENAME Functor::result_type result_type;
   typedef CPPTL_TYPENAME Functor::arg1_type arg1_type;
   typedef arg1_type first_argument_type;
   typedef CPPTL_TYPENAME Functor::arg2_type arg2_type;
   typedef CPPTL_TYPENAME Functor::arg3_type arg3_type;
   typedef CPPTL_TYPENAME Functor::arg4_type arg4_type;
   typedef CppTL::Functor1<arg1_type> functor_type;
   Bind234( const Functor &functor, arg2_type a2, arg3_type a3, arg4_type a4 )
      : functor_( functor ), a2_( a2 ), a3_( a3 ), a4_( a4 )
   {
   }

   void operator()( const arg1_type &arg1 ) const
   {
      functor_( arg1, a2_, a3_, a4_ );
   }

private:
   Functor functor_;
   arg2_type a2_;
   arg3_type a3_;
   arg4_type a4_;
};


/// @todo Extends functor API to allow not binding all parameters.
template<typename Functor, typename A2, typename A3, typename A4>
CppTL::Functor1<CPPTL_TYPENAME Functor::arg1_type>
bind234( const Functor &functor, A2 a2, A3 a3, A4 a4 )
{
   return CppTL::fn1( Bind234<Functor>( functor, a2, a3, a4 ) );
}

/* \brief Input based test for table with action based on column header.
 * \code
 * class OperationInputTest;
 * CPPTL_DECLARE_TYPE_AND_PTR_INFO( OperationInputTest );     // Not required if RTTI are always enabled
 * CPPTL_REFLECT_REGISTER_CLASS( OperationInputTest )
 * class OperationInputTest : public CppUT::ColumnInputTest
 * {
 * public:
 *    CPPUT_INPUT_FIXTURE_BEGIN( OperationInputTest )
 *       CPPTL_REFLECT_METHOD_WITH_RETURN( result )
 *       CPPTL_REFLECT_RENAMED_ATTRIBUT( lhs_, "leftHandSide" )
 *       CPPTL_REFLECT_RENAMED_ATTRIBUT( rhs_, "rightHandSide" )
 *       CPPTL_REFLECT_RENAMED_ATTRIBUT( operation_, "operation" )
 *    CPPUT_INPUT_FIXTURE_END()
 *    int result()
 *    {
 *       if ( operation_ == "add" )
 *          return lhs_ + rhs_;
 *       else if ( operation_ == "substract" )
 *          return lhs_ - rhs_;
 *       CPPUT_CHECKING_FAIL( "Unsupported operation: " + operation_ );
 *       return 0;
 *    }
 *    std::string operation_;
 *    int lhs_;
 *    int rhs_;
 * };
 * \endcode
 */
class CPPUT_API ColumnInputTest : public InputTest
{
public:
   ColumnInputTest()
   {
      this_ = CppTL::makeAny( this );
   }

   virtual const CppTL::Class *getClass() const = 0;

   virtual CppTL::Any getThis() = 0;

public: // overridden from InputTest
   virtual void operator()( const TableDataSource &table )
   {
      // Parse the header:
      // For each column, generate an action that process the value.
      // Assign order to each action: setup or execution ? assertion ?
      this_ = getThis();
      int rowCount = table.rowCount();
      int columnCount = table.columnCount();
      if ( rowCount < 1  ||  columnCount < 1)
      {
         CPPUT_CHECKING_FAIL( "Table row header is missing." );
         return;
      }
      const CppTL::Class *aClass = getClass();
      if ( !aClass )
      {
         CPPUT_CHECKING_FAIL( "Reflection class not found for input test." );
         return;
      }
      for ( int column = 0; column < columnCount; ++column )
      {
         CppTL::Any value = table.getCellValue( column, 0 );
         const std::string *headerName = ::get( &value, CppTL::Type<std::string>() );
         if ( headerName == 0 )
         {
            CPPUT_CHECKING_FAIL( "Can not convert header column to string." );   // @todo add col index
            return;
         }
         if ( headerName->empty() )
            continue;
         if ( headerName->at( headerName->length() - 1 ) == '?' ) // check action
         {
            CppTL::ConstString name( headerName->c_str(), headerName->c_str() + headerName->length() - 1 );
            CppTL::Functor0R<CppTL::Any> actual;
            const CppTL::Attribut *attribut = aClass->findAttribut( name );
            if ( attribut != 0 )
            {
               actual = CppTL::bindr( CppTL::memfn1r( this, &ColumnInputTest::actionGetAttribut ), 
                                      CppTL::cref( *attribut ) );
            }
            else
            {
               const CppTL::Method *method = aClass->findMethod( name );
               if ( method == 0 )
               {
                  CPPUT_CHECKING_FAIL( "No method or attribut found by reflection matching header name: " + name );
                  return;
               }
               actual = CppTL::bindr( CppTL::memfn1r( this, &ColumnInputTest::actionGetResult ), 
                                      CppTL::cref( *method ) );
            }
            Action action = bind234( CppTL::memfn4( this, &ColumnInputTest::actionCheckEquals ),
                                                    column,
                                                    CppTL::cref( table ),
                                                    actual );
            checkActions_.push_back( action );
         }
         else // set attribut or invoke method action
         {
            const CppTL::Attribut *attribut = aClass->findAttribut( headerName->c_str() );
            if ( attribut != 0 )
            {
               Action action = bind234( CppTL::memfn4( this, &ColumnInputTest::actionSetAttribut ),
                                                       column,
                                                       CppTL::cref( table ),
                                                       CppTL::cref( *attribut ) );
               setAttributActions_.push_back( action );
            }
            else
            {
               const CppTL::Method *method = aClass->findMethod( headerName->c_str() );
               if ( method == 0 )
               {
                  CPPUT_CHECKING_FAIL( "No method or attribut found by reflection matching header name: " + *headerName );
                  return;
               }
               Action action = bind234( CppTL::memfn4( this, &ColumnInputTest::actionInvokeMethod ),
                                                       column,
                                                       CppTL::cref( table ),
                                                       CppTL::cref( *method ) );
               checkActions_.push_back( action );
            }
         }
      }
      for ( int row = 1; row < rowCount; ++row )
      {
         processActions( setAttributActions_, row );
         processActions( invokeActions_, row );
         processActions( checkActions_, row );
      }
   }

private:
   typedef CppTL::Functor1<int> Action;
   typedef std::deque<Action> Actions;

private:
   void processActions( const Actions &actions, int row )
   {
      Actions::const_iterator it = actions.begin();
      Actions::const_iterator itEnd = actions.end();
      for ( ; it != itEnd; ++it )
      {
         const Action &action = *it;
         action( row );
      }
   }

   void actionSetAttribut( int row,
                           int column,
                           const TableDataSource &table,
                           const CppTL::Attribut &attribut )
   {
      attribut.set( this_, table.getCellValue( column, row ) );
   }

   void actionInvokeMethod( int row,
                            int column,
                            const TableDataSource &table,
                            const CppTL::Method &method )
   {
      CppTL::MethodParameters parameters;
      parameters.push_back( table.getCellValue( column, row ) );
      method.invoke( this_, parameters );
   }

   void actionCheckEquals( int row,
                           int column,
                           const TableDataSource &table,
                           CppTL::Functor0R<CppTL::Any> actual )
   {
      CppTL::Any actualValue = actual();
      CppTL::Any expectedValue = table.getCellValue( column, row );
      bool hasSameType = actualValue.hasSameType( expectedValue );
      CPPUT_CHECK( hasSameType, "Can not compare cell value: different type!" ); // @todo dump type
      if ( !hasSameType )
         return;
      typedef CppTL::Type<int> TypeInt;
      typedef CppTL::Type<double> TypeDouble;
      typedef CppTL::Type<std::string> TypeString;
      if ( actualValue.type() == CppTL::typeId( TypeInt() ) )
      {
         CPPUT_CHECK_COMPARE( any_cast( expectedValue, TypeInt() ),
                              ==,
                              any_cast( actualValue, TypeInt() ) );
      } 
      else if ( actualValue.type() == CppTL::typeId( TypeDouble() ) )
      {
         CPPUT_CHECK_COMPARE( any_cast( expectedValue, TypeDouble() ),
                              ==,
                              any_cast( actualValue, TypeDouble() ) );
      }
      else if ( actualValue.type() == CppTL::typeId( TypeString() ) )
      {
         CPPUT_CHECK_COMPARE( any_cast( expectedValue, TypeString() ),
                              ==,
                              any_cast( actualValue, TypeString() ) );
      }
      else
      {
         CPPUT_CHECKING_FAIL( "Unsupported type for equality test in cell value." );
      }
   }

   CppTL::Any actionGetAttribut( const CppTL::Attribut &attribut )
   {
      return attribut.get( this_ );
   }

   CppTL::Any actionGetResult( const CppTL::Method &method )
   {
      CppTL::MethodParameters parameters;
      return method.invoke( this_, parameters );
   }

private:
   CppTL::Any this_;
   Actions setAttributActions_;
   Actions invokeActions_;
   Actions checkActions_;
};


#define CPPUT_INPUT_FIXTURE_BEGIN( ClassType )     \
   public: /* overridden from ColumnInputTest */   \
      virtual CppTL::Any getThis()                 \
      {                                            \
         return CppTL::makeAny( this );            \
      }                                            \
   CPPTL_REFLECT_CLASS_BEGIN( ClassType )          \

#define CPPUT_INPUT_FIXTURE_END() \
   CPPTL_REFLECT_CLASS_END()

} // namespace CppUT



#endif // CPPUT_INPUTTEST_H_INCLUDED
