#include <cpput/assertcommon.h>
#include <cpptl/functor.h>
#include <cpptl/sharedptr.h>
#include <stdio.h>


namespace {

   bool callbackCalled;
   bool badValue;
   std::string expected;

   static void callback0()
   {
      callbackCalled = true;
   }

   static void callback1( bool value )
   {
      callbackCalled = value;
   }

   static void callback12( const std::string &str, int x )
   {
      callbackCalled = true;
   }

   static void callback1string( const std::string &str )
   {
      callbackCalled = true;
      badValue = expected == str;
   }

   static double returnCallback1( int x )
   {
      return x / 2.0;
   }

   struct TestFunctor
   {
      TestFunctor() // avoid bogus suncc warning
      {
      }

      void operator()() const
      {
         callbackCalled = true;
      }

      void operator()( bool value ) const
      {
         callbackCalled = value;
      }
   };

   struct HelperObject
   {
      bool flag_;
      int value_;

      HelperObject()
         : flag_( false )
         , value_( 0 )
      {
      }

      HelperObject( const HelperObject &other )
         : flag_( other.flag_ )
         , value_( other.value_ + 1 )
      {
      }

      HelperObject &operator =( const HelperObject &other )
      {
         flag_ = other.flag_;
         value_ = other.value_ + 1;
         return *this;
      }

      void setFlagOn()
      {
         flag_ = true;
      }

      void setFlag( bool value )
      {
         flag_ = value;
      }

      double compute1( int x )
      {
         return (x + value_) / 2.0;
      }
   };


}

static void testFunctor()
{
   CppTL::Functor0 fn = CppTL::cfn0( &callback0 );
   callbackCalled = false;
   fn();
   CPPUT_ASSERT( callbackCalled, "Functor() did not call C function." );

   callbackCalled = false;
   TestFunctor testFunctor;
   fn = CppTL::fn0( testFunctor );
   fn();
   CPPUT_ASSERT( callbackCalled, "Functor() did not call functor object." );

   CppTL::SharedPtr<HelperObject> helper( new HelperObject() );
   helper->flag_ = false;
   fn = CppTL::memfn0( helper, &HelperObject::setFlagOn );
   fn();
   CPPUT_ASSERT( helper->flag_, "Functor() did not call object method (shared_ptr)." );

   helper->flag_ = false;
   fn = CppTL::memfn0( helper.get(), &HelperObject::setFlagOn );
   fn();
   CPPUT_ASSERT( helper->flag_, "Functor() did not call object method (ptr)." );
}


static void testFunctor1()
{
   CppTL::Functor1<bool> fn = CppTL::cfn1( &callback1 );
   callbackCalled = false;
   fn( true );
   CPPUT_ASSERT( callbackCalled, "Functor1(true) did not call C function." );
   fn( false );
   CPPUT_ASSERT_FALSE( callbackCalled, "Functor1(false) did not call C function." );

   CppTL::SharedPtr<HelperObject> helper( new HelperObject() );
   helper->flag_ = false;
   fn = CppTL::memfn1( helper, &HelperObject::setFlag );
   fn( true );
   CPPUT_ASSERT( helper->flag_, "Functor1(true) did not call object method." );
   fn( false );
   CPPUT_ASSERT_FALSE( helper->flag_, "Functor1(false) did not call object method." );
}


static void testFunctor1R()
{
   CppTL::Functor1R<double,int> fn = CppTL::cfn1r( &returnCallback1 );
   callbackCalled = false;
   CPPUT_ASSERT_EQUAL( 1.0, fn( 2 ) );

   CppTL::SharedPtr<HelperObject> helper( new HelperObject() );
   helper->value_ = 0;
   fn = CppTL::memfn1r( helper, &HelperObject::compute1 );
   CPPUT_ASSERT_EQUAL( 1.0, fn( 2 ) );
   helper->value_ = 10;
   CPPUT_ASSERT_EQUAL( 6.0, fn( 2 ) );
}

namespace {

   int extractHelperObjectValue( const HelperObject &object )
   {
      return object.value_;
   }

   void doubleHelperObjectValue( HelperObject &object )
   {
      object.value_ *= 2;
   }

} // anonymous namespace


static void testConstReferenceParameterSupport()
{
   CppTL::Functor1R<int,const HelperObject &> fncref1( CppTL::cfn1r( &extractHelperObjectValue ) );
   HelperObject object;
   object.value_ = 12345678;
   CPPUT_ASSERT_EQUAL( 12345678, fncref1( object ) );
}


static void testReferenceParameterSupport()
{
   CppTL::Functor1<HelperObject &> fnref1( CppTL::cfn1( &doubleHelperObjectValue ) );
   HelperObject object;
   object.value_ = 1111;
   fnref1( object );
   CPPUT_ASSERT_EQUAL( 1111*2, object.value_ );
}


namespace {
   int multiply( int x, int y )
   {
      return x * y;
   }
}

static void testBindValue()
{
   CppTL::Functor2R<int,int,int> fn2 = CppTL::cfn2r( &multiply );
   CppTL::Functor1R<int,int> fn1 = CppTL::fn1r( CppTL::bind2r( fn2, 3 ) );
   CPPUT_ASSERT_EQUAL( 2*3, fn1(2) );
   CPPUT_ASSERT_EQUAL( 5*3, fn1(5) );
}

static void testBindConstRef()
{
   CppTL::Functor1R<int,const HelperObject &> fn1( CppTL::cfn1r( &extractHelperObjectValue ) );
   HelperObject object;
   object.value_ = 12345678;
   CppTL::Functor0R<int> fn0 = CppTL::fn0r( CppTL::bind1r( fn1, CppTL::cref(object) ) );
   CPPUT_ASSERT_EQUAL( 12345678, fn0() );
}


static void testBindRef()
{
   CppTL::Functor1<HelperObject &> fnref1( CppTL::cfn1( &doubleHelperObjectValue ) );
   HelperObject object;
   object.value_ = 1111;
   CppTL::Functor0 fn0 = CppTL::fn0( CppTL::bind1( fnref1, CppTL::ref( object ) ) );
   fn0();
   CPPUT_ASSERT_EQUAL( 1111*2, object.value_ );
}


bool testFunctors()
{
   printf( "Running bootstrap test: testFunctors()...\n" );
   try
   {
      CppUT::TestInfo::threadInstance().startNewTest();
      testFunctor();
      testFunctor1();
      testFunctor1R();

      testConstReferenceParameterSupport();
	   testReferenceParameterSupport();
      testBindValue();
      testBindConstRef();
      testBindRef();
   }
   catch ( const CppUT::AbortingAssertionException &e )
   {
      printf( "testFunctors() failed: %s\n", e.what() );
      return false;
   }
   catch ( ... )
   {
      printf( "testFunctors() failed (uncaught exception)." );
      return false;
   }

   return true;
}
