#ifndef CPPUT_TESTINFO_H_INCLUDED
# define CPPUT_TESTINFO_H_INCLUDED

# include <cpput/forwards.h>
# include <cpput/message.h>
# include <cpptl/intrusiveptr.h>
# include <json/value.h>
# include <stdexcept>

namespace CppUT {


   /*! \brief Exception thrown when an aborting assertion fails.
    * @todo It would probably be better for those exception not to subclass
    *       std::exception (avoid this exception beeing caught by the test).
    */
   class CPPUT_API AbortingAssertionException : public std::runtime_error
   {
   public:
      AbortingAssertionException( const std::string &message )
         : std::runtime_error( "AbortingAssertionException:\n" + message )
      {
      }

      virtual ~AbortingAssertionException() CPPTL_NO_THROW
      {
      }
   };


   /*! \brief Exception thrown to skip the current test.
    * @todo It would probably be better for those exception not to subclass
    *       std::exception (avoid this exception beeing caught by the test).
    */
   class CPPUT_API SkipTestException : public std::runtime_error
   {
   public:
      SkipTestException()
         : std::runtime_error( "SkipTestException" )
      {
      }

      virtual ~SkipTestException() throw()
      {
      }
   };


   /*! \brief Represents the location of an assertion in the source code.
    */
   class SourceLocation
   {
   public:
      SourceLocation( const char *file = 0,
                      unsigned int line =0 )
         : file_( file )
         , line_( line )
      {
      }

      void clear()
      {
         file_ = 0;
         line_ = 0;
      }

      bool isValid() const
      {
         return file_ != 0;
      }

      const char *file_;
      unsigned int line_;
   };

   /*! \brief Represents a failed assertion.
    * \ingroup group_custom_assertions
    */
   class CPPUT_API Assertion
   {
   public:
      enum Kind
      {
         fault = 1,
         assertion = 2
      };

      Assertion( Kind kind = assertion,
                 const SourceLocation &sourceLocation = SourceLocation() );

      void setLocation( const SourceLocation &location );
      const SourceLocation &location() const;

      void setKind( Kind kind_ );
      Kind kind() const;

      void setDetail( const CheckerResult &detail );
      const Json::Value &detail() const;
      Json::Value &detail();

      void setIgnoredFailure();
      bool isIgnoredFailure() const;

      std::string toString() const;

   private:
      Json::Value detail_;
      SourceLocation location_;
      Kind kind_;
      bool isIgnoredFailure_;
   };


   /*! \brief Represents the status of the current test.
    * \ingroup group_custom_assertions
    */
   class CPPUT_API TestStatus
   {
   public:
      enum Status
      {
         passed = 1,
         skipped,
         failed
      };

      TestStatus( Status status = passed );

      void setStatus( Status status );

      Status status() const;

      bool hasFailed() const;
      bool hasPassed() const;
      bool wasSkipped() const;

      void setStatistics( const std::string &name,
                          const Json::Value &value );
      //Json::Value getStatistics( const std::string &name );

      void addSpecific( const std::string &type,
                        const Json::Value &value );

      int assertionCount() const;
      int failedAssertionCount() const;
      int ignoredFailureCount() const;

      void increaseAssertionCount( int delta = 1 );
      void increaseFailedAssertionCount( int delta = 1 );
      void increaseIgnoredFailureCount( int delta = 1 );
   private:
      Json::Value statistics_;
      Json::Value specifics_;
      Status status_;
      unsigned int assertionCount_;
      unsigned int failedAssertionCount_;
      unsigned int ignoredFailureCount_;
   };


   /*! \brief Represents the result of an assertion predicate.
    * All checker functions used in assert and check macro should returns an object of this type.
    *
    * The result of an assertion predicate is made of:
    * - Predicate definition: its name and parameter values
    * - Diagnostic information: a list of properties with values
    *
    * The name of the predicate can be set using setName(). 
    * Predicate definition can be set using result.predicate("expected") = value;
    * Diagnostic information can be set using result.diagnostic("actual") = value;
    *
    * The order of predicate and diagnostic information are provided is stored.
    * The properties are displayed in the same order in the output assertion message.
    */
   class CheckerResult
   {
      friend class Assertion;
   public:
      /// Constructs a result with TestStatus::Status passed.
      CheckerResult();

      /*! \brief Constructs a result from a bool predicate value.
      /* @param succeed If \c true then the status is set to passed, otherwise the
       *        status is set to failed.
       */
      CheckerResult( bool succeeded );

      void setFailed();

      void appendMessages( const CppUT::Message &message );
      void appendMessage( const std::string &message );

      void compose( const std::string &name, 
                    const CheckerResult &other );

      /// Sets the "name" of the predicate being tested
      /// Example: "actual == expected"
      void setName( const std::string &name );

      /// Sets a property value of the predicate
      Json::Value &predicate( const char *name );

      /// Sets a property value of the failure diagnostic
      Json::Value &diagnostic( const char *name );

      /*! \brief Sets the predicate call expression code.
       * Notes: this is intended to be used in assertion macro.
       * The expression code is added to a new CheckerResult object that is returned.
       * \see CPPUT_ASSERT_PREDICATE implementation for an example of usage.
       */
      CheckerResult setPredicateExpression( const char *expression ) const;

      TestStatus::Status status_;
   private:
      // JSON object containing "data", "composite" and "name"
      Json::Value result_;
   };
   
   namespace Impl {

      /// Implementation detail of CPPUT_BEGIN_ASSERTION_MACRO() & CPPUT_BEGIN_CHECKING_MACRO().
      class CheckerLineLocation
      {
      public:
         const CheckerResult *result_;
         unsigned long line_;
      };

      struct FileLocationMarker {};


      /// Implementation detail of CPPUT_BEGIN_ASSERTION_MACRO() & CPPUT_BEGIN_CHECKING_MACRO().
      class CheckerFileLocation
      {
      public:
         const CheckerLineLocation *lineData_;
         const char *file_;
      };

      /// Implementation detail of CPPUT_BEGIN_ASSERTION_MACRO() & CPPUT_BEGIN_CHECKING_MACRO().
      class AssertionTrigger
      {
      public:
         static AssertionTrigger aborting;
         static AssertionTrigger checking;

         AssertionTrigger &operator +=( const CheckerFileLocation &fileLocation );
      };

   } // end of namespace Impl


   /*! Starts an aborting assertion macro (throw an exception on assertion failure).
    * \ingroup group_custom_assertions
    * This macro provides the following properties for assertion:
    * - condition is evaluated first to ease debugging of unit tests
    * - assertion can take a variable number of parameters (typicaly an optional message).
    *
    * Typically, as little code as possible is put in assertion macro code to ease debugging.
    * Usually, an assertion macro just start a function call:
    * \code
    * #define CPPUT_ASSERT_IS_EVEN \
    *    CPPUT_BEGIN_ASSERTION_MACRO() checkIsEven
    * #define CPPUT_ASSERT_IS_EVEN \
    *    CPPUT_BEGIN_CHECKING_MACRO() checkIsEven
    *
    * CheckerResult checkIsEven( int x, const char *message = "" )
    * {
    *    CheckerResult result;
    *    if ( x % 2 != 0 )
    *    {
    *       result.setFailed();
    *       result.message_.add( message );
    *    }
    *    return result;
    * }
    * void someTest() 
    * {
    *    CPPUT_CHECK_IS_EVEN( 1 );
    *    CPPUT_CHECK_IS_EVEN( 2 );
    * }
    * \endcode
    *
    * In the example above, both a checking and asserting macro have been implemented. The code to
    * check the assertion is factorized in a single function than can take a variable number of parameters
    * and be easily debugged.
    *
    * Implementation notes:
    *
    * This is achieved using a trick based on operator evaluation order described below:
    * Basicaly an assertion has the following form:
    * AssertionTrigger::aborting += __FILE__ + __LINE__ / checkCondition( condition expression );
    * For simplicity, we will use the following compact form for reference :
    * x += f + l / checkCond( condition ); 
    * checkCond must returns an CheckerResult which as overloaded operator /.
    * operator += evaluate right to left
    * operator + evaluate left to right
    * operator / evaluate left to right and as priority over +.
    * So 'condition expression' is evaluted first, then checkCond(condition) is evaluated. 
    * Then 'l / checkCond(condition)' is evaluated,
    * followed by f + l/ checkCond(condition) which yield to an CheckerFileLocation. Finally, 
    * x += f + l / checkCond( condition ) is evaluated, basically calling AssertionTrigger 
    * overloaded operator += with the final CheckerFileLocation that contains details about the assertion, 
    * as well as file/line location.
    * ms-help://MS.VSCC.v90/MS.MSDNQTR.v90.en/dv_vclang/html/95c1f0ba-dad8-4034-b039-f79a904f112f.htm
    * \see CPPUT_BEGIN_CHECKING_MACRO.
    */
   # define CPPUT_BEGIN_ASSERTION_MACRO() \
      CppUT::Impl::AssertionTrigger::aborting += (const CppUT::Impl::FileLocationMarker *)__FILE__ + __LINE__ / 

   /*! Starts a checking assertion macro (continue test execution even on failure).
    * \ingroup group_custom_assertions
    * \see CPPUT_BEGIN_ASSERTION_MACRO() for example and implementation notes.
    */
   # define CPPUT_BEGIN_CHECKING_MACRO() \
      CppUT::Impl::AssertionTrigger::checking += (const CppUT::Impl::FileLocationMarker *)__FILE__ + __LINE__ / 



   /*! \brief Provides notification of failed assertions and log events.
    */
   class CPPUT_API TestResultUpdater : public CppTL::IntrusiveCount
   {
   public:
      virtual ~TestResultUpdater()
      {
      }

      virtual void addResultLog( const Json::Value &log ) = 0;

      virtual void addResultAssertion( const Assertion &assertion ) = 0;
   };

   /*! \brief Indiciates if on failure the assertion it will abort the current test.
    * \ingroup group_custom_assertions
    */
   enum AssertionType
   {
      abortingAssertion = 1,   ///< The assertion will abort the test on failure
      checkingAssertion        ///< The test will continue on assertion failure
   };

   /*! \brief Indicates if AbortingAssertionException contains detail about the failed assertion.
    * Used by CppUnit 2 to test itself without relying on TestResultUpdater.
    */
   enum AbortingAssertionMode
   {
      fastAbortingAssertion = 1,
      richAbortingAssertion
   };

   /*! \brief Heart of the test system: hold current test status and the state of the current assertion.
    * \ingroup group_custom_assertions
    * \todo find a away to integrate context with multiple thread.
    */
   class TestInfo : public CppTL::IntrusiveCount
      // @todo inherit CppTL::NonCopyable
   {
   public:
      class CPPUT_API ScopedContextOverride
      {
      public:
         ScopedContextOverride();

         ~ScopedContextOverride();

      protected:
         TestInfoPtr context_;
      };

      class CPPUT_API IgnoreFailureScopedContextOverride : public ScopedContextOverride
                                                         , private TestResultUpdater
      {
      public:
         IgnoreFailureScopedContextOverride( bool &assertionFailed );

         ~IgnoreFailureScopedContextOverride();

      private: // overridden from TestResultUpdater
         virtual void addResultLog( const Json::Value &log );

         virtual void addResultAssertion( const Assertion &assertion );

      private:
         Assertion *assertion_;
         bool &assertionFailed_;
      };

      friend class IgnoreFailureScopedContextOverride;


      /*! Returns the TestInfo instance for the current thread.
       * Notes: one TestInfo instance is created per thread to guaranty
       * thread-safety.
       */
      static TestInfo &threadInstance();

      TestInfo();

      void setTestResultUpdater( TestResultUpdater &updater );
      void removeTestResultUpdater();

      /*! \brief Must be called at the beginning of each test case.
       * Set test status to 'passed', and assertion type to abortingAssertion.
       */
      void startNewTest();

      TestStatus &testStatus();

      void handleAssertion( const char *file, 
                            unsigned int line, 
                            const CheckerResult &result,
                            bool isAbortingAssertion );

      void handleUnexpectedException( const CheckerResult &detail );

      void setAbortingAssertionMode( AbortingAssertionMode mode );

      void log( const Json::Value &log );

   private:
      TestStatus testStatus_;
      Assertion currentAssertion_;
      AssertionType assertionType_;
      AbortingAssertionMode abortingAssertionMode_;
      TestResultUpdater *updater_;
   };

   /*! \brief Log an event.
    * \sa TestInfo
    */
   void CPPUT_API log( const Json::Value &log );

   /*! \brief Log an event.
    * \sa TestInfo
    */
   void CPPUT_API log( const std::string &log );

   /*! \brief Log an event.
    * \sa TestInfo
    */
   void CPPUT_API log( const char *log );

   /*! \brief Log an event.
    * \sa TestInfo
    */
   void CPPUT_API log( const CppTL::ConstString &log );

} // namespace CppUT


inline CppUT::Impl::CheckerLineLocation operator /( unsigned long line, const CppUT::CheckerResult &result )
{
   CppUT::Impl::CheckerLineLocation lineData;
   lineData.result_ = &result;
   lineData.line_ = line;
   return lineData;
}


inline CppUT::Impl::CheckerFileLocation operator +( const CppUT::Impl::FileLocationMarker *file, const CppUT::Impl::CheckerLineLocation &lineData )
{
   CppUT::Impl::CheckerFileLocation fileData;
   fileData.file_ = (const char *)file;
   fileData.lineData_ = &lineData;
   return fileData;
}



#endif // CPPUT_TESTINFO_H_INCLUDED
