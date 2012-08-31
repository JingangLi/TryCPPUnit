#ifndef CPPUT_FORWARDS_H_INCLUDED
# define CPPUT_FORWARDS_H_INCLUDED

# include <cpptl/forwards.h>
# include <cpput/config.h>

namespace CppUT {

class AbortingAssertionException;
class TestCase;
class Message;
class TestMeta;

// impl/testing.h, impl/testingepilog.h
namespace Impl {
   class DescriptionData;
   class DependenciesData;
   class GroupData;
   class RegistryImpl;
   class SuiteImpl;
   class TestExtendedDataList;
   class TimeOutData;
} // end namespace Impl

// testing.h
class ExceptionGuardContext;
class ExceptionGuard;
class MetaData;
class Registry;
class StaticSuite;
class Suite;
class SuiteMeta;
class TestExtendedData;
template<class AType, class BType>
struct EqualityTraits;

// testcase.h
typedef TestCase *(*TestCaseFactoryFn)();

// testinfo.h
class CheckerResult;
class TestInfo;
class TestResultUpdater;

// for ...
typedef CppTL::IntrusivePtr<TestCase> TestCasePtr;
//typedef CppTL::IntrusivePtr<MetaData> TestPtr;
typedef CppTL::IntrusivePtr<TestInfo> TestInfoPtr;
typedef CppTL::IntrusivePtr<TestResultUpdater> TestResultUpdaterPtr;


} // namespace CppUT



#endif // CPPUT_FORWARDS_H_INCLUDED
