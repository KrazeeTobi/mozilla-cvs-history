/*
*****************************************************************************************
*                                                                                       *
* COPYRIGHT:                                                                            *
*   (C) Copyright Taligent, Inc.,  1996                                                 *
*   (C) Copyright International Business Machines Corporation,  1996                    *
*   Licensed Material - Program-Property of IBM - All Rights Reserved.                  *
*   US Government Users Restricted Rights - Use, duplication, or disclosure             *
*   restricted by GSA ADP Schedule Contract with IBM Corp.                              *
*                                                                                       *
*****************************************************************************************
*/
//===============================================================================
//
// File sortkey.h
//
// 
//
// Created by: Helena Shih
//
// Modification History:
//
//  Date         Name          Description
//
//  6/20/97      helena        Java class name change.
//  8/18/97     helena      Added internal API documentation. 
//===============================================================================

#ifndef _SORTKEY
#define _SORTKEY

#ifndef _PTYPES
#include "ptypes.h"
#endif

#ifndef _UNISTRING
#include "unistring.h"
#endif

#ifndef _COLL
#include "coll.h"
#endif

/**
 * Collation keys are generated by the Collator class.  Use the CollationKey objects
 * instead of Collator to compare strings multiple times.  A CollationKey
 * preprocesses the comparison information from the Collator object to
 * make the comparison faster.  If you are not going to comparing strings
 * multiple times, then using the Collator object is generally faster,
 * since it only processes as much of the string as needed to make a
 * comparison.
 * <p> For example (with strength == tertiary)
 * <p>When comparing "Abernathy" to "Baggins-Smythworthy", Collator
 * only needs to process a couple of characters, while a comparison
 * with CollationKeys will process all of the characters.  On the other hand,
 * if you are doing a sort of a number of fields, it is much faster to use
 * CollationKeys, since you will be comparing strings multiple times.
 * <p>Typical use of CollationKeys are in databases, where you store a CollationKey
 * in a hidden field, and use it for sorting or indexing.
 *
 * <p>Example of use:
 * <pre>
 * .   ErrorCode status = ZERO_ERROR;
 * .   Collator *myCollation = Collator::getDefault(Locale::FRANCE, status);
 * .   if (FAILURE(status)) return;
 * .   // Set to ignore the accent differences
 * .   myCollation->setStrength(Collator::PRIMARY);
 * .   UniChar sortchars[][MAX_TOKEN] = { 
 * .        {'a', 'b', 'c', 0}, 
 * .        {0x00e4, 'b', 'c', 0}, 
 * .        {0x00c4, 'B', 'C', 0},
 * .        {0x00c4, 'b', 'c', 0},
 * .        {'r', 'e', 's', 'u', 'm', 'e', 0}, 
 * .        {'r', 0x00e9, 's', 'u', 'm', 0x00e9, 0},
 * .        {'R', 'E', 'S', 'U', 'M', 'E', 0},
 * .        {'R', 0x00e9, 's', 'u', 'm', 0x00e9, 0}
 * .   };
 * .   UnicodeString *sortlist[8];
 * .   CollationKey sortKeys[8];
 * .   myCollation->setStrength(Collator::SECONDARY);
 * .   int i;
 * .   for (i = 0; i &lt; 8; i++) {
 * .        ErrorCode keyStatus = ZERO_ERROR;
 * .        sortlist[i] = new UnicodeString(sortchars[i]);
 * .        sortKeys[i] = myCollation->getCollationKey(*sortlist[i], sortKeys[i], keyStatus);
 * .        if (FAILURE(keyStatus)) { delete myCollation; return; }  // getCollationKey failed.
 * .   }
 * .   // query the "RESUME", "r�sum�", and "R�sum�" collation keys
 * .   // will return the objects that compares equal
 * .   if ((sortKeys[4] == sortKeys[5]) &&
 * .        (sortKeys[5] == sortKeys[6]) &&
 * .        (sortKeys[6] == sortKey2[7]))
 * .        printf("Test passes!\n");
 * </pre>
 * <p>Because Collator::compare()'s algorithm is complex, it is faster to sort
 * long lists of words by retrieving collation keys with Collator::getCollationKey().
 * You can then cache the collation keys and compare them using CollationKey::compareTo().
 * @see          Collator
 * @see          RuleBasedCollator
 * @version      1.3 12/18/96
 * @author       Helena Shih
 */
#ifdef NLS_MAC
#pragma export on
#endif
class T_COLLATE_API CollationKey {
public :
    /**
     * This creates an empty collation key based on the null string.  An empty 
     * collation key contains no sorting information.  When comparing two empty
     * collation keys, the result is Collator::EQUAL.  Comparing empty collation key
     * with non-empty collation key is always Collator::LESS.
     */
                                    CollationKey();
    /**
     * Creates a collation key based on the collation key values.  
     * @param values the collation key values
     * @param count number of collation key values
     * @see #createBits
     */
                                    CollationKey(const  t_uint8*    values,
                                                        t_int32     count);

    /**
     * Copy constructor.
     */
                                    CollationKey(const CollationKey& other);
    /** 
     * Sort key destructor.
     */
                                    ~CollationKey();

    /**
     * Assignment operator
     */
    const   CollationKey&           operator=(const CollationKey& other);

    /**
     * Compare if two collation keys are the same.
     * @param source the collation key to compare to.
     * @return Returns true if two collation keys are equal, false otherwise.
     */
            t_bool                  operator==(const CollationKey& source) const;

    /**
     * Compare if two collation keys are not the same.
     * @param source the collation key to compare to.
     * @return Returns true if two collation keys are different, false otherwise.
     */
            t_bool                  operator!=(const CollationKey& source) const;


    /** 
     * Extracts the collation key values.  
     * @param count the output parameter of number of collation key values
     */
            t_uint8*                toByteArray(    t_int32&    count) const;
            t_uint8*                createBits(     t_int32&    count) const;

    /**
     * Convenience method which does a string(bit-wise) comparison of the
     * two collation keys.
     * @param sourceKey source collation key
     * @param targetKey target collation key
     * @return Returns Collator::LESS if sourceKey &lt; targetKey, 
     * Collator::GREATER if sourceKey > targetKey and Collator::EQUAL
     * otherwise.
     * @see UnicodeString::compare
     */
    Collator::EComparisonResult    compareTo(const CollationKey& target) const;
    Collator::EComparisonResult    compare(const CollationKey& target) const;

    /**
     * Creates an integer that is unique to the collation key.  NOTE: this
     * is not the same as String.hashCode.
     * <p>Example of use:
     * <pre>
     * .    ErrorCode status = ZERO_ERROR;
     * .    Collator *myCollation = Collator::createInstance(Locale::US, status);
     * .    if (FAILURE(status)) return;
     * .    CollationKey key1, key2;
     * .    ErrorCode status1 = ZERO_ERROR, status2 = ZERO_ERROR;
     * .    myCollation->getCollationKey("abc", key1, status1);
     * .    if (FAILURE(status1)) { delete myCollation; return; }
     * .    myCollation->getCollationKey("ABC", key2, status2);
     * .    if (FAILURE(status2)) { delete myCollation; return; }
     * .    // key1.hashCode() != key2.hashCode()
     * </pre>
     * @return the hash value based on the string's collation order.
     * @see UnicodeString#hashCode
     */
            t_int32                 hashCode() const;

    const   UniChar*                getValues(t_int32& size) const;

private:
    /*
     * Creates a collation key with a string.
     */
                                    CollationKey(const UnicodeString& value);
    friend  class       RuleBasedCollator;
    UnicodeString       strValue;
};
#ifdef NLS_MAC
#pragma export off
#endif

inline const    CollationKey&
CollationKey::operator=(    const   CollationKey&   other)
{
    if (this != &other) {
        strValue = other.strValue;
    }
    return *this;
}

inline t_bool
CollationKey::operator==(const CollationKey& source) const
{
    return (this->strValue == source.strValue);
}

inline t_bool
CollationKey::operator!=(   const   CollationKey&   other) const
{
    return !(*this == other);
}

inline t_int32 
CollationKey::hashCode() const
{
    return (strValue.hashCode());
}

// Bitwise comparison for the collation keys.
inline Collator::EComparisonResult
CollationKey::compareTo(const CollationKey& target) const
{
    return (Collator::EComparisonResult)strValue.compare(target.strValue);
}

inline Collator::EComparisonResult  
CollationKey::compare(const CollationKey& target) const
{
    return compareTo(target);
}

inline  t_uint8*
CollationKey::createBits(       t_int32&    count) const
{
    return toByteArray(count);
}

inline const   UniChar* 
CollationKey::getValues(t_int32& size) const
{
    size = strValue.size();
    return ((const UniChar*)strValue);
}

#endif
