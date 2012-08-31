#include <cpput/parametrizedsource.h>
#include <cpput/testing.h>
#include <cpput/tablefixture.h>

namespace CppUT {

int 
TestParameterFactoryImpl::testCaseCount()
{
   return tests_.size();
}


std::string 
TestParameterFactoryImpl::testCaseNameSuffix( int index )
{
# ifdef CPPUT_HAS_VECTOR_AT
   return tests_.at( index ).first;
# else
   return tests_[ index ].first;
# endif
}


ParameterNodePtr 
TestParameterFactoryImpl::testCaseParameter( int index )
{
# ifdef CPPUT_HAS_VECTOR_AT
   return tests_.at( index ).second;
# else
   return tests_[ index ].second;
# endif
}


void 
TestParameterFactoryImpl::addTestCase( const std::string &nameSuffix,
                                      const ParameterNodePtr &parameterNode )
{
   tests_.push_back( TestData( nameSuffix, parameterNode ) );
}





ParameterSourceRegistry &
ParameterSourceRegistry::instance()
{
   static ParameterSourceRegistry registry;
   return registry;
}


void 
ParameterSourceRegistry::addSuiteSource(  
                        const std::string &suiteName,
                        const std::string &complementaryName,
                        SuiteParameterFactory suiteParameterFactory )
{
   suitePath_[ SuiteKey(suiteName,complementaryName) ] = suiteParameterFactory;
}


SuiteParameterFactory 
ParameterSourceRegistry::getSuiteParameterFactory( 
                        const std::string &suiteName,
                        const std::string &complementaryName ) const
{
   SuiteKey key( suiteName, complementaryName );
   SuitePaths::const_iterator it = suitePath_.find( key );
   if ( it != suitePath_.end() )
      return it->second;
   return SuiteParameterFactory();
}




void 
ParameterNode::throwOperationNotSupported( const std::string &operationName ) const
{
   throw std::invalid_argument( "Operation not supported, ParameterNode::" 
      + operationName );
}


ParameterNodePtr 
ParameterNode::childNamed( const std::string &nodeName ) const
{
   int count = childCount();
   for ( int index = 0; index < count; ++index )
   {
      const ParameterNodePtr &node = childAt( index );
      if ( node->nodeName() == nodeName )
         return node;
   }

   throw std::invalid_argument( "ParameterNode::childNamed(), no node named " +
      nodeName + "." );
}

} // namespace CppUT
