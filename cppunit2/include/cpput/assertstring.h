#ifndef CPPUT_ASSERTSTRING_H_INCLUDED
# define CPPUT_ASSERTSTRING_H_INCLUDED

# include <cpput/assertcommon.h>

namespace CppUT {

   CheckerResult CPPUT_API checkStdStringStartsWith( const std::string &string,
                                                     const std::string &pattern,
                                                     const Message &message );

   CheckerResult CPPUT_API checkStdStringEndsWith( const std::string &string,
                                                   const std::string &pattern,
                                                   const Message &message );

   CheckerResult CPPUT_API checkStdStringContains( const std::string &string,
                                                   const std::string &pattern,
                                                   const Message &message );

   CheckerResult CPPUT_API checkStdStringEquals( const std::string &expected,
                                                 const std::string &actual,
                                                 const Message &message );

   template<typename String1
           ,typename String2>
   CheckerResult checkStringStartsWith( const String1 &string,
                                        const String2 &pattern,
                                        const Message &message = Message() )
   {
      return checkStdStringStartsWith( convertToString( string ),
                                       convertToString( pattern ),
                                       message );
   }

   template<typename String1
           ,typename String2>
   CheckerResult checkStringEndsWith( const String1 &string,
                                      const String2 &pattern,
                                      const Message &message = Message() )
   {
      return checkStdStringEndsWith( convertToString( string ),
                                     convertToString( pattern ),
                                     message );
   }

   template<typename String1
           ,typename String2>
   CheckerResult checkStringContains( const String1 &string,
                                      const String2 &pattern,
                                      const Message &message = Message() )
   {
      return checkStdStringContains( convertToString( string ),
                                     convertToString( pattern ),
                                     message );
   }


   template<typename String1
           ,typename String2>
   CheckerResult checkStringEquals( const String1 &expected,
                                    const String2 &actual,
                                    const Message &message = Message() )
   {
      return checkStdStringEquals( convertToString( expected ),
                                   convertToString( actual ),
                                   message );
   }


// string assertions

/*! \brief Asserts that a string starts with the specified character sequence.
 * \ingroup group_assertions
 */
# define CPPUT_ASSERTSTR_START   \
   CPPUT_BEGIN_ASSERTION_MACRO() \
   ::CppUT::checkStringStartsWith

/*! \brief Asserts that a string ends with the specified character sequence.
 * \ingroup group_assertions
 */
# define CPPUT_ASSERTSTR_END     \
   CPPUT_BEGIN_ASSERTION_MACRO() \
   ::CppUT::checkStringEndsWith

/*! \brief Asserts that a string contains the specified character sequence.
 * \ingroup group_assertions
 */
# define CPPUT_ASSERTSTR_CONTAIN \
   CPPUT_BEGIN_ASSERTION_MACRO() \
   ::CppUT::checkStringContains

/*! \brief Asserts that two string are identical.
 * \ingroup group_assertions
 *
 * Useful to compare \c const \c char \c * or string of different C++ types
 * (MFC \c CString and \c std::string for example).
 */
# define CPPUT_ASSERTSTR_EQUAL   \
   CPPUT_BEGIN_ASSERTION_MACRO() \
   ::CppUT::checkStringEquals



// string checks

/*! \brief Asserts that a string starts with the specified character sequence.
 * \ingroup group_assertions
 */
# define CPPUT_CHECKSTR_START   \
   CPPUT_BEGIN_CHECKING_MACRO() \
   ::CppUT::checkStringStartsWith

/*! \brief Asserts that a string ends with the specified character sequence.
 * \ingroup group_assertions
 */
# define CPPUT_CHECKSTR_END     \
   CPPUT_BEGIN_CHECKING_MACRO() \
   ::CppUT::checkStringEndsWith

/*! \brief Asserts that a string contains the specified character sequence.
 * \ingroup group_assertions
 */
# define CPPUT_CHECKSTR_CONTAIN \
   CPPUT_BEGIN_CHECKING_MACRO() \
   ::CppUT::checkStringContains

/*! \brief Asserts that two string are identical.
 * \ingroup group_assertions
 *
 * Useful to compare \c const \c char \c * or string of different C++ types
 * (MFC \c CString and \c std::string for example).
 */
# define CPPUT_CHECKSTR_EQUAL   \
   CPPUT_BEGIN_CHECKING_MACRO() \
   ::CppUT::checkStringEquals



} // namespace CppUT


#endif // CPPUT_ASSERTSTRING_H_INCLUDED
