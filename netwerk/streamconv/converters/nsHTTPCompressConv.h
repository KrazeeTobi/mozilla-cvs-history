/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * The contents of this file are subject to the Netscape Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code is mozilla.org code.
 *
 * The Initial Developer of the Original Code is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s): 
 */

#if !defined (__nsHTTPCompressConv__h__)
#define	__nsHTTPCompressConv__h__	1

#include "nsIStreamConverter.h"
#include "nsIFactory.h"
#include "nsCOMPtr.h"

#define NS_HTTPCOMPRESSCONVERTER_CID                \
{                                                   \
    /* 66230b2b-17fa-4bd3-abf4-07986151022d */      \
    0x66230b2b,                                     \
    0x17fa,                                         \
    0x4bd3,                                         \
    {0xab, 0xf4, 0x07, 0x98, 0x61, 0x51, 0x02, 0x2d}\
};

static NS_DEFINE_CID(kHTTPCompressConverterCID, NS_HTTPCOMPRESSCONVERTER_CID);

#define	HTTP_DEFLATE_TYPE		"deflate"
#define	HTTP_GZIP_TYPE	        "gzip"
#define	HTTP_X_GZIP_TYPE	    "x-gzip"
#define	HTTP_COMPRESS_TYPE	    "compress"
#define	HTTP_X_COMPRESS_TYPE	"x-compress"
#define	HTTP_IDENTITY_TYPE	    "identity"
#define	HTTP_UNCOMPRESSED_TYPE	"uncompressed"

typedef enum    {
        HTTP_COMPRESS_GZIP,
        HTTP_COMPRESS_DEFLATE,
        HTTP_COMPRESS_COMPRESS,
        HTTP_COMPRESS_IDENTITY
    }   CompressMode;

class nsHTTPCompressConv	: public nsIStreamConverter	{
public:
    // nsISupports methods
    NS_DECL_ISUPPORTS

	NS_DECL_NSISTREAMOBSERVER
    NS_DECL_NSISTREAMLISTENER

    // nsIStreamConverter methods
    NS_DECL_NSISTREAMCONVERTER


    nsHTTPCompressConv ();
    virtual ~nsHTTPCompressConv ();

private:

    nsIStreamListener   *mListener; // this guy gets the converted data via his OnDataAvailable ()
	CompressMode        mMode;

    unsigned char *mOutBuffer;
    unsigned char *mInpBuffer;

    PRUint32	mOutBufferLen;
    PRUint32	mInpBufferLen;
	
    nsCOMPtr<nsISupports>   mAsyncConvContext;

    nsresult do_OnDataAvailable (nsIChannel *aChannel, nsISupports *aContext, PRUint32 aSourceOffset, char *buffer, PRUint32 aCount);
};


#endif
