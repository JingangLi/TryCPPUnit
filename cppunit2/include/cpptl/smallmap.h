#ifndef CPPTL_VECTOR_H_INCLUDED
# define CPPTL_VECTOR_H_INCLUDED

# include "_stlimpl.h"

// Notes: exception handling is bugged:
// In case where an exception is thrown by a copy constructor while copying a range,
// All destructors are not called correctly.


namespace CppTL {

template<class Item>
class SmallMapBase
{
public:
   typedef unsigned int size_t;

   SmallMapBase()
      : data_( 0 )
      , size_( 0 )
      , capacity_( 0 )
   {
   }

   SmallMapBase( Item *data, size_t size, size_t capacity )
      : data_( data )
      , size_( size )
      , capacity_( capacity )
   {
   }

   Item *data_;
   size_t size_;
   size_t capacity_;
};


template<class T>
class SmallMapIteratorBase
{
public:
   typedef unsigned int size_t;
   typedef int difference_type;
   typedef SmallMapIteratorBase<T> SelfType;
   typedef T Item;

   size_t index() const
   {
      CPPTL_ASSERT_MESSAGE( map_ != 0, "SmallMapIterator::index(): invalid iterator" );
      return current_ - map_->data_;
   }

   bool operator ==( const SelfType &other ) const
   {
      return isEqual( other );
   }

   bool operator !=( const SelfType &other ) const
   {
      return !isEqual( other );
   }

   bool operator <( const SelfType &other ) const
   {
      return isLess( other );
   }

   bool operator <=( const SelfType &other ) const
   {
      return !other.isLess( *this );
   }

   bool operator >=( const SelfType &other ) const
   {
      return !isLess( other );
   }

   bool operator >( const SelfType &other ) const
   {
      return other.isLess( *this );
   }

protected:
   SmallMapIteratorBase()
      : map_( 0 )
      , current_( 0 )
   {
   }

   SmallMapIteratorBase( const SmallMapBase<Item> &map,
                         Item *current )
      : map_( &map )
      , current_( current )
   {
   }


   T &deref() const
   {
      CPPTL_ASSERT_MESSAGE( current_ != 0,
                            "SmallMapIterator: dereferencing invalid iterator" );
      return *current_;
   }

   void increment()
   {
      CPPTL_ASSERT_MESSAGE( map_  &&  ( current_ < map_->data_ + map_->size_ ), 
                           "SmallMapIterator::increment: incrementing beyond end" );
      ++current_;
   }

   void decrement()
   {
      CPPTL_ASSERT_MESSAGE( map_  &&  ( current_ > map_->data_ ), 
                           "SmallMapIterator::decrement: decrementing beyond beginning" );
      --current_;
   }

   void advance( difference_type n )
   {
      if ( n != 0 )
      {
         CPPTL_ASSERT_MESSAGE( map_  &&  map_->size_  &&
                              ( current_+n <= map_->data_ + map_->size_  &&  current_+n >= map_->data_), 
                              "SmallMapIterator::advance: advancing beyond end or beginning" );
         current_ += n;
      }
   }

   difference_type computeDistance( const SelfType &other ) const
   {
      CPPTL_ASSERT_MESSAGE( map_->data_ == other.map_->data_, "Comparing iterator on different container." );
      return current_ - other.current_;
   }

   bool isEqual( const SmallMapIteratorBase &other ) const
   {
      return current_ == other.current_;
   }

   bool isLess( const SmallMapIteratorBase &other ) const
   {
      return current_ < other.current_;
   }

protected:
   const SmallMapBase<Item> *map_;
   Item *current_;
};




template<class T, class Traits>
class SmallMapIterator : public SmallMapIteratorBase<T>
{
public:
   typedef unsigned int size_t;
   typedef int difference_type;
   typedef T value_type;
   typedef CPPTL_TYPENAME Traits::reference reference;
   typedef CPPTL_TYPENAME Traits::pointer pointer;

   typedef SmallMapIterator<T,Traits> SelfType;
   typedef SmallMapIteratorBase<T> SuperClass;
   typedef SmallMapIterator< T, ::CppTL::Impl::NonConstIteratorTraits<T> > NonConstSmallMapIterator;

   SmallMapIterator()
   {
   }

   // This constructor ensure that it is possible to construct a
   // const iterator from a non const iterator.
   SmallMapIterator( const NonConstSmallMapIterator &other )
      : SuperClass( other )
   {
   }

   SmallMapIterator( const SmallMapBase<T> &map, T *current )
      : SuperClass( map, current )
   {
   }

   SmallMapIterator( const SmallMapBase<T> &map, size_t currentIndex )
      : SuperClass( map, map.data_ + currentIndex )
   {
   }

   SelfType &operator++()
   {
      this->increment();
      return *this;
   }

   SelfType operator++( int )
   {
      SelfType temp( *this );
      ++*this;
      return temp;
   }

   SelfType &operator--()
   {
      this->decrement();
      return *this;
   }

   SelfType operator--( int )
   {
      SelfType temp( *this );
      --*this;
      return temp;
   }

   SelfType &operator +=( difference_type n )
   {
      this->advance( n );
      return *this;
   }

   SelfType operator +( difference_type n ) const
   {
      SelfType temp( *this );
      return temp += n;
   }

   SelfType &operator -=( difference_type n )
   {
      this->advance( -n );
      return *this;
   }

   SelfType operator -( difference_type n ) const
   {
      SelfType temp( *this );
      return temp -= n;
   }

   reference operator[]( difference_type n ) const
   {
      return *( *this + n );
   }

   reference operator *() const
   {
      return this->deref();
   }

   pointer operator->() const
   {
      return &(this->deref());
   }

   difference_type operator -( const SelfType &other ) const
   {
      return computeDistance( other );
   }
};



/// Store elements in a sorted vector.
template< class Key
        , class Value
        , class PredLess = CppTL::LessPred<Key>
        , class Allocator = CppTL::MallocAllocator< Pair<Key,Value> > >
class SmallMap : private SmallMapBase< Pair<Key,Value> >
{
public:
   typedef Pair<Key,Value> Item;

   typedef Item value_type;
   typedef Key key_type;
   typedef Value mapped_type;
   typedef SmallMapIterator<Item, ::CppTL::Impl::NonConstIteratorTraits<Item> > iterator;
   typedef SmallMapIterator<Item, ::CppTL::Impl::ConstIteratorTraits<Item> > const_iterator;
   typedef SmallMap<Key, Value, PredLess, Allocator> SelfType;
   typedef SmallMapBase< Pair<Key,Value> > SuperClass;

   SmallMap()
   {
   }

   SmallMap( const SmallMap &other )
      : SuperClass( 0, other.size_, other.size_ )
      , allocator_( other.allocator_ )
   {
      this->data_ = allocator_.allocateArray( this->size_ );
      CPPTL_TRY_BEGIN
      {
         ::CppTL::Impl::copy_with_construct( other.begin(), other.end(), this->data_ );
      }
      CPPTL_TRY_END_CLEANUP( allocator.releaseArray( this->data_, this->size_ ) )
   }

   ~SmallMap()
   {
   }

   SelfType &operator =( const SmallMap &other )
   {
      SelfType temp( other );
      swap( temp );
      return *this;
   }

   size_t size() const
   {
      return this->size_;
   }

   const_iterator find( const Key &key ) const
   {
      Item *found = lookUp( key );
      if ( found )
         return const_iterator( *this, found );
      return end();
   }

   iterator find( const Key &key )
   {
      Item *found = lookUp( key );
      if ( found )
         return iterator( *this, found );
      return end();
   }

   iterator lower_bound( const Key &key )
   {
      size_t index = insertionIndex( key, 0 );
      return iterator( *this, index );
   }

   iterator upper_bound( const Key &key )
   {
      size_t index = insertionIndex( key, 0 );
      return iterator( *this, index );
   }

   Pair<iterator,iterator> equal_range( const Key &key )
   {
      size_t index = insertionIndex( key, 0 );
      iterator it( *this, index );
      return Pair<iterator,iterator>( it, it );
   }

   const_iterator lower_bound( const Key &key ) const
   {
      size_t index = insertionIndex( key, 0 );
      return const_iterator( *this, index );
   }

   const_iterator upper_bound( const Key &key ) const
   {
      size_t index = insertionIndex( key, 0 );
      return const_iterator( *this, index );
   }

   Pair<const_iterator,const_iterator> equal_range( const Key &key ) const
   {
      size_t index = insertionIndex( key, 0 );
      const_iterator it( *this, index );
      return Pair<const_iterator,const_iterator>( it, it );
   }

   void clear()
   {
      SelfType temp;
      swap( temp );
   }

   iterator begin()
   {
      return iterator( *this, this->data_ );
   }

   iterator end()
   {
      return iterator( *this, this->size_ );
   }

   const_iterator begin() const
   {
      return const_iterator( *this, this->data_ );
   }

   const_iterator end() const
   {
      return const_iterator( *this, this->size_ );
   }

   size_t count( const Key &key ) const
   {
      return lookUp( key ) ? 1 : 0;
   }

   bool empty() const
   {
      return this->size_ == 0;
   }

   // equal_range

   iterator erase( iterator where )
   {
      CPPTL_ASSERT_MESSAGE( where >= begin()  &&  where < end(), "SmallMap<T>::erase(): invalid iterator" );
      ::CppTL::Impl::copy_and_destroy( where+1, end(), where );
      --(this->size_);
      return where;
   }

   iterator erase( iterator first, iterator last )
   {
      size_t whereIndex = first.index();
      size_t count = last - first;
      ::CppTL::Impl::copy_and_destroy( last, end(), first );
      this->size_ -= count;
      return iterator( *this, whereIndex );
   }

   iterator erase( const Key &key )
   {
      Item *found = lookUp( key );
      if ( found )
         return erase( iterator( *this, found ) );
      return end();
   }

   Value &operator[]( const Key &key )
   {
      typedef Pair<iterator,bool> ResultType;
      size_t whereIndex = insertionIndex( key, 0 );
      if ( whereIndex < this->size_
           &&  !less_( key, this->data_[whereIndex].first ) )
      {
         return this->data_[whereIndex].second;
      }
      return doInsert( whereIndex, Item( key, Value() ) )->second;
   }

   Pair<iterator,bool> insert( const Item &item )
   {
      typedef Pair<iterator,bool> ResultType;
      size_t whereIndex = insertionIndex( item.first, 0 );
      if ( whereIndex < this->size_
           &&  !less_( item.first, this->data_[whereIndex].first ) )
      {
         return ResultType( iterator( *this, whereIndex ), false );
      }
      return ResultType( doInsert( whereIndex, item ), true );
   }

   iterator insert( iterator where, const Item &item )
   {
      size_t whereIndex = insertionIndex( item.first, where.index() );
      if ( this->size_ == this->capacity_ )
      {
         if ( reserve( 1, whereIndex, &item ) )  // element was inserted when increasing capacity.
            return iterator( *this, whereIndex );
      }
      ++(this->size_);
      iterator itWhere( *this, whereIndex );
      ::CppTL::Impl::copy_backwards_and_destroy( itWhere, end()-1, end() );
      ::CppTL::Impl::construct( *itWhere, item );
      return itWhere;
   }

   void insert( const_iterator first , const_iterator last )
   {
      reserve( last - first );
      iterator itHint = begin();
      for ( ; first != last; ++first )
         itHint = insert( itHint, *first ).first;
   }

   void swap( SelfType &other )
   {
      CppTL::trivialSwap( this->data_, other.data_ );
      CppTL::trivialSwap( this->size_, other.size_ );
      CppTL::trivialSwap( this->capacity_, other.capacity_ );
      allocator_.swap( other.allocator_ );
      CppTL::trivialSwap( less_, other.less_ );
   }

   bool operator <( const SelfType &other ) const
   {
      if ( this->size_ < other.size_ )
         return true;
      if ( this->size_ > other.size_  ||  this->size_ == 0 )
         return false;
      Item *i1 = this->data_, *i2 = other.data_;
      Item *i1end = this->data_ + this->size_;
      for ( ; i1 != i1end; ++i1, ++i2 )
      {
         if ( itemIsLess( *i1, *i2 ) )
            return true;
         if ( itemIsLess( *i2, *i1 ) )
            return false;
      }
      return false;
   }

   bool operator ==( const SelfType &other ) const
   {
      if ( this->size_ != other.size_ )
         return false;
      Item *i1 = this->data_, *i2 = other.data_;
      Item *i1end = this->data_ + this->size_;
      for ( ; i1 != i1end; ++i1, ++i2 )
      {
         if ( itemIsLess( *i1, *i2 )  ||  itemIsLess( *i2, *i1 ) )
            return false;
      }
      return true;
   }

private:
   // Ensure capacity is large enough to add count elements.
   // Optionaly, insert a new item in the middle at position insertIndex.
   bool reserve( size_t count, size_t insertIndex = 0, const Item *item = 0)
   {
      if ( this->size_ + count <= this->capacity_ )
         return false;
      const size_t initialCapacity = 8;
      const size_t minCapacityIncrement = 8;
      size_t newCapacity = this->size_ + count;
      newCapacity += CPPTL_MAX( newCapacity / 4, minCapacityIncrement );
      newCapacity = CPPTL_MAX( initialCapacity, newCapacity );
      Item *newData = allocator_.allocateArray( newCapacity );
      CPPTL_TRY_BEGIN
      {
         if ( !item )
            insertIndex = this->size_;
         ::CppTL::Impl::copy_and_destroy( begin(), begin() + insertIndex, newData );
         if ( item )
         {
            new (newData+insertIndex) Item( *item );
            ::CppTL::Impl::copy_and_destroy( begin() + insertIndex, end(), 
                                             newData + (insertIndex+1) );
         }
      }
      CPPTL_TRY_END_CLEANUP( allocator_.release( newData, newCapacity ) );
      if ( this->data_ )
         allocator_.releaseArray( this->data_, this->capacity_ );
      this->data_ = newData;
      this->capacity_ = newCapacity;
      if ( item )
         ++(this->size_);
      return true;
   }

   size_t insertionIndex( const Key &key, size_t min = 0 ) const
   {
      size_t max = this->size_;
      while ( min != max )
      {
         size_t mid = (max+min) / 2;
         if ( less_( this->data_[mid].first, key ) )
            min = mid + 1;
         else
            max = mid;
      }
      return min;
   }

   Item *lookUp( const Key &key ) const
   {
      size_t pos = insertionIndex( key, 0 );
      if ( pos  < this->size_  &&  !less_( key, this->data_[pos].first ) )
         return this->data_ + pos;
      return 0;
   }

   iterator doInsert( size_t whereIndex, const Item &item )
   {
      if ( this->size_ == this->capacity_ )
      {
         if ( reserve( 1, whereIndex, &item ) )  // element was inserted when increasing capacity.
            return begin() + whereIndex;
      }
      ++(this->size_);
      iterator itWhere( *this, whereIndex );
      iterator itEnd = end();
      if ( whereIndex != this->size_ - 1 )
         ::CppTL::Impl::copy_backwards_and_destroy( itWhere, itEnd-1, itEnd );
      ::CppTL::Impl::construct( *itWhere, item);
      return itWhere;
   }

   bool itemIsLess( const Item &a, const Item &b ) const
   {
      return less_( a.first, b.first )  ||
             ( !less_(b.first, a.first) &&  a.second < b.second );
   }


private:
   Allocator allocator_;
   PredLess less_;
};



} // namespace CppTL


#endif // CPPTL_VECTOR_H_INCLUDED
