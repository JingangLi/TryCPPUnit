#ifndef CPPTL_STRINGTOOLS_H_INCLUDED
# define CPPTL_STRINGTOOLS_H_INCLUDED

# include <cpptl/conststring.h>
# include <float.h> // toString( double )
# include <stdio.h> // sprintf

namespace CppTL {

   inline bool isDigit( char c )
   {
      return unsigned(c) >= '0'  && unsigned(c) <= '9';
   }

   inline char digit( unsigned number )
   {
      static const char *digits = "0123456789";
      return digits[number];
   }

   inline char hexaDigit( unsigned number )
   {
      static const char *digit = "0123456789abcdef";
      return digit[number];
   }

   template<class IntType>
   struct SignedPolicy
   {
      void makePositive( IntType &value )
      {
         isNegative_ = value < 0;
         if ( isNegative_ )
            value *= -1;
      }

      bool needMinusSign() const
      {
         return isNegative_;
      }

      bool isNegative_;
   };

   template<class IntType>
   struct UnsignedPolicy
   {
      void makePositive( IntType & )
      {
      }

      bool needMinusSign() const
      {
         return false;
      }
   };

   template<class IntType, class SignPolicy>
   CppTL::ConstString integerToString( IntType v, SignPolicy sign )
   {
      char buffer[32];
      char *current = &buffer[32];
      *--current = 0;
      if ( v == 0 )
         *--current = '0';
      else
      {
         sign.makePositive( v );
         while ( v != 0 )
         {
            *--current = digit( unsigned(v % 10) );
            v /= 10;
         }
         if ( sign.needMinusSign() )
            *--current = '-';
      }
      return current;
   }


   template<class UnsignedIntegerType>
   CppTL::ConstString 
   integerToHexaString( UnsignedIntegerType value )
   {
      const int bufferLength = sizeof(UnsignedIntegerType) * 2 + 1;
      char buffer[ bufferLength ];
      char *current = &buffer[ bufferLength ];
      char *end = current;
      *--current = 0;
      do
      {
         *--current += hexaDigit( size_type(value & 15) );
         value /= 16;
      }
      while ( value != 0 );

      return CppTL::ConstString( current, end );
   }


   //template<class IntType>
   //CppTL::ConstString integerToString( IntType v )
   //{
   //   CppTL::ConstString s;
   //   if ( v == 0 )
   //      s += '0';
   //   else
   //   {
   //      bool isSigned = v < 0;
   //      if ( isSigned )
   //         v *= -1;

   //      while ( v != 0 )
   //      {
   //         // Thoses cast are necessary to resolve an ambiguity between insert( char *, unsigned, char ) 
   //         // and insert( unsigned, unsigned, char) with Sun CC.
   //         s.insert( (unsigned int)0, (unsigned int)1, digits[v % 10] ); 
   //         v /= 10;
   //      }

   //      if ( isSigned )
   //         s.insert( 0, "-" );
   //   }
   //   return s;
   //}

   inline CppTL::ConstString toString( char c )
   {
      return CppTL::ConstString( &c, 1 );
   }

   inline CppTL::ConstString toString( int v )
   {
      return integerToString( v, SignedPolicy<int>() );
   }

   inline CppTL::ConstString toString( unsigned int v )
   {
      return integerToString( v, UnsignedPolicy<unsigned int>() );
   }

   inline CppTL::ConstString toString( long v )
   {
      return integerToString( v, SignedPolicy<long>() );
   }

   inline CppTL::ConstString toString( unsigned long v )
   {
      return integerToString( v, UnsignedPolicy<unsigned long>() );
   }

   inline CppTL::ConstString toString( short v )
   {
      return integerToString( v, SignedPolicy<short>() );
   }

   inline CppTL::ConstString toString( unsigned short v )
   {
      return integerToString( v, UnsignedPolicy<unsigned short>() );
   }

#ifndef CPPTL_NO_INT64

   inline CppTL::ConstString toString( int64_t v )
   {
      return integerToString( v, SignedPolicy<int64_t>() );
   }

   inline CppTL::ConstString toString( uint64_t v )
   {
      return integerToString( v, UnsignedPolicy<uint64_t>() );
   }
#endif

   inline CppTL::ConstString toString( float v )
   {
#ifdef FLT_DIG
       const int precision = FLT_DIG;
#else
       const int precision = 6;
#endif  // #ifdef FLT_DIG
       char buffer[128];
#ifdef __STDC_SECURE_LIB__ // Use secure version with visual studio 2005 to avoid warning.
       sprintf_s(buffer, sizeof(buffer), "%.*g", precision, v); 
#else	
       sprintf(buffer, "%.*g", precision, v); 
#endif
       return buffer;
   }

   inline CppTL::ConstString toString( double v )
   {
#ifdef DBL_DIG
       const int precision = DBL_DIG;
#else
       const int precision = 15;
#endif  // #ifdef DBL_DIG
       char buffer[128];
#ifdef __STDC_SECURE_LIB__ // Use secure version with visual studio 2005 to avoid warning.
       sprintf_s(buffer, sizeof(buffer), "%.*g", precision, v); 
#else	
       sprintf(buffer, "%.*g", precision, v); 
#endif
       return buffer;
   }

   inline CppTL::ConstString toString( long double v )
   {
#ifdef DBL_DIG
       const int precision = DBL_DIG;
#else
       const int precision = 15;
#endif  // #ifdef DBL_DIG
       char buffer[128];
#ifdef __STDC_SECURE_LIB__ // Use secure version with visual studio 2005 to avoid warning.
       sprintf_s(buffer, sizeof(buffer), "%.*g", precision, v); 
#else	
       sprintf(buffer, "%.*Lg", precision, v); 
#endif
       return buffer;
   }

   inline CppTL::ConstString toString( bool v )
   {
      return v ? "true" : "false";
   }


   inline CppTL::ConstString toHexaString( unsigned int v )
   {
      return integerToHexaString( v );
   }


#ifndef CPPTL_NO_INT64
   inline CppTL::ConstString toHexaString( uint64_t v )
   {
      return integerToHexaString( v );
   }
#endif


inline void
escapeControl( char c, CppTL::StringBuffer &escaped, const char *newLineEscape = "\\n" )
{
   switch ( c )
   {
   case '\n':
      escaped += newLineEscape;
      break;
   case '\t':
      escaped += CppTL::ConstCharView( "\\t", 2 );
      break;
   case '\r':
      escaped += CppTL::ConstCharView( "\\r", 2 );
      break;
   case '\v':
      escaped += CppTL::ConstCharView( "\\v", 2 );
      break;
   default:
      {
         char hexa[2];
         hexa[0] = hexaDigit( (c >> 4) & 15 );
         hexa[1] = hexaDigit( c & 15 );
         escaped += CppTL::ConstCharView( "\\x", 2 ) 
                    + ConstCharView( hexa, hexa + sizeof(hexa) );
      }
      break;
   }
}


inline void
escape( char c, CppTL::StringBuffer &escaped, const char *newLineEscape = "\\n" )
{
   if ( c >=0  &&  c < 32 )
      escapeControl( c, escaped, newLineEscape );
   else if ( c == '\\' )
      escaped += CppTL::ConstCharView( "\\\\", 2 );
   escaped += CppTL::ConstCharView( &c, 1 );
}


// need to expose some of those utility functions...
inline void
escapeRange( const char *first,
             const char *last, 
             CppTL::StringBuffer &escaped, 
             const char *newLineEscape = "\\n" )
{
   while ( first != last )
      escape( *first++, escaped, newLineEscape );
}

inline void
escape( const CppTL::ConstString &text, 
        CppTL::StringBuffer &escaped, 
        const char *newLineEscape = "\\n" )
{
   escapeRange( text.c_str(), text.c_str() + text.length(), escaped );
}


inline CppTL::ConstString 
quoteStringRange( const char *first,
                  const char *last,
                  const char *newLineEscape = "\\n" )
{
   CppTL::StringBuffer escaped( size_type((last-first) * 1.2) + 64 );
   escaped += CppTL::ConstCharView( "\"", 1 );
   escapeRange(first, last, escaped, newLineEscape );
   escaped += CppTL::ConstCharView( "\"", 1 );
   return escaped;
}


inline CppTL::ConstString 
quoteString( const CppTL::ConstString &text, 
             const char *newLineEscape = "\\n" )
{
   return quoteStringRange( text.c_str(), text.c_str() + text.length() );
}


inline CppTL::ConstString 
quoteMultiLineStringRange( const char *first, 
                           const char *last )
{
   return quoteStringRange( first, last, "\\n\n" );
}


inline CppTL::ConstString 
quoteMultiLineString( const CppTL::ConstString &str )
{
   return quoteString( str, "\\n\n" );
}


} // namespace CppTL

#endif // CPPTL_STRINGTOOLS_H_INCLUDED

