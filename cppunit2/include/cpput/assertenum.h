#ifndef CPPUT_ASSERTENUM_H_INCLUDED
# define CPPUT_ASSERTENUM_H_INCLUDED

# include <cpput/assertcommon.h>
# include <cpptl/enumerator.h>
# include <functional>
# include <deque>

//todo: for sequence, list 'common' sequence part before expected & actual divergence

namespace CppUT
{
   template<class AType
           ,class BType>
   struct DefaultComparator
   {
      DefaultComparator() // Declaration needed to avoid bogus suncc warning.
      {
      }

      bool operator()( const AType &a, const BType &b ) const
      {
         return equalityTest( a, b );
      }
   };

   template<class AType
            ,class BType>
   struct DeRefComparator
   {
      bool operator()( const AType *a, const BType *b ) const
      {
         return a == b  ||  
                ( !a  &&  !b  &&  *a == *b );
      }
   };

   template<class AType
            ,class BType>
   struct RefComparator
   {
      bool operator()( AType a, BType b ) const
      {
         return equalityTest( a, b );
      }
   };

# ifndef CPPUT_NO_DEFAULT_ENUM_ITEM_STRINGIZE
   template<typename ItemType>
   std::string stringizeEnumItem( const ItemType &item )
   {
      return stringize( item );
   }
# endif // CPPUT_NO_DEFAULT_ENUM_ITEM_STRINGIZE


   /// \cond implementation_detail
   namespace Impl {

      enum { noDifference = -1 };

      template<class ExpectedEnumType
              ,class ActualEnumType
              ,class EqualityPredicate>
      unsigned int getSequenceDiffIndex( ExpectedEnumType expected, 
                                         ActualEnumType actual,
                                         EqualityPredicate comparator )
      {
         unsigned int diffIndex = 0;
         while ( expected.is_open() )
         {
            if ( !actual.is_open() )
               return diffIndex;
            if ( !comparator(expected.current(), actual.current() ) )
               return diffIndex;
            expected.advance();
            actual.advance();
            ++diffIndex;
         }
         if ( actual.is_open() )
            return diffIndex;
         return (unsigned int)noDifference;
      }


      template<class ExpectedSetType
               ,class ActualSetType
               ,class MissingSeqType
               ,class ExtraneousSeqType
               ,class EqualityPredicate>
      void getSetDifference( ExpectedSetType expectedEnum,
                             ActualSetType actualEnum,
                             MissingSeqType &missing,
                             ExtraneousSeqType &extraneous,
                             EqualityPredicate predicate )
      {
         for ( ; actualEnum.is_open(); actualEnum.advance() )
            extraneous.push_back( actualEnum.current() );

         for ( ; expectedEnum.is_open(); expectedEnum.advance() )
         {
            CPPTL_TYPENAME ExpectedSetType::value_type expectedItem = expectedEnum.current();
            bool found = false;
            CPPTL_TYPENAME ExtraneousSeqType::iterator it = extraneous.begin();
            for ( ; it != extraneous.end(); ++it )
            {
               const CPPTL_TYPENAME ExtraneousSeqType::value_type &actualItem = *it;
               if ( predicate( expectedItem, actualItem ) )
               {
                  it = extraneous.erase( it );
                  found = true;
                  break;
               }
            }

            if ( !found )
               missing.push_back( expectedItem );
         }
      }

   } // namespace Impl
   /// \endcond

   template<class ExpectedEnumerator
           ,class ActualEnumerator
           ,class ExpectedStringizer
           ,class ActualStringizer
           ,class EqualityPredicate>
   CheckerResult checkCustomHeterogeneousSequenceEqual( const ExpectedEnumerator &expected,
                                                        const ActualEnumerator &actual,
                                                        ExpectedStringizer expectedStringizer,
                                                        ActualStringizer actualStringizer,
                                                        EqualityPredicate comparator,
                                                        const Message &message )
   {
      CheckerResult result;
      unsigned int diffIndex = Impl::getSequenceDiffIndex( expected, 
                                                           actual, 
                                                           comparator );
      if ( diffIndex == Impl::noDifference )
         return result;

      typedef CppTL::SliceEnumerator<ExpectedEnumerator> ExpectedEnumeratorSlice;
      ExpectedEnumeratorSlice common = CppTL::Enum::slice( expected, 0, diffIndex );
      ExpectedEnumeratorSlice expectedDiff = CppTL::Enum::slice( expected, diffIndex );
      typedef CppTL::SliceEnumerator<ActualEnumerator> ActualEnumeratorSlice;
      ActualEnumeratorSlice actualDiff = CppTL::Enum::slice( actual, diffIndex );

      result.setFailed();
      result.appendMessages( message );
      result.appendMessage( translate( "Sequences are not identical." ) );
      result.setName( "actual ordered sequence == expected ordered sequence" );
      result.diagnostic("divergence index") = stringize(diffIndex);
      if ( common.is_open() )
      {
         result.diagnostic("common") = enumToStringCustom( common,
                                                           expectedStringizer );
      }
      result.diagnostic("actual") = enumToStringCustom( actualDiff, 
                                                        actualStringizer );
      result.predicate("expected") = enumToStringCustom( expectedDiff, 
                                                         expectedStringizer );
      // @todo add actual suffix, expected suffix
      return result;
   }

   template<class EnumeratorType
           ,class StringizerType
           ,class EqualityPredicate>
   CheckerResult checkCustomSequenceEqual( const EnumeratorType &expected,
                                           const EnumeratorType &actual,
                                           StringizerType stringizer,
                                           EqualityPredicate comparator,
                                           const Message &message )
   {
      return checkCustomHeterogeneousSequenceEqual( expected, actual,
                                                    stringizer, stringizer,
                                                    comparator, message );
   }

   template<class EnumeratorType
           ,class StringizerType
           ,class EqualityPredicate>
   CheckerResult checkCustomStringSequenceEqual( const EnumeratorType &expected,
                                                 const EnumeratorType &actual,
                                                 StringizerType stringizer,
                                                 const Message &message = Message() )
   {
      typedef CPPTL_TYPENAME EnumeratorType::value_type ValueType;
      DefaultComparator<ValueType,ValueType> comparator;
      return checkCustomHeterogeneousSequenceEqual( expected, actual,
                                                    stringizer, stringizer,
                                                    comparator, message );
   }

   template<class ExpectedEnumerator
           ,class ActualEnumerator
           ,class EqualityPredicate>
   CheckerResult checkCustomEqualitySequenceEqual( const ExpectedEnumerator &expected,
                                                   const ActualEnumerator &actual,
                                                   EqualityPredicate comparator,
                                                   const Message &message = Message() )
   {
      typedef DefaultStringizer<CPPTL_TYPENAME ExpectedEnumerator::value_type> ExpectedStringizer;
      typedef DefaultStringizer<CPPTL_TYPENAME ActualEnumerator::value_type> ActualStringizer;
      return checkCustomHeterogeneousSequenceEqual( expected, actual,
                                                    ExpectedStringizer(), ActualStringizer(),
                                                    comparator, message );
   }


   template<class ExpectedEnumeratorType
           ,class ActualEnumeratorType>
   CheckerResult checkSequenceEqual( const ExpectedEnumeratorType &expected,
                                     const ActualEnumeratorType &actual,
                                     const Message &message  = Message() )
   {
      DefaultComparator<CPPTL_TYPENAME ExpectedEnumeratorType::value_type
                       ,CPPTL_TYPENAME ActualEnumeratorType::value_type> comparator;
      return checkCustomEqualitySequenceEqual( expected, actual, comparator, message );
   }

   template<class ExpectedEnumeratorType
           ,class ActualEnumeratorType
           ,class EqualityPredicate>
   CheckerResult checkCustomEqualityStlSequenceEqual( const ExpectedEnumeratorType &expected,
                                                      const ActualEnumeratorType &actual,
                                                      EqualityPredicate comparator,
                                                      const Message &message = Message() )
   {
      return checkCustomEqualitySequenceEqual( CppTL::Enum::container( expected ), 
                                               CppTL::Enum::container( actual ), 
                                               comparator, message );
   }

   template<class ExpectedEnumeratorType
           ,class ActualEnumeratorType>
   CheckerResult checkStlSequenceEqual( const ExpectedEnumeratorType &expected,
                                        const ActualEnumeratorType &actual,
                                        const Message &message  = Message() )
   {
      return checkSequenceEqual( CppTL::Enum::container( expected ), 
                                 CppTL::Enum::container( actual ), 
                                 message );
   }

   template<class ExpectedEnumerator
           ,class ActualEnumerator
           ,class ExpectedStringizer
           ,class ActualStringizer
           ,class EqualityPredicate>
   CheckerResult checkCustomHeterogeneousSetEqual( const ExpectedEnumerator &expected,
                                                   const ActualEnumerator &actual,
                                                   ExpectedStringizer expectedStringizer,
                                                   ActualStringizer actualStringizer,
                                                   EqualityPredicate predicate,
                                                   const Message &message )
   {
      CheckerResult result;
      std::deque<CPPTL_TYPENAME ExpectedEnumerator::value_type> missing;
      std::deque<CPPTL_TYPENAME ActualEnumerator::value_type> extraneous;
      Impl::getSetDifference( expected, actual,
                              missing, extraneous,
                              predicate );
      if ( missing.empty()  &&  extraneous.empty() )
         return result;

      result.setFailed();
      result.appendMessages( message );
      result.appendMessage( translate( "Sets do not contain the same items." ) );
      result.setName( "actual set == expected set" );
      result.diagnostic("actual") = enumToStringCustom(actual,actualStringizer);
      if ( missing.size() > 0 )
      {
         result.diagnostic("missing") = 
            enumToStringCustom( CppTL::Enum::container(missing),
                                expectedStringizer );
      }
      if ( extraneous.size() > 0 )
      {
         result.diagnostic("extraneous") = 
            enumToStringCustom( CppTL::Enum::container(extraneous), 
                                actualStringizer );
      }
      return result;
   }

   template<class ExpectedEnumerator
           ,class ActualEnumerator
           ,class ItemStringizer
           ,class EqualityPredicate>
   CheckerResult checkCustomSetEqual( const ExpectedEnumerator &expected,
                                      const ActualEnumerator &actual,
                                      ItemStringizer itemStringizer,
                                      EqualityPredicate comparator,
                                      const Message &message = Message() )
   {
      return checkCustomHeterogeneousSetEqual( expected, actual, 
                                               itemStringizer, itemStringizer, 
                                               comparator, message );
   }

   template<class ExpectedEnumerator
           ,class ActualEnumerator
           ,class ItemStringizer>
   CheckerResult checkCustomStringSetEqual( const ExpectedEnumerator &expected,
                                            const ActualEnumerator &actual,
                                            ItemStringizer itemStringizer,
                                            const Message &message = Message() )
   {
      DefaultComparator<CPPTL_TYPENAME ExpectedEnumerator::value_type
                       ,CPPTL_TYPENAME ActualEnumerator::value_type> comparator;
      return checkCustomSetEqual( expected, actual, itemStringizer, comparator, message );
   }

   template<class ExpectedEnumerator
           ,class ActualEnumerator
           ,class EqualityPredicate>
   CheckerResult checkCustomEqualitySetEqual( const ExpectedEnumerator &expected,
                                              const ActualEnumerator &actual,
                                              EqualityPredicate comparator,
                                              const Message &message = Message() )
   {
      typedef DefaultStringizer<CPPTL_TYPENAME ExpectedEnumerator::value_type> ExpectedStringizer;
      typedef DefaultStringizer<CPPTL_TYPENAME ActualEnumerator::value_type> ActualStringizer;
      return checkCustomHeterogeneousSetEqual( expected, actual, 
                                               ExpectedStringizer(), ActualStringizer(),
                                               comparator, message );
   }

   template<class ExpectedEnumerator
           ,class ActualEnumerator>
   CheckerResult checkSetEqual( const ExpectedEnumerator &expected,
                                const ActualEnumerator &actual,
                                const Message &message = Message() )
   {
      DefaultComparator<CPPTL_TYPENAME ExpectedEnumerator::value_type
                       ,CPPTL_TYPENAME ActualEnumerator::value_type> comparator;
      return checkCustomEqualitySetEqual( expected, actual, comparator, message );
   }

   template<class ExpectedStlSet
           ,class ActualStlSet>
   CheckerResult checkStlSetEqual( const ExpectedStlSet &expected,
                                   const ActualStlSet &actual,
                                   const Message &message = Message() )
   {
      return checkSetEqual( CppTL::Enum::container( expected ), 
                            CppTL::Enum::container( actual ), 
                            message );
   }

   template<class ExpectedSetType
           ,class ActualSetType
           ,class EqualityPredicate>
   CheckerResult checkCustomEqualityStlSetEqual( const ExpectedSetType &expected,
                                                 const ActualSetType &actual,
                                                 EqualityPredicate predicate,
                                                 const Message &message = Message() )
   {
      return checkSetEqual( CppTL::Enum::container( expected ), 
                            CppTL::Enum::container( actual ), 
                            predicate, message );
   }


   template<class EnumeratorType
           ,class ItemStringizerType>
   std::string enumToStringCustom( EnumeratorType enumerator,
                                   ItemStringizerType itemStringizer,
                                   const std::string &separator = "; ",
                                   const std::string &begin = "{ ",
                                   const std::string &end = " }" )
   {
      std::string str;
      for ( ; enumerator.is_open(); enumerator.advance() )
      {
         std::string item = itemStringizer( enumerator.current() );
         if ( !str.empty() )
            str += separator;
         str += item;
      }
      return begin + str + end;
   }


   template<class EnumType>
   std::string enumToString( const EnumType &enumerator,
                             const std::string &separator = "; ",
                             const std::string &begin = "{ ",
                             const std::string &end = " }" )
   {
      return enumToStringCustom( enumerator, 
                                 DefaultStringizer<CPPTL_TYPENAME EnumType::value_type>(), 
                                 separator, 
                                 begin, 
                                 end );
   }

} // namespace CppUT


/*! \brief Asserts that two ordered sequence (list) are identical.
 * \ingroup group_assertions
 */
# define CPPUT_ASSERT_SEQUENCE_EQUAL  \
   CPPUT_BEGIN_ASSERTION_MACRO() \
   ::CppUT::checkSequenceEqual

/*! \brief Asserts that two unordered sequence (set,bag) are identical.
 * \ingroup group_assertions
 */
# define CPPUT_ASSERT_SET_EQUAL \
   CPPUT_BEGIN_ASSERTION_MACRO() \
   ::CppUT::checkSetEqual

/*! \brief Asserts that two STL ordered sequence are identical.
 * \ingroup group_assertions
 */
# define CPPUT_ASSERT_STL_SEQUENCE_EQUAL  \
   CPPUT_BEGIN_ASSERTION_MACRO() \
   ::CppUT::checkStlSequenceEqual

/*! \brief Asserts that two STL unordered sequence are identical.
 * \ingroup group_assertions
 */
# define CPPUT_ASSERT_STL_SET_EQUAL \
   CPPUT_BEGIN_ASSERTION_MACRO() \
   ::CppUT::checkStlSetEqual


/*! \brief Checks that two ordered sequence (list) are identical.
 * \ingroup group_assertions
 */
# define CPPUT_CHECK_SEQUENCE_EQUAL  \
   CPPUT_BEGIN_CHECKING_MACRO() \
   ::CppUT::checkSequenceEqual

/*! \brief Checks that two unordered sequence (set,bag) are identical.
 * \ingroup group_assertions
 */
# define CPPUT_CHECK_SET_EQUAL \
   CPPUT_BEGIN_CHECKING_MACRO() \
   ::CppUT::checkSetEqual


/*! \brief Asserts that two STL ordered sequence are identical.
 * \ingroup group_assertions
 */
# define CPPUT_CHECK_STL_SEQUENCE_EQUAL  \
   CPPUT_BEGIN_CHECKING_MACRO() \
   ::CppUT::checkStlSequenceEqual

/*! \brief Checks that two STL unordered sequence are identical.
 * \ingroup group_assertions
 */
# define CPPUT_CHECK_STL_SET_EQUAL \
   CPPUT_BEGIN_CHECKING_MACRO() \
   ::CppUT::checkStlSetEqual

#endif // CPPUT_ASSERTENUM_H_INCLUDED

