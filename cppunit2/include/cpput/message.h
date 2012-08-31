#ifndef CPPUT_MESSAGE_H_INCLUDED
# define CPPUT_MESSAGE_H_INCLUDED

# include <cpput/forwards.h>
# include <cpptl/conststring.h>
# include <vector>


namespace CppUT {

/*! \brief Carry informative message about a failed assertion.
 * \ingroup group_assertions
 */
class CPPUT_API Message
{
public:
   static Message none;

   Message();

   Message( const CppTL::StringBuffer &shortDescription );

   Message( const CppTL::StringConcatenator &shortDescription );

   Message( const CppTL::ConstString &shortDescription );

   Message( const std::string &shortDescription );

   Message( const char *shortDescription );

   bool empty() const;

   void insertAt( int index, 
                  const char *detail );

   void insertAt( int index, 
                  const std::string &detail );

   void insertAt( int index,
                  const Message &other );

   void add( const char *detail );   // needed to solve ambiguity due to implicit constructor

   void add( const std::string &detail );

   void add( const CppTL::ConstString &detail );

   void extend( const Message &other );

   int count() const;

   std::string at( int index ) const;

   std::string toString() const;

private:
   typedef std::vector<CppTL::ConstString> Details;
   Details details_;
};


class CPPUT_API LazyMessage
{
public:
   static LazyMessage none;

   LazyMessage();

   LazyMessage( const CppTL::StringBuffer &shortDescription );

   LazyMessage( const CppTL::StringConcatenator &shortDescription );

   LazyMessage( const CppTL::ConstString &shortDescription );

   LazyMessage( const std::string &shortDescription );

   LazyMessage( const char *shortDescription );

   LazyMessage( const Message &message );

   Message makeMessage() const;

private:
   union {
      const char *cz_;
      const std::string *stdString_;
      const CppTL::ConstString *constString_;
      const CppTL::StringConcatenator *concatenor_;
      const CppTL::StringBuffer *stringBuffer_;
      const Message *message_;
   } init_;
   enum Kind {
      defaultConstructor = 1,
      constChar,
      stdString,
      constString,
      concatenator,
      buffer,
      message
   };
   Kind kind_;
};

} // namespace CppUT


#endif // CPPUT_MESSAGE_H_INCLUDED

