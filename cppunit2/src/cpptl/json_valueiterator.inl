// included by json_value.cpp
// everything is within Json namespace


// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// class ValueIteratorBase
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////

ValueIteratorBase::ValueIteratorBase()
{
}


ValueIteratorBase::ValueIteratorBase( const Value::ObjectValues::iterator &current )
   : current_( current )
{
}

Value &
ValueIteratorBase::deref() const
{
   return current_->second;
}


void 
ValueIteratorBase::increment()
{
   ++current_;
}


void 
ValueIteratorBase::decrement()
{
   --current_;
}


ValueIteratorBase::difference_type 
ValueIteratorBase::computeDistance( const SelfType &other ) const
{
#ifdef JSON_USE_CPPTL_SMALLMAP
   return current_ - other.current_;
#elif defined(_RWSTD_VER) 
   // RogueWave STL do not define the standard std::distance on
   // Sun Studio 12.
   difference_type d = 0;
   std::distance( current_, other.current_, d);
   return d;
#else
   return difference_type( std::distance( current_, other.current_ ) );
#endif
}


bool 
ValueIteratorBase::isEqual( const SelfType &other ) const
{
   return current_ == other.current_;
}


void 
ValueIteratorBase::copy( const SelfType &other )
{
   current_ = other.current_;
}


Value 
ValueIteratorBase::key() const
{
   const Value::CZString czstring = (*current_).first;
   if ( czstring.c_str() )
   {
      if ( czstring.isStaticString() )
         return Value( StaticString( czstring.c_str() ) );
      return Value( czstring.c_str() );
   }
   return Value( czstring.index() );
}


Value::UInt 
ValueIteratorBase::index() const
{
   const Value::CZString czstring = (*current_).first;
   if ( !czstring.c_str() )
      return czstring.index();
   return Value::UInt( -1 );
}


const char *
ValueIteratorBase::memberName() const
{
   const char *name = (*current_).first.c_str();
   return name ? name : "";
}


// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// class ValueConstIterator
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////

ValueConstIterator::ValueConstIterator()
{
}


ValueConstIterator::ValueConstIterator( const Value::ObjectValues::iterator &current )
   : ValueIteratorBase( current )
{
}

ValueConstIterator &
ValueConstIterator::operator =( const ValueIteratorBase &other )
{
   copy( other );
   return *this;
}


// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// class ValueIterator
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////

ValueIterator::ValueIterator()
{
}


ValueIterator::ValueIterator( const Value::ObjectValues::iterator &current )
   : ValueIteratorBase( current )
{
}

ValueIterator::ValueIterator( const ValueConstIterator &other )
   : ValueIteratorBase( other )
{
}

ValueIterator::ValueIterator( const ValueIterator &other )
   : ValueIteratorBase( other )
{
}

ValueIterator &
ValueIterator::operator =( const SelfType &other )
{
   copy( other );
   return *this;
}
