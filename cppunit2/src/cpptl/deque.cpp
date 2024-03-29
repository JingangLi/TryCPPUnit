#include <cpptl/deque.h>

namespace CppTL {

// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// class DequeBase
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////

DequeBase::DequeBase()
{
   initialize();
}

DequeBase::~DequeBase()
{
   delete [] pageMapBuffer_;
}


void 
DequeBase::initialize()
{
   pageMapBuffer_ = 0;
   pageMap_ = 0;
   pageMapBufferSize_ = 0;
   usedPages_ = 0;
   firstPageStartIndex_ = 0;
   lastPageEndIndex_ = 0;
}


void 
DequeBase::swap( DequeBase &other )
{
   CppTL::swap( pageMapBuffer_, other.pageMapBuffer_ );
   CppTL::swap( pageMap_, other.pageMap_ );
   CppTL::swap( pageMapBufferSize_, other.pageMapBufferSize_ );
   CppTL::swap( usedPages_, other.usedPages_ );
   CppTL::swap( firstPageStartIndex_, other.firstPageStartIndex_ );
   CppTL::swap( lastPageEndIndex_, other.lastPageEndIndex_ );
}


void 
DequeBase::prependPage( void *newPage )
{
   growPageMap( 1, 0 );
   CPPTL_ASSERT_MESSAGE( pageMap_ > pageMapBuffer_, "DequeBase::growPageMap(): logic bug" );
   *--pageMap_ = newPage;
   ++usedPages_;
}

void 
DequeBase::appendPage( void *newPage )
{
   growPageMap( 0, 1 );
   CPPTL_ASSERT_MESSAGE( usedPages_ < pageMapBufferSize_, 
                         "DequeBase::growPageMap(): logic bug" );
   pageMap_[ usedPages_++ ] = newPage;
}


void 
DequeBase::growPageMap( size_t pagesToPrepend, size_t pagesToAppend )
{
   size_t minNewSize = usedPages_ + pagesToPrepend + pagesToAppend;
   size_t maxAvailableFront = pageMap_ - pageMapBuffer_;
   size_t maxAvailableBack = pageMapBufferSize_ - usedPages_;
   if ( minNewSize > pageMapBufferSize_  ||  
        maxAvailableFront < pagesToPrepend  ||
        maxAvailableBack < pagesToAppend )      // not enough space, need to realloc
   {
      const size_t minGrowth = 32;
      size_t baseSize = CPPTL_MAX( minNewSize, pageMapBufferSize_ + minGrowth );
      size_t newSize = baseSize/2 + baseSize;
      void **oldBuffer = pageMapBuffer_;
      typedef void *VoidPtr;
      pageMapBuffer_ = new VoidPtr[pageMapBufferSize_]; // may throw if not enough memory
      delete [] oldBuffer;
      pageMapBufferSize_ = newSize;
      size_t zeroIndex = (pageMapBufferSize_ - minNewSize) / 2;
      pageMap_ = pageMapBuffer_ + sizeof(void*) * zeroIndex;
      memmove( pageMap_,
               pageMapBuffer_ + sizeof(void*) * maxAvailableFront,
               sizeof(void*) * usedPages_ );
   }
}




// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// class DequeIteratorCommonBase
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
// //////////////////////////////////////////////////////////////////
DequeIteratorCommonBase::DequeIteratorCommonBase()
   : pageMapBegin_( 0 )
   , pageMapEnd_( 0 )
   , pageMapCurrent_( 0 )
   , indexInCurrentPage_( 0 )
   , firstPageStartIndex_( 0 )
   , lastPageStartIndex_( 0 )
{
}


DequeIteratorCommonBase::DequeIteratorCommonBase( void **pageMapBegin,
                                                  void **pageMapEnd,
                                                  void **pageMapCurrent,
                                                  size_t indexInCurrentPage,
                                                  size_t firstPageStartIndex,
                                                  size_t lastPageStartIndex )
   : pageMapBegin_( pageMapBegin )
   , pageMapEnd_( pageMapEnd )
   , pageMapCurrent_( pageMapCurrent )
   , indexInCurrentPage_( indexInCurrentPage )
   , firstPageStartIndex_( firstPageStartIndex )
   , lastPageStartIndex_( lastPageStartIndex )
{
}


void 
DequeIteratorCommonBase::increment()
{
   CPPTL_ASSERT_MESSAGE( pageMapCurrent_ != pageMapEnd_  
                           ||  indexInCurrentPage_ != lastPageStartIndex_,
                           "DequeIterator: incrementing beyond end" );
   ++indexInCurrentPage_;
   if ( indexInCurrentPage_ == elementPerPage )
   {
      ++pageMapCurrent_;
      indexInCurrentPage_ = 0;
   }
}


void 
DequeIteratorCommonBase::decrement()
{
   CPPTL_ASSERT_MESSAGE( pageMapCurrent_ != pageMapBegin_  
                           ||  indexInCurrentPage_ != firstPageStartIndex_,
                           "DequeIterator: decrementing beyond beginning" );
   if ( indexInCurrentPage_ == 0 )
   {
      --pageMapCurrent_;
      indexInCurrentPage_ = elementPerPage - 1;
   }
   else
   {
      --indexInCurrentPage_;
   }
}


void 
DequeIteratorCommonBase::advance( difference_type n )
{
   if ( n > 0 )
   {
      n += indexInCurrentPage_;
      size_t inPageAdvance = n % elementPerPage;
      size_t fullPageAdvance = n / elementPerPage;
      CPPTL_ASSERT_MESSAGE( pageMapEnd_-pageMapCurrent_ < fullPageAdvance,
                           "DequeIterator: advancing beyond end" );
      CPPTL_ASSERT_MESSAGE( pageMapCurrent_ + fullPageAdvance + 1 != pageMapEnd_
                           ||  inPageAdvance < lastPageStartIndex_,
                           "DequeIterator: advancing beyond end" );
      pageMapCurrent_ += fullPageAdvance;
      indexInCurrentPage_ = inPageAdvance;
   }
   else if ( n < 0 )
   {
      n = indexInCurrentPage_ - n;
      if ( n >= 0 )
      {
         indexInCurrentPage_ = n;
      }
      else
      {
         size_t inPageAdvance = -n % elementPerPage;
         size_t fullPageAdvance = -n / elementPerPage;
         CPPTL_ASSERT_MESSAGE( pageMapCurrent_-pageMapBegin_ < fullPageAdvance,
                              "DequeIterator: advancing beyond beginning" );
         CPPTL_ASSERT_MESSAGE( pageMapCurrent_ - fullPageAdvance == pageMapBegin_
                              ||  inPageAdvance >= firstPageStartIndex_,
                              "DequeIterator: advancing beyond beginning" );
         pageMapCurrent_ -= fullPageAdvance;
         indexInCurrentPage_ = inPageAdvance;
      }
   }
}


DequeIteratorCommonBase::difference_type 
DequeIteratorCommonBase::computeDistance( const DequeIteratorCommonBase &other ) const
{
   difference_type pageDistance = 
      difference_type(pageMapCurrent_ - pageMapBegin_) -
      difference_type(other.pageMapCurrent_ - other.pageMapBegin_);
   difference_type indexDistance( indexInCurrentPage_ - other.indexInCurrentPage_ );
   return pageDistance * elementPerPage + indexDistance;
}


DequeIteratorCommonBase::size_t 
DequeIteratorCommonBase::index() const
{
   return (pageMapCurrent_ - pageMapBegin_) * elementPerPage + 
          indexInCurrentPage_ - firstPageStartIndex_;
}



} // namespace CppTL

