#ifndef CPPUT_TABLEFIXTURE_H_INCLUDED
# define CPPUT_TABLEFIXTURE_H_INCLUDED

# include "testcase.h"
# include "testsuite.h"
# include "translate.h"
# include <cpptl/any.h>
# include <deque>
# include <map>

namespace CppUT {

class Table;
class TableRow;
class TableRowHelper;
typedef CppTL::IntrusivePtr<Table> TablePtr;


class CPPUT_API InvalidColumnName : public std::invalid_argument
{
public:
   InvalidColumnName( const std::string &columnName )
      : std::invalid_argument( translate( "Table has no column named: " ) + columnName )
   {
   }
};


class CPPUT_API DuplicateColumnName : public std::invalid_argument
{
public:
   DuplicateColumnName( const std::string &columnName )
      : std::invalid_argument( translate( "Table already has a column named: " ) + columnName )
   {
   }
};


class CPPUT_API InvalidColumnIndex : public std::invalid_argument
{
public:
   InvalidColumnIndex()
      : std::invalid_argument( translate( "Can not add cell. All columns already set." ) )
   {
   }
};


class CPPUT_API BadTableCellCast : public std::runtime_error
{
public:
   BadTableCellCast( const char *columnName )
      : std::runtime_error( 
         translate( "Column value can not be converted to specified type: " ) + columnName )
   {
   }
};


/*! \brief Table data for use with table test cases.
 */
class CPPUT_API Table : public CppTL::IntrusiveCount
{
public:
   /*! \brief Add a column to the table.
    */
   void addColumn( const std::string &columnName )
   {
      int index = int(columnsByName_.size());
      if ( !(columnsByName_.insert( ColumnsByName::value_type( columnName.c_str(), index ) ).second) )
         throw DuplicateColumnName( columnName );
   }
   
   /*! \brief Add a new row to the table with the specified testName.
    */
   TableRowHelper newRow( const std::string &testName );

   /*! \brief Returns the test name associated to a row.
    */
   const char *rowTestName( int row ) const
   {
      CPPTL_ASSERT_MESSAGE( row < int(rows_.size()), "Invalid row for cell." );
      return rows_[row].testName_.c_str();
   }

   /*! \brief Returns the number of rows in the table.
    */
   int rowCount() const
   {
      return int( rows_.size() );
   }

   /*! \brief Set the value of a cell.
    * \exception InvalidColumnIndex if all cells are set.
    */
   void setCellValue( int column, int row, const CppTL::Any &value )
   {
      CPPTL_ASSERT_MESSAGE( row < int(rows_.size()), "Invalid row for cell." );
      if ( column > int(columnsByName_.size()) )
         throw InvalidColumnIndex();
      rows_[row].cells_[column] = value;
   }

   /*! \brief Set the value of a cell.
    * \exception InvalidColumnName if there is no column matching the specified name.
    */
   void setCellValue( const std::string &columnName, int row, const CppTL::Any &value )
   {
      CPPTL_ASSERT_MESSAGE( row < int(rows_.size()), "Invalid row for cell." );
      ColumnsByName::const_iterator it = columnsByName_.find( columnName );
      if ( it == columnsByName_.end() )
         throw InvalidColumnName( columnName );
      int column = it->second;
      rows_[row].cells_[column] = value;
   }

   /*! \brief Get the value of a cell.
    * \exception InvalidColumnName if there is no column matching the specified name.
    */
   const CppTL::Any &getCellValue( const char *columnName, int row ) const
   {
      CPPTL_ASSERT_MESSAGE( row < int(rows_.size()), "Invalid row for cell." );
      ColumnsByName::const_iterator it = columnsByName_.find( columnName );
      if ( it == columnsByName_.end() )
         throw InvalidColumnName( columnName );
      int column = it->second;
      return rows_[row].cells_[column];
   }

   /// Returns new row index.
   int addRow( const std::string &testName )
   {
      rows_.push_back( Row( testName, int(columnsByName_.size()) ) );
      return int( rows_.size() - 1 );
   }

private:
   typedef std::vector<CppTL::Any> RowData;
   struct Row
   {
      Row( const std::string &testName, int columnCount )
         : testName_( testName )
         , cells_( columnCount )
      {
      }

      RowData cells_;
      CppTL::ConstString testName_;
   };
   typedef std::deque< Row > Rows;
//   std::vector<CppTL::ConstString> columns_;
   typedef std::map<CppTL::ConstString,int> ColumnsByName;
   ColumnsByName columnsByName_;
   Rows rows_;
};



/*! \brief Table helper to add data in a row.
 * \see Table.
 */
class CPPUT_API TableRowHelper
{
public:
   TableRowHelper( Table &table, int row )
      : table_(table)
      , row_( row )
      , column_( 0 )
   {
   }

   TableRowHelper &operator <<( const CppTL::Any &value )
   {
      table_.setCellValue( column_++, row_, value );
      return *this;
   }

   TableRowHelper &set( const std::string &columnName, const CppTL::Any &value )
   {
      table_.setCellValue( columnName, row_, value );
      return *this;
   }

private:
   Table &table_;
   int row_;
   int column_;
};


inline TableRowHelper 
Table::newRow( const std::string &testName )
{
   int row = addRow( testName );
   return TableRowHelper( *this, row );
}


/*! \brief Table row proxy passed to the test case.
 * \see CPPUT_TABLE_FETCH()
 */
class CPPUT_API TableRow
{
public:
   TableRow( const TablePtr &table, int row )
      : table_( table )
      , row_( row )
   {
   }

   const CppTL::Any &getCellValue( const char *columnName ) const
   {
      return table_->getCellValue( columnName, row_ );
   }

private:
   TablePtr table_;
   int row_;
};


// implementation detail.
template<class ValueType>
const ValueType &
getTableCell( const TableRow &row, const char *columnName, CppTL::Type<ValueType> type )
{
   const ValueType *value = ::get( &(row.getCellValue( columnName )), type );
   if ( value == 0 )
      throw BadTableCellCast( columnName );
   return *value;
}


/*! \brief Fetch the value of the cell with the specified column name.
 * \exception InvalidColumnName if there is no column matching the specified name.
 * \exception BadTableCellCast if the cell value can not be converted into the specified type.
 * \todo allow usage of converter for convertion between similar types.
 */
#define CPPUT_TABLE_FETCH( CellType, columnName ) \
   CellType columnName = ::CppUT::getTableCell( row, #columnName, ::CppTL::Type<CellType>() );


/*! \brief Declares a table test case for a TestFixture.
 * Create a nested TestSuite in the fixture and instantiate a test case for each row
 * of the table.
 * \param testMethod Must have the following signature: void testMethod( const TableRow &row );
 *                   Another static function with the name of testMethod suffixed with Data
 *                   must exist with the following signature:
 *                   void testMethodData( Table &table );
 * \code
 * class MyTableTest : public TestTableFixture
 * {
 *     CPPUT_TESTSUITE_BEGIN( MyTableTest );
 *     CPPUT_TABLE_TEST( testSum );
 *     CPPUT_TESTSUITE_END();
 *     static void testSumData( CppUT::Table &table )
 *     {
 *         table.addColumn( "value1" );
 *         table.addColumn( "value2" );
 *         table.addColumn( "sum" );
 *         table.newRow("positive") << 1 << 2 << 3;
 *         table.newRow("negative") << -5 << -6 << -11;
 *     }
 *
 *     void testSum( const CppUT::TableRow &row )
 *     {
 *         CPPUT_TABLE_FETCH( int, v1 );
 *         CPPUT_TABLE_FETCH( int, v2 );
 *         CPPUT_TABLE_FETCH( int, sum );
 *         CPPUT_CHECK_TRUE( v1+v2 == sum );
 *     }
 * };
 * \endcode
 * Implementation notes:
 * - Create a suite named after the test function.
 * - Create Table and initialize TablePtr.
 * - Generate the table calling the static data population function taking the Table as parameter.
 *   Function is static to avoid possible interaction between test cases.
 * - For each row, create a test case and bind it to a TableRow, adding it to the TestSuite.
 * \see Table, TableRowHelper, CPPUT_TABLE_FETCH.
 * \todo handle exception thrown during data setup.
 */
# define CPPUT_TEST_TABLE( testMethod )                                          \
   {                                                                             \
      ::CppUT::TestSuitePtr tableSuite = ::CppUT::makeTestSuite( #testMethod );  \
      ::CppUT::TablePtr table( new ::CppUT::Table() );                           \
      testMethod##Data( *table );                                               \
      int rowCount = table->rowCount();                                          \
      for ( int row = 0; row < rowCount; ++row )                                 \
      {                                                                          \
         fixture = fixtureFactory();                                             \
         tableSuite->add( ::CppUT::makeFixtureTestCase( fixture,                 \
            ::CppTL::bind( ::CppTL::memfn1( fixture,                             \
                                            &CppUT_ThisType::testMethod ),       \
                           ::CppUT::TableRow( table, row ) ),                    \
                          table->rowTestName(row) ) );                           \
      }                                                                          \
      suite->add( tableSuite );                                                  \
   }


class CPPUT_API TestFunctionTableSuiteFactory
{
public:
   typedef TestPtr result_type;
   typedef void (*DataFn)( Table &table );
   typedef void (*TestFn)( const TableRow &row );

   TestFunctionTableSuiteFactory( const char *suiteName, DataFn data, TestFn test )
      : suiteName_( suiteName )
      , data_( data )
      , test_( test )
   {
   }

   /*! \brief create the test suite and populate it with test case from the table.
    * \todo handle exception thrown during data setup.
    */
   TestPtr operator()() const
   {
      TestSuitePtr suite( new TestSuite( suiteName_ ) );
      TablePtr table( new Table() );
      data_( *table );
      int rowCount = table->rowCount();
      for ( int row = 0; row < rowCount; ++row )
      {
         suite->add( makeTestCase( CppTL::bind( CppTL::cfn1( test_ ), 
                                                TableRow( table, row ) ),
                                   table->rowTestName(row) ) );
      }
      return suite.get();
   }

private:
   const char *suiteName_;
   DataFn data_;
   TestFn test_;
};


/*! \brief Create a table fixture TestSuite and register it to the default Registry suite.
 *
 * Generate test case for a table in a suite using a C function to setup the table, and
 * another one to execute the test cases.
 *
 * \param testFunction Must have the following signature: void testFunction( const TableRow &row )
 *                   Another static function with the name of testMethod suffixed with Data
 *                   must exist with the following signature:
 *                   void testMethodData( Table &table );
 *
 *
 */
# define CPPUT_REGISTER_TEST_FUNCTION_TABLE( testFunction )                \
   CPPUT_REGISTER_TESTFACTORY_TO_DEFAULT(                                  \
      CppTL::fn0r( ::CppUT::TestFunctionTableSuiteFactory( #testFunction,  \
                                                  testFunction##Data,      \
                                                  testFunction ) ) )

# define CPPUT_REGISTER_TEST_FUNCTION_TABLE_IN( testFunction, parentSuiteName )     \
   CPPUT_REGISTER_TESTFACTORY_IN(                                                   \
      CppTL::fn0r( ::CppUT::TestFunctionTableSuiteFactory( #testFunction,           \
                                                  testFunction##Data,               \
                                                  testFunction ) ),                 \
                                  parentSuiteName )

# define CPPUT_TEST_FUNCTION_TABLE( testFunction )                         \
   static void testFunction( const ::CppUT::TableRow &row );               \
   CPPUT_REGISTER_TEST_FUNCTION_TABLE( testFunction );                     \
   static void testFunction( const ::CppUT::TableRow &row )

# define CPPUT_TEST_FUNCTION_TABLE_IN( testFunction, parentSuiteName )     \
   static void testFunction( const ::CppUT::TableRow &row );               \
   CPPUT_REGISTER_TEST_FUNCTION_TABLE_IN( testFunction, parentSuiteName ); \
   static void testFunction( const ::CppUT::TableRow &row )


#if 0

// old stuff

   class CellBinder;
   class TableFixture;
   class TableRow;
   typedef CppTL::SharedPtr<CellBinder> CellBinderPtr;

   class Table
   {
   public:
      virtual ~Table()
      {
      }


   };

   class TableRow
   {
   public:
      TableRow( const Table &table,
                int rowIndex );

      virtual ~TableRow()
      {
      }

      enum { invalidRow = -1 };

      // invalidRow if it does not exist
      virtual int getCellIndex( const std::string &name ) = 0;

      virtual int cellCount() = 0;
      virtual bool isCellEmpty( int index ) = 0;
      virtual int cellAsInt( int index ) = 0;
      virtual unsigned int cellAsUnsignedInt( int index ) = 0;
      virtual char cellAsChar( int index ) = 0;
      virtual float cellAsFloat( int index ) = 0;
      virtual double cellAsDouble( int index ) = 0;
      virtual std::string cellAsString( double index ) = 0;
   };

   class CellBinder
   {
   public:
      virtual ~CellBinder()
      {
      }

      virtual void bind( TableFixture &fixture, 
                         const TableRow &row, 
                         int cellIndex ) const = 0;
   };

   template<class MemberType>
   class CommonCellBinder : public CellBinder
   {
   public:
      CommonCellBinder( MemberType TableFixture::*input,
                        MemberType (TableRow::*cellGetter)() )
      {
      }

      // overridden from CellBinder
      void bind( TableFixture &fixture, 
                 const TableRow &row, 
                 int cellIndex ) const
      {
         fixture->*input_ = (row->*cellGetter_)( cellIndex );
      }

   private:
      MemberType TableFixture::*input_;
      MemberType (TableRow::*cellGetter_)();
   };


   class TableFixture : public TestCase
   {
   public:
      typedef void (TableFixture::*ActionFn)();
      typedef char TableFixture::*CharInput;
      typedef int TableFixture::*IntInput;
      typedef unsigned int TableFixture::*UnsignedIntInput;
      typedef float TableFixture::*FloatInput;
      typedef double TableFixture::*DoubleInput;
      typedef std::string TableFixture::*StringInput;

      void bindAction( const std::string &actionName,
                       ActionFn actionMethod );

      void bindInput( const std::string &inputName, CharInput input );
      void bindInput( const std::string &inputName, IntInput input );
      void bindInput( const std::string &inputName, UnsignedIntInput input );
      void bindInput( const std::string &inputName, FloatInput input );
      void bindInput( const std::string &inputName, DoubleInput input );
      void bindInput( const std::string &inputName, StringInput input );
      void bindInput( const std::string &inputName, const CellBinderPtr &binder );

      //void beginProcessTable( const Table &table );
      //void endProcessTable( const Table &table );
      virtual void beginProcessRow( const TableRow &row );
      virtual void endProcessRow( const TableRow &row );
      virtual void processRow( const TableRow &row );

   private:
      typedef std::map< std::string, CellBinderPtr > Binders;
      Binders binders_;

      typedef std::map< std::string, CppTL::Functor0 > Actions;
      Actions actions_;
   };
# endif

} // namespace CppUT


#endif // CPPUT_TABLEFIXTURE_H_INCLUDED

