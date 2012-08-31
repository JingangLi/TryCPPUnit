#include <cpput/testing.h>
#include <cpput/translate.h>
#include <cpptl/scopedptr.h>
#include <cpptl/thread.h>
#include <deque>



namespace CppUT {

   /// \cond implementation_detail
namespace Impl {
   //typedef volatile unsigned int ReferenceCounter;

// class RegistryImpl
// //////////////////////////////////////////////////////////////////

#define CPPUT_CHECK_REGISTRY_VALID()      \
      CPPTL_ASSERT_MESSAGE( isValid(),    \
         "Attempting to register test after test suite registry destruction!" )


   class RegistryImpl
   {
      enum {
         isAliveMagicValue = 501318481
      };
   public:
      RegistryImpl();
      ~RegistryImpl();
      bool isValid() const;

      SuiteMeta addRootSuite( const std::string &packedName );
      SuiteImpl *createOrphanedSuite( const std::string &name );

      Suite getRootSuite();

      //void addMetaReference( SuiteId suiteId );
      //void removeMetaReference( SuiteId suiteId );

      //void addSuiteReference( SuiteImpl *impl );
      //void removeSuiteReference( SuiteImpl *impl );

      // Returns the default root suite
      // Usually called during static initialization
      SuiteMeta &rootSuite();

      // Returns the suite last set by setCurrentRootSuite().
      // Usually called during static initialization
      SuiteMeta currentParentSuite();

      // Sets the current root suite.
      // Usually called during static initialization
      // if isDefault is true, any later attempt to create
      // a new root suite will cause it to be a child of the 
      // default root suite.
      void setCurrentRootSuite( SuiteMeta &suite, 
                                bool isDefault );

      // Sets the current root suite to the top root suite.
      void setCurrentRootSuiteToTopRootSuite();

      std::string suiteName( SuiteImpl *suite ) const;
      void addSuiteTestCase( SuiteImpl *suite, 
                             const TestMeta &testCase );
      int nestedSuiteCount( SuiteImpl *suite ) const;
      Suite nestedSuiteAt( SuiteImpl *suite, 
                           int index ) const;
      Suite nestedSuiteByName( SuiteImpl *suite,
                               const std::string &name ) const;
      Suite makeNestedSuite( SuiteImpl *suite, 
                             const std::string &name );
      void reparentSuite( Suite &parentSuite, 
                          Suite &suite );
      int suiteTestCaseCount( SuiteImpl *suite ) const;
      TestMeta *suiteTestCaseAt( SuiteImpl *suite, int index ) const;
      Suite getParentSuite( SuiteImpl *suite ) const;

      std::string dump() const;

   private:
      mutable CppTL::Mutex lock_;
      typedef std::set<SuiteImpl *> OrphanedSuites;
      OrphanedSuites orphanedSuites_;
      unsigned int magic_;
      bool isValid_;
      CppTL::ScopedPtr<SuiteImpl> rootSuite_;
      SuiteMeta rootSuiteMeta_;
      SuiteImpl *defaultRootSuite_;
      SuiteImpl *currentParentSuite_;
   };



// class SuiteImpl
// //////////////////////////////////////////////////////////////////

   class SuiteImpl : private CppTL::NonCopyable
   {
      // Notes: all access must go through RegistryImpl to guaranty thread-safety
      friend class RegistryImpl;
   public:
      ~SuiteImpl();
   private:
      typedef std::deque<TestMeta> TestCases;
      typedef std::vector<SuiteImpl *> NestedSuites;

      explicit SuiteImpl( const std::string &name,
                          SuiteImpl *parentSuite );

      // Returns the matching nested suite if found, NULL otherwise.
      SuiteImpl *nestedSuiteByName( const std::string &name ) const;

      void removeNestedSuite( SuiteImpl *suiteToRemove );

      // Remove childSuite from its current parent and adds it to this suite.
      void reparent( SuiteImpl *childSuite );

      void dump() const;

      NestedSuites nestedSuites_;
      TestCases testCases_;
      const CppTL::ConstString name_;
      SuiteImpl *parentSuite_;
      //ReferenceCounter refCount_;
   };


// implementation of class RegistryImpl
// //////////////////////////////////////////////////////////////////

   inline static RegistryImpl &
   registryInstance()
   {
      static RegistryImpl instance;
      return instance;
   }


   RegistryImpl::RegistryImpl()
      : magic_( isAliveMagicValue )
      , isValid_( true )
      , currentParentSuite_( 0 )
      , defaultRootSuite_( 0 )
      , rootSuiteMeta_( 0 )
   {
      // Creates and register the root suite meta to the registry
      rootSuite_.reset( new SuiteImpl( "", 0 ) );
      rootSuiteMeta_ = SuiteMeta( rootSuite_.get() );
   }


   RegistryImpl::~RegistryImpl()
   {
      OrphanedSuites::iterator itEnd = orphanedSuites_.end();
      for ( OrphanedSuites::iterator it = orphanedSuites_.begin(); 
            it != itEnd;
            ++it )
      {
         delete *it;
      }
      magic_ = 0;
      isValid_ = false;
   }


   bool 
   RegistryImpl::isValid() const
   {
      // The magic value is used to work possible
      // static uninitialization order issue
      // (e.g. the instance has been constructed, then destroyed and is still
      // referenced by some other static).
      return magic_ == isAliveMagicValue  &&  isValid_;
   }

   typedef std::pair<int,int> Slice;
   typedef std::vector< Slice> Slices;

   /*! \brief Split a packed suite name along the '/'.
    * The packed name "/" and "//" resolves to a blank string and generate no slice.
    * Packed suite name are implicitly rooted. The initial '/' is always ignored if present.
    * @returns list of name between '/'.
    */
   static void 
   splitPackedName( const std::string &packedName,
                    Slices &slices )
   {
      std::string::size_type index = 0;
      std::string::size_type length = packedName.size();
      while ( index < length )
      {
         std::string::size_type indexEnd = packedName.find( '/', index );
         if ( indexEnd == std::string::npos )
         {
            indexEnd = length;
         }
         if ( indexEnd > index ) // skip empty slice
         {
            slices.push_back( Slice( index, indexEnd - index ) );
         }
         index = indexEnd + 1;
      }
   }


   SuiteMeta 
   RegistryImpl::addRootSuite( const std::string &packedName )
   {
      CPPUT_CHECK_REGISTRY_VALID();
      Slices slices;
      splitPackedName( packedName, slices );

      SuiteImpl *parentSuite = 0;
      {
         CppTL::Mutex::ScopedLockGuard guard( lock_ );
         if ( defaultRootSuite_ != 0 )
         {
            printf( "Creating suite rooted to default: %s\n", packedName.c_str() );
            parentSuite = defaultRootSuite_;
         }
         else
         {
            printf( "Creating suite rooted to top root: %s\n", packedName.c_str() );
            parentSuite = rootSuite_.get();
         }
         Slices::iterator itEnd = slices.end();
         for ( Slices::iterator it = slices.begin(); it != itEnd; ++it )
         {
            std::string name = packedName.substr( it->first, it->second );
            SuiteImpl *nestedSuite = parentSuite->nestedSuiteByName( name );
            if ( nestedSuite != 0 )
            {
               parentSuite = nestedSuite;
            }
            else
            {
               // Parent takes the ownership of the created suite
               parentSuite = new SuiteImpl( name, parentSuite );
            }
         }
      }
      return SuiteMeta( parentSuite );
   }

   SuiteImpl *
   RegistryImpl::createOrphanedSuite( const std::string &name )
   {
      CPPUT_CHECK_REGISTRY_VALID();
      SuiteImpl *suite = new SuiteImpl( name, 0 );
      orphanedSuites_.insert( suite );
      return suite;
   }


   Suite 
   RegistryImpl::getRootSuite()
   {
      CPPUT_CHECK_REGISTRY_VALID();
      return Suite( rootSuite_.get() );
   }

   std::string 
   RegistryImpl::suiteName( SuiteImpl *suite ) const
   {
      CPPUT_CHECK_REGISTRY_VALID();
      // Do not need to lock, ConstString::str() is thread-safe
      return suite->name_.str();
   }

   void 
   RegistryImpl::addSuiteTestCase( SuiteImpl *suite, 
                                   const TestMeta &testCase )
   {
      CPPUT_CHECK_REGISTRY_VALID();
      CppTL::Mutex::ScopedLockGuard guard( lock_ );
      suite->testCases_.push_back( testCase );
   }


   int 
   RegistryImpl::nestedSuiteCount( SuiteImpl *suite ) const
   {
      CPPUT_CHECK_REGISTRY_VALID();
      CppTL::Mutex::ScopedLockGuard guard( lock_ );
      return int( suite->nestedSuites_.size() );
   }


   Suite 
   RegistryImpl::nestedSuiteAt( SuiteImpl *suite, 
                                int index ) const
   {
      CPPUT_CHECK_REGISTRY_VALID();
      CppTL::Mutex::ScopedLockGuard guard( lock_ );
      if ( index < 0  ||  index  >= suite->nestedSuites_.size() )
      {
         return Suite();
      }
      return Suite( suite->nestedSuites_[index] );
   }


   Suite 
   RegistryImpl::nestedSuiteByName( SuiteImpl *suite,
                                    const std::string &name ) const
   {
      CPPUT_CHECK_REGISTRY_VALID();
      CppTL::Mutex::ScopedLockGuard guard( lock_ );
      SuiteImpl *nestedSuite = suite->nestedSuiteByName( name );
      if ( nestedSuite != 0 )
      {
         return Suite( nestedSuite );
      }
      return Suite();
   }


   Suite 
   RegistryImpl::makeNestedSuite( SuiteImpl *suite, 
                                  const std::string &name )
   {
      CPPUT_CHECK_REGISTRY_VALID();
      SuiteImpl *nestedSuite = 0;
      {
         CppTL::Mutex::ScopedLockGuard guard( lock_ );
         nestedSuite = suite->nestedSuiteByName( name );
         if ( nestedSuite == 0 ) // create the suite
         {
            nestedSuite = new SuiteImpl( name, suite );
         }
      }
      return Suite( nestedSuite );
   }


   void 
   RegistryImpl::reparentSuite( Suite &parentSuite, 
                                Suite &suite )
   {
      CPPUT_CHECK_REGISTRY_VALID();
      CPPTL_ASSERT_MESSAGE( parentSuite.impl_ != 0  &&  suite.impl_ != 0, 
         "Attempting to reparent invalid suites." );
      if ( suite.impl_->parentSuite_ == 0 )  // if orphan, remove from orphan list
      {
         orphanedSuites_.erase( suite.impl_ );
      }
      parentSuite.impl_->reparent( suite.impl_ );
      if ( parentSuite.impl_ == 0 ) // made orphan, add to orphan list
      {
         orphanedSuites_.insert( suite.impl_ );
      }
   }


   int 
   RegistryImpl::suiteTestCaseCount( SuiteImpl *suite ) const
   {
      CPPUT_CHECK_REGISTRY_VALID();
      CppTL::Mutex::ScopedLockGuard guard( lock_ );
      return int(suite->testCases_.size());
   }


   TestMeta *
   RegistryImpl::suiteTestCaseAt( SuiteImpl *suite, int index ) const
   {
      CPPUT_CHECK_REGISTRY_VALID();
      CppTL::Mutex::ScopedLockGuard guard( lock_ );
      if ( index < 0  ||  index >= suite->testCases_.size() )
      {
         return 0;
      }
      return &(suite->testCases_[index]);
   }

   Suite 
   RegistryImpl::getParentSuite( SuiteImpl *suite ) const
   {
      CPPUT_CHECK_REGISTRY_VALID();
      CppTL::Mutex::ScopedLockGuard guard( lock_ );
      return Suite( suite->parentSuite_ );
   }


   //void 
   //RegistryImpl::addMetaReference( Impl::SuiteId suiteId )
   //{
   //}


   //void 
   //RegistryImpl::removeMetaReference( Impl::SuiteId suiteId )
   //{
   //}


   //void 
   //RegistryImpl::addSuiteReference( SuiteImpl *impl )
   //{
   //   if ( !isValid() ) // After static uninitialisation, all suite have been destroyed
   //   {
   //      return;
   //   }

   //   CppTL::Mutex::ScopedLockGuard guard( lock_ );
   //   ++(impl->refCount_);
   //}


   //void 
   //RegistryImpl::removeSuiteReference( SuiteImpl *impl )
   //{
   //   if ( !isValid() ) // After static uninitialisation, all suite have been destroyed
   //   {
   //      return;
   //   }

   //   CppTL::Mutex::ScopedLockGuard guard( lock_ );
   //   CPPTL_ASSERT_MESSAGE( impl->refCount_ > 0, "Bad reference count balance" );
   //   if ( --(impl->refCount_) == 0 )
   //   {
   //      // @todo likely more stuff to clean-up
   //      delete impl;
   //   }
   //}


   SuiteMeta &
   RegistryImpl::rootSuite()
   {
      CPPUT_CHECK_REGISTRY_VALID();
      return rootSuiteMeta_;
   }


   SuiteMeta
   RegistryImpl::currentParentSuite()
   {
      CPPUT_CHECK_REGISTRY_VALID();
      CPPTL_ASSERT_MESSAGE( currentParentSuite_ != 0  ||  defaultRootSuite_ != 0, 
         "Attempting to register test after test suite registry destruction!" );
      if ( currentParentSuite_ != 0 )
      {
      return SuiteMeta( currentParentSuite_ );
   }
      return SuiteMeta( defaultRootSuite_ );
   }

   
   void 
   RegistryImpl::setCurrentRootSuite( SuiteMeta &suite,
                                      bool isDefault )
   {
      if ( isDefault )
      {
         defaultRootSuite_ = suite.impl_;
         currentParentSuite_ = 0;
      }
      else
      {
         currentParentSuite_ = suite.impl_;
      }
   }

   void
   RegistryImpl::setCurrentRootSuiteToTopRootSuite()
   {
      defaultRootSuite_ = 0;
      currentParentSuite_ = rootSuite_.get();
   }


   std::string 
   RegistryImpl::dump() const
   {
      CppTL::Mutex::ScopedLockGuard guard( lock_ );
      std::string out;
      typedef std::pair<SuiteImpl *,std::string> Context;
      std::deque<Context> parents;
      parents.push_back( Context(rootSuite_.get(), "") );
      while ( !parents.empty() )
      {
         Context context = parents.front();
         parents.pop_front();
         SuiteImpl *suite = context.first;
         std::string suitePath = context.second;
         out += "suite:" + suitePath + "/\n";
         for ( SuiteImpl::NestedSuites::iterator it = suite->nestedSuites_.begin();
               it != suite->nestedSuites_.end();
               ++it )
         {
            SuiteImpl *nestedSuite = *it;
            std::string nestedPath = suitePath + "/" + nestedSuite->name_.str();
            parents.push_back( Context(nestedSuite, nestedPath ) );
         }
         for ( SuiteImpl::TestCases::iterator itTest = suite->testCases_.begin();
               itTest != suite->testCases_.end();
               ++itTest )
         {
            out += "test: " + suitePath + "/" + itTest->name() + "\n";
         }
      }
      return out;
   }



// implementation of class SuiteImpl
// //////////////////////////////////////////////////////////////////

   SuiteImpl::SuiteImpl( const std::string &name,
                         SuiteImpl *parentSuite )
      : name_( name )
      , parentSuite_( parentSuite )
   {
      if ( parentSuite != 0 )
      {
         CPPTL_ASSERT_MESSAGE( parentSuite != this,
                               "Suite can not parent itself." );
         parentSuite->nestedSuites_.push_back( this );
      }
   }

   SuiteImpl::~SuiteImpl()
   {
      NestedSuites::iterator itEnd = nestedSuites_.end();
      for ( NestedSuites::iterator it = nestedSuites_.begin();
            it != itEnd;
            ++it )
      {
         SuiteImpl *nestedSuite = *it;
         delete nestedSuite;
      }
   }


   SuiteImpl *
   SuiteImpl::nestedSuiteByName( const std::string &name ) const
   {
      NestedSuites::const_iterator itEnd = nestedSuites_.end();
      for ( NestedSuites::const_iterator it = nestedSuites_.begin();
            it != itEnd;
            ++it )
      {
         SuiteImpl *nestedSuite = *it;
         if ( nestedSuite->name_ == name )
         {
            return nestedSuite;
         }
      }
      return 0;
   }


   void 
   SuiteImpl::removeNestedSuite( SuiteImpl *suiteToRemove )
   {
      NestedSuites::iterator itEnd = nestedSuites_.end();
      for ( NestedSuites::iterator it = nestedSuites_.begin();
            it != itEnd;
            ++it )
      {
         SuiteImpl *nestedSuite = *it;
         if ( nestedSuite == suiteToRemove )
         {
            nestedSuites_.erase( it );
            return;
         }
      }
      // logic error: should have found the nested suite
      CPPTL_DEBUG_ASSERT_UNREACHABLE;  
   }


   void 
   SuiteImpl::reparent( SuiteImpl *childSuite )
   {
      printf( "Reparenting suite %s %p with parent %s %p to %s %p\n",
         childSuite->name_.c_str(), childSuite,
         childSuite->parentSuite_ ? childSuite->parentSuite_->name_.c_str()
                                  : "<orphan>", 
         childSuite->parentSuite_,
         name_.c_str(), this );
      CPPTL_ASSERT_MESSAGE( childSuite != this,
                            "Suite can not parent itself." );
      if ( childSuite->parentSuite_ != 0 )
      {
         childSuite->parentSuite_->removeNestedSuite( childSuite );
      }
      childSuite->parentSuite_ = this;
      nestedSuites_.push_back( childSuite );
   }

   void
   SuiteImpl::dump() const
   {
      printf( "  Suite %s %p has parent %p\n", name_.c_str(), this, parentSuite_ );
      SuiteImpl::NestedSuites::const_iterator itEnd = nestedSuites_.end();
      for ( NestedSuites::const_iterator it = nestedSuites_.begin();
            it != itEnd;
            ++it )
      {
         SuiteImpl *nestedSuite = *it;
         printf( "   has nested suite %s %p with parent %p\n", 
                 nestedSuite->name_.c_str(), nestedSuite, nestedSuite->parentSuite_ );
      }
   }



// currentSuite()
// //////////////////////////////////////////////////////////////////

   SuiteMeta
   currentSuite()
   {
      return Impl::registryInstance().currentParentSuite();
   }


// StaticSuite factory functions
// //////////////////////////////////////////////////////////////////

// The registry assumes ownership of all suites created by the factory
// functions.

   StaticSuite *
   makeNestedSuiteInDefaultRootSuite( const char *name )
   {
      RegistryImpl &instance = Impl::registryInstance();
      SuiteMeta suite = instance.addRootSuite( name );
      instance.setCurrentRootSuite( suite, false );
      return 0;
   }


   StaticSuite *
   resetDefaultRootSuite()
   {
      Impl::registryInstance().setCurrentRootSuiteToTopRootSuite();
      return 0;
   }

   StaticSuite *
   makeAndSetDefaultRootSuite( const char *name )
   {
      RegistryImpl &instance = Impl::registryInstance();
      SuiteMeta suite = instance.addRootSuite( name );
      instance.setCurrentRootSuite( suite, true );
      return 0;
   }

   StaticSuite *
   reparentSuite( Suite parentSuite, 
                  Suite suite )
   {
      Impl::registryInstance().reparentSuite( parentSuite, suite );
      return 0;
   }



} // namespace Impl
/// \endcond

// class SuiteMeta
// //////////////////////////////////////////////////////////////////
SuiteMeta::SuiteMeta( Impl::SuiteImpl *impl )
   : Suite( impl )
{
}

SuiteMeta::~SuiteMeta()
{
}


// class Suite
// //////////////////////////////////////////////////////////////////

// Notes: at the current time, all suites are owned by RegistryImpl
// All suites are destroyed when RegistryImpl is destroyed.
// => this may need to be changed in the future to support unloading
// suite contained in a dynamic library.

Suite::Suite()
   : impl_( 0 )
{
}


Suite::Suite( const std::string &name )
   : impl_( Impl::registryInstance().createOrphanedSuite( name ) )
{
}


//Suite::Suite( const Suite &other )
//   : impl_( other.impl_ )
//{
//   if ( impl_ )
//   {
//      Impl::registryInstance().addSuiteReference( impl_ );
//   }
//}


Suite::Suite( Impl::SuiteImpl *suite )
   : impl_( suite )
{
   //if ( impl_ )
   //{
   //   Impl::registryInstance().addSuiteReference( impl_ );
   //}
}


Suite::~Suite()
{
   //if ( impl_ )
   //{
   //   Impl::registryInstance().removeSuiteReference( impl_ );
   //}
}


//Suite &
//Suite::operator =( const Suite &other )
//{
//   Impl::RegistryImpl &instance = Impl::registryInstance();
//   if ( impl_ )
//   {
//      instance.removeSuiteReference( impl_ );
//   }
//   impl_ = other.impl_;
//   if ( impl_ )
//   {
//      instance.addSuiteReference( impl_ );
//   }
//   return *this;
//}

Suite 
Suite::parent() const
{
   return Impl::registryInstance().getParentSuite( impl_ );
}


bool 
Suite::isValid() const
{
   return impl_ != 0;
}


std::string
Suite::name() const
{
   if ( impl_ != 0 )
      return Impl::registryInstance().suiteName( impl_ );
   return std::string();
}


int 
Suite::nestedSuiteCount() const
{
   if ( impl_ != 0 )
   {
      return Impl::registryInstance().nestedSuiteCount( impl_ );
   }
   return 0;
}


Suite 
Suite::nestedSuiteAt( int index ) const
{
   if ( impl_ != 0 )
   {
      return Impl::registryInstance().nestedSuiteAt( impl_, index );
   }
   return Suite();
}


Suite 
Suite::nestedSuiteByName( const std::string &name ) const
{
   if ( impl_ != 0 )
   {
      return Impl::registryInstance().nestedSuiteByName( impl_, name );
   }
   return Suite();
}


Suite 
Suite::makeNestedSuite( const std::string &name )
{
   if ( impl_ != 0 )
   {
      return Impl::registryInstance().makeNestedSuite( impl_, name );
   }
   return Suite();
}


int 
Suite::testCaseCount() const
{
   if ( impl_ != 0 )
   {
      return Impl::registryInstance().suiteTestCaseCount( impl_ );
   }
   return 0;
}


const TestMeta *
Suite::testCaseAt( int index ) const
{
   if ( impl_ != 0 )
   {
      return Impl::registryInstance().suiteTestCaseAt( impl_, index );
   }
   return 0;
}


void 
Suite::add( const TestMeta &testCase )
{
   CPPTL_ASSERT_MESSAGE( impl_ != 0, 
      "Attempting to add a test case to an invalid suite!" );
   if ( impl_ != 0 )
   {
      Impl::registryInstance().addSuiteTestCase( impl_, testCase );
   }
}


bool 
Suite::operator <( const Suite &other ) const
{
   return impl_ < other.impl_;
}


bool 
Suite::operator ==( const Suite &other ) const
{
   return impl_ == other.impl_;
}


unsigned int 
Suite::hash() const
{
   return reinterpret_cast<unsigned int>( impl_ );
}



// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// class Registry
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////


Suite
Registry::getRootSuite()
{
   return Impl::registryInstance().getRootSuite();
}


std::string 
Registry::dump()
{
   return Impl::registryInstance().dump();
}


//
//std::string 
//Registry::defaultParentSuiteName()
//{
//   return "";
//}
//
//
//Registry &
//Registry::instance()
//{
//   static Registry registry;     // @todo make this thread safe if multi-threading is enabled.
//   return registry;
//}
//
//
//Registry::Registry()
//   : nextId_( 0 )
//{
//}
//
//
//void 
//Registry::addChild( const std::string &parentSuiteName,
//                    const std::string &childSuiteName )
//{
////   ParentChildRelationShips::iterator it = relations_.find( childSuiteName );
//
////   relations_.erase( childSuiteName );     // erase inversed relationship
//   relations_.insert( ParentChildRelationShips::value_type( parentSuiteName, childSuiteName ) );
//}
//
//
//void 
//Registry::addChildToDefault( const std::string &childSuiteName )
//{
//   addChild( defaultParentSuiteName(), childSuiteName );
//}
//
//
//bool 
//Registry::removeChild( const std::string &parentSuiteName,
//                       const std::string &childSuiteName )
//{
//   ParentChildRelationShips::iterator it = relations_.find( parentSuiteName );
//   for ( ; it != relations_.end()  &&  it->first == parentSuiteName; ++it )
//   {
//      const CppTL::ConstString &currentChildName = it->second;
//      if ( currentChildName == childSuiteName )
//      {
//         relations_.erase( it );
//         return true;
//      }
//   }
//   return false;
//}
//
//
//TestFactoryId
//Registry::add( const std::string &parentSuiteName,
//               const TestFactory &testFactory )
//{
//   CppTL::ConstString actualParentSuiteName = parentSuiteName;
//   if ( parentSuiteName.empty() )
//      actualParentSuiteName = defaultParentSuiteName();
//   TestFactoryId id = nextFactoryId();
//   registry_[ actualParentSuiteName ].push_back( TestFactoryWithId( testFactory, id ) );
//   parentSuiteById_[ id ] = actualParentSuiteName;
//   return id;
//}
//
//
//TestFactoryId 
//Registry::add( const std::string &parentSuiteName,
//               TestPtr (*factory)() )
//{
//   return add( parentSuiteName, CppTL::cfn0r( factory ) );
//}
//
//
//TestFactoryId 
//Registry::add( const std::string &parentSuiteName,
//               TestPtr (*factory)( const std::string &suiteName ),
//               const std::string &suiteName )
//{
//   return add( parentSuiteName, 
//               CppTL::fn0r( Impl::ThreadSafeSuiteFactory( factory, suiteName ) ) );
//}
//
//
//TestFactoryId
//Registry::addToDefault( const TestFactory &testFactory )
//{
//   return add( defaultParentSuiteName(), testFactory );
//}
//
//TestFactoryId 
//Registry::addToDefault( TestPtr (*factory)() )
//{
//   return addToDefault( CppTL::cfn0r( factory ) );
//}
//
//TestFactoryId 
//Registry::addToDefault( TestPtr (*factory)( const std::string &suiteName ),
//                        const std::string &suiteName)
//{
//   return addToDefault( CppTL::fn0r( Impl::ThreadSafeSuiteFactory( factory, 
//                                                                   suiteName ) ) );
//}
//
//
//bool 
//Registry::remove( TestFactoryId testFactoryId )
//{
//   TestFactoryParentSuiteById::iterator parentIt = parentSuiteById_.find( testFactoryId );
//   if ( parentIt == parentSuiteById_.end() )
//      return false;
//
//   CppTL::ConstString parentSuiteName = parentIt->second;
//   TestFactoryRegistry::iterator itRegistry = registry_.find( parentSuiteName );
//   if ( itRegistry == registry_.end() )
//      return false;
//
//   TestFactories &factories = itRegistry->second;
//   TestFactories::iterator itFactory = factories.begin();
//   bool found = false;
//   while ( itFactory != factories.end() )
//   {
//      if ( itFactory->second == testFactoryId )
//      {
//         itFactory = factories.erase( itFactory );
//         found = true;
//      }
//      else
//         ++itFactory;
//   }
//
//   return found;
//}
//
//
//
///*
//void 
//Registry::destroy( const CppTL::ConstString &suiteName )
//{
//   registry_.erase( suiteName );
//   ParentChildRelationShips::iterator it = relations_.begin();
//   ParentChildRelationShips::iterator itEnd = relations_.end();
//   while ( it != itEnd )
//   {
//      if ( it->first == suiteName  ||  it->second == suiteName )
//        relations_.erase( it++ );
//      else
//         ++it;
//   }
//}
//*/
//
//TestSuitePtr 
//Registry::createTests( const std::string &suiteName ) const
//{
//   if ( suiteName == defaultParentSuiteName() )
//      return createDefaultTests();
//
//   TestSuitePtr suite( makeTestSuite( suiteName ) );
//   addCreatedTests( suiteName, suite );
//   return suite;
//}
//
//
//TestSuitePtr 
//Registry::createDefaultTests() const
//{
//   TestSuitePtr suite( makeTestSuite( translate( "All Tests" ) ) );
//   addCreatedTests( defaultParentSuiteName(), suite );
//   return suite;
//}
//
//
//void 
//Registry::addCreatedTests( const std::string &suiteName,
//                           const TestSuitePtr &suite ) const
//{
//   addChildSuite( suiteName, suite );
//   addSuiteRegisteredTests( suiteName, suite );
//}
//
//
//void 
//Registry::addChildSuite( const std::string &suiteName, 
//                         const TestSuitePtr &suite ) const
//{
//   ParentChildRelationShips::const_iterator itChild = relations_.lower_bound( suiteName );
//   ParentChildRelationShips::const_iterator itChildEnd = relations_.upper_bound( suiteName );
//   while ( itChild != itChildEnd )
//   {
//      const CppTL::ConstString &childSuiteName = itChild->second;
//      suite->add( createTests( childSuiteName.str() ) );
//      ++itChild;
//   }
//}
//
//
//void 
//Registry::addSuiteRegisteredTests( const std::string &suiteName, 
//                                   const TestSuitePtr &suite ) const
//{
//   TestFactoryRegistry::const_iterator itSuite = registry_.find( suiteName );
//   if ( itSuite == registry_.end() )
//      return;
//
//   const TestFactories &factories = itSuite->second;
//   TestFactories::const_iterator itFactory = factories.begin();
//   TestFactories::const_iterator itFactoryEnd = factories.end();
//   while ( itFactory != itFactoryEnd )
//   {
//      TestFactory factory = (*itFactory++).first;
//      suite->add( factory() );
//   }
//}
//
//
//TestFactoryId 
//Registry::nextFactoryId()
//{
//   return nextId_++;
//}


} // namespace CppUT
