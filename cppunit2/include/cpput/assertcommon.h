#ifndef CPPUT_ASSERT_H_INCLUDED
# define CPPUT_ASSERT_H_INCLUDED

# include <cpput/testing.h>
# include <cpput/message.h>
# include <cpput/testinfo.h>
# include <cpput/translate.h>
# include <cpptl/typename.h>

namespace CppUT
{
   CheckerResult CPPUT_API pass();

   CheckerResult CPPUT_API fail();

   CheckerResult CPPUT_API fail( const LazyMessage &message );

   CheckerResult CPPUT_API checkTrue( bool shouldBeTrue,
                                      const LazyMessage &message = LazyMessage::none );

   CheckerResult CPPUT_API checkFalse( bool shouldBeFalse,
                                       const LazyMessage &message = LazyMessage::none );

   CheckerResult CPPUT_API checkExprTrue( bool shouldBeTrue,
                                          const char *expressionCode );

   CheckerResult CPPUT_API checkExprFalse( bool shouldBeFalse,
                                           const char *expressionCode );

   CheckerResult CPPUT_API checkAssertionFail( bool assertionFailed,
                                               const LazyMessage &message = LazyMessage::none );

   CheckerResult CPPUT_API checkAssertionPass( bool assertionFailed,
                                               const LazyMessage &message = LazyMessage::none );

   void
   makeComparisonFailedMessage( CheckerResult &result, 
                                const char *comparisonOperator,
                                const std::string &expected,
                                const std::string &actual );

   template<typename FirstType
           ,typename SecondType>
   CheckerResult checkCompare( const FirstType &actual,
                               const SecondType &expected,
                               Impl::LesserThan )
   {
      if ( actual < expected )
      {
         return CheckerResult();
      }
      CheckerResult result;
      makeComparisonFailedMessage( result, "<",
                                   stringize( expected ), 
                                   stringize( actual ) );
      return result;
   }

   template<typename FirstType
           ,typename SecondType>
   CheckerResult checkCompare( const FirstType &actual,
                               const SecondType &expected,
                               Impl::LesserOrEqual )
   {
      if ( actual <= expected )
      {
         return CheckerResult();
      }
      CheckerResult result;
      makeComparisonFailedMessage( result, "<=",
                                   stringize( expected ), 
                                   stringize( actual ) );
      return result;
   }

   template<typename FirstType
           ,typename SecondType>
   CheckerResult checkCompare( const FirstType &actual,
                               const SecondType &expected,
                               Impl::Equal )
   {
      if ( actual == expected )
      {
         return CheckerResult();
      }
      CheckerResult result;
      makeComparisonFailedMessage( result, "==",
                                   stringize( expected ), 
                                   stringize( actual ) );
      return result;
   }

   template<typename FirstType
           ,typename SecondType>
   CheckerResult checkCompare( const FirstType &actual,
                               const SecondType &expected,
                               Impl::NotEqual )
   {
      if ( actual != expected )
      {
         return CheckerResult();
      }
      CheckerResult result;
      makeComparisonFailedMessage( result, "!=",
                                   stringize( expected ), 
                                   stringize( actual ) );
      return result;
   }

   template<typename FirstType
           ,typename SecondType>
   CheckerResult checkCompare( const FirstType &actual,
                               const SecondType &expected,
                               Impl::GreaterOrEqual )
   {
      if ( actual >= expected )
      {
         return CheckerResult();
      }
      CheckerResult result;
      makeComparisonFailedMessage( result, ">=",
                                   stringize( expected ), 
                                   stringize( actual ) );
      return result;
   }

   template<typename FirstType
           ,typename SecondType>
   CheckerResult checkCompare( const FirstType &actual,
                               const SecondType &expected,
                               Impl::GreaterThan )
   {
      if ( actual > expected )
      {
         return CheckerResult();
      }
      CheckerResult result;
      makeComparisonFailedMessage( result, ">",
                                   stringize( expected ), 
                                   stringize( actual ) );
      return result;
   }

   // Notes: while the assertion has been removed, the predicate
   // is preserved to allow for easy predicate composition.
   // (which is used in testinfotest.cpp)
   // @todo blep the order of the expected/actual values does not match checkCompare order.
   template<typename FirstType
           ,typename SecondType>
   CheckerResult checkEquals( const FirstType &expected,
                              const SecondType &actual,
                              const char *message = "" )
   {
      // @todo blep do not discard message
      return checkCompare( actual, expected, Impl::Equal() );
   }


   CheckerResult CPPUT_API checkDoubleEquals( double expected, 
                                              double actual, 
                                              double tolerance, 
                                              const LazyMessage &message = LazyMessage::none );

   void CPPUT_API skipCurrentTest();

} // namespace CppUT


// basic assertions


/*! \brief Always fails and abort current test with the given message.
 * \ingroup group_assertions
 * \see CppUT::fail
 */
# define CPPUT_FAIL              \
   CPPUT_BEGIN_ASSERTION_MACRO() \
   ::CppUT::fail

/*! \brief Always fails with the given message, but continue current test.
 * \ingroup group_assertions
 * \see CppUT::fail
 */
# define CPPUT_CHECKING_FAIL    \
   CPPUT_BEGIN_CHECKING_MACRO() \
   ::CppUT::fail

/*! \brief Always succeed. Only used for succeed branch of a fail/pass assertion.
 * \ingroup group_assertions
 * This is required to increase the assertion count made in the test, in the case
 * policy such as 'test fails in no assertion are done.'
 * This is used for example in CPPUT_ASSERT_THROW if the expected exception
 * is caught, because such assertion can not be written as a simple predicate.
 */
# define CPPUT_PASS             \
   CPPUT_BEGIN_CHECKING_MACRO() \
   ::CppUT::pass


/*! \brief Asserts that an expression evaluate to true.
 * \ingroup group_assertions
 * \see CppUT::checkTrue
 */
# define CPPUT_ASSERT            \
   CPPUT_BEGIN_ASSERTION_MACRO() \
   ::CppUT::checkTrue

/*! \brief Checks that an expression evaluate to true.
 * \ingroup group_assertions
 * \see CppUT::checkTrue
 */
# define CPPUT_CHECK            \
   CPPUT_BEGIN_CHECKING_MACRO() \
   ::CppUT::checkTrue

/*! \brief Asserts that an expression evaluate to true.
 * \ingroup group_assertions
 * The expression source code is written in the assertion diagnostic in case of failure.
 * \see CppUT::checkTrue
 */
# define CPPUT_ASSERT_EXPR( expression )                 \
   CPPUT_BEGIN_ASSERTION_MACRO()                         \
   ::CppUT::checkExprTrue( (expression), #expression )

/*! \brief Checks that an expression evaluate to true.
 * \ingroup group_assertions
 * The expression source code is written in the assertion diagnostic in case of failure.
 * \see CppUT::checkTrue
 */
# define CPPUT_CHECK_EXPR( expression )                  \
   CPPUT_BEGIN_CHECKING_MACRO()                          \
   ::CppUT::checkExprTrue( (expression), #expression )

/*! \brief Asserts that an expression evaluate to false.
 * \ingroup group_assertions
 * \see CppUT::checkFalse
 */
# define CPPUT_ASSERT_FALSE      \
   CPPUT_BEGIN_ASSERTION_MACRO() \
   ::CppUT::checkFalse

/*! \brief Checks that an expression evaluate to false.
 * \ingroup group_assertions
 * \see CppUT::checkFalse
 */
# define CPPUT_CHECK_FALSE      \
   CPPUT_BEGIN_CHECKING_MACRO() \
   ::CppUT::checkFalse

/*! \brief Asserts that an expression evaluate to false.
 * \ingroup group_assertions
 * The expression source code is written in the assertion diagnostic in case of failure.
 * \see CppUT::checkFalse
 */
# define CPPUT_ASSERT_EXPR_FALSE( expression )              \
   CPPUT_BEGIN_ASSERTION_MACRO()                            \
   ::CppUT::checkExprFalse( (expression), #expression )

/*! \brief Checks that an expression evaluate to false.
 * \ingroup group_assertions
 * The expression source code is written in the assertion diagnostic in case of failure.
 * \see CppUT::checkFalse
 */
# define CPPUT_CHECK_EXPR_FALSE( expression )               \
   CPPUT_BEGIN_CHECKING_MACRO()                             \
   ::CppUT::checkExprFalse( (expression), #expression )

/*! \brief Asserts that two values are equal.
 * \ingroup group_assertions
 * @todo blep Removes this compatibility macro once it is no longer used by cpputtest.
 * - The values must be comparable using CppUT::equalityTest
 * - The values must be convertible to std::string using CppUT::stringize
 * \see CppUT::checkEquals
 */
# define CPPUT_ASSERT_EQUAL(expected, actual ) \
   CPPUT_ASSERT_COMPARE( (actual), ==, (expected) )

/*! Checks that the comparison evaluate to \c true: actual comparisonOperator expected.
 * \ingroup group_assertions
 * \param actual Actual value expression. Must be convertible to std::string using CppUT::stringize.
 * \param comparisonOperator One of: <, <=, ==, != >=, >
 * \param expected Expected value expression. Must be convertible to std::string using CppUT::stringize.
 * \see CppUT::checkCompare
 */
#define CPPUT_CHECK_COMPARE( actual, comparisonOperator, expected )              \
   CPPUT_BEGIN_CHECKING_MACRO()                                                  \
   ::CppUT::checkCompare( (actual), (expected),                                  \
                          _CPPUT_COMPARISON_OPERATOR_TYPE(comparisonOperator)()  \
                          )


/*! Asserts that the comparison evaluate to \c true: actual comparisonOperator expected.
 * \ingroup group_assertions
 * \param actual Actual value expression. Must be convertible to std::string using CppUT::stringize.
 * \param comparisonOperator One of: <, <=, ==, != >=, >
 * \param expected Expected value expression. Must be convertible to std::string using CppUT::stringize.
 * \see CppUT::checkCompare
 */
#define CPPUT_ASSERT_COMPARE( actual, comparisonOperator, expected )             \
   CPPUT_BEGIN_ASSERTION_MACRO()                                                 \
   ::CppUT::checkCompare( (actual), (expected),                                  \
                          _CPPUT_COMPARISON_OPERATOR_TYPE(comparisonOperator)()  \
                          )

/*! \brief Asserts that two double are equals given a tolerance
 * \ingroup group_assertions
 * \see ::CppUT::checkDoubleEquals
 */
# define CPPUT_ASSERT_DOUBLE_EQUAL \
   CPPUT_BEGIN_ASSERTION_MACRO()   \
   ::CppUT::checkDoubleEquals

/*! \brief Checks that two double are equals given a tolerance
 * \ingroup group_assertions
 * \see ::CppUT::checkDoubleEquals
 */
# define CPPUT_CHECK_DOUBLE_EQUAL  \
   CPPUT_BEGIN_CHECKING_MACRO()    \
   ::CppUT::checkDoubleEquals

/*! \internal
 * @todo catch other unexpected exception. Use generic get exception type handler.
 * Implementation notes: we need to call the failMacro outside the try/catch to
 * ensure the AbortingAssertionException is not caught by the catch clause.
 */
# define _CPPUT_ASSERT_THROW_IMPL( failMacro, expression, ExceptionType )      \
   try {                                                                   \
      bool noExceptionThrown = true;                                       \
      try {                                                                \
         expression;                                                       \
      } catch ( const ExceptionType & ) {                                  \
         CPPUT_PASS();                                                     \
         noExceptionThrown = false;                                        \
      }                                                                    \
      if ( noExceptionThrown )                                             \
         failMacro( "Expected exception: " #ExceptionType                  \
                    " not thrown." );                                      \
   } catch ( ... ) { /* catch like do while(false), but avoid warning */   \
      throw;                                                               \
   }


/*! \brief Asserts that an expression throw an exception of a specified type
 * \ingroup group_assertions
 */
# define CPPUT_ASSERT_THROW( expression, ExceptionType )                    \
   _CPPUT_ASSERT_THROW_IMPL( CPPUT_FAIL, expression, ExceptionType )

/*! \brief Checks that an expression throw an exception of a specified type
 * \ingroup group_assertions
 */
# define CPPUT_CHECK_THROW( expression, ExceptionType )                     \
   _CPPUT_CHECK_THROW_IMPL( CPPUT_CHECKING_FAIL, expression, ExceptionType )

/*! \internal
 * @todo replace with generic exception handler to get type & data.
 */
# define _CPPUT_ASSERT_NO_THROW_IMPL( failMacro, expression )          \
   try {                                                                   \
      expression;                                                          \
   } catch ( const std::exception &e ) {                                   \
      ::CppUT::Message message( "Unexpected exception caught" );           \
      message.add( "Type: " +                                              \
                   ::CppTL::getObjectTypeName( e, "std::exception" ) );    \
      message.add( std::string("What: ") + e.what() );                     \
      failMacro( message );                                                \
   } catch ( ... ) {                                                       \
      failMacro( "Unexpected exception caught" );                          \
   }


/*! \brief Asserts that an expression does not throw any exception
 * \ingroup group_assertions
 */
# define CPPUT_ASSERT_NO_THROW( expression )                               \
   _CPPUT_ASSERT_NO_THROW_IMPL( CPPUT_FAIL, expression )

/*! \brief Checks that an expression does not throw any exception
 * \ingroup group_assertions
 */
# define CPPUT_CHECK_NO_THROW( expression )                                \
   _CPPUT_ASSERT_NO_THROW_IMPL( CPPUT_CHECKING_FAIL, expression )


/*! \internal
 * Notes: implementing that assertion checking is a bit tricky since all the state
 * about the current test/assertions is stored in a "static" and
 * we don't want the tested assertion to polute the test result
 * of the current test.
 * TestInfo::ScopedContextOverride is used to create a temporary
 * context for the tested assertion. After the assertion, the
 * result of the assertion (did it failed), and the test result
 * properties are captured for later inspection.
 */
# define _CPPUT_ASSERT_ASSERTION_FAIL_MESSAGE_IMPL( beginMacro, assertion, message )               \
   {                                                                                               \
      bool assertionFailedCppUT_ = false;                                                          \
      {                                                                                            \
         ::CppUT::TestInfo::ScopedContextOverride contextSwitchCppUT_;                             \
         try {                                                                                     \
            assertion;                                                                             \
         } catch ( const ::CppUT::AbortingAssertionException & ) {                                 \
         }                                                                                         \
         assertionFailedCppUT_ = ::CppUT::TestInfo::threadInstance().testStatus().hasFailed();     \
      }                                                                                            \
      beginMacro() ::CppUT::checkAssertionFail( assertionFailedCppUT_, message );                  \
   }

/*! \brief Asserts that an assertion fails (for use to unit test custom assertion)
 * \ingroup group_assertions
 */
# define CPPUT_ASSERT_ASSERTION_FAIL_MESSAGE( assertion, message )            \
   _CPPUT_ASSERT_ASSERTION_FAIL_MESSAGE_IMPL( CPPUT_BEGIN_ASSERTION_MACRO, assertion, message )

/*! \brief Checks that an assertion fails (for use to unit test custom assertion)
 * \ingroup group_assertions
 */
# define CPPUT_CHECK_ASSERTION_FAIL_MESSAGE( assertion, message )            \
   _CPPUT_CHECK_ASSERTION_FAIL_MESSAGE_IMPL( CPPUT_BEGIN_CHECKING_MACRO, assertion, message )

/*! \brief Asserts that an assertion fails (for use to unit test custom assertion)
 * \ingroup group_assertions
 */
# define CPPUT_ASSERT_ASSERTION_FAIL( assertion )                     \
   CPPUT_ASSERT_ASSERTION_FAIL_MESSAGE( assertion, ::CppUT::Message() )

/*! \brief Checks that an assertion fails (for use to unit test custom assertion)
 * \ingroup group_assertions
 */
# define CPPUT_CHECK_ASSERTION_FAIL( assertion )                     \
   CPPUT_CHECK_ASSERTION_FAIL_MESSAGE( assertion, ::CppUT::Message() )

/*! \internal
 */
# define _CPPUT_ASSERT_ASSERTION_PASS_MESSAGE_IMPL( beginMacro, assertion, message )               \
   {                                                                                               \
      bool assertionFailedCppUT_ = false;                                                          \
      {                                                                                            \
         ::CppUT::TestInfo::ScopedContextOverride contextSwitchCppUT_;                             \
         try {                                                                                     \
            assertion;                                                                             \
         } catch ( const ::CppUT::AbortingAssertionException & ) {                                 \
         }                                                                                         \
         assertionFailedCppUT_ = ::CppUT::TestInfo::threadInstance().testStatus().hasFailed();     \
      }                                                                                            \
      beginMacro()  ::CppUT::checkAssertionPass( assertionFailedCppUT_, message );                 \
   }


/*! \brief Asserts that an assertion is successful (for use to unit test custom assertion)
 * \ingroup group_assertions
 */
# define CPPUT_ASSERT_ASSERTION_PASS_MESSAGE( assertion, message )            \
   _CPPUT_ASSERT_ASSERTION_PASS_MESSAGE_IMPL( CPPUT_BEGIN_ASSERTION_MACRO, assertion, message )


/*! \brief Checks that an assertion is successful (for use to unit test custom assertion)
 * \ingroup group_assertions
 */
# define CPPUT_CHECK_ASSERTION_PASS_MESSAGE( assertion, message )            \
   _CPPUT_CHECK_ASSERTION_PASS_MESSAGE_IMPL( CPPUT_BEGIN_CHECKING_MACRO, assertion, message )


/*! \brief Asserts that an assertion is successful (for use to unit test custom assertion)
 * \ingroup group_assertions
 */
# define CPPUT_ASSERT_ASSERTION_PASS( assertion )                   \
   CPPUT_ASSERT_ASSERTION_PASS_MESSAGE( assertion, ::CppUT::Message() )

/*! \brief Checks that an assertion is successful (for use to unit test custom assertion)
 * \ingroup group_assertions
 */
# define CPPUT_CHECK_ASSERTION_PASS( assertion )                    \
   CPPUT_CHECK_ASSERTION_PASS_MESSAGE( assertion, ::CppUT::Message() )

/*! \brief Skips the current test (test is aborted via thrown exception).
 * \ingroup group_assertions
 *
 * Cause the current test to be skipped. Its status will be 'TestStatus::skipped',
 * unless an assertion failed before.
 */
# define CPPUT_SKIP_TEST \
   ::CppUT::skipCurrentTest

/*! \brief Checks that an assertion fail, but ignore its failure
 * \ingroup group_assertions
 *
 * A typical usage of CPPUT_IGNORE_FAILURE is to ignore a failing assertion temporary.
 * Even if the assertion fails, it does not cause the test to fail, but increased
 * CppUT::TestStatus::ignoredFailureCount() instead.
 *
 * Notes that CPPUT_IGNORE_FAILURE also checks that the \c assertion fails (just like
 * CPPUT_CHECK_ASSERTION_FAIL) and will therefore cause the test to fail if the
 * assertion did not fail.
 *
 * \code
 * CPPUT_IGNORE_FAILURE(( CPPUT_ASSERT_EQUAL( 1, 2 ) ));
 * \endcode
 *
 * \warning Be sure to always use <b>double braces</b> around the macro parameter to avoid
 *          preprocessor mess and portability issues.
 */
# define CPPUT_IGNORE_FAILURE( assertion )                                                         \
   {                                                                                               \
      bool failedCppUT_;                                                                           \
      {                                                                                            \
         ::CppUT::TestInfo::IgnoreFailureScopedContextOverride contextSwitchCppUT_( failedCppUT_ );\
         try {                                                                                     \
            assertion;                                                                             \
         } catch ( const ::CppUT::AbortingAssertionException & ) {                                 \
         }                                                                                         \
      }                                                                                            \
      CPPUT_BEGIN_CHECKING_MACRO()                                                                 \
      ::CppUT::checkAssertionFail( failedCppUT_, #assertion );                                     \
   }

/*! \brief Asserts that the predicate is true.
 * @param predicate An expression of type \c bool or CppUT::CheckerResult.
 * The assertions succeed if \c predicate either evaluate to \c true or the CppUT::CheckerResult
 * status is CppUT::TestStatus::passed.
 *
 * The following code example show a boolean predicate:
 * \code
 * struct Point { int x_, y_; };
 * static bool checkSameX( const Point &a, const Point &b ) {
 *    return a.x_ == b.x_;
 * }
 * CPPUT_TEST_FUNCTION( testPointCoord1 ) {
 *    Points points[] = { {1,2}, {2,4} };
 *    CPPUT_ASSERT_PREDICATE( checkSameX( points[0], points[1] ) );
 * }
 * \endcode
 *
 * Notes that the boolean predicate does not provide any diagnostic information. This can
 * be done by writing it as follow:
 * \code
 * struct Point { int x_, y_; };
 * static CppUT::CheckerResult checkSameX( const Point &a, const Point &b ) {
 *    CppUT::CheckerResult result( a.x_ == b.x_ );
 *    result.diagnostic( "left.x_" ) = a.x_;
 *    result.diagnostic( "right.x_" ) = b.x_;
 *    return result;
 * }
 * CPPUT_TEST_FUNCTION( testPointCoord1 ) {
 *    Points points[] = { {1,2}, {2,4} };
 *    CPPUT_ASSERT_PREDICATE( checkSameX( points[0], points[1] ) );
 * }
 * \endcode
 */
# define CPPUT_ASSERT_PREDICATE( predicate )                            \
   CPPUT_BEGIN_ASSERTION_MACRO()                                        \
   CppUT::CheckerResult(predicate).setPredicateExpression( #predicate )

# define CPPUT_CHECK_PREDICATE( predicate )                             \
   CPPUT_BEGIN_CHECKING_MACRO()                                         \
   CppUT::CheckerResult(predicate).setPredicateExpression( #predicate )
   


#endif // CPPUT_ASSERT_H_INCLUDED

