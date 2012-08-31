#include "enumeratortest.h"
#include "testing.h"
#include <algorithm>
#include <iterator>

// @todo: add test for custom equality predicates
// @todo add test for different enumerated type

CPPUT_SUITE_REGISTER( EnumeratorTest );

namespace { // anonymous namespace
   template<class EnumeratorType>
   void checkHasValueType( const EnumeratorType & )
   {
      typedef CPPTL_TYPENAME EnumeratorType::value_type ValueType;
   }

   template<typename EnumeratorType>
   void checkHasNoMoreElements( const EnumeratorType &e )
   {
      CppUT::checkFalse( e.hasNext(), "Enumerated should not have anymore elements." );
      CppUT::checkFalse( e.is_open(), "Enumerated should not have anymore elements." );
   }

   template<typename EnumeratorType>
   void checkContent( EnumeratorType e )
   {  
      checkHasValueType( e );

      int v1 = 12;
      int v2 = 23;
      int v3 = 34;
      CPPUT_ASSERT_EXPR( e.is_open() );
      CPPUT_ASSERT_EQUAL( v1, e.current() );
      CPPUT_ASSERT_EQUAL( v1, *e );
      CPPUT_ASSERT_EQUAL( v1, e.next() );
      CPPUT_ASSERT_EXPR( e.is_open() );
      CPPUT_ASSERT_EQUAL( v2, e.current() );
      CPPUT_ASSERT_EQUAL( v2, *e );
      CPPUT_ASSERT_EQUAL( v2, *e++ );
      CPPUT_ASSERT_EXPR( e.is_open() );
      CPPUT_ASSERT_EQUAL( v3, e.current() );
      CPPUT_ASSERT_EQUAL( v3, *e );
      CPPUT_ASSERT_EQUAL( v3, e.next() );
      checkHasNoMoreElements( e );
   }

   template<typename EnumeratorType>
   void checkTransformContent( EnumeratorType e, double divider )
   {  
      checkHasValueType( e );

      double v1 = 12.0 / divider;
      double v2 = 23.0 / divider;
      double v3 = 34.0 / divider;
      const double tolerance = 0.000001;
      CPPUT_ASSERT_EXPR( e.is_open() );
      CPPUT_ASSERT_DOUBLE_EQUAL( v1, e.current(), tolerance );
      CPPUT_ASSERT_DOUBLE_EQUAL( v1, *e, tolerance );
      CPPUT_ASSERT_DOUBLE_EQUAL( v1, e.next(), tolerance );
      CPPUT_ASSERT_EXPR( e.is_open() );
      CPPUT_ASSERT_DOUBLE_EQUAL( v2, e.current(), tolerance );
      CPPUT_ASSERT_DOUBLE_EQUAL( v2, *e++, tolerance );
      CPPUT_ASSERT_EXPR( e.is_open() );
      CPPUT_ASSERT_DOUBLE_EQUAL( v3, e.current(), tolerance );
      CPPUT_ASSERT_DOUBLE_EQUAL( v3, *e, tolerance );
      CPPUT_ASSERT_DOUBLE_EQUAL( v3, e.next(), tolerance );
      checkHasNoMoreElements( e );
   }

   template<typename EnumeratorType>
   void checkStringContent( EnumeratorType e )
   {  
      checkHasValueType( e );

      std::string v1 = "abc";
      std::string v2 = "def";
      std::string v3 = "ghi";
      CPPUT_ASSERT_EXPR( e.is_open() );
      CPPUT_ASSERT_EQUAL( v1, e.current() );
      CPPUT_ASSERT_EQUAL( v1, *e );
      CPPUT_ASSERT_EQUAL( v1, e.next() );
      CPPUT_ASSERT_EXPR( e.is_open() );
      CPPUT_ASSERT_EQUAL( v2, e.current() );
      CPPUT_ASSERT_EQUAL( v2, *e );
      CPPUT_ASSERT_EQUAL( v2, *e++ );
      CPPUT_ASSERT_EXPR( e.is_open() );
      CPPUT_ASSERT_EQUAL( v3, e.current() );
      CPPUT_ASSERT_EQUAL( v3, *e );
      CPPUT_ASSERT_EQUAL( v3, e.next() );
      checkHasNoMoreElements( e );
   }

   template<typename EnumeratorType>
   void checkFilteredContent( EnumeratorType e, int skew )
   {  
      checkHasValueType( e );

      int v1 = 0 + skew;
      int v2 = 2 + skew;
      int v3 = 4 + skew;
      int v4 = 6 + skew;
      int v5 = 8 + skew;
      CPPUT_ASSERT_EXPR( e.is_open() );
      CPPUT_ASSERT_EQUAL( v1, e.current() );
      CPPUT_ASSERT_EQUAL( v1, *e );
      CPPUT_ASSERT_EQUAL( v1, e.next() );
      CPPUT_ASSERT_EXPR( e.is_open() );
      CPPUT_ASSERT_EQUAL( v2, e.current() );
      CPPUT_ASSERT_EQUAL( v2, *e );
      CPPUT_ASSERT_EQUAL( v2, *e++ );
      CPPUT_ASSERT_EXPR( e.is_open() );
      CPPUT_ASSERT_EQUAL( v3, e.current() );
      CPPUT_ASSERT_EQUAL( v3, *e );
      CPPUT_ASSERT_EQUAL( v3, e.next() );
      CPPUT_ASSERT_EXPR( e.is_open() );
      CPPUT_ASSERT_EQUAL( v4, e.next() );
      CPPUT_ASSERT_EXPR( e.is_open() );
      CPPUT_ASSERT_EQUAL( v5, e.next() );
      checkHasNoMoreElements( e );
   }

   template<typename EnumeratorType>
   void checkContentRange( EnumeratorType e, int start, int end )
   {  
      checkHasValueType( e );
      for ( int index = start; index < end; ++index )
         CPPUT_ASSERT_EQUAL( index, e.next() );
      checkHasNoMoreElements( e );
   }

   template<typename SequenceContainer>
   void fillStringSequence( SequenceContainer &sequence )
   {
      sequence.push_back( "abc" );
      sequence.push_back( "def" );
      sequence.push_back( "ghi" );
   }

   struct Divide
   {
      typedef double result_type;

      Divide( double divider )
         : divider_( divider )
      {
      }

      double operator()( int v ) const
      {
         return double(v) / divider_;
      }

      double divider_;
   };

   struct Int2StringAdaptor
   {
      typedef std::string result_type;

      std::string operator()( int v ) const
      {
         switch ( v )
         {
         case 12: return "abc";
         case 23: return "def";
         case 34: return "ghi";
         default: return "?";
         }
      }
   };

   struct IsEvenFilter
   {
      IsEvenFilter( int skew = 0 )
         : skew_( skew )
      {
      }

      bool operator()( int v ) const
      {
         return (v+skew_) % 2 == 0;
      }

      int skew_;
   };

   // Thin enumerator to test sugar()
   class SerieGeneratorEnumerator
   {
   public:
      typedef int value_type;

      SerieGeneratorEnumerator( int start = 0, int end = 0, int step = 1 )
         : index_( start )
         , end_( end )
         , step_( step )
      {
      }

      bool is_open() const
      {
         return index_ < end_;
      }

      value_type current() const
      {
         return index_;
      }

      void advance()
      {
         index_ += step_;
      }

   private:
      int index_;
      int end_;
      int step_;
   };

} // anonymous namespace



EnumeratorTest::EnumeratorTest()
{
}


EnumeratorTest::~EnumeratorTest()
{
}


void 
EnumeratorTest::setUp()
{
   deque1_.clear();
   deque1_.push_back( 12 );   // must be ordered
   deque1_.push_back( 23 );
   deque1_.push_back( 34 );

   vector1_.clear();
   std::copy( deque1_.begin(), deque1_.end(), std::back_inserter( vector1_ ) );

   set1_.clear();
   std::copy( deque1_.begin(), deque1_.end(), std::inserter( set1_, set1_.begin() ) );

   list1_.clear();
   std::copy( deque1_.begin(), deque1_.end(), std::back_inserter( list1_ ) );

   map1_.clear();
   map1_.insert( IntStringMap::value_type( 12, "abc" ) );
   map1_.insert( IntStringMap::value_type( 23, "def" ) );
   map1_.insert( IntStringMap::value_type( 34, "ghi" ) );
}


void 
EnumeratorTest::tearDown()
{
   map1_.clear();
   list1_.clear();
   set1_.clear();
   vector1_.clear();
   deque1_.clear();
}


void 
EnumeratorTest::testEmptyEnumerator()
{
   CppTL::EmptyEnumerator<int> e;
   checkHasValueType( e );
   CPPUT_ASSERT_EXPR_FALSE( e.is_open() );
   CPPUT_ASSERT_EXPR_FALSE( e.hasNext() );
}


void 
EnumeratorTest::testEmptyAnyEnumerator()
{  // Tests template instantiation with EmptyEnumerator. any*() generator functions
   // are tested in each enumerator test case
   CppTL::AnyEnumerator<int> empty;
   checkHasNoMoreElements( empty );
}


void 
EnumeratorTest::testForwardItEnumerator()
{
   checkContent( CppTL::Enum::range( deque1_.begin(), deque1_.end() ) );
#if !defined(CPPTL_NO_VECTOR_ITERATOR_VALUE_TYPE_DEDUCTION)
// NOTES: supported only on platform where std::vector<>::iterator is not a pointer
// or with template partial specialization
   checkContent( CppTL::Enum::range( vector1_.begin(), vector1_.end() ) );
#endif

#if !defined(CPPTL_NO_TEMPLATE_PARTIAL_SPECIALIZATION)
// NOTES: to be portable, the compiler need to be able to remove the optional 'const' 
// before the deduced enumerator type. This can only be done using partial 
// specialilzation. For portability, it is strongly recommanded to use the portable
// syntax by passing the return type explicitely.
   checkContent( CppTL::Enum::range( set1_.begin(), set1_.end() ) );
#endif

   checkContent( CppTL::Enum::range( list1_.begin(), list1_.end() ) );

   checkContent( CppTL::Enum::anyRange( list1_.begin(), list1_.end() ) );

   // Specifying Type ensure portability across all platforms.
   checkContent( CppTL::Enum::range( deque1_.begin(), deque1_.end(), CppTL::Type<int>() ) );
   checkContent( CppTL::Enum::range( vector1_.begin(), vector1_.end(), CppTL::Type<int>() ) );
   checkContent( CppTL::Enum::range( set1_.begin(), set1_.end(), CppTL::Type<int>() ) );
   checkContent( CppTL::Enum::range( list1_.begin(), list1_.end(), CppTL::Type<int>() ) );
   checkContent( CppTL::Enum::anyRange( list1_.begin(), list1_.end(), CppTL::Type<int>() ) );
}

void 
EnumeratorTest::testTransformForwardItEnumerator()
{
   Divide divider( 10 );
   checkTransformContent( CppTL::Enum::rangeTransform( deque1_.begin(), 
                                                       deque1_.end(), 
                                                       divider ), 10.0 );

   checkTransformContent( CppTL::Enum::rangeTransform( vector1_.begin(), 
                                                       vector1_.end(), 
                                                       divider ), 10.0 );

   checkTransformContent( CppTL::Enum::rangeTransform( set1_.begin(), 
                                                       set1_.end(), 
                                                       divider ), 10.0 );

   checkTransformContent( CppTL::Enum::rangeTransform( list1_.begin(), 
                                                       list1_.end(), 
                                                       divider ), 10.0 );

   checkTransformContent( CppTL::Enum::anyRangeTransform( list1_.begin(), 
                                                          list1_.end(), 
                                                          divider ), 10.0 );
}


void 
EnumeratorTest::testContainerEnumerator()
{
   checkContent( CppTL::Enum::container( deque1_ ) );
   checkContent( CppTL::Enum::container( vector1_ ) );
   checkContent( CppTL::Enum::container( set1_ ) );
   checkContent( CppTL::Enum::container( list1_ ) );
   checkContent( CppTL::Enum::anyContainer( list1_ ) );

   checkContent( CppTL::Enum::container( deque1_, CppTL::Type<int>() ) );
   checkContent( CppTL::Enum::container( vector1_, CppTL::Type<int>() ) );
   checkContent( CppTL::Enum::container( set1_, CppTL::Type<int>() ) );
   checkContent( CppTL::Enum::container( list1_, CppTL::Type<int>() ) );
   checkContent( CppTL::Enum::anyContainer( list1_, CppTL::Type<int>() ) );
}


void 
EnumeratorTest::testMapKeysEnumerator()
{
   checkContent( CppTL::Enum::keys( map1_ ) );

   checkContent( CppTL::Enum::keys( map1_, CppTL::Type<int>() ) );

   checkContent( CppTL::Enum::keysRange( map1_.begin(), map1_.end() ) );

   checkContent( CppTL::Enum::anyKeys( map1_ ) );

   checkContent( CppTL::Enum::anyKeysRange( map1_.begin(), map1_.end() ) );

   checkContent( CppTL::Enum::keysRange( map1_.begin(), map1_.end(), CppTL::Type<int>() ) );

   checkContent( CppTL::Enum::anyKeys( map1_, CppTL::Type<int>() ) );

   checkContent( CppTL::Enum::anyKeysRange( map1_.begin(), map1_.end(), CppTL::Type<int>() ) );
}


void 
EnumeratorTest::testMapValuesEnumerator()
{
   checkStringContent( CppTL::Enum::values( map1_ ) );

   checkStringContent( CppTL::Enum::valuesRange( map1_.begin(), map1_.end() ) );

   checkStringContent( CppTL::Enum::values( map1_, CppTL::Type<std::string>() ) );

   checkStringContent( CppTL::Enum::valuesRange( map1_.begin(), 
                                                 map1_.end(), 
                                                 CppTL::Type<std::string>() ) );

   checkStringContent( CppTL::Enum::anyValues( map1_ ) );

   checkStringContent( CppTL::Enum::anyValuesRange( map1_.begin(), map1_.end() ) );

   checkStringContent( CppTL::Enum::anyValuesRange( map1_.begin(), map1_.end(), 
                                                    CppTL::Type<std::string>() ) );

   checkStringContent( CppTL::Enum::anyValues( map1_, CppTL::Type<std::string>() ) );
}


void 
EnumeratorTest::testTransformEnumerator()
{
   checkStringContent( CppTL::Enum::transform( CppTL::Enum::container(deque1_),
                                               Int2StringAdaptor() ) );
   checkStringContent( CppTL::Enum::transform( CppTL::Enum::container(list1_),
                                               Int2StringAdaptor() ) );
   checkStringContent( CppTL::Enum::transform( CppTL::Enum::keys(map1_),
                                               Int2StringAdaptor() ) );
   checkStringContent( CppTL::Enum::anyTransform( CppTL::Enum::container(list1_),
                                                  Int2StringAdaptor() ) );
}


void 
EnumeratorTest::testFilterEnumerator()
{  // Tests with both even and odd filter to ensure skipping is correctly
   // done at the beginning and at the end of the enumeration.
   IntDeque even;
   checkHasNoMoreElements( CppTL::Enum::filter( CppTL::Enum::container( even ), 
                                                IsEvenFilter(0) ) );

   for ( int index =0; index <10; ++index )
      even.push_back( index );

   checkFilteredContent( CppTL::Enum::filter( CppTL::Enum::container( even ), 
                                              IsEvenFilter(0) ),
                         0 );

   IntDeque odd( even );
   checkFilteredContent( CppTL::Enum::filter( CppTL::Enum::container( even ), 
                                              IsEvenFilter( 1 ) ), 
                         1 );
   checkFilteredContent( CppTL::Enum::anyFilter( CppTL::Enum::container( even ), 
                                                 IsEvenFilter( 1 ) ), 
                         1 );
}


void 
EnumeratorTest::testSliceEnumerator()
{
   IntDeque serie;
   for ( int index =0; index <10; ++index )
      serie.push_back( index );

   int length = serie.size();
   checkContentRange( CppTL::Enum::slice( CppTL::Enum::container( serie ), 0, 0 ), 
                      0, 0 ); 
   checkContentRange( CppTL::Enum::slice( CppTL::Enum::container( serie ), 0 ), 
                      0, length ); 
   checkContentRange( CppTL::Enum::slice( CppTL::Enum::container( serie ), length ), 
                      length, length ); 
   checkContentRange( CppTL::Enum::slice( CppTL::Enum::container( serie ), 1, length ), 
                      1, length ); 
   checkContentRange( CppTL::Enum::slice( CppTL::Enum::container( serie ), 0, length-1 ), 
                      0, length-1 ); 
   checkContentRange( CppTL::Enum::slice( CppTL::Enum::container( serie ), 3, 7 ), 
                      3, 7 ); 
   checkContentRange( CppTL::Enum::slice( CppTL::Enum::container( serie ), 3, 3 ), 
                      3, 3 ); 
   checkContentRange( CppTL::Enum::anySlice( CppTL::Enum::container( serie ), 3, 7 ), 
                      3, 7 ); 
}


void 
EnumeratorTest::testSugarEnumerator()
{
   checkContentRange( CppTL::Enum::sugar( SerieGeneratorEnumerator() ), 
                      0, 0 );
   checkContentRange( CppTL::Enum::sugar( SerieGeneratorEnumerator(0, 10) ), 
                      0, 10 );
   checkContentRange( CppTL::Enum::sugar( SerieGeneratorEnumerator(3, 7) ), 
                      3, 7 );
   checkFilteredContent( CppTL::Enum::sugar( SerieGeneratorEnumerator(0, 10, 2) ), 0 );
   checkFilteredContent( CppTL::Enum::sugar( SerieGeneratorEnumerator(1, 11, 2) ), 1 );

   checkContentRange( CppTL::Enum::anySugar( SerieGeneratorEnumerator(3, 7) ), 
                      3, 7 );
   checkFilteredContent( CppTL::Enum::anySugar( SerieGeneratorEnumerator(1, 11, 2) ), 1 );
}


/*
void 
EnumeratorTest::testStlEnumerator()
{
   IntEnum e1 = CppUT::enumStl( deque1_ );
   checkContent( e1 );

   IntEnum e2 = CppUT::enumStl( vector1_ );
   checkContent( e2 );

   IntEnum e3 = CppUT::enumStl( set1_ );
   checkContent( e3 );

   CharEnum e4 = CppUT::enumStl( deque1_, CppUT::Type<char>() );
   checkContent( e4 );

   CharEnum e5 = CppUT::enumStl( vector1_, CppUT::Type<char>() );
   checkContent( e5 );

   CharEnum e6 = CppUT::enumStl( set1_, CppUT::Type<char>() );
   checkContent( e6 );
} 


void 
EnumeratorTest::testStlRangeEnumerator()
{
   IntEnum e1a = CppUT::enumStlRange( deque1_.begin(), deque1_.begin()+2 );
   IntEnum e1b = CppUT::enumStlRange( deque1_.begin()+1, deque1_.begin()+3 );
   checkRangesContent( e1a, e1b );

// Range enumerator for vector type is not supported because return type can not be deduced.
//   IntEnum e2a = CppUT::enumStlRange( vector1_.begin(), vector1_.begin()+2 );
//   IntEnum e2b = CppUT::enumStlRange( vector1_.begin()+1, vector1_.begin()+3 );
//   checkRangesContent( e2a, e2b );

   IntSet::iterator itEnd = set1_.end();
   --itEnd;
   IntSet::iterator itStart = set1_.begin();
   itStart++;

   IntEnum e3a = CppUT::enumStlRange( set1_.begin(), itEnd );
   IntEnum e3b = CppUT::enumStlRange( itStart, set1_.end() );
   checkRangesContent( e3a, e3b );

   CharEnum e4a = CppUT::enumStlRange( deque1_.begin(), deque1_.begin()+2, CppUT::Type<char>() );
   CharEnum e4b = CppUT::enumStlRange( deque1_.begin()+1, deque1_.begin()+3, CppUT::Type<char>() );
   checkRangesContent( e4a, e4b );

   CharEnum e5a = CppUT::enumStlRange( vector1_.begin(), vector1_.begin()+2, CppUT::Type<char>() );
   CharEnum e5b = CppUT::enumStlRange( vector1_.begin()+1, vector1_.begin()+3, CppUT::Type<char>() );
   checkRangesContent( e5a, e5b );

   CharEnum e6a = CppUT::enumStlRange( set1_.begin(), itEnd, CppUT::Type<char>() );
   CharEnum e6b = CppUT::enumStlRange( itStart, set1_.end(), CppUT::Type<char>() );
   checkRangesContent( e6a, e6b );
} 

*/
