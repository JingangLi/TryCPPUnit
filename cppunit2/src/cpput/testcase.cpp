#include <cpput/testing.h>
#include <cpput/assertcommon.h>
#include <cpput/message.h>
#include <cpptl/functor.h>
#include <cpptl/scopedptr.h>


namespace CppUT {



// Class TestCase
// ////////////////////////////////////////////////////////////////////


TestCase::TestCase()
{
}


TestCase::~TestCase()
{
}


void 
TestCase::setUp()
{
}


void 
TestCase::tearDown()
{
}

// Class Impl::TestCaseFunction
// ////////////////////////////////////////////////////////////////////
namespace Impl {

   // May be replaced by:
   // typedef FunctorTestCase<void (*)()> TestCaseFunction;
   // @todo check portability with older compilers.
   class TestCaseFunction : public TestCase
   {
   public:
      typedef void (*RunFn)();

      static TestCase *factory( RunFn run )
      {
         return new TestCaseFunction( run );
      }

      TestCaseFunction( RunFn run )
         : run_( run )
      {
      }

   public: // overriden from TestCase
      virtual void run()
      {
         run_();
      }

   private:
      RunFn run_;
   };

} // namespace Impl


// Class TestMeta
// ////////////////////////////////////////////////////////////////////
   
TestMeta::TestMeta( const TestCaseFactory &factory,
                    const std::string &name )
   : MetaData( name )
   , factory_( factory )
{
}


TestMeta::TestMeta( TestCaseFactoryFn factory,
                    const std::string &name )
   : MetaData( name )
   , factory_( CppTL::cfn0r<TestCase *>( factory ) )
{
}
   
TestMeta::TestMeta( const TestCaseFactory &factory,
                    const MetaData &metaData )
   : MetaData( metaData )
   , factory_( factory )
{
}


TestMeta::TestMeta( TestCaseFactoryFn factory,
                    const MetaData &metaData )
   : MetaData( metaData )
   , factory_( CppTL::cfn0r<TestCase *>( factory ) )
{
}


bool 
TestMeta::runTest() const
{
   ExceptionGuard guardsChain;
   return runTest( guardsChain );
}


/// @todo move this implementation
class TestCaseHandle
{
public:
   /// Safely instantiate the TestCase using the factory and given exception guards
   TestCaseHandle( const TestCaseFactory &factory,
                   const ExceptionGuard &guardsChain )
       : factory_( factory )
       , testCase_( 0 )
       , guardsChain_( guardsChain )
   {
      if ( !guardsChain.protect( 
               CppTL::memfn0( this, &TestCaseHandle::create ) ) )
      {
         testCase_ = 0;
      }
   }

   /// Safely destroy the TestCase using the exception guards chain.
   ~TestCaseHandle()
   {
      release();
   }

   bool release()
   {
      if ( !testCase_ )
         return true;

      return guardsChain_.protect( 
               CppTL::memfn0( this, &TestCaseHandle::safeRelease ) );
   }

   TestCase *get() const
   {
      return testCase_;
   }

private:
   void create()
   {
      testCase_ = factory_();
   }

   void safeRelease()
   {
      TestCase *instance = testCase_;
      testCase_ = 0;
      delete instance;
   }

private:
   const TestCaseFactory &factory_;
   const ExceptionGuard &guardsChain_;
   TestCase *testCase_;
};



static void safeCreateTestCase( const TestCaseFactory &factory,
                                CppTL::ScopedPtr<TestCase> &instance )
{
   instance.reset( factory() );
}


static void safeDestroyTestCase( TestCase *testCase )
{
   delete testCase;
}


bool 
TestMeta::runTest( const ExceptionGuard &guardsChain ) const
{
   TestInfo &testInfo = TestInfo::threadInstance();
   testInfo.startNewTest();
   TestCaseHandle testCase( factory_, guardsChain );
   if ( !testCase.get() )
   {
      if (  !testInfo.testStatus().hasFailed() ) // Factory returned a NULL pointer
      {
         testInfo.log( "Failed to instantiate TestCase." );
         testInfo.testStatus().setStatus( TestStatus::failed );
      }
      return false;
   }

   bool initialized = guardsChain.protect( 
      CppTL::memfn0( testCase.get(), &TestCase::setUp ) );

   if ( initialized )
   {
      guardsChain.protect( CppTL::memfn0( testCase.get(), &TestCase::run ) );
      guardsChain.protect( CppTL::memfn0( testCase.get(), &TestCase::tearDown) );
   }

   // The C++ run-time will call terminate() if an exception is thrown while 
   // another one is also being thrown, which usually means that the stack
   // is being unwinded. 
   // We explicitely call the TestCase destructor instead of relying on RAII to avoid
   // this situation if TestCase destructor throws an exception.
   // While the situation is somewhat recovered, it likely means that some memory was 
   // leaked by the delete operator.
   testCase.release();

   return !testInfo.testStatus().hasFailed();
}


TestMeta 
makeTestCase( void (*run)(),
              const std::string &name )
{
   return TestMeta( CppTL::bind_cfnr( Impl::TestCaseFunction::factory, run ), name );
}


TestMeta 
makeTestCase( void (*run)(),
              const MetaData &metaData )
{
   return TestMeta( CppTL::bind_cfnr( Impl::TestCaseFunction::factory, run ), metaData );
}



namespace Impl {
   class FailingTestCase : public TestCase
   {
   public:
      static TestCase *factory( const CppUT::Message &message )
      {
         return new FailingTestCase( message );
      }

      FailingTestCase( const CppUT::Message &message ) 
         : message_( message )
      {
      }
   private: // overridden from TestCase
      virtual void run()
      {
         CPPUT_FAIL( message_ );
      }

      Message message_;
   };
}


TestMeta CPPUT_API makeFailingTestCase( const std::string &name,
                                        const Message &message )
{
   return TestMeta( CppTL::bind_cfnr( Impl::FailingTestCase::factory, message ), 
                    name );
}


} // namespace CppUT

