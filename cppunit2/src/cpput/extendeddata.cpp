#include <cpput/testing.h>

namespace CppUT {

// class TestExtendedDataHelper
// //////////////////////////////////////////////////////////////////

TestExtendedDataHelper::TestExtendedDataHelper( MetaData &test )
   : test_( test )
{
}


TestExtendedDataHelper::~TestExtendedDataHelper()
{
}


MetaData &
TestExtendedDataHelper::operator()( const TestExtendedData &data ) const
{
   data.apply( test_ );
   return test_;
}


// class TestExtendedData
// //////////////////////////////////////////////////////////////////
TestExtendedData::~TestExtendedData()
{
}


Impl::TestExtendedDataList 
TestExtendedData::operator ,( const TestExtendedData &other ) const
{
   return Impl::TestExtendedDataList( *this, other );
}


// class TestExtendedDataList
// //////////////////////////////////////////////////////////////////
namespace Impl {

TestExtendedDataList::TestExtendedDataList( const TestExtendedData &left,
                                            const TestExtendedData &right )
   : left_( left )
   , right_( right )
{
}

void 
TestExtendedDataList::apply( MetaData &test ) const
{
   left_.apply( test );
   right_.apply( test );
}


// class DefaultTestExtendedData
// //////////////////////////////////////////////////////////////////

void 
DefaultTestExtendedData::apply( MetaData &test ) const
{
}


// class DescriptionData
// //////////////////////////////////////////////////////////////////

DescriptionData::DescriptionData( const std::string &description )
   : description_( description )
{
}


void 
DescriptionData::apply( MetaData &test ) const
{
   test.setDescription( description_.c_str() );
}


// class TimeOutData
// //////////////////////////////////////////////////////////////////

TimeOutData::TimeOutData( double timeOutInSeconds )
   : timeOutInSeconds_( timeOutInSeconds )
{
}

void 
TimeOutData::apply( MetaData &test ) const
{
   test.setTimeOut( timeOutInSeconds_ );
}


// class DependenciesData
// //////////////////////////////////////////////////////////////////

DependenciesData::DependenciesData(  const std::string &dependencies  )
   : dependencies_( dependencies )
{
}

void 
DependenciesData::apply( MetaData &test ) const
{
//   test.setDependenciesFromPackedString( dependencies );
}


// class GroupData
// //////////////////////////////////////////////////////////////////

GroupData::GroupData( const std::string &groupName )
   : groupName_( groupName )
{
}

void 
GroupData::apply( MetaData &test ) const
{
   test.addToGroup( groupName_ );
}

} // end namespace Impl


// class TestExtendedDataFactory
// //////////////////////////////////////////////////////////////////

TestExtendedDataFactory::~TestExtendedDataFactory()
{
}


Impl::DescriptionData 
TestExtendedDataFactory::describe( const std::string &description )
{
   return Impl::DescriptionData( description );
}


Impl::TimeOutData 
TestExtendedDataFactory::timeOut( double timeOutInSeconds )
{
   return Impl::TimeOutData( timeOutInSeconds );
}

Impl::DependenciesData 
TestExtendedDataFactory::depends( const std::string &dependencies )
{
   return Impl::DependenciesData( dependencies );
}

Impl::GroupData 
TestExtendedDataFactory::group( const std::string &groupName )
{
   return Impl::GroupData( groupName );
}


} // namespace CppUT
