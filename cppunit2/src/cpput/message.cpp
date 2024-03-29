#include <cpput/message.h>
#include <string>

namespace CppUT {

// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// class Message
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////

Message Message::none;

Message::Message()
{
}

Message::Message( const CppTL::StringBuffer &shortDescription )
{
   add( shortDescription );
}

Message::Message( const CppTL::StringConcatenator &shortDescription )
{
   add( shortDescription );
}

Message::Message( const CppTL::ConstString &shortDescription )
{
   add( shortDescription );
}

Message::Message( const std::string &shortDescription )
{
   add( shortDescription );
}

Message::Message( const char *shortDescription )
{
   add( shortDescription );
}

bool 
Message::empty() const
{
   return details_.empty();
}

void 
Message::insertAt( int index, 
                   const char *detail )
{
   insertAt( index, std::string(detail) );
}

void 
Message::insertAt( int index, 
                   const std::string &detail )
{
   details_.insert( details_.begin() + index, detail );
}

void 
Message::insertAt( int index,
                   const Message &other )
{
   details_.insert( details_.begin() + index, 
                     other.details_.begin(),
                     other.details_.end() );
}

void 
Message::add( const char *detail )   // needed to solve ambiguity due to implicit constructor
{
   add( CppTL::ConstString(detail) );
}

void 
Message::add( const std::string &detail )
{
   details_.push_back( detail );
}

void 
Message::add( const CppTL::ConstString &detail )
{
   details_.push_back( detail );
}

void 
Message::extend( const Message &other )
{
   insertAt( int(details_.size()), other );
}

int 
Message::count() const
{
   return int(details_.size());
}

std::string 
Message::at( int index ) const
{
   return CPPTL_AT( details_, index ).str();
}

std::string 
Message::toString() const
{
   CppTL::StringBuffer message;
   Details::const_iterator it = details_.begin();
   Details::const_iterator itEnd = details_.end();

   while ( it != itEnd )
   {
      message += *it++;
      message += "\n";

      if ( it != itEnd )
         message += "- ";
   }

   return message.c_str();
}



// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// class LazyMessage
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////

LazyMessage LazyMessage::none;

LazyMessage::LazyMessage()
   : kind_( defaultConstructor )
{
}

LazyMessage::LazyMessage( const CppTL::StringBuffer &shortDescription )
   : kind_( buffer )
{
   init_.stringBuffer_ = &shortDescription;
}

LazyMessage::LazyMessage( const CppTL::StringConcatenator &shortDescription )
   : kind_( concatenator )
{
   init_.concatenor_ = &shortDescription;
}

LazyMessage::LazyMessage( const CppTL::ConstString &shortDescription )
   : kind_( constString )
{
   init_.constString_ = &shortDescription;
}

LazyMessage::LazyMessage( const std::string &shortDescription )
   : kind_( stdString )
{
   init_.stdString_ = &shortDescription;
}

LazyMessage::LazyMessage( const char *shortDescription )
   : kind_( constChar )
{
   init_.cz_ = shortDescription;
}

LazyMessage::LazyMessage( const Message &aMessage )
   : kind_( message )
{
   init_.message_ = &aMessage;
}


Message 
LazyMessage::makeMessage() const
{
   switch ( kind_ )
   {
   default:
      CPPTL_DEBUG_ASSERT_UNREACHABLE;
   case defaultConstructor:
      return Message();
   case constChar:
      return Message(init_.cz_);
   case stdString:
      return Message(*(init_.stdString_));
   case constString:
      return Message(*(init_.constString_));
   case concatenator:
      return Message(*(init_.concatenor_));
   case buffer:
      return Message(*(init_.stringBuffer_));
   case message:
      return Message(*(init_.message_));
   }
}


} // namespace CppUT
