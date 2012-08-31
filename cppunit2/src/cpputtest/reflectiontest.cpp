#include "reflectiontest.h"
#include "testing.h"
#include <cpptl/reflection.h>


CPPUT_SUITE_REGISTER( ReflectionTest );


namespace { // anonymous namespace
class Sample1;
} // anonymous namespace

CPPTL_DECLARE_TYPE_AND_PTR_INFO( Sample1 );     // Not required if RTTI are always enabled

namespace { // anonymous namespace
class Sample1
{
public:
   CPPTL_REFLECT_CLASS_BEGIN( Sample1 )
      CPPTL_REFLECT_METHOD( initialize )
      CPPTL_REFLECT_METHOD( add )
      CPPTL_REFLECT_METHOD( multiplyAndAdd )
      CPPTL_REFLECT_METHOD( checkTotal )
      CPPTL_REFLECT_METHOD_WITH_RETURN( isInitialized )
      CPPTL_REFLECT_METHOD_WITH_RETURN( getTotal )
      CPPTL_REFLECT_ATTRIBUT( initialized_ )
      CPPTL_REFLECT_RENAMED_ATTRIBUT( total_, "total" )
   CPPTL_REFLECT_CLASS_END()

   Sample1()
      : initialized_( false )
      , total_( 0 )
   {
   }

   void initialize()
   {
      initialized_ = true;
   }

   void add( int value )
   {
      total_ += value;
   }

   void multiplyAndAdd( int x, int y )
   {
      total_ += x * y;
   }

   void checkTotal( int expected ) const
   {
      // @todo blep Restore message once supported by CPPUT_ASSERT_COMPARE
      //, "Total did not match expected value" );
      CPPUT_ASSERT_EQUAL( expected, total_ ); 
   }

   bool isInitialized() const
   {
      return initialized_;
   }

   int getTotal()
   {
      return total_;
   }

   bool initialized_;
   int total_;
};

} // anonymous namespace


CPPTL_REFLECT_REGISTER_CLASS( Sample1 )


ReflectionTest::ReflectionTest()
{
}


ReflectionTest::~ReflectionTest()
{
}


void 
ReflectionTest::setUp()
{
}


void 
ReflectionTest::tearDown()
{
}


void 
ReflectionTest::testBasicReflection()
{
   Sample1 sampleTarget;
   CppTL::Any target = CppTL::makeAny( &sampleTarget );

   const CppTL::Class *class_ = CppTL::Class::findClass( "Sample1" );
   CPPUT_ASSERT( class_ != 0, "Class 'Sample1' not found by reflection." );

   const CppTL::Method *initializeMethod = class_->findMethod( "initialize" );
   CPPUT_ASSERT( initializeMethod != 0, "Method 'initialize' not found by reflection." );
   CppTL::MethodParameters argsInitialize;
   initializeMethod->invoke( target, argsInitialize );
   CPPUT_ASSERT_EQUAL( true, sampleTarget.initialized_ );
// @todo blep restore message on CPPUT_ASSERT_COMPARE support them
//   CPPUT_ASSERT_EQUAL( true, sampleTarget.initialized_, "'initialize' was not called" );

   const CppTL::Method *addMethod = class_->findMethod( "add" );
   CPPUT_ASSERT( addMethod != 0, "Method 'add' not found by reflection." );
   CppTL::MethodParameters argsAdd;
   argsAdd.push_back( CppTL::makeAny( 7 ) );
   addMethod->invoke( target, argsAdd );
   CPPUT_ASSERT_EQUAL( 7, sampleTarget.total_ );

   const CppTL::Method *multiplyAndAddMethod = class_->findMethod( "multiplyAndAdd" );
   CPPUT_ASSERT( multiplyAndAddMethod != 0, "Method 'multiplyAndAdd' not found by reflection." );
   CppTL::MethodParameters argsMultiplyAndAdd;
   argsMultiplyAndAdd.push_back( CppTL::makeAny( 2 ) );
   argsMultiplyAndAdd.push_back( CppTL::makeAny( 6 ) );
   multiplyAndAddMethod->invoke( target, argsMultiplyAndAdd );
   CPPUT_ASSERT_EQUAL( 7 + 2*6, sampleTarget.total_ );

   const CppTL::Method *checkTotalMethod = class_->findMethod( "checkTotal" );
   CPPUT_ASSERT( checkTotalMethod != 0, "Method 'checkTotal' not found by reflection." );
   CppTL::MethodParameters argsCheckTotal;
   argsCheckTotal.push_back( CppTL::makeAny( 7 + 2 * 6 ) );
   checkTotalMethod->invoke( target, argsCheckTotal );
   CPPUT_ASSERT_EQUAL( 7 + 2*6, sampleTarget.total_ );

   const CppTL::Method *getTotalMethod = class_->findMethod( "getTotal" );
   CPPUT_ASSERT( getTotalMethod != 0, "Method 'getTotal' not found by reflection." );
   CppTL::MethodParameters argsGetTotal;
   CPPUT_ASSERT_EQUAL( 7 + 2*6, any_cast( getTotalMethod->invoke( target, argsGetTotal ), 
                                          CppTL::Type<int>() ) );

   const CppTL::Method *isInitializedMethod = class_->findMethod( "isInitialized" );
   CPPUT_ASSERT( isInitializedMethod != 0, "Method 'isInitialized' not found by reflection." );
   CppTL::MethodParameters argsIsInitialized;
   CPPUT_ASSERT_EQUAL( true, any_cast( isInitializedMethod->invoke( target, argsIsInitialized ), 
                                       CppTL::Type<bool>() ) );

   const CppTL::Attribut *initializedAttribut = class_->findAttribut( "initialized_" );
   CPPUT_ASSERT( initializedAttribut != 0, "Attribut 'initialized_' not found by reflection." );
   CPPUT_ASSERT_EQUAL( true, any_cast( initializedAttribut->get( target ), 
                                       CppTL::Type<bool>() ) );

   const CppTL::Attribut *totalAttribut = class_->findAttribut( "total" );
   CPPUT_ASSERT( totalAttribut != 0, "Attribut 'total' not found by reflection." );
   CPPUT_ASSERT_EQUAL( 7 + 2*6, any_cast( totalAttribut->get( target ), 
                                          CppTL::Type<int>() ) );
   totalAttribut->set( target, CppTL::makeAny( 945 ) );
   CPPUT_ASSERT_EQUAL( 945, sampleTarget.total_ );
}
