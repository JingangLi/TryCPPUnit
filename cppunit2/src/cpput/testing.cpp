#include <cpput/testing.h>
#include <cpptl/conststring.h>


namespace CppUT {

// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// class MetaData
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////

MetaData::MetaData( const std::string &name )
{
   info_["configuration/name"] = name;
   info_["input"] = Json::Value( Json::objectValue );
}


MetaData::MetaData( const std::string &name,
                    const TestExtendedData &metaDataFactory )
{
   info_["configuration/name"] = name;
   metaDataFactory.apply( *this );
}

MetaData::~MetaData()
{
}


void 
MetaData::setDescription( const std::string &description )
{
   info_["configuration/description"] = description;
}


std::string 
MetaData::description() const
{
   return info_.get("configuration/description", "" ).asString();
}


std::string 
MetaData::name() const
{
   return info_.get( "configuration/name", "" ).asString();
}


void 
MetaData::setTimeOut( double timeOutInSeconds )
{
   info_["configuration/timeOut"] = timeOutInSeconds;
}


double 
MetaData::timeOut() const
{
   return info_.get( "configuration/timeOut", 0.0 ).asDouble();
}


void 
MetaData::addToGroup( const std::string &groupName )
{
   info_["configuration"]["groups"].append( groupName );
}


int 
MetaData::groupCount() const
{
   return info_["configuration"]["groups"].size();
}


std::string 
MetaData::groupAt( unsigned int index ) const
{
   return info_["configuration"]["groups"][index].asString();
}


Json::Value &
MetaData::input()
{
   return info_["input"];
}


const Json::Value &
MetaData::input() const
{
   return info_["input"];
}



// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// Lightweight test fixture
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
namespace Impl {
TestMeta 
declareFixtureTestCase( TestCaseFactoryFn factory,
                        const char *fixtureSuiteName,
                        const char *testCaseName,
                        Suite parentSuite )
{
   TestMeta test( factory, testCaseName );
   parentSuite.makeNestedSuite( fixtureSuiteName ).add( test );
   return test;
}

TestMeta 
declareFixtureTestCase( TestCaseFactoryFn factory,
                        const char *fixtureSuiteName,
                        const char *testCaseName,
                        const ::CppUT::TestExtendedData &metaData,
                        Suite parentSuite )
{
   MetaData testMetaData( testCaseName );
   metaData.apply( testMetaData );
   TestMeta test( factory, testMetaData );
   parentSuite.makeNestedSuite( fixtureSuiteName ).add( test );
   return test;
}

} // end namespace Impl {


// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// Free function based test cases
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////

namespace Impl {

TestMeta 
declareTestCaseFunction( void (*run)(), 
                         const char *name, 
                         Suite parentSuite )
{
   TestMeta test = makeTestCase( run, name );
   parentSuite.add( test );
   return test;
}


TestMeta 
declareTestCaseFunction( void (*run)(), 
                         const MetaData &metaData,
                         Suite parentSuite )
{
   TestMeta test = makeTestCase( run, metaData );
   parentSuite.add( test );
   return test;
}

} // end namespace Impl {

} // end namespace CppUT
