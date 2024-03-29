#ifndef CPPTL_FORMAT_INCLUDED
# define CPPTL_FORMAT_INCLUDED

# include <cpptl/forwards.h>
# include <cpptl/stringtools.h>
# include <string>
# include <assert.h>

namespace CppTL {

class FormatArg;
class FormatArgSpec;
class Formatter;
class FormatParseContext;

typedef unsigned int FormatStringIndex;

class FormatArg
{
public:
   FormatArg( char v );
   FormatArg( LargestInt v );
   FormatArg( LargestUnsignedInt v );
   FormatArg( int v );
   FormatArg( unsigned int v );
   FormatArg( double v );
   FormatArg( void *p );
   FormatArg( const char *s );
   FormatArg( const ConstString &s );
   FormatArg( const std::string &s );
   FormatArg( const Formatter &s );
   FormatArg( const StringConcatenator &s );

   bool isNegative() const;

   enum ArgType
   {
      argChar,
      argInt,
      argUInt,
      argDouble,
      argPointer,
      argString,
      argFormatter,
      argConcatenator
   };

   struct StringInfo
   {
      const char *begin_;
      const char *end_;
   };

   union 
   {
      char c_;
      LargestInt int_;
      LargestUnsignedInt uint_;
      double double_;
      void *pointer_;
      StringInfo string_;
      const Formatter *formatter_;
      const StringConcatenator *concatenator_;
   };
   ArgType type_;
};


class FormatArgs
{
public:
   enum { maxFormatArgs = 10 };

   FormatArgs();
   FormatArgs( const FormatArg &arg1 );
   FormatArgs( const FormatArg &arg1, const FormatArg &arg2 );
   FormatArgs( const FormatArg &arg1, const FormatArg &arg2, const FormatArg &arg3 );

   const FormatArg *args_[ maxFormatArgs ];
   int argCount_;
};


class FormatArgSpec
{
public:
   enum OutputType 
   {
      decimal = 1,
      hexaDecimal,
      octal,
      real,
      string,
      character,
      trailingText,
      automatic
   };

   enum Flags
   {
      flagRequireSign = 1,                      // '+'
      flagLeftAlign = flagRequireSign << 1,     // '-'
      flagShowBasePoint = flagLeftAlign << 1,   // '#'
      flagPadWithZero = flagShowBasePoint << 1, // '0'
      flagPadWithSpace = flagPadWithZero << 1,  // ' '
      flagUseUpperCase = flagPadWithSpace << 1, // 'X'
   };
};

class Formatter
{
public:
   Formatter( const char *format, const FormatArgs &args );

   FormatStringIndex length() const;

   const char *format_;
   const FormatArgs *args_;
};

class FormatParseContext
{
public:
   void setTrailingTextOnly( const char *begin, const char *end );
   const char *checkpointBegin_;
   const char *checkpointEnd_;
   const char *formatSpecBegin_;
   const char *formatSpecEnd_;
   const char *formatTypeLetter_;
   unsigned int flags_;
   int width_;
   int precision_;
   FormatArgSpec::OutputType type_;
   const FormatArg *arg_;

   bool hasFlag( unsigned int flag ) const 
   {
      return (flags_ & flag) != 0;
   }
};

class FormatLengthComputer
{
public:
   FormatLengthComputer()
      : length_( 0 )
   {
   }

   ~FormatLengthComputer()
   {
   }

   void operator()( const FormatParseContext &context );

   void handleNumber( const FormatParseContext &context,
                        bool isNegative,
                        FormatStringIndex numberDigits,
                        FormatStringIndex prefixLength );

   FormatStringIndex length_;
   char buffer[1024];   // for float conversion
};


template<class ArgProcessor>
inline void 
formatParse( const Formatter &formatter,
             ArgProcessor &processor )
{
   FormatParseContext context;
   const char *current = formatter.format_;
   context.checkpointBegin_ = current;
   const char *end = formatter.format_ + strlen( formatter.format_ );
   int currentArg = 1;
   while ( current < end )
   {
      if ( *current++ == '%' )  // start of format spec
      {
         if ( *current == '%' )  // escape for '%'
         {
            context.checkpointEnd_ = current;
            context.type_ = FormatArgSpec::trailingText;
            processor( context );
            ++current;
            context.checkpointBegin_ = current;
         }
         else
         {
            context.checkpointEnd_ = current - 1;  // omit '%'
            context.flags_ = formatParseFlags( current, end );
            int argIndex = 0;
            context.width_ = formatParseNumber( current, end );
            context.precision_ = -1;
            context.type_ = FormatArgSpec::automatic;
            bool isPositionalShortCut = false;
            context.formatSpecBegin_ = current;
            context.formatSpecEnd_ = 0;
            context.formatTypeLetter_ = 0;
            if ( current < end  &&  context.flags_ == 0  &&  context.width_ != -1 )
            {
               if ( *current == '%' ) // %N%, positional argument short-cut for %N$s
               {
                  ++current;
                  argIndex = context.width_;
                  context.width_ = -1;
                  isPositionalShortCut = true;
                  context.formatSpecBegin_ = current;
                  context.formatSpecEnd_ = current;
               }
               else if ( *current == '$' )   // standard positional argument: %N$width.precTYPE
               {
                  ++current;
                  argIndex = context.width_;
                  context.flags_ = formatParseFlags( current, end );
                  context.width_ = formatParseNumber( current, end );
                  context.formatSpecBegin_ = current;
               }
            }

            if ( !isPositionalShortCut )
            {
               if ( current < end  &&  *current == '.' )
               {
                  ++current;
                  context.precision_ = formatParseNumber( current, end );
               }
               context.formatSpecEnd_ = current;
               // May insert map look-up there '(key)'
               context.formatTypeLetter_ = current;
               formatParseType( current, end, context );
            }

            if ( argIndex == 0 )
               argIndex = currentArg++;
            if ( argIndex > 0  &&  argIndex <= formatter.args_->argCount_ )
            {
               context.arg_ = formatter.args_->args_[argIndex-1];
               formatSetAutoArgType( context );
               processor( context );
            }

            context.checkpointBegin_ = current;
         }         
      }
   };

   if ( context.checkpointBegin_ != end )
   {
      context.checkpointEnd_ = current;
      context.type_ = FormatArgSpec::trailingText;
      processor( context );
   }
}


inline void 
formatSetAutoArgType( FormatParseContext &context )
{
   switch ( context.arg_->type_ )
   {
   case FormatArg::argChar:
      if ( context.type_ != FormatArgSpec::string  &&  
            context.type_ != FormatArgSpec::character )
      {
         context.type_ = FormatArgSpec::character;
      }
      break;
   case FormatArg::argInt:
   case FormatArg::argUInt:
   case FormatArg::argPointer:
      if ( context.type_ != FormatArgSpec::decimal  &&
            context.type_ != FormatArgSpec::hexaDecimal  &&
            context.type_ != FormatArgSpec::octal )
      {
         context.type_ = FormatArgSpec::decimal;
      }
      break;
   case FormatArg::argDouble:
      context.type_ = FormatArgSpec::real;
      break;
   case FormatArg::argString:
   case FormatArg::argFormatter:
   case FormatArg::argConcatenator:
      if ( context.type_ != FormatArgSpec::string  &&  
            context.type_ != FormatArgSpec::character )
      {
         context.type_ = FormatArgSpec::string;
      }
      break;
   default:
      CPPTL_DEBUG_ASSERT_UNREACHABLE;
      break;
   }
}


inline void
formatParseType( const char *&current, 
                  const char *end, 
                  FormatParseContext &context )
{
   switch ( *current )
   {
   case 'p': 
      context.type_ = FormatArgSpec::hexaDecimal;
      context.flags_ |= FormatArgSpec::flagShowBasePoint;
      break;
   case 'X':
      context.flags_ |= FormatArgSpec::flagUseUpperCase;
      // fall through 'x'
   case 'x':
      context.type_ = FormatArgSpec::hexaDecimal;
      break;
   case 'd':
   case 'u':
   case 'i':
      context.type_ = FormatArgSpec::decimal;
      break;
   case 'o':
      context.type_ = FormatArgSpec::octal;
      break;
   case 'f':
   case 'F':
   case 'e':
   case 'E':
   case 'g':
   case 'G':
      context.type_ = FormatArgSpec::real;
      break;
   case 's':
      context.type_ = FormatArgSpec::string;
      break;
   case 'c':
      context.type_ = FormatArgSpec::character;
      break;
   default:
      break;
   }
   if ( context.type_ != FormatArgSpec::automatic )
      ++current;
}

inline unsigned int formatParseFlags( const char * &current, 
                                       const char *end )
{
   unsigned int flags = 0;

   while ( current < end  )
   {
      switch ( *current )
      {
      case '+': flags |= FormatArgSpec::flagRequireSign;
         break;
      case '-': flags |= FormatArgSpec::flagLeftAlign;
         break;
      case '#': flags |= FormatArgSpec::flagShowBasePoint;
         break;
      case '0': flags |= FormatArgSpec::flagPadWithZero;
         break;
      case ' ': flags |= FormatArgSpec::flagPadWithSpace;
         break;
      default: 
         return flags;
      }
      ++current;
   }
   return flags;
}

inline int formatParseNumber( const char * &current, 
                              const char *end )
{
   const char *begin = current;
   int width = 0;
   while ( current < end  &&  isDigit( *current ) )
   {
      width = width * 10 + int(*current - '0');
      ++current;
   }
   if ( begin == current )
      width = -1;
   return width;
}





template<const int radix>
class FormatIntegerTools
{
public:
   static int length( LargestUnsignedInt v )
   {
      int length = 0;
      for ( ; v != 0; v /= radix )
         ++length;
      return length ? length : 1;
   }
};


// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// class FormatLengthComputer
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////


inline void
FormatLengthComputer::operator()( const FormatParseContext &context )
{
   length_ += context.checkpointEnd_ - context.checkpointBegin_;
   if ( context.type_ == FormatArgSpec::trailingText )
      return;

   assert( context.arg_ != 0 );
   const FormatArg &arg = *context.arg_;
   switch ( context.type_ )
   {
   case FormatArgSpec::character:
      length_ += CPPTL_MAX( 1, context.width_ );
      break;
   case FormatArgSpec::string:
      {
         int stringLength = arg.string_.begin_ - arg.string_.end_;
         if ( context.width_ > 0 )
            length_ += CPPTL_MIN( context.width_, stringLength );
         else
            length_ += stringLength;
      }
      break;
   case FormatArgSpec::real:
      {
         const int symbolsLength = 16;  // for exponential, signs...
         const int maxWidth = CPPTL_ARRAY_SIZE(buffer) - symbolsLength;
         int actualWidth = CPPTL_MAX( context.width_, 0 );
         actualWidth += CPPTL_MAX( context.precision_, 0 );
         if ( actualWidth <= maxWidth )
         {
            char format[32];
            FormatStringIndex formatLength = context.formatSpecEnd_ - context.formatSpecBegin_;
            CPPTL_ASSERT_MESSAGE( formatLength < CPPTL_ARRAY_SIZE( format ) - 1,
                                    "Format string too long" );
            memcpy( format, context.formatSpecBegin_, formatLength );
            format[formatLength] = *context.formatTypeLetter_;
            format[formatLength+1] = 0;
            int actualLength = 0;
#ifdef CPPTL_HAS__SNPRINTF
            actualLength = _snprintf( buffer, CPPTL_ARRAY_SIZE(buffer)-1, format, arg.double_ );
#else
            actualLength = sprintf( buffer, format, arg.double_ );
#endif
            actualLength /= sizeof(buffer[0]);  // sprintf/snprintf returns number of bytes
            length_ += actualLength;
         }
         else // We have a problem... The buffer is not large enough
         {
         }
      }
      break;
   case FormatArgSpec::decimal:
      if ( arg.isNegative() )
         handleNumber( context, true, FormatIntegerTools<10>::length( -(arg.int_) ), 0 );
      else
         handleNumber( context, false, FormatIntegerTools<10>::length( arg.uint_ ), 0 );
      break;
   case FormatArgSpec::hexaDecimal:
      if ( arg.isNegative() )
         handleNumber( context, true, FormatIntegerTools<16>::length( -(arg.int_) ), 2 );
      else
         handleNumber( context, false, FormatIntegerTools<16>::length( arg.uint_ ), 2 );
      break;
   case FormatArgSpec::octal:
      if ( arg.isNegative() )
         handleNumber( context, true, FormatIntegerTools<8>::length( -(arg.int_) ), 1 );
      else
         handleNumber( context, false, FormatIntegerTools<8>::length( arg.uint_ ), 1 );
      break;
   default:
      CPPTL_DEBUG_ASSERT_UNREACHABLE;
      break;
   }
}

inline void 
FormatLengthComputer::handleNumber( const FormatParseContext &context,
                                    bool isNegative,
                                    FormatStringIndex numberDigits,
                                    FormatStringIndex prefixLength )
{
   int numberWidth = numberDigits;
   if ( context.hasFlag( FormatArgSpec::flagRequireSign )  ||  isNegative )
      ++numberWidth;
   if ( context.hasFlag( FormatArgSpec::flagShowBasePoint ) )
      numberWidth += prefixLength;
   length_ += CPPTL_MAX( numberWidth, context.width_ );
}


// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// class Formatter
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////


inline 
Formatter::Formatter( const char *format, 
                      const FormatArgs &args )
{
   format_ = format;
   args_ = &args;
}


inline FormatStringIndex 
Formatter::length() const
{
   FormatLengthComputer processor;
   formatParse( *this, processor );
   return processor.length_;
}


// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// class FormatArg
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
inline
FormatArg::FormatArg( char v )
   : type_( argChar )
   , c_( v )
{
}

inline
FormatArg::FormatArg( LargestInt v )
   : type_( argInt )
   , int_( v )
{
}

inline
FormatArg::FormatArg( LargestUnsignedInt v )
   : type_( argUInt )
   , uint_( v )
{
}

inline
FormatArg::FormatArg( int v )
   : type_( argInt )
   , int_( v )
{
}

inline
FormatArg::FormatArg( unsigned int v )
   : type_( argUInt )
   , uint_( v )
{
}

inline
FormatArg::FormatArg( double v )
   : type_( argDouble )
   , double_( v )
{
}

inline
FormatArg::FormatArg( void *p )
   : type_( argPointer )
   , pointer_( p )
{
}

inline
FormatArg::FormatArg( const char *s )
   : type_( argString )
{
   string_.begin_ = s;
   string_.end_ = s ? s + strlen(s) 
                    : 0;
}

inline
FormatArg::FormatArg( const ConstString &s )
   : type_( argString )
{
   string_.begin_ = s.c_str();
   string_.end_ = string_.begin_ + s.length();
}

inline
FormatArg::FormatArg( const std::string &s )
   : type_( argString )
{
   string_.begin_ = s.c_str();
   string_.end_ = string_.begin_ + s.length();
}

inline
FormatArg::FormatArg( const Formatter &s )
   : type_( argFormatter )
   , formatter_( &s )
{
}

inline
FormatArg::FormatArg( const StringConcatenator &s )
   : type_( argConcatenator )
   , concatenator_( &s )
{
}

inline bool 
FormatArg::isNegative() const
{
   return type_ == argInt  &&  int_ < 0;
}

// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// class FormatArgs
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
inline 
FormatArgs::FormatArgs()
   : argCount_( 0 )
{
}

inline 
FormatArgs::FormatArgs( const FormatArg &arg1 )
   : argCount_( 1 )
{
   args_[0] = &arg1;
}

inline 
FormatArgs::FormatArgs( const FormatArg &arg1, const FormatArg &arg2 )
   : argCount_( 2 )
{
   args_[0] = &arg1;
   args_[1] = &arg2;
}

inline 
FormatArgs::FormatArgs( const FormatArg &arg1, const FormatArg &arg2, const FormatArg &arg3 )
   : argCount_( 3 )
{
   args_[0] = &arg1;
   args_[1] = &arg2;
   args_[2] = &arg3;
}

// Helper functions

inline FormatArgs args( const FormatArg &arg1 )
{
   return FormatArgs( arg1 );
}


inline FormatArgs args( const FormatArg &arg1, const FormatArg &arg2 )
{
   return FormatArgs( arg1, arg2 );
}


inline FormatArgs args( const FormatArg &arg1, const FormatArg &arg2, const FormatArg &arg3 )
{
   return FormatArgs( arg1, arg2, arg3 );
}

} // namespace CppTL


#endif // CPPTL_FORMAT_INCLUDED

