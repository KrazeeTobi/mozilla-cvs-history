/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/*
 * The contents of this file are subject to the Mozilla Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 * 
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 * 
 * The Original Code is Mozilla.
 * 
 * The Initial Developer of the Original Code is Netscape
 * Communications.  Portions created by Netscape Communications are
 * Copyright (C) 2001 by Netscape Communications.  All
 * Rights Reserved.
 * 
 * Contributor(s): 
 *   Vidur Apparao <vidur@netscape.com> (original author)
 */

#ifndef __nsSchemaPrivate_h__
#define __nsSchemaPrivate_h__

#include "nsISchema.h"

// XPCOM Includes
#include "nsCOMPtr.h"
#include "nsSupportsArray.h"
#include "nsHashtable.h"
#include "nsString.h"

#define NS_SCHEMA_2001_NAMESPACE "http://www.w3.org/2001/XMLSchema"
#define NS_SCHEMA_1999_NAMESPACE "http://www.w3.org/1999/XMLSchema"
#define NS_SOAP_1_1_ENCODING_NAMESPACE \
   "http://schemas.xmlsoap.org/soap/encoding/"
#define NS_SOAP_1_2_ENCODING_NAMESPACE \
   "http://www.w3.org/2001/09/soap-encoding"

class nsSchema : public nsISchema 
{
public:
  nsSchema(nsISchemaCollection* aCollection,
           const nsAReadableString& aTargetNamespace,
           const nsAReadableString& aSchemaNamespace);
  virtual ~nsSchema();

  NS_DECL_ISUPPORTS
  NS_DECL_NSISCHEMACOMPONENT
  NS_DECL_NSISCHEMA

  NS_IMETHOD AddType(nsISchemaType* aType);
  NS_IMETHOD AddAttribute(nsISchemaAttribute* aAttribute);
  NS_IMETHOD AddElement(nsISchemaElement* aElement);
  NS_IMETHOD AddAttributeGroup(nsISchemaAttributeGroup* aAttributeGroup);
  NS_IMETHOD AddModelGroup(nsISchemaModelGroup* aModelGroup);
  void DropCollectionReference();
  nsresult ResolveTypePlaceholder(nsISchemaType* aPlaceholder,
                                  nsISchemaType** aType);

protected:
  nsString mTargetNamespace;
  nsString mSchemaNamespace;
  nsSupportsArray mTypes;
  nsSupportsHashtable mTypesHash;
  nsSupportsArray mAttributes;
  nsSupportsHashtable mAttributesHash;
  nsSupportsArray mElements;
  nsSupportsHashtable mElementsHash;
  nsSupportsArray mAttributeGroups;
  nsSupportsHashtable mAttributeGroupsHash;
  nsSupportsArray mModelGroups;
  nsSupportsHashtable mModelGroupsHash;
  nsISchemaCollection* mCollection;  // [WEAK] it owns me
};

class nsSchemaComponentBase {
public:
  nsSchemaComponentBase(nsSchema* aSchema);
  virtual ~nsSchemaComponentBase();

  NS_IMETHOD GetTargetNamespace(nsAWritableString& aTargetNamespace);

protected:
  nsSchema* mSchema;  // [WEAK] It owns me
  // Used to prevent infinite recursion for cycles in the object graph
  PRPackedBool mIsResolved;
  PRPackedBool mIsCleared;
};

#define NS_IMPL_NSISCHEMACOMPONENT_USING_BASE                           \
  NS_IMETHOD GetTargetNamespace(nsAWritableString& aTargetNamespace) {  \
    return nsSchemaComponentBase::GetTargetNamespace(aTargetNamespace); \
  }                                                                     \
  NS_IMETHOD Resolve();                                                 \
  NS_IMETHOD Clear();

class nsSchemaBuiltinType : public nsISchemaBuiltinType
{
public:
  nsSchemaBuiltinType(PRUint16 aBuiltinType);
  virtual ~nsSchemaBuiltinType();

  NS_DECL_ISUPPORTS
  NS_DECL_NSISCHEMACOMPONENT
  NS_DECL_NSISCHEMATYPE
  NS_DECL_NSISCHEMASIMPLETYPE
  NS_DECL_NSISCHEMABUILTINTYPE

protected:
  PRUint16 mBuiltinType;
};

class nsSchemaListType : public nsSchemaComponentBase,
                         public nsISchemaListType
{
public:
  nsSchemaListType(nsSchema* aSchema, const nsAReadableString& aName);
  virtual ~nsSchemaListType();

  NS_DECL_ISUPPORTS
  NS_IMPL_NSISCHEMACOMPONENT_USING_BASE
  NS_DECL_NSISCHEMATYPE
  NS_DECL_NSISCHEMASIMPLETYPE
  NS_DECL_NSISCHEMALISTTYPE
  
  NS_IMETHOD SetListType(nsISchemaSimpleType* aListType);

protected:
  nsString mName;
  nsCOMPtr<nsISchemaSimpleType> mListType;
};

class nsSchemaUnionType : public nsSchemaComponentBase,
                          public nsISchemaUnionType
{
public:
  nsSchemaUnionType(nsSchema* aSchema, const nsAReadableString& aName);
  virtual ~nsSchemaUnionType();
  
  NS_DECL_ISUPPORTS
  NS_IMPL_NSISCHEMACOMPONENT_USING_BASE
  NS_DECL_NSISCHEMATYPE
  NS_DECL_NSISCHEMASIMPLETYPE
  NS_DECL_NSISCHEMAUNIONTYPE

  NS_IMETHOD AddUnionType(nsISchemaSimpleType* aUnionType);

protected:
  nsString mName;
  nsSupportsArray mUnionTypes;
};

class nsSchemaRestrictionType : public nsSchemaComponentBase,
                                public nsISchemaRestrictionType
{
public:
  nsSchemaRestrictionType(nsSchema* aSchema, const nsAReadableString& aName);
  virtual ~nsSchemaRestrictionType();

  NS_DECL_ISUPPORTS
  NS_IMPL_NSISCHEMACOMPONENT_USING_BASE
  NS_DECL_NSISCHEMATYPE
  NS_DECL_NSISCHEMASIMPLETYPE
  NS_DECL_NSISCHEMARESTRICTIONTYPE

  NS_IMETHOD SetBaseType(nsISchemaSimpleType* aBaseType);
  NS_IMETHOD AddFacet(nsISchemaFacet* aFacet);

protected:
  nsString mName;
  nsCOMPtr<nsISchemaSimpleType> mBaseType;
  nsSupportsArray mFacets;
};

class nsComplexTypeArrayInfo {
public:
  nsComplexTypeArrayInfo(nsISchemaType* aType, PRUint32 aDimension) :
    mType(aType), mDimension(aDimension) {}
  ~nsComplexTypeArrayInfo() {}

  void GetType(nsISchemaType** aType) { *aType = mType; NS_ADDREF(*aType); }
  PRUint32 GetDimension() { return mDimension; }

private:
  nsCOMPtr<nsISchemaType> mType;
  PRUint32 mDimension;
};

class nsSchemaComplexType : public nsSchemaComponentBase,
                            public nsISchemaComplexType
{
public:
  nsSchemaComplexType(nsSchema* aSchema, const nsAReadableString& aName,
                      PRBool aAbstract);
  virtual ~nsSchemaComplexType();

  NS_DECL_ISUPPORTS
  NS_IMPL_NSISCHEMACOMPONENT_USING_BASE
  NS_DECL_NSISCHEMATYPE
  NS_DECL_NSISCHEMACOMPLEXTYPE

  NS_IMETHOD SetContentModel(PRUint16 aContentModel);
  NS_IMETHOD SetDerivation(PRUint16 aDerivation, nsISchemaType* aBaseType);
  NS_IMETHOD SetSimpleBaseType(nsISchemaSimpleType* aSimpleBaseType);
  NS_IMETHOD SetModelGroup(nsISchemaModelGroup* aModelGroup);
  NS_IMETHOD AddAttribute(nsISchemaAttributeComponent* aAttribute);
  NS_IMETHOD SetArrayInfo(nsISchemaType* aType, PRUint32 aDimension);
  
protected:
  nsString mName;
  PRPackedBool mAbstract;
  PRUint16 mContentModel;
  PRUint16 mDerivation;
  nsCOMPtr<nsISchemaType> mBaseType;
  nsCOMPtr<nsISchemaSimpleType> mSimpleBaseType;
  nsCOMPtr<nsISchemaModelGroup> mModelGroup;
  nsSupportsArray mAttributes;
  nsSupportsHashtable mAttributesHash;
  nsComplexTypeArrayInfo* mArrayInfo;
};

class nsSchemaTypePlaceholder : public nsSchemaComponentBase,
                                public nsISchemaSimpleType
{
public:
  nsSchemaTypePlaceholder(nsSchema* aSchema, const nsAReadableString& aName);
  virtual ~nsSchemaTypePlaceholder();

  NS_DECL_ISUPPORTS
  NS_IMPL_NSISCHEMACOMPONENT_USING_BASE
  NS_DECL_NSISCHEMATYPE
  NS_DECL_NSISCHEMASIMPLETYPE

protected:
  nsString mName;
};

class nsSchemaParticleBase : public nsSchemaComponentBase
{
public:  
  nsSchemaParticleBase(nsSchema* aSchema);
  virtual ~nsSchemaParticleBase();

  NS_IMETHOD GetMinOccurs(PRUint32 *aMinOccurs);
  NS_IMETHOD GetMaxOccurs(PRUint32 *aMaxOccurs);

  NS_IMETHOD SetMinOccurs(PRUint32 aMinOccurs);
  NS_IMETHOD SetMaxOccurs(PRUint32 aMaxOccurs);

protected:
  PRUint32 mMinOccurs;
  PRUint32 mMaxOccurs;
};

#define NS_IMPL_NSISCHEMAPARTICLE_USING_BASE                           \
  NS_IMETHOD GetMinOccurs(PRUint32 *aMinOccurs) {                      \
    return nsSchemaParticleBase::GetMinOccurs(aMinOccurs);             \
  }                                                                    \
  NS_IMETHOD GetMaxOccurs(PRUint32 *aMaxOccurs) {                      \
    return nsSchemaParticleBase::GetMaxOccurs(aMaxOccurs);             \
  }                                                                    \
  NS_IMETHOD SetMinOccurs(PRUint32 aMinOccurs) {                       \
    return nsSchemaParticleBase::SetMinOccurs(aMinOccurs);             \
  }                                                                    \
  NS_IMETHOD SetMaxOccurs(PRUint32 aMaxOccurs) {                       \
    return nsSchemaParticleBase::SetMaxOccurs(aMaxOccurs);             \
  }                                                                    \
  NS_IMETHOD GetParticleType(PRUint16 *aParticleType);                 \
  NS_IMETHOD GetName(nsAWritableString& aName);                        

class nsSchemaModelGroup : public nsSchemaParticleBase,
                           public nsISchemaModelGroup
{
public:
  nsSchemaModelGroup(nsSchema* aSchema, 
                     const nsAReadableString& aName);
  virtual ~nsSchemaModelGroup();

  NS_DECL_ISUPPORTS
  NS_IMPL_NSISCHEMACOMPONENT_USING_BASE
  NS_IMPL_NSISCHEMAPARTICLE_USING_BASE
  NS_DECL_NSISCHEMAMODELGROUP

  NS_IMETHOD SetCompositor(PRUint16 aCompositor);
  NS_IMETHOD AddParticle(nsISchemaParticle* aParticle);

protected:
  nsString mName;
  PRUint16 mCompositor;
  nsSupportsArray mParticles;
};

class nsSchemaModelGroupRef : public nsSchemaParticleBase,
                              public nsISchemaModelGroup
{
public:
  nsSchemaModelGroupRef(nsSchema* aSchema, 
                        const nsAReadableString& aRef);
  virtual ~nsSchemaModelGroupRef();

  NS_DECL_ISUPPORTS
  NS_IMPL_NSISCHEMACOMPONENT_USING_BASE
  NS_IMPL_NSISCHEMAPARTICLE_USING_BASE
  NS_DECL_NSISCHEMAMODELGROUP

protected:
  nsString mRef;
  nsCOMPtr<nsISchemaModelGroup> mModelGroup;
};

class nsSchemaAnyParticle : public nsSchemaParticleBase,
                            public nsISchemaAnyParticle
{
public:
  nsSchemaAnyParticle(nsSchema* aSchema);
  virtual ~nsSchemaAnyParticle();

  NS_DECL_ISUPPORTS
  NS_IMPL_NSISCHEMACOMPONENT_USING_BASE
  NS_IMPL_NSISCHEMAPARTICLE_USING_BASE
  NS_DECL_NSISCHEMAANYPARTICLE

  NS_IMETHOD SetProcess(PRUint16 aProcess);
  NS_IMETHOD SetNamespace(const nsAReadableString& aNamespace);

protected:
  PRUint16 mProcess;
  nsString mNamespace;
};

class nsSchemaElement : public nsSchemaParticleBase,
                        public nsISchemaElement
{
public:
  nsSchemaElement(nsSchema* aSchema, const nsAReadableString& aName);
  virtual ~nsSchemaElement();

  NS_DECL_ISUPPORTS
  NS_IMPL_NSISCHEMACOMPONENT_USING_BASE
  NS_IMPL_NSISCHEMAPARTICLE_USING_BASE
  NS_DECL_NSISCHEMAELEMENT

  NS_IMETHOD SetType(nsISchemaType* aType);
  NS_IMETHOD SetConstraints(const nsAReadableString& aDefaultValue,
                            const nsAReadableString& aFixedValue);
  NS_IMETHOD SetFlags(PRBool aNillable, PRBool aAbstract);

protected:
  nsString mName;
  nsCOMPtr<nsISchemaType> mType;
  nsString mDefaultValue;
  nsString mFixedValue;
  PRPackedBool mNillable;
  PRPackedBool mAbstract;
};

class nsSchemaElementRef : public nsSchemaParticleBase,
                           public nsISchemaElement
{
public:
  nsSchemaElementRef(nsSchema* aSchema, const nsAReadableString& aRef);
  virtual ~nsSchemaElementRef();

  NS_DECL_ISUPPORTS
  NS_IMPL_NSISCHEMACOMPONENT_USING_BASE
  NS_IMPL_NSISCHEMAPARTICLE_USING_BASE
  NS_DECL_NSISCHEMAELEMENT

protected:
  nsString mRef;
  nsCOMPtr<nsISchemaElement> mElement;
};

class nsSchemaAttribute : public nsSchemaComponentBase,
                          public nsISchemaAttribute 
{
public:
  nsSchemaAttribute(nsSchema* aSchema, const nsAReadableString& aName);
  virtual ~nsSchemaAttribute();

  NS_DECL_ISUPPORTS
  NS_IMPL_NSISCHEMACOMPONENT_USING_BASE
  NS_DECL_NSISCHEMAATTRIBUTECOMPONENT
  NS_DECL_NSISCHEMAATTRIBUTE

  NS_IMETHOD SetType(nsISchemaSimpleType* aType);
  NS_IMETHOD SetConstraints(const nsAReadableString& aDefaultValue,
                            const nsAReadableString& aFixedValue);
  NS_IMETHOD SetUse(PRUint16 aUse);

protected:
  nsString mName;
  nsCOMPtr<nsISchemaSimpleType> mType;
  nsString mDefaultValue;
  nsString mFixedValue;
  PRUint16 mUse;
};

class nsSchemaAttributeRef : public nsSchemaComponentBase,
                             public nsISchemaAttribute 
{
public:
  nsSchemaAttributeRef(nsSchema* aSchema, const nsAReadableString& aRef);
  virtual ~nsSchemaAttributeRef();
  
  NS_DECL_ISUPPORTS
  NS_IMPL_NSISCHEMACOMPONENT_USING_BASE
  NS_DECL_NSISCHEMAATTRIBUTECOMPONENT
  NS_DECL_NSISCHEMAATTRIBUTE

  NS_IMETHOD SetConstraints(const nsAReadableString& aDefaultValue,
                            const nsAReadableString& aFixedValue);
  NS_IMETHOD SetUse(PRUint16 aUse);

protected:
  nsString mRef;
  nsCOMPtr<nsISchemaAttribute> mAttribute;
  nsString mDefaultValue;
  nsString mFixedValue;
  PRUint16 mUse;
};

class nsSchemaAttributeGroup : public nsSchemaComponentBase,
                               public nsISchemaAttributeGroup
{
public:
  nsSchemaAttributeGroup(nsSchema* aSchema, const nsAReadableString& aName);
  virtual ~nsSchemaAttributeGroup();
  
  NS_DECL_ISUPPORTS
  NS_IMPL_NSISCHEMACOMPONENT_USING_BASE
  NS_DECL_NSISCHEMAATTRIBUTECOMPONENT
  NS_DECL_NSISCHEMAATTRIBUTEGROUP
  
  NS_IMETHOD AddAttribute(nsISchemaAttributeComponent* aAttribute);

protected:
  nsString mName;
  nsSupportsArray mAttributes;
  nsSupportsHashtable mAttributesHash;
};

class nsSchemaAttributeGroupRef : public nsSchemaComponentBase,
                                  public nsISchemaAttributeGroup
{
public:
  nsSchemaAttributeGroupRef(nsSchema* aSchema, const nsAReadableString& aRef);
  virtual ~nsSchemaAttributeGroupRef();
  
  NS_DECL_ISUPPORTS
  NS_IMPL_NSISCHEMACOMPONENT_USING_BASE
  NS_DECL_NSISCHEMAATTRIBUTECOMPONENT
  NS_DECL_NSISCHEMAATTRIBUTEGROUP

protected:
  nsString mRef;
  nsCOMPtr<nsISchemaAttributeGroup> mAttributeGroup;
};

class nsSchemaAnyAttribute : public nsSchemaComponentBase,
                             public nsISchemaAnyAttribute
{
public:
  nsSchemaAnyAttribute(nsSchema* aSchema);
  virtual ~nsSchemaAnyAttribute();

  NS_DECL_ISUPPORTS
  NS_IMPL_NSISCHEMACOMPONENT_USING_BASE
  NS_DECL_NSISCHEMAATTRIBUTECOMPONENT
  NS_DECL_NSISCHEMAANYATTRIBUTE
  
  NS_IMETHOD SetProcess(PRUint16 aProcess);
  NS_IMETHOD SetNamespace(const nsAReadableString& aNamespace);

protected:
  PRUint16 mProcess;
  nsString mNamespace;
};

class nsSchemaFacet : public nsSchemaComponentBase,
                      public nsISchemaFacet
{
public:
  nsSchemaFacet(nsSchema* aSchema);
  virtual ~nsSchemaFacet();

  NS_DECL_ISUPPORTS
  NS_IMPL_NSISCHEMACOMPONENT_USING_BASE
  NS_DECL_NSISCHEMAFACET

  NS_IMETHOD SetFacetType(PRUint16 aFacetType);
  NS_IMETHOD SetIsFixed(PRBool aIsFixed);
  NS_IMETHOD SetValue(const nsAReadableString& aStrValue);
  NS_IMETHOD SetUintValue(PRUint32 aUintValue);
  NS_IMETHOD SetWhitespaceValue(PRUint16 aWhitespaceValue);

protected:
  PRUint16 mFacetType;
  PRPackedBool mIsFixed;
  nsString mStrValue;
  PRUint32 mUintValue;
  PRUint16 mWhitespaceValue;
};

class nsSOAPArray : public nsISchemaComplexType
{
public:
  nsSOAPArray(nsISchemaType* aAnyType);
  virtual ~nsSOAPArray();

  NS_DECL_ISUPPORTS
  NS_DECL_NSISCHEMACOMPONENT
  NS_DECL_NSISCHEMATYPE
  NS_DECL_NSISCHEMACOMPLEXTYPE

protected:
  nsCOMPtr<nsISchemaType> mAnyType;
};

class nsSOAPArrayType : public nsISchemaRestrictionType
{
public:
  nsSOAPArrayType();
  virtual ~nsSOAPArrayType();

  NS_DECL_ISUPPORTS
  NS_DECL_NSISCHEMACOMPONENT
  NS_DECL_NSISCHEMATYPE
  NS_DECL_NSISCHEMASIMPLETYPE
  NS_DECL_NSISCHEMARESTRICTIONTYPE
};

#define NS_SCHEMA_CID                              \
{ /* 58870ef5-cb65-4220-8317-dbe236059c58 */       \
 0x58870ef5, 0xcb65, 0x4220,                       \
 {0x83, 0x17, 0xdb, 0xe2, 0x36, 0x05, 0x9c, 0x58}}

#define NS_SCHEMA_CONTRACTID    \
"@mozilla.org/xmlextras/schemas/schema;1"

#define NS_SCHEMABUILTINTYPE_CID                   \
{ /* c1db07bc-1095-4a44-9ed6-f4a00a116b4a */       \
 0xc1db07bc, 0x1095, 0x4a44,                       \
 {0x9e, 0xd6, 0xf4, 0xa0, 0x0a, 0x11, 0x6b, 0x4a}}

#define NS_SCHEMABUILTINTYPE_CONTRACTID    \
"@mozilla.org/xmlextras/schemas/schemabuiltintype;1"

#define NS_SCHEMALISTTYPE_CID                      \
{ /* 2053b685-e541-45f9-bb00-9d8897b8887d */       \
 0x2053b685, 0xe541, 0x45f9,                       \
 {0xbb, 0x00, 0x9d, 0x88, 0x97, 0xb8, 0x88, 0x7d}}

#define NS_SCHEMALISTTYPE_CONTRACTID    \
"@mozilla.org/xmlextras/schemas/schemalisttype;1"

#define NS_SCHEMAUNIONTYPE_CID                     \
{ /* ef8b74ae-da9a-4fa2-b6f1-b587b02b2262 */       \
 0xef8b74ae, 0xda9a, 0x4fa2,                       \
 {0xb6, 0xf1, 0xb5, 0x87, 0xb0, 0x2b, 0x22, 0x62}}

#define NS_SCHEMAUNIONTYPE_CONTRACTID    \
"@mozilla.org/xmlextras/schemas/schemauniontype;1"

#define NS_SCHEMARESTRICTIONTYPE_CID               \
{ /* 38622f82-c10e-48cc-aea9-1a6ed31078e5 */       \
 0x38622f82, 0xc10e, 0x48cc,                       \
 {0xae, 0xa9, 0x1a, 0x6e, 0xd3, 0x10, 0x78, 0xe5}}

#define NS_SCHEMARESTRICTIONTYPE_CONTRACTID    \
"@mozilla.org/xmlextras/schemas/schemarestrictiontype;1"

#define NS_SCHEMACOMPLEXTYPE_CID                   \
{ /* e8266b17-b5ed-4228-be18-473dbfe68f67 */       \
 0xe8266b17, 0xb5ed, 0x4228,                       \
 {0xbe, 0x18, 0x47, 0x3d, 0xbf, 0xe6, 0x8f, 0x67}}

#define NS_SCHEMACOMPLEXTYPE_CONTRACTID    \
"@mozilla.org/xmlextras/schemas/schemacomplextype;1"

#define NS_SCHEMATYPEPLACEHOLDER_CID               \
{ /* 50e026ef-e0ef-408c-a5ed-8492d7f3604e */       \
 0x50e026ef, 0xe0ef, 0x408c,                       \
 {0xa5, 0xed, 0x84, 0x92, 0xd7, 0xf3, 0x60, 0x4e}}

#define NS_SCHEMATYPEPLACEHOLDER_CONTRACTID    \
"@mozilla.org/xmlextras/schemas/schematypeplaceholder;1"

#define NS_SCHEMAMODELGROUP_CID                    \
{ /* 8f75863d-a724-40d5-a271-72bb67ef2105 */       \
 0x8f75863d, 0xa724, 0x40d5,                       \
 {0xa2, 0x71, 0x72, 0xbb, 0x67, 0xef, 0x21, 0x05}}

#define NS_SCHEMAMODELGROUP_CONTRACTID    \
"@mozilla.org/xmlextras/schemas/schemamodelgroup;1"

#define NS_SCHEMAMODELGROUPREF_CID                 \
{ /* 2e9dca3a-5684-44a1-99cb-94f8aef95c03 */       \
 0x2e9dca3a, 0x5684, 0x44a1,                       \
 {0x99, 0xcb, 0x94, 0xf8, 0xae, 0xf9, 0x5c, 0x03}}

#define NS_SCHEMAMODELGROUPREF_CONTRACTID    \
"@mozilla.org/xmlextras/schemas/schemamodelgroupref;1"

#define NS_SCHEMAANYPARTICLE_CID                   \
{ /* ab78c787-a356-483a-9153-92c50ba8a80d */       \
 0xab78c787, 0xa356, 0x483a,                       \
 {0x91, 0x53, 0x92, 0xc5, 0x0b, 0xa8, 0xa8, 0x0d}}

#define NS_SCHEMAANYPARTICLE_CONTRACTID    \
"@mozilla.org/xmlextras/schemas/schemaanyparticle;1"

#define NS_SCHEMAELEMENT_CID                       \
{ /* 9c3e5c69-2d47-475d-9cc8-1d6905d0f7dc */       \
 0x9c3e5c69, 0x2d47, 0x475d,                       \
 {0x9c, 0xc8, 0x1d, 0x69, 0x05, 0xd0, 0xf7, 0xdc}}

#define NS_SCHEMAELEMENT_CONTRACTID    \
"@mozilla.org/xmlextras/schemas/schemaelement;1"

#define NS_SCHEMAELEMENTREF_CID                    \
{ /* a62b4292-5d30-4085-9dbb-3f478d639188 */       \
 0xa62b4292, 0x5d30, 0x4085,                       \
 {0x9d, 0xbb, 0x3f, 0x47, 0x8d, 0x63, 0x91, 0x88}}

#define NS_SCHEMAELEMENTREF_CONTRACTID    \
"@mozilla.org/xmlextras/schemas/schemaelementref;1"

#define NS_SCHEMAATTRIBUTE_CID                     \
{ /* 7b3820e6-0fd9-4025-9ba8-73f3a1ab1ba6 */       \
 0x7b3820e6, 0x0fd9, 0x4025,                       \
 {0x9b, 0xa8, 0x73, 0xf3, 0xa1, 0xab, 0x1b, 0xa6}}

#define NS_SCHEMAATTRIBUTE_CONTRACTID    \
"@mozilla.org/xmlextras/schemas/schemaattribute;1"

#define NS_SCHEMAATTRIBUTEREF_CID                  \
{ /* 53fe1960-e0df-487f-a26c-a3073a8e863a */       \
 0x53fe1960, 0xe0df, 0x487f,                       \
 {0xa2, 0x6c, 0xa3, 0x07, 0x3a, 0x8e, 0x86, 0x3a}}

#define NS_SCHEMAATTRIBUTEREF_CONTRACTID    \
"@mozilla.org/xmlextras/schemas/schemaattributeref;1"

#define NS_SCHEMAATTRIBUTEGROUP_CID                \
{ /* 1f3d1ad4-2501-4097-9293-6e40890cf72a */       \
 0x1f3d1ad4, 0x2501, 0x4097,                       \
 {0x92, 0x93, 0x6e, 0x40, 0x89, 0x0c, 0xf7, 0x2a}}

#define NS_SCHEMAATTRIBUTEGROUP_CONTRACTID    \
"@mozilla.org/xmlextras/schemas/schemaattributegroup;1"

#define NS_SCHEMAATTRIBUTEGROUPREF_CID             \
{ /* 99596213-d550-4b46-ae04-7dd38dd7abff */       \
 0x99596213, 0xd550, 0x4b46,                       \
 {0xae, 0x04, 0x7d, 0xd3, 0x8d, 0xd7, 0xab, 0xff}}

#define NS_SCHEMAATTRIBUTEGROUPREF_CONTRACTID    \
"@mozilla.org/xmlextras/schemas/schemaattributegroupref;1"

#define NS_SCHEMAANYATTRIBUTE_CID                  \
{ /* 4e248fa3-79d7-4821-bc69-63e5275e879c */       \
 0x4e248fa3, 0x79d7, 0x4821,                       \
 {0xbc, 0x69, 0x63, 0xe5, 0x27, 0x5e, 0x87, 0x9c}}

#define NS_SCHEMAANYATTRIBUTE_CONTRACTID    \
"@mozilla.org/xmlextras/schemas/schemaanyattribute;1"

#define NS_SCHEMAFACET_CID                         \
{ /* fbaa0537-f931-4160-86a0-de849de08635 */       \
 0xfbaa0537, 0xf931, 0x4160,                       \
 {0x86, 0xa0, 0xde, 0x84, 0x9d, 0xe0, 0x86, 0x35}}

#define NS_SCHEMAFACET_CONTRACTID    \
"@mozilla.org/xmlextras/schemas/schemafacet;1"

#define NS_SOAPARRAY_CID                         \
{ /* 0df92b09-36d6-4bed-ac6f-dbc195e35218 */       \
 0x0df92b09, 0x36d6, 0x4bed,                       \
 {0xac, 0x6f, 0xdb, 0xc1, 0x95, 0xe3, 0x52, 0x18}}

#define NS_SOAPARRAY_CONTRACTID    \
"@mozilla.org/xmlextras/schemas/soaparray;1"

#define NS_SOAPARRAYTYPE_CID                       \
{ /* 9df7bcdb-3676-49dd-b5b9-4257dc326231 */       \
 0x9df7bcdb, 0x3676, 0x49dd,                       \
 {0xb5, 0xb9, 0x42, 0x57, 0xdc, 0x32, 0x62, 0x31}}

#define NS_SOAPARRAYTYPE_CONTRACTID    \
"@mozilla.org/xmlextras/schemas/soaparraytype;1"

#endif // __nsSchemaPrivate_h__


