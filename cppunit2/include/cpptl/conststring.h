#ifndef CPPTL_CONSTSTRING_H_INCLUDED
# define CPPTL_CONSTSTRING_H_INCLUDED

# include <cpptl/forwards.h>
# include <string.h>
# ifndef CPPTL_CONSTSTRING_NO_STDSTRING
#  include <string> // operator << & >>
#  include <iostream> // operator << & >>
# endif

/// @todo optimize case where length == 0 in ConstString constructor.

namespace CppTL {
   class ConstCharView
   {
   public:
      typedef unsigned int size_type;
      typedef char value_type;

      ConstCharView()
         : begin_( "" )
         , end_( begin_ )
      {
      }

      ConstCharView( const value_type *czstr )
         : begin_( czstr )
      {
         CPPTL_ASSERT_MESSAGE( czstr != 0, "Invalid zero terminated string" );
         end_ = begin_ + strlen( begin_ );
      }

      ConstCharView( const value_type *cbegin, size_type length )
         : begin_( cbegin )
         , end_( cbegin + length )
      {
      }

      ConstCharView( const value_type *cbegin, const value_type *cend )
         : begin_( cbegin )
         , end_( cend )
      {
      }

      size_type length() const
      {
         return size_type(end_ - begin_);
      }

      size_type size() const
      {
         return length();
      }

      const value_type *c_str() const
      {
         return begin_;
      }

      const value_type *begin() const
      {
         return begin_;
      }

      const value_type *end() const
      {
         return end_;
      }

   private:
      const value_type *begin_;
      const value_type *end_;
   };


   class StringConcatenator
   {
   public:
      typedef unsigned int size_type;
      typedef char value_type;

   private:
      class SubString
      {
      public:
         SubString( const value_type *csz );
         SubString( const StringConcatenator &concatenator );
         SubString( const ConstString &string );
         SubString( const StringBuffer &buffer );
         SubString( const ConstCharView &constCharView );
# ifndef CPPTL_CONSTSTRING_NO_STDSTRING
         SubString( const std::string &string );
# endif
         size_type length() const;

         void copyToBuffer( value_type *buffer) const;

      private:
         enum Kind
         {
            concatenator = 0,
            cszString,
            string,
            stringBuffer,
            constCharView,
            stdString
         } kind_;
         union
         {
            const char *csz_;
            const StringConcatenator *concatenator_;
            const ConstString *string_;
            const StringBuffer *buffer_;
            const ConstCharView *constCharView_;
# ifndef CPPTL_CONSTSTRING_NO_STDSTRING
            const std::string *stdString_;
# endif
         };
         mutable size_type length_;
      };

   public:
      StringConcatenator( const SubString &left,
                          const SubString &right );

      size_type length() const;

      void copyToBuffer( value_type *buffer ) const;

   private:
      SubString left_;
      SubString right_;
   };


   class ConstStringIterator
   {
   public:
      typedef char value_type;
      typedef unsigned int size_type;

      ConstStringIterator()
         : current_( 0 )
      {
      }

      ConstStringIterator( const value_type *current )
         : current_( current )
      {
      }

      char operator *() const
      {
         return *current_;
      }

      char operator[]( size_type index ) const
      {
         return current_[index];
      }

      ConstStringIterator &operator ++()
      {
         ++current_;
         return *this;
      }

      ConstStringIterator operator ++(int)
      {
         ConstStringIterator it( *this );
         ++current_;
         return it;
      }

      ConstStringIterator &operator --()
      {
         --current_;
         return *this;
      }

      ConstStringIterator operator --(int)
      {
         ConstStringIterator it( *this );
         --current_;
         return it;
      }

      bool operator ==( const ConstStringIterator &other ) const
      {
         return current_ == other.current_;
      }

      bool operator !=( const ConstStringIterator &other ) const
      {
         return current_ != other.current_;
      }

      bool operator <( const ConstStringIterator &other ) const
      {
         return current_ < other.current_;
      }

      bool operator <=( const ConstStringIterator &other ) const
      {
         return current_ <= other.current_;
      }

      bool operator >( const ConstStringIterator &other ) const
      {
         return current_ > other.current_;
      }

      bool operator >=( const ConstStringIterator &other ) const
      {
         return current_ >= other.current_;
      }

   private:
      const value_type *current_;
   };


   class ConstString
   {
   public:
      typedef unsigned int size_type;
      typedef char value_type;
      typedef ConstStringIterator const_iterator;

      ConstString();
      ConstString( const value_type *csz );
      ConstString( const value_type *begin, const value_type *end );
      ConstString( const value_type *begin, size_type length );
      ConstString( const StringConcatenator &concatenator );
      ConstString( const StringBuffer &buffer );
      ConstString( const ConstCharView &view );
# ifndef CPPTL_CONSTSTRING_NO_STDSTRING
      ConstString( const std::string &string );
# endif
      ConstString( const ConstString &other );

      ~ConstString();

      ConstString &operator =( const ConstString &other );
      ConstString &operator =( const char *other );
      ConstString &operator =( const StringConcatenator &concatenator );
      ConstString &operator =( const StringBuffer &buffer );
      ConstString &operator =( const ConstCharView &view );
# ifndef CPPTL_CONSTSTRING_NO_STDSTRING
      ConstString &operator =( const std::string &other );
# endif
      void swap( ConstString &other );

      ConstString &operator +=( const ConstString &other );
      ConstString &operator +=( const char *other );
      ConstString &operator +=( const StringConcatenator &concatenator );
      ConstString &operator +=( const StringBuffer &buffer );
      ConstString &operator +=( const ConstCharView &view );
# ifndef CPPTL_CONSTSTRING_NO_STDSTRING
      ConstString &operator +=( const std::string &string );
# endif

      const value_type *c_str() const;
      const value_type *end_c_str() const;
# ifndef CPPTL_CONSTSTRING_NO_STDSTRING
      std::string str() const;
# endif

      ConstStringIterator begin() const;
      ConstStringIterator end() const;

      size_type length() const;
      size_type size() const;
      bool empty() const;

      value_type operator[]( size_type index ) const;

      ConstString substr( size_type index, 
                          size_type length = size_type(-1) ) const;

   private:
      size_type length_;
      char *buffer_;
   };

   bool operator ==( const ConstString &left, const char *other );
   bool operator !=( const ConstString &left, const char *other );
   bool operator <( const ConstString &left, const char *other );
   bool operator <=( const ConstString &left, const char *other );
   bool operator >=( const ConstString &left, const char *other );
   bool operator >( const ConstString &left, const char *other );

   bool operator ==( const ConstString &left, const ConstString &other );
   bool operator !=( const ConstString &left, const ConstString &other );
   bool operator <( const ConstString &left, const ConstString &other );
   bool operator <=( const ConstString &left, const ConstString &other );
   bool operator >=( const ConstString &left, const ConstString &other );
   bool operator >( const ConstString &left, const ConstString &other );

# ifndef CPPTL_CONSTSTRING_NO_STDSTRING
   std::ostream &operator <<( std::ostream &os, const ConstString &str );
   std::istream &operator >>( std::istream &os, ConstString &str );
# endif

   class StringBuffer
   {
   public:
      typedef unsigned int size_type;
      typedef char value_type;

      StringBuffer( unsigned int capacity = 127 );
      StringBuffer( const StringBuffer &other );
      ~StringBuffer();

      StringBuffer &operator =( const StringBuffer &other );
      void swap( StringBuffer &other );

      void reserve( size_type newCapacity );
      size_type capacity() const;

      size_type length() const;
      void truncate( size_type length );

      StringBuffer &operator +=( const StringConcatenator &other );
      StringBuffer &operator +=( const ConstString &other );
      StringBuffer &operator +=( const char *other );
      StringBuffer &operator +=( const ConstCharView &other );
# ifndef CPPTL_CONSTSTRING_NO_STDSTRING
      StringBuffer &operator +=( const std::string &other );
# endif

      char &operator[]( size_type index );
      char operator[]( size_type index ) const;
      const char *c_str() const;

      ConstString substr( size_type index, 
                          size_type length = size_type(-1) ) const;

   private:
      void prepareBuffer( size_type length );

      char *buffer_;
      size_type length_;
      size_type capacity_;
   };

} // namespace CppTL



// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// operator +, in global namespace... 
// implementation generated by conststring.py
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////

inline CppTL::StringConcatenator 
operator +( const char *left, const CppTL::ConstString &right ) {
   return CppTL::StringConcatenator( left, right );
}

inline CppTL::StringConcatenator 
operator +( const char *left, const CppTL::StringConcatenator &right ) {
   return CppTL::StringConcatenator( left, right );
}

inline CppTL::StringConcatenator 
operator +( const char *left, const CppTL::StringBuffer &right ) {
   return CppTL::StringConcatenator( left, right );
}

inline CppTL::StringConcatenator 
operator +( const char *left, const CppTL::ConstCharView &right ) {
   return CppTL::StringConcatenator( left, right );
}


inline CppTL::StringConcatenator 
operator +( const CppTL::ConstString &left, const char *right ) {
   return CppTL::StringConcatenator( left, right );
}

inline CppTL::StringConcatenator 
operator +( const CppTL::ConstString &left, const CppTL::ConstString &right ) {
   return CppTL::StringConcatenator( left, right );
}

inline CppTL::StringConcatenator 
operator +( const CppTL::ConstString &left, const CppTL::StringConcatenator &right ) {
   return CppTL::StringConcatenator( left, right );
}

inline CppTL::StringConcatenator 
operator +( const CppTL::ConstString &left, const CppTL::StringBuffer &right ) {
   return CppTL::StringConcatenator( left, right );
}

inline CppTL::StringConcatenator 
operator +( const CppTL::ConstString &left, const CppTL::ConstCharView &right ) {
   return CppTL::StringConcatenator( left, right );
}


inline CppTL::StringConcatenator 
operator +( const CppTL::StringConcatenator &left, const char *right ) {
   return CppTL::StringConcatenator( left, right );
}

inline CppTL::StringConcatenator 
operator +( const CppTL::StringConcatenator &left, const CppTL::ConstString &right ) {
   return CppTL::StringConcatenator( left, right );
}

inline CppTL::StringConcatenator 
operator +( const CppTL::StringConcatenator &left, const CppTL::StringConcatenator &right ) {
   return CppTL::StringConcatenator( left, right );
}

inline CppTL::StringConcatenator 
operator +( const CppTL::StringConcatenator &left, const CppTL::StringBuffer &right ) {
   return CppTL::StringConcatenator( left, right );
}

inline CppTL::StringConcatenator 
operator +( const CppTL::StringConcatenator &left, const CppTL::ConstCharView &right ) {
   return CppTL::StringConcatenator( left, right );
}


inline CppTL::StringConcatenator 
operator +( const CppTL::StringBuffer &left, const char *right ) {
   return CppTL::StringConcatenator( left, right );
}

inline CppTL::StringConcatenator 
operator +( const CppTL::StringBuffer &left, const CppTL::ConstString &right ) {
   return CppTL::StringConcatenator( left, right );
}

inline CppTL::StringConcatenator 
operator +( const CppTL::StringBuffer &left, const CppTL::StringConcatenator &right ) {
   return CppTL::StringConcatenator( left, right );
}

inline CppTL::StringConcatenator 
operator +( const CppTL::StringBuffer &left, const CppTL::StringBuffer &right ) {
   return CppTL::StringConcatenator( left, right );
}

inline CppTL::StringConcatenator 
operator +( const CppTL::StringBuffer &left, const CppTL::ConstCharView &right ) {
   return CppTL::StringConcatenator( left, right );
}


inline CppTL::StringConcatenator 
operator +( const CppTL::ConstCharView &left, const char *right ) {
   return CppTL::StringConcatenator( left, right );
}

inline CppTL::StringConcatenator 
operator +( const CppTL::ConstCharView &left, const CppTL::ConstString &right ) {
   return CppTL::StringConcatenator( left, right );
}

inline CppTL::StringConcatenator 
operator +( const CppTL::ConstCharView &left, const CppTL::StringConcatenator &right ) {
   return CppTL::StringConcatenator( left, right );
}

inline CppTL::StringConcatenator 
operator +( const CppTL::ConstCharView &left, const CppTL::StringBuffer &right ) {
   return CppTL::StringConcatenator( left, right );
}

inline CppTL::StringConcatenator 
operator +( const CppTL::ConstCharView &left, const CppTL::ConstCharView &right ) {
   return CppTL::StringConcatenator( left, right );
}


# ifndef CPPTL_CONSTSTRING_NO_STDSTRING
inline CppTL::StringConcatenator 
operator +( const CppTL::ConstString &left, const std::string &right ) {
   return CppTL::StringConcatenator( left, right );
}

inline CppTL::StringConcatenator 
operator +( const CppTL::StringConcatenator &left, const std::string &right ) {
   return CppTL::StringConcatenator( left, right );
}

inline CppTL::StringConcatenator 
operator +( const CppTL::StringBuffer &left, const std::string &right ) {
   return CppTL::StringConcatenator( left, right );
}

inline CppTL::StringConcatenator 
operator +( const CppTL::ConstCharView &left, const std::string &right ) {
   return CppTL::StringConcatenator( left, right );
}

inline CppTL::StringConcatenator 
operator +( const std::string &left, const CppTL::ConstString &right ) {
   return CppTL::StringConcatenator( left, right );
}

inline CppTL::StringConcatenator 
operator +( const std::string &left, const CppTL::StringConcatenator &right ) {
   return CppTL::StringConcatenator( left, right );
}

inline CppTL::StringConcatenator 
operator +( const std::string &left, const CppTL::StringBuffer &right ) {
   return CppTL::StringConcatenator( left, right );
}

inline CppTL::StringConcatenator 
operator +( const std::string &left, const CppTL::ConstCharView &right ) {
   return CppTL::StringConcatenator( left, right );
}

# endif



namespace CppTL {

// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// class StringConcatenator
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////

inline
StringConcatenator::StringConcatenator( const SubString &left,
                                        const SubString &right )
   : left_( left )
   , right_( right )
{
}


inline StringConcatenator::size_type 
StringConcatenator::length() const
{
   return left_.length() + right_.length();
}


inline void 
StringConcatenator::copyToBuffer( value_type *buffer ) const
{
   left_.copyToBuffer( buffer );
   right_.copyToBuffer( buffer + left_.length() );
}

// class StringConcatenator::SubString
// //////////////////////////////////////////////////////////////////

inline 
StringConcatenator::SubString::SubString( const value_type *csz )
   : kind_( cszString )
   , csz_( csz )
   , length_( value_type(-1) )
{
}


inline
StringConcatenator::SubString::SubString( const StringConcatenator &other )
   : kind_( concatenator )
   , concatenator_( &other )
   , length_( value_type(-1) )
{
}


inline
StringConcatenator::SubString::SubString( const ConstString &source )
   : kind_( string )
   , string_( &source )
   , length_( value_type(-1) )
{
}


inline
StringConcatenator::SubString::SubString( const StringBuffer &source )
   : kind_( stringBuffer )
   , buffer_( &source )
   , length_( value_type(-1) )
{
}


inline
StringConcatenator::SubString::SubString( const ConstCharView &view )
   : kind_( constCharView )
   , constCharView_( &view )
   , length_( view.length() )
{
}


# ifndef CPPTL_CONSTSTRING_NO_STDSTRING
inline
StringConcatenator::SubString::SubString( const std::string &source )
   : kind_( stdString )
   , stdString_( &source )
   , length_( value_type(-1) )
{
}
# endif

inline StringConcatenator::size_type 
StringConcatenator::SubString::length() const
{
   if ( length_ != value_type(-1) )
      return length_;

   switch ( kind_ )
   {
   case concatenator:
      length_ = concatenator_->length();
      break;
   case cszString:
      length_ = size_type( strlen( csz_ ) );
      break;
   case string:
      length_ = string_->length();
      break;
   case stringBuffer:
      length_ = buffer_->length();
      break;
   case constCharView:
      length_ = constCharView_->length();
      break;
# ifndef CPPTL_CONSTSTRING_NO_STDSTRING
   case stdString:
      length_ = size_type( stdString_->length() );
      break;
# endif
   default:
      CPPTL_ASSERT_MESSAGE( false, "unreachable" );
      break;
   }
   return length_;
}

inline void 
StringConcatenator::SubString::copyToBuffer( value_type *buffer ) const
{
   switch ( kind_ )
   {
   case concatenator:
      concatenator_->copyToBuffer( buffer );
      break;
   case cszString:
      memcpy( buffer, csz_, length() );
      break;
   case string:
      memcpy( buffer, string_->c_str(), length() );
      break;
   case stringBuffer:
      memcpy( buffer, buffer_->c_str(), length() );
      break;
   case constCharView:
      memcpy( buffer, constCharView_->begin(), length() );
      break;
# ifndef CPPTL_CONSTSTRING_NO_STDSTRING
   case stdString:
      memcpy( buffer, stdString_->c_str(), length() );
      break;
# endif
   default:
      CPPTL_ASSERT_MESSAGE( false, "unreachable" );
      break;
   }
}



// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// class ConstString
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////

inline
ConstString::ConstString()
   : length_( 0 )
   , buffer_( 0 )
{
}


inline
ConstString::ConstString( const value_type *csz )
   : length_( csz ? size_type(strlen( csz )) : 0 )
   , buffer_( new value_type[length_+1] )
{
   memcpy( buffer_, csz, length_ );
   buffer_[ length_ ] = 0;
}


inline
ConstString::ConstString( const value_type *begin, const value_type *end )
   : length_( size_type(end - begin) )
   , buffer_( new value_type[end-begin+1] )
{
   memcpy( buffer_, begin, end-begin );
   buffer_[ length_ ] = 0;
}


inline
ConstString::ConstString( const value_type *csz, size_type length )
   : length_( length )
   , buffer_( new value_type[length+1] )
{
   memcpy( buffer_, csz, length );
   buffer_[ length_ ] = 0;
}


inline
ConstString::ConstString( const ConstCharView &view )
   : length_( view.length() )
   , buffer_( new value_type[view.length()+1] )
{
   memcpy( buffer_, view.begin(), view.length() );
   buffer_[ length_ ] = 0;
}


inline
ConstString::ConstString( const StringConcatenator &concatenator )
   : length_( concatenator.length() )
   , buffer_( new value_type[length_+1] )
{
   buffer_[length_] = 0;
   concatenator.copyToBuffer( buffer_ );
}


inline
ConstString::ConstString( const StringBuffer &buffer )
   : length_( buffer.length() )
   , buffer_( new value_type[length_+1] )
{
   buffer_[length_] = 0;
   memcpy( buffer_, buffer.c_str(), length_ );
}


# ifndef CPPTL_CONSTSTRING_NO_STDSTRING
inline
ConstString::ConstString( const std::string &string )
   : length_( size_type(string.length()) )
   , buffer_( new value_type[length_+1] )
{
   buffer_[length_] = 0;
   memcpy( buffer_, string.c_str(), length_ );
}
# endif


inline
ConstString::ConstString( const ConstString &other )
   : length_( other.length_ )
   , buffer_( 0 )
{
   if ( length_ > 0 )
   {
      buffer_ = new value_type[length_+1];
      memcpy( buffer_, other.buffer_, length_+1 );
   }
}


inline
ConstString::~ConstString()
{
   delete [] buffer_;
}


inline ConstString &
ConstString::operator =( const ConstString &other )
{
   ConstString temp( other );
   swap( temp );
   return *this;
}


inline ConstString &
ConstString::operator =( const char *other )
{
   ConstString temp( other );
   swap( temp );
   return *this;
}


inline ConstString &
ConstString::operator =( const StringConcatenator &concatenator )
{
   ConstString temp( concatenator );
   swap( temp );
   return *this;
}


inline ConstString &
ConstString::operator =( const StringBuffer &buffer )
{
   ConstString temp( buffer );
   swap( temp );
   return *this;
}


inline ConstString &
ConstString::operator =( const ConstCharView &view )
{
   ConstString temp( view );
   swap( temp );
   return *this;
}


# ifndef CPPTL_CONSTSTRING_NO_STDSTRING
inline ConstString &
ConstString::operator =( const std::string &string )
{
   ConstString temp( string );
   swap( temp );
   return *this;
}
# endif


inline ConstString &
ConstString::operator +=( const ConstString &other )
{
   return *this = *this + other;
}


inline ConstString &
ConstString::operator +=( const char *other )
{
   return *this = *this + other;
}


inline ConstString &
ConstString::operator +=( const StringConcatenator &concatenator )
{
   return *this = *this + concatenator;
}


inline ConstString &
ConstString::operator +=( const StringBuffer &buffer )
{
   return *this = *this + buffer;
}


inline ConstString &
ConstString::operator +=( const ConstCharView &view )
{
   return *this = *this + view;
}


# ifndef CPPTL_CONSTSTRING_NO_STDSTRING
inline ConstString &
ConstString::operator +=( const std::string &string )
{
   return *this = *this + string;
}
# endif


inline void 
ConstString::swap( ConstString &other )
{
   CppTL::trivialSwap( length_, other.length_ );
   CppTL::trivialSwap( buffer_, other.buffer_ );
}


inline const ConstString::value_type *
ConstString::c_str() const
{
   return buffer_ ? buffer_ : "";
}


inline
const ConstString::value_type *
ConstString::end_c_str() const
{
   return buffer_ + length_;
}


# ifndef CPPTL_CONSTSTRING_NO_STDSTRING
inline std::string 
ConstString::str() const
{
   return buffer_ ? std::string( c_str(), end_c_str() ) 
                  : std::string();
}
# endif


inline ConstStringIterator 
ConstString::begin() const
{
   return buffer_;
}

inline ConstStringIterator 
ConstString::end() const
{
   return buffer_ + length_;
}


inline
ConstString::size_type
ConstString::length() const
{
   return length_;
}


inline
ConstString::size_type
ConstString::size() const
{
   return length();
}


inline bool 
ConstString::empty() const
{
   return length_ == 0;
}

inline ConstString::value_type
ConstString::operator[]( size_type index ) const
{
   return buffer_[index];
}


inline ConstString 
ConstString::substr( size_type index, 
                     size_type length ) const
{
   if ( index > length_ )
      index = length_;
   if ( length > length_ - index )
      length = length_ - index;
   return ConstString( buffer_ + index, length );
}

// ConstString / const char *

inline bool operator ==( const ConstString &left, const char *other )
{
   return strcmp( left.c_str(), other ) == 0;
}

inline bool operator !=( const ConstString &left, const char *other )
{
   return !(left == other );
}

inline bool operator <( const ConstString &left, const char *other )
{
   return strcmp( left.c_str(), other ) < 0;
}

inline bool operator <=( const ConstString &left, const char *other )
{
   return strcmp( left.c_str(), other ) <= 0;
}

inline bool operator >=( const ConstString &left, const char *other )
{
   return strcmp( left.c_str(), other ) >= 0;
}

inline bool operator >( const ConstString &left, const char *other )
{
   return strcmp( left.c_str(), other ) > 0;
}


// ConstString / ConstString


inline bool operator ==( const ConstString &left, const ConstString &other )
{
   return left.length() == other.length()  &&
          strcmp( left.c_str(), other.c_str() ) == 0;
}

inline bool operator !=( const ConstString &left, const ConstString &other )
{
   return !(left == other );
}

inline bool operator <( const ConstString &left, const ConstString &other )
{
   return strcmp( left.c_str(), other.c_str() ) < 0;
}

inline bool operator <=( const ConstString &left, const ConstString &other )
{
   return !( other > left );
}

inline bool operator >=( const ConstString &left, const ConstString &other )
{
   return !( other < left );
}

inline bool operator >( const ConstString &left, const ConstString &other )
{
   return strcmp( left.c_str(), other.c_str() ) > 0;
}


# ifndef CPPTL_CONSTSTRING_NO_STDSTRING
inline std::ostream &operator <<( std::ostream &os, const ConstString &str )
{
   return os << str.c_str();
}

inline std::istream &operator >>( std::istream &is, ConstString &str )
{
   std::string istr;
   is >> istr;
   str = istr;
   return is;
}
# endif

// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// class StringBuffer
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////

inline
StringBuffer::StringBuffer( unsigned int capacity )
   : capacity_( capacity )
   , length_( 0 )
   , buffer_( new char[capacity+1] )
{
   buffer_[0] = 0;
}

inline
StringBuffer::StringBuffer( const StringBuffer &other )
   : capacity_( other.length_ )
   , length_( other.length_ )
   , buffer_( new char[other.length_+1] )
{
   memcpy( buffer_, other.buffer_, length_+1 );
}

inline
StringBuffer::~StringBuffer()
{
   delete [] buffer_;
}

inline StringBuffer &
StringBuffer::operator =( const StringBuffer &other )
{
   StringBuffer temp( other );
   swap( temp );
   return *this;
}

inline void 
StringBuffer::swap( StringBuffer &other )
{
   CppTL::trivialSwap( capacity_, other.capacity_ );
   CppTL::trivialSwap( length_, other.length_ );
   CppTL::trivialSwap( buffer_, other.buffer_ );
}


inline void 
StringBuffer::reserve( size_type newCapacity )
{
   if ( newCapacity <= length_ + 1 )
      return;
   
   char *newBuffer = new char[newCapacity+1];
   memcpy( newBuffer, buffer_, length_+1 );
   delete [] buffer_;
   buffer_ = newBuffer;

   capacity_ = newCapacity;
}

inline StringBuffer::size_type 
StringBuffer::capacity() const
{
   return capacity_;
}


inline StringBuffer::size_type 
StringBuffer::length() const
{
   return length_;
}


inline void 
StringBuffer::truncate( size_type length )
{
   buffer_[length] = 0;
   length_ = length;
}


inline StringBuffer &
StringBuffer::operator +=( const StringConcatenator &other )
{
   size_type newLength = length_ + other.length();
   prepareBuffer( newLength );
   other.copyToBuffer( &buffer_[length_] );
   truncate(newLength);
   return *this;
}

inline StringBuffer &
StringBuffer::operator +=( const ConstString &other )
{
   size_type newLength = length_ + other.length();
   prepareBuffer( newLength );
   memcpy( &buffer_[length_], other.c_str(), other.length() );
   truncate(newLength);
   return *this;
}


inline StringBuffer &
StringBuffer::operator +=( const char *other )
{
   size_type otherLength = size_type(strlen(other));
   size_type newLength = length_ + otherLength;
   prepareBuffer( newLength );
   memcpy( &buffer_[length_], other, otherLength );
   truncate(newLength);
   return *this;
}


inline StringBuffer &
StringBuffer::operator +=( const ConstCharView &other )
{
   size_type newLength = length_ + other.length();
   prepareBuffer( newLength );
   memcpy( &buffer_[length_], other.begin(), other.length() );
   truncate(newLength);
   return *this;
}


# ifndef CPPTL_CONSTSTRING_NO_STDSTRING
inline StringBuffer &
StringBuffer::operator +=( const std::string &other )
{
   size_type newLength = length_ + other.length();
   prepareBuffer( newLength );
   memcpy( &buffer_[length_], other.c_str(), other.length() );
   truncate(newLength);
   return *this;
}
# endif


inline char &
StringBuffer::operator[]( size_type index )
{
   return buffer_[index];
}


inline char 
StringBuffer::operator[]( size_type index ) const
{
   return buffer_[index];
}


inline const char *
StringBuffer::c_str() const
{
   return buffer_;
}


inline ConstString 
StringBuffer::substr( size_type index, 
                      size_type length ) const
{
   if ( index > length_ )
      index = length_;
   if ( length > length_ - index )
      length = length_ - index;
   return ConstString( buffer_ + index, length );
}


inline void 
StringBuffer::prepareBuffer( size_type length )
{
   if ( length <= capacity_ )
      return;
   size_type newCapacity = (size_type(capacity_ * 1.43 + 16) & ~15) -1;
   if ( length > newCapacity )
      newCapacity = ((length + 16) & ~15) -1;
   reserve( newCapacity );
}

} // namespace CppTL



#endif // CPPTL_CONSTSTRING_H_INCLUDED
