/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 *
 * The contents of this file are subject to the Netscape Public License
 * Version 1.0 (the "NPL"); you may not use this file except in
 * compliance with the NPL.  You may obtain a copy of the NPL at
 * http://www.mozilla.org/NPL/
 *
 * Software distributed under the NPL is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the NPL
 * for the specific language governing rights and limitations under the
 * NPL.
 *
 * The Initial Developer of this code under the NPL is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation.  All Rights
 * Reserved.
 */

#if defined(XP_PC)
#include <windows.h> // Interlocked increment...
#endif

#include "nspr.h"
#include "nsCRT.h"
#include "nsCOMPtr.h"
#include "nsIServiceManager.h"
#include "nscore.h"
#include "netCore.h"
#include "nsIStreamListener.h"
#include "nsSocketTransport.h"
#include "nsSocketTransportService.h"
#include "nsIBuffer.h"
#include "nsIBufferInputStream.h"
#include "nsIBufferOutputStream.h"
#include "nsAutoLock.h"

static NS_DEFINE_CID(kEventQueueService, NS_EVENTQUEUESERVICE_CID);


//
// This is the State table which maps current state to next state
// for each socket operation...
//
nsSocketState gStateTable[eSocketOperation_Max][eSocketState_Max] = {
  // eSocketOperation_None:
  {
    eSocketState_Error,         // Created        -> Error
    eSocketState_Error,         // WaitDNS        -> Error
    eSocketState_Error,         // Closed         -> Error
    eSocketState_Error,         // WaitConnect    -> Error
    eSocketState_Error,         // Connected      -> Error
    eSocketState_Error,         // WaitReadWrite  -> Error
    eSocketState_Error,         // DoneRead       -> Error
    eSocketState_Error,         // DoneWrite      -> Error
    eSocketState_Error,         // Done           -> Error
    eSocketState_Error,         // Timeout        -> Error
    eSocketState_Error          // Error          -> Error
  },
  // eSocketOperation_Connect:
  {
    eSocketState_WaitDNS,       // Created        -> WaitDNS
    eSocketState_Closed,        // WaitDNS        -> Closed
    eSocketState_WaitConnect,   // Closed         -> WaitConnect
    eSocketState_Connected,     // WaitConnect    -> Connected
    eSocketState_Connected,     // Connected      -> Done
    eSocketState_Error,         // WaitReadWrite  -> Error
    eSocketState_Error,         // DoneRead       -> Error
    eSocketState_Error,         // DoneWrite      -> Error
    eSocketState_Connected,     // Done           -> Connected
    eSocketState_Error,         // Timeout        -> Error
    eSocketState_Closed         // Error          -> Closed
  },
  // eSocketOperation_ReadWrite:
  {
    eSocketState_WaitDNS,       // Created        -> WaitDNS
    eSocketState_Closed,        // WaitDNS        -> Closed
    eSocketState_WaitConnect,   // Closed         -> WaitConnect
    eSocketState_Connected,     // WaitConenct    -> Connected
    eSocketState_WaitReadWrite, // Connected      -> WaitReadWrite
    eSocketState_Done,          // WaitReadWrite  -> Done
    eSocketState_Connected,     // DoneRead       -> Connected
    eSocketState_Connected,     // DoneWrite      -> Connected
    eSocketState_Connected,     // Done           -> Connected
    eSocketState_Error,         // Timeout        -> Error
    eSocketState_Closed         // Error          -> Closed
  },
};

//
// This is the timeout value (in milliseconds) for calls to PR_Connect(...).
//
// The gConnectTimeout gets initialized the first time a nsSocketTransport
// is created...  This interval is then passed to all PR_Connect() calls...
//
#define CONNECT_TIMEOUT_IN_MS 20

static int gTimeoutIsInitialized = 0;
static PRIntervalTime gConnectTimeout = PR_INTERVAL_NO_TIMEOUT;

//
// This is the global buffer used by all nsSocketTransport instances when
// reading from or writing to the network.
//
static char gIOBuffer[MAX_IO_BUFFER_SIZE];

#if defined(PR_LOGGING)
//
// Log module for SocketTransport logging...
//
// To enable logging (see prlog.h for full details):
//
//    set NSPR_LOG_MODULES=nsSocketTransport:5
//    set NSPR_LOG_FILE=nspr.log
//
// this enables PR_LOG_DEBUG level information and places all output in
// the file nspr.log
//
PRLogModuleInfo* gSocketLog = nsnull;

#endif /* PR_LOGGING */

nsSocketTransport::nsSocketTransport()
{
  NS_INIT_REFCNT();

  PR_INIT_CLIST(&mListLink);

  mHostName     = nsnull;
  mPort         = 0;
  mSocketFD     = nsnull;
  mLock         = nsnull;

  mSuspendCount = 0;
  mIsWaitingForRead = PR_FALSE;

  mCurrentState = eSocketState_Created;
  mOperation    = eSocketOperation_None;
  mSelectFlags  = 0;

  mReadBuffer   = nsnull;
  mReadStream   = nsnull;
  mReadContext  = nsnull;
  mReadListener = nsnull;

  mWriteStream    = nsnull;
  mWriteContext   = nsnull;
  mWriteObserver  = nsnull;

  mWriteCount     = 0;
  mSourceOffset   = 0;
  mService        = nsnull;
  mLoadAttributes = LOAD_NORMAL;

  //
  // Set up Internet defaults...
  //
  memset(&mNetAddress, 0, sizeof(mNetAddress));
	PR_InitializeNetAddr(PR_IpAddrNull, 0, &mNetAddress);

  //
  // Initialize the global connect timeout value if necessary...
  //
  if (0 == gTimeoutIsInitialized) {
    gConnectTimeout = PR_MillisecondsToInterval(CONNECT_TIMEOUT_IN_MS);
    gTimeoutIsInitialized = 1;
  }

#if defined(PR_LOGGING)
  //
  // Initialize the global PRLogModule for socket transport logging 
  // if necessary...
  //
  if (nsnull == gSocketLog) {
    gSocketLog = PR_NewLogModule("nsSocketTransport");
  }
#endif /* PR_LOGGING */

  PR_LOG(gSocketLog, PR_LOG_DEBUG, 
         ("Creating nsSocketTransport [this=%x].\n", this));

}


nsSocketTransport::~nsSocketTransport()
{
  PR_LOG(gSocketLog, PR_LOG_DEBUG, 
         ("Deleting nsSocketTransport [this=%x].\n", this));

  NS_IF_RELEASE(mReadListener);
  NS_IF_RELEASE(mReadStream);
  NS_IF_RELEASE(mReadBuffer);
  NS_IF_RELEASE(mReadContext);

  NS_IF_RELEASE(mWriteObserver);
  NS_IF_RELEASE(mWriteStream);
  NS_IF_RELEASE(mWriteContext);
  
  NS_IF_RELEASE(mService);

  if (mHostName) {
    nsCRT::free(mHostName);
  }

  if (mSocketFD) {
    PR_Close(mSocketFD);
    mSocketFD = nsnull;
  }

  if (mLock) {
    PR_DestroyLock(mLock);
    mLock = nsnull;
  }
}


nsresult nsSocketTransport::Init(nsSocketTransportService* aService,
                                 const char* aHost, 
                                 PRInt32 aPort)
{
  nsresult rv = NS_OK;

  mService = aService;
  NS_ADDREF(mService);

  mPort = aPort;
  if (aHost) {
    mHostName = nsCRT::strdup(aHost);
    if (!mHostName) {
      rv = NS_ERROR_OUT_OF_MEMORY;
    }
  } 
  // aHost was nsnull...
  else {
    rv = NS_ERROR_NULL_POINTER;
  }

  //
  // Create the lock used for synchronizing access to the transport instance.
  //
  if (NS_SUCCEEDED(rv)) {
    mLock = PR_NewLock();
    if (!mLock) {
      rv = NS_ERROR_OUT_OF_MEMORY;
    }
  }

  PR_LOG(gSocketLog, PR_LOG_DEBUG, 
         ("Initializing nsSocketTransport [this=%x].  rv = %x. \t"
          "aHost = %s.\t"
          "aPort = %d.\n",
          this, rv, aHost, aPort));

  return rv;
}


nsresult nsSocketTransport::Process(PRInt16 aSelectFlags)
{
  nsresult rv = NS_OK;
  PRBool done = PR_FALSE;

  //
  // Enter the socket transport lock...  
  // This lock protects access to socket transport member data...
  //
  nsAutoLock lock(mLock);

  PR_LOG(gSocketLog, PR_LOG_DEBUG, 
         ("+++ Entering nsSocketTransport::Process() [this=%x].\t"
          "aSelectFlags = %x.\t"
          "CurrentState = %d\n",
          this, aSelectFlags, mCurrentState));

  while (!done)
  {
    //
    // If the transport has been suspended, then return NS_OK immediately...
    // This removes the transport from the select list...
    //
    if (mSuspendCount) {
      PR_LOG(gSocketLog, PR_LOG_DEBUG, 
             ("Transport [this=%x] is suspended.\n", this));
  
      done = PR_TRUE;
      rv = NS_OK;
      continue;
    }

    switch (mCurrentState) {
      case eSocketState_Created:
      case eSocketState_Closed:
        break;

      case eSocketState_Connected:
        // Fire a notification for read...
        if (mReadListener) {
          mReadListener->OnStartRequest(this, mReadContext);
        }
        // Fire a notification for write...
        if (mWriteObserver) {
          mWriteObserver->OnStartRequest(this, mWriteContext);
        }
        break;

      case eSocketState_Done:
      case eSocketState_Error:
      case eSocketState_DoneRead:
      case eSocketState_DoneWrite:
        PR_LOG(gSocketLog, PR_LOG_DEBUG, 
               ("Transport [this=%x] is in done state %d.\n", this, mCurrentState));

        // Fire a notification that the read has finished...
        if (eSocketState_DoneWrite != mCurrentState) {
          if (mReadListener) {
            mReadListener->OnStopRequest(this, mReadContext, rv, nsnull);
            NS_RELEASE(mReadListener);
            NS_IF_RELEASE(mReadContext);
          }
          NS_IF_RELEASE(mReadStream);
          // XXX: The buffer should be reused...
          NS_IF_RELEASE(mReadBuffer);
        }

        // Fire a notification that the write has finished...
        if (eSocketState_DoneRead != mCurrentState) {
          if (mWriteObserver) {
            mWriteObserver->OnStopRequest(this, mWriteContext, rv, nsnull);
            NS_RELEASE(mWriteObserver);
            NS_IF_RELEASE(mWriteContext);
          }
          NS_IF_RELEASE(mWriteStream);
        }

        //
        // Set up the connection for the next operation...
        //
        if (mReadStream || mWriteStream) {
          mCurrentState = eSocketState_WaitReadWrite;
        } else {
          mCurrentState = gStateTable[mOperation][mCurrentState];
          mOperation    = eSocketOperation_None;
          done = PR_TRUE;
        }
        continue;

      case eSocketState_WaitDNS:
        rv = doResolveHost();
        break;

      case eSocketState_WaitConnect:
        rv = doConnection(aSelectFlags);
        break;

      case eSocketState_WaitReadWrite:
        if (mReadStream) {
          rv = doRead(aSelectFlags);
          if (NS_OK == rv) {
            mCurrentState = eSocketState_DoneRead;
            continue;
          }
        }

        if (NS_SUCCEEDED(rv) && mWriteStream) {
          rv = doWrite(aSelectFlags);
          if (NS_OK == rv) {
            mCurrentState = eSocketState_DoneWrite;
            continue;
          }
        }
        break;

      case eSocketState_Timeout:
        NS_ASSERTION(0, "Unexpected state...");
        rv = NS_ERROR_FAILURE;
        break;

      default:
        NS_ASSERTION(0, "Unexpected state...");
        rv = NS_ERROR_FAILURE;
        break;
    }
    //
    // If the current state has successfully completed, then move to the
    // next state for the current operation...
    //
    if (NS_OK == rv) {
      mCurrentState = gStateTable[mOperation][mCurrentState];
    } 
    else if (NS_FAILED(rv)) {
      mCurrentState = eSocketState_Error;
    }
    else if (NS_BASE_STREAM_WOULD_BLOCK == rv) {
      done = PR_TRUE;
    }
    // 
    // Any select flags are *only* valid the first time through the loop...
    // 
    aSelectFlags = 0;
  }

  PR_LOG(gSocketLog, PR_LOG_DEBUG, 
         ("--- Leaving nsSocketTransport::Process() [this=%x]. rv = %x.\t"
          "CurrentState = %d\n\n",
          this, rv, mCurrentState));

  return rv;
}


//-----
//
//  doResolveHost:
//
//  This method is called while holding the SocketTransport lock.  It is
//  always called on the socket transport thread...
//
//  Return Codes:
//    NS_OK
//    NS_ERROR_HOST_NOT_FOUND
//    NS_ERROR_FAILURE
//
//-----
nsresult nsSocketTransport::doResolveHost(void)
{
  PRStatus status;
  nsresult rv = NS_OK;

  NS_ASSERTION(eSocketState_WaitDNS == mCurrentState, "Wrong state.");

  PR_LOG(gSocketLog, PR_LOG_DEBUG, 
         ("+++ Entering nsSocketTransport::doResolveHost() [this=%x].\n", 
          this));

  //
  // Initialize the port used for the connection...
  //
  // XXX: The list of ports must be restricted - see net_bad_ports_table[] in 
  //      mozilla/network/main/mkconect.c
  //
  mNetAddress.inet.port = PR_htons(mPort);

  //
  // Resolve the address of the given host...
  //
  char dbbuf[PR_NETDB_BUF_SIZE];
  PRHostEnt hostEnt;

  status = PR_GetHostByName(mHostName, dbbuf, sizeof(dbbuf), &hostEnt);
  if (PR_SUCCESS == status) {
    if (hostEnt.h_addr_list) {
      memcpy(&mNetAddress.inet.ip, hostEnt.h_addr_list[0], 
                                   sizeof(mNetAddress.inet.ip));
    } else {
      // XXX: What should happen here?  The GetHostByName(...) succeeded but 
      //      there are *no* A records...
    }

    PR_LOG(gSocketLog, PR_LOG_DEBUG, 
           ("Host name resolved [this=%x].  Name is %s\n", this, mHostName));
  } 
  // DNS lookup failed...
  else {
    PR_LOG(gSocketLog, PR_LOG_ERROR, 
           ("Host name resolution FAILURE [this=%x].\n", this));

    rv = NS_ERROR_FAILURE;
  }

  PR_LOG(gSocketLog, PR_LOG_DEBUG, 
         ("--- Leaving nsSocketTransport::doResolveHost() [this=%x].\t"
          "rv = %x.\n\n",
          this, rv));

  return rv;
}

//-----
//
//  doConnection:
//
//  This method is called while holding the SocketTransport lock.  It is
//  always called on the socket transport thread...
//
//  Return values:
//    NS_OK
//    NS_BASE_STREAM_WOULD_BLOCK
//
//    NS_ERROR_CONNECTION_REFUSED
//    NS_ERROR_FAILURE
//    NS_ERROR_OUT_OF_MEMORY
//
//-----
nsresult nsSocketTransport::doConnection(PRInt16 aSelectFlags)
{
  PRStatus status;
  nsresult rv = NS_OK;

  NS_ASSERTION(eSocketState_WaitConnect == mCurrentState, "Wrong state.");

  PR_LOG(gSocketLog, PR_LOG_DEBUG, 
         ("+++ Entering nsSocketTransport::doConnection() [this=%x].\t"
          "aSelectFlags = %x.\n",
          this, aSelectFlags));

  //
  // Step 1:
  //    Create a new TCP socket structure (if necessary)...
  //
  if (!mSocketFD) {
    mSocketFD = PR_NewTCPSocket();
    if (mSocketFD) {
      PRSocketOptionData opt;

      // Make the socket non-blocking...
      opt.option = PR_SockOpt_Nonblocking;
      opt.value.non_blocking = PR_TRUE;
      status = PR_SetSocketOption(mSocketFD, &opt);
      if (PR_SUCCESS != status) {
        rv = NS_ERROR_FAILURE;
      }

      // XXX: Is this still necessary?
#if defined(XP_WIN16) || (defined(XP_OS2) && !defined(XP_OS2_DOUGSOCK))
      opt.option = PR_SockOpt_Linger;
      opt.value.linger.polarity = PR_TRUE;
      opt.value.linger.linger = PR_INTERVAL_NO_WAIT;
      PR_SetSocketOption(*sock, &opt);
#endif /* XP_WIN16 || XP_OS2*/
    } 
    else {
      rv = NS_ERROR_OUT_OF_MEMORY;
    }
  }

  //
  // Step 2:
  //    Initiate the connect() to the host...  
  //
  //    This is only done the first time doConnection(...) is called.
  //    If aSelectFlags == 0 then this is the first time... Otherwise,
  //    PR_Poll(...) would have set the flags non-zero.
  //
  if (NS_SUCCEEDED(rv) && (0 == aSelectFlags)) {
    status = PR_Connect(mSocketFD, &mNetAddress, gConnectTimeout);
    if (PR_SUCCESS != status) {
      PRErrorCode code = PR_GetError();
      //
      // If the PR_Connect(...) would block, then return WOULD_BLOCK...
      // It is the callers responsibility to place the transport on the
      // select list of the transport thread...
      //
      if ((PR_WOULD_BLOCK_ERROR == code) || 
          (PR_IN_PROGRESS_ERROR == code)) {

        // Set up the select flags for connect...
        mSelectFlags = (PR_POLL_READ | PR_POLL_EXCEPT | PR_POLL_WRITE);
        rv = NS_BASE_STREAM_WOULD_BLOCK;
      } 
      //
      // If the socket is already connected, then return success...
      //
      else if (PR_IS_CONNECTED_ERROR == code) {
        rv = NS_OK;
      }
      //
      // The connection was refused...
      //
      else {
        // Connection refused...
        PR_LOG(gSocketLog, PR_LOG_ERROR, 
               ("Connection Refused [this=%x].  PRErrorCode = %x\n",
                this, code));

        rv = NS_ERROR_CONNECTION_REFUSED;
      }
    }
  }
  //
  // Step 3:
  //    Process the flags returned by PR_Poll() if any...
  //
  else if (NS_SUCCEEDED(rv) && aSelectFlags) {
    if (PR_POLL_EXCEPT & aSelectFlags) {
      PR_LOG(gSocketLog, PR_LOG_ERROR, 
             ("Connection Refused via PR_POLL_EXCEPT. [this=%x].\n", this));

      rv = NS_ERROR_CONNECTION_REFUSED;
    }
    //
    // The connection was successful...  
    //
    // PR_Poll(...) returns PR_POLL_WRITE to indicate that the connection is
    // established...
    //
    else if (PR_POLL_WRITE & aSelectFlags) {
      rv = NS_OK;
    }
  }

  PR_LOG(gSocketLog, PR_LOG_DEBUG, 
         ("--- Leaving nsSocketTransport::doConnection() [this=%x].\t"
          "rv = %x.\n\n",
          this, rv));

  return rv;
}


NS_METHOD
nsReadFromSocket(void* closure,
                 char* toRawSegment,
                 PRUint32 offset,
                 PRUint32 count,
                 PRUint32 *readCount)
{
  nsresult rv = NS_OK;
  PRInt32 len;
  PRErrorCode code;

  PRFileDesc* fd = (PRFileDesc*)closure;

  *readCount = 0;

  len = PR_Read(fd, toRawSegment, count);
  if (len > 0) {
    *readCount = (PRUint32)len;
  } 
  //
  // The read operation has completed...
  //
  else if (len == 0) {
    rv = NS_BASE_STREAM_EOF;
  }
  //
  // Error...
  //
  else {
    code = PR_GetError();

    if (PR_WOULD_BLOCK_ERROR == code) {
      rv = NS_BASE_STREAM_WOULD_BLOCK;
    } 
    else {
      PR_LOG(gSocketLog, PR_LOG_ERROR, 
             ("PR_Read() failed. PRErrorCode = %x\n", code));

      // XXX: What should this error code be?
      rv = NS_ERROR_FAILURE;
    }
  }

  PR_LOG(gSocketLog, PR_LOG_DEBUG, 
         ("nsReadFromSocket [fd=%x].  rv = %x. Buffer space = %d.  Bytes read =%d\n",
                  fd, rv, count, *readCount));

  return rv;
}

//-----
//
//  doRead:
//
//  This method is called while holding the SocketTransport lock.  It is
//  always called on the socket transport thread...
//
//  Return values:
//    NS_OK
//    NS_BASE_STREAM_WOULD_BLOCK
//
//    NS_ERROR_FAILURE
//
//-----
nsresult nsSocketTransport::doRead(PRInt16 aSelectFlags)
{
  PRUint32 totalBytesWritten;
  nsresult rv = NS_OK;

  NS_ASSERTION(eSocketState_WaitReadWrite == mCurrentState, "Wrong state.");

  PR_LOG(gSocketLog, PR_LOG_DEBUG, 
         ("+++ Entering nsSocketTransport::doRead() [this=%x].\t"
          "aSelectFlags = %x.\n",
          this, aSelectFlags));

  //
  // Check for an error during PR_Poll(...)
  //
  if (PR_POLL_EXCEPT & aSelectFlags) {
    PR_LOG(gSocketLog, PR_LOG_ERROR, 
           ("PR_Read() failed via PR_POLL_EXCEPT. [this=%x].\n", this));

    // XXX: What should this error code be?
    rv = NS_ERROR_FAILURE;
  }
  //
  // Fill the stream with as much data from the network as possible...
  //
  //
  else {
    totalBytesWritten = 0;
    rv = mReadBuffer->WriteSegments(nsReadFromSocket, (void*)mSocketFD, 
                                    MAX_IO_TRANSFER_SIZE, &totalBytesWritten);
    PR_LOG(gSocketLog, PR_LOG_DEBUG, 
           ("FillStream [fd=%x].  rv = %x. Bytes read =%d\n",
           mSocketFD, rv, totalBytesWritten));

    //
    // Deal with the possible return values...
    //
    if (NS_BASE_STREAM_FULL == rv) {
      rv = NS_BASE_STREAM_WOULD_BLOCK;
    }
    else if (NS_BASE_STREAM_EOF == rv) {
      mSelectFlags &= (~PR_POLL_READ);
      rv = NS_OK;
    } 
    else if (NS_SUCCEEDED(rv)) {
      // continue to return WOULD_BLOCK until we've completely finished this read
      rv = NS_BASE_STREAM_WOULD_BLOCK;
    }

    //
    // Fire a single OnDataAvaliable(...) notification once as much data has
    // been filled into the stream as possible...
    //
    if (totalBytesWritten && mReadListener) {
      nsresult rv1;

      rv1 = mReadListener->OnDataAvailable(this, mReadContext, mReadStream, mSourceOffset, 
                                           totalBytesWritten);
      //
      // If the consumer returns failure, then cancel the operation...
      //
      if (NS_FAILED(rv1)) {
        rv = rv1;
      }
      mSourceOffset += totalBytesWritten;
    }
  }

  //
  // Set up the select flags for read...
  //
  if (NS_BASE_STREAM_WOULD_BLOCK == rv) {
    mSelectFlags |= (PR_POLL_READ | PR_POLL_EXCEPT);
  }

  PR_LOG(gSocketLog, PR_LOG_DEBUG, 
         ("--- Leaving nsSocketTransport::doRead() [this=%x]. rv = %x.\t"
          "Total bytes read: %d\n\n",
          this, rv, totalBytesWritten));

  return rv;
}


//-----
//
//  doWrite:
//
//  This method is called while holding the SocketTransport lock.  It is
//  always called on the socket transport thread...
//
//  Return values:
//    NS_OK
//    NS_BASE_STREAM_WOULD_BLOCK
//
//    NS_ERROR_FAILURE
//
//-----
nsresult nsSocketTransport::doWrite(PRInt16 aSelectFlags)
{
  PRUint32 bytesRead, totalBytesRead;
  PRInt32 maxBytesToRead, len;
  PRErrorCode code;
  nsresult rv = NS_OK;

  NS_ASSERTION(eSocketState_WaitReadWrite == mCurrentState, "Wrong state.");

  PR_LOG(gSocketLog, PR_LOG_DEBUG, 
         ("+++ Entering nsSocketTransport::doWrite() [this=%x].\t"
          "aSelectFlags = %x.\t"
          "mWriteCount = %d\n",
          this, aSelectFlags, mWriteCount));

  //
  // Check for an error during PR_Poll(...)
  //
  if (PR_POLL_EXCEPT & aSelectFlags) {
    PR_LOG(gSocketLog, PR_LOG_ERROR, 
           ("PR_Write() failed via PR_POLL_EXCEPT. [this=%x].\n", this));

    // XXX: What should this error code be?
    rv = NS_ERROR_FAILURE;
  }
  else {
    totalBytesRead = 0;
    while (NS_OK == rv) {
      // Determine the amount of data to read from the input stream...
      if ((mWriteCount > 0) && (mWriteCount < MAX_IO_TRANSFER_SIZE)) {
        maxBytesToRead = mWriteCount;
      } else {
        maxBytesToRead = sizeof(gIOBuffer);
      }
      rv = mWriteStream->Read(gIOBuffer, maxBytesToRead, &bytesRead);
      if (NS_SUCCEEDED(rv) && bytesRead) {
        // Update the counters...
        totalBytesRead += bytesRead;
        if (mWriteCount > 0) {
          mWriteCount -= bytesRead;
        }
        // Write the data to the socket...
        len = PR_Write(mSocketFD, gIOBuffer, bytesRead);

        if (len < 0) {
          code = PR_GetError();

          if (PR_WOULD_BLOCK_ERROR == code) {
            rv = NS_BASE_STREAM_WOULD_BLOCK;
          } 
          else {
            PR_LOG(gSocketLog, PR_LOG_ERROR, 
                   ("PR_Write() failed. [this=%x].  PRErrorCode = %x\n",
                    this, code));

            // XXX: What should this error code be?
            rv = NS_ERROR_FAILURE;
          }
        }
      }
      //
      // The write operation has completed...
      //
      if ((mWriteCount == 0) || (NS_BASE_STREAM_EOF == rv) ) {
        mSelectFlags &= (~PR_POLL_WRITE);
        rv = NS_OK;
        break;
      }
    }
  }

  //
  // Set up the select flags for connect...
  //
  if (NS_BASE_STREAM_WOULD_BLOCK == rv) {
    mSelectFlags |= (PR_POLL_WRITE | PR_POLL_EXCEPT);
  }

  PR_LOG(gSocketLog, PR_LOG_DEBUG, 
         ("--- Leaving nsSocketTransport::doWrite() [this=%x]. rv = %x.\t"
          "Total bytes written: %d\n\n",
          this, rv, totalBytesRead));

  return rv;
}


nsresult nsSocketTransport::CloseConnection(void)
{
  PRStatus status;
  nsresult rv = NS_OK;

  NS_ASSERTION(mSocketFD, "Socket does not exist");

  status = PR_Close(mSocketFD);
  if (PR_SUCCESS != status) {
    rv = NS_ERROR_FAILURE;
  }
  mSocketFD = nsnull;

  if (NS_SUCCEEDED(rv)) {
    mCurrentState = eSocketState_Closed;
  }
  
  return rv;
}


//
// --------------------------------------------------------------------------
// nsISupports implementation...
// --------------------------------------------------------------------------
//

NS_IMPL_THREADSAFE_ADDREF(nsSocketTransport);
NS_IMPL_THREADSAFE_RELEASE(nsSocketTransport);

NS_IMETHODIMP
nsSocketTransport::QueryInterface(const nsIID& aIID, void* *aInstancePtr)
{
  if (NULL == aInstancePtr) {
    return NS_ERROR_NULL_POINTER; 
  } 
  if (aIID.Equals(nsCOMTypeInfo<nsIChannel>::GetIID()) ||
      aIID.Equals(nsCOMTypeInfo<nsISupports>::GetIID())) {
    *aInstancePtr = NS_STATIC_CAST(nsIChannel*, this); 
    NS_ADDREF_THIS(); 
    return NS_OK; 
  } 
  if (aIID.Equals(nsCOMTypeInfo<nsIBufferObserver>::GetIID())) {
    *aInstancePtr = NS_STATIC_CAST(nsIBufferObserver*, this); 
    NS_ADDREF_THIS(); 
    return NS_OK; 
  } 
  return NS_NOINTERFACE; 
}



//
// --------------------------------------------------------------------------
// nsIRequest implementation...
// --------------------------------------------------------------------------
//
NS_IMETHODIMP
nsSocketTransport::IsPending(PRBool *result)
{
  *result = mCurrentState != eSocketState_Created
    && mCurrentState != eSocketState_Closed;
  return NS_OK;
}

NS_IMETHODIMP
nsSocketTransport::Cancel(void)
{
  nsresult rv = NS_OK;

  rv = NS_ERROR_NOT_IMPLEMENTED;

  PR_LOG(gSocketLog, PR_LOG_DEBUG, 
         ("Canceling nsSocketTransport [this=%x].  rv = %x\n",
          this, rv));

  return rv;
}

NS_IMETHODIMP
nsSocketTransport::Suspend(void)
{
  nsresult rv = NS_OK;

  // Enter the socket transport lock...
  nsAutoLock lock(mLock);

  mSuspendCount += 1;
  //
  // Wake up the transport on the socket transport thread so it can
  // be removed from the select list...  
  //
  // Only do this the first time a transport is suspended...
  //
  if (1 == mSuspendCount) {
    rv = mService->AddToWorkQ(this);
  }

  PR_LOG(gSocketLog, PR_LOG_DEBUG, 
         ("Suspending nsSocketTransport [this=%x].  rv = %x\t"
          "mSuspendCount = %d.\n",
          this, rv, mSuspendCount));

  return rv;
}


NS_IMETHODIMP
nsSocketTransport::Resume(void)
{
  nsresult rv = NS_OK;

  // Enter the socket transport lock...
  nsAutoLock lock(mLock);

  if (mSuspendCount) {
    mSuspendCount -= 1;
    //
    // Wake up the transport on the socket transport thread so it can
    // be resumed...
    //
    if (0 == mSuspendCount) {
      rv = mService->AddToWorkQ(this);
    }
  } else {
    // Only a suspended transport can be resumed...
    rv = NS_ERROR_FAILURE;
  }

  PR_LOG(gSocketLog, PR_LOG_DEBUG, 
         ("Resuming nsSocketTransport [this=%x].  rv = %x\t"
          "mSuspendCount = %d.\n",
          this, rv, mSuspendCount));

  return rv;
}

//
// --------------------------------------------------------------------------
// nsIBufferObserver implementation...
// --------------------------------------------------------------------------
//
NS_IMETHODIMP
nsSocketTransport::OnFull(nsIBuffer* buffer)
{
  PR_LOG(gSocketLog, PR_LOG_DEBUG, 
         ("nsSocketTransport::OnFull() [this=%x].\n", this));

  //
  // Block the transport...  It is assumed that the calling thread *is* 
  // holding the socket transport lock so Suspend(...) cannot be called.
  // However, it is safe to directly modify the suspend count...
  //
  if (!mIsWaitingForRead) {
    mSuspendCount += 1;
    mIsWaitingForRead = PR_TRUE;
  }

  return NS_OK;
}


NS_IMETHODIMP 
nsSocketTransport::OnEmpty(nsIBuffer* buffer)
{
  nsresult rv = NS_OK;

  PR_LOG(gSocketLog, PR_LOG_DEBUG, 
         ("nsSocketTransport::OnEmpty() [this=%x].\n", this));

  //
  // Unblock the transport...  It is assumed that the calling thread is not
  // holding the socket transport lock so it is safe to call Resume() 
  // directly...
  //
  if (mIsWaitingForRead) {
    rv = Resume();
    mIsWaitingForRead = PR_FALSE;
  }

  return rv;
}


//
// --------------------------------------------------------------------------
// nsIChannel implementation...
// --------------------------------------------------------------------------
//
NS_IMETHODIMP
nsSocketTransport::GetURI(nsIURI * *aURL)
{
  return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP
nsSocketTransport::AsyncRead(PRUint32 startPosition, PRInt32 readCount,
                             nsISupports* aContext,
                             nsIStreamListener* aListener)
{
  // XXX deal with startPosition and readCount parameters
  nsresult rv = NS_OK;
  nsCOMPtr<nsIEventQueue> eventQ;

  // Enter the socket transport lock...
  nsAutoLock lock(mLock);

  PR_LOG(gSocketLog, PR_LOG_DEBUG, 
         ("+++ Entering nsSocketTransport::AsyncRead() [this=%x]\t"
          "readCount = %d.\n", 
          this, readCount));

  // If a read is already in progress then fail...
  if (mReadListener) {
    rv = NS_ERROR_IN_PROGRESS;
  }

  // Get the event queue of the current thread...
  NS_WITH_SERVICE(nsIEventQueueService, eventQService, kEventQueueService, &rv);
  if (NS_SUCCEEDED(rv)) {
    rv = eventQService->GetThreadEventQueue(PR_CurrentThread(), 
                                            getter_AddRefs(eventQ));
  }

  // Create a new non-blocking input stream for reading data into...
  if (NS_SUCCEEDED(rv) && !mReadStream) {
    rv = NS_NewBuffer(&mReadBuffer, MAX_IO_BUFFER_SIZE/2, 
                      2*MAX_IO_BUFFER_SIZE, this);

    if (NS_SUCCEEDED(rv)) {
      rv = NS_NewBufferInputStream(&mReadStream, mReadBuffer, PR_FALSE);
    }
  }

  if (NS_SUCCEEDED(rv)) {
    // Store the context used for this read...
    NS_IF_RELEASE(mReadContext);
    mReadContext = aContext;
    NS_IF_ADDREF(mReadContext);

    // Create a marshalling stream listener to receive notifications...
    rv = NS_NewAsyncStreamListener(&mReadListener, eventQ, aListener);
  }

  if (NS_SUCCEEDED(rv)) {
    mOperation = eSocketOperation_ReadWrite;

    rv = mService->AddToWorkQ(this);
  }

  PR_LOG(gSocketLog, PR_LOG_DEBUG, 
         ("--- Leaving nsSocketTransport::AsyncRead() [this=%x]. rv = %x.\n",
          this, rv));

  return rv;
}


NS_IMETHODIMP
nsSocketTransport::AsyncWrite(nsIInputStream* aFromStream, 
                              PRUint32 startPosition, PRInt32 writeCount,
                              nsISupports* aContext,
                              nsIStreamObserver* aObserver)
{
  // XXX deal with startPosition and writeCount parameters
  nsresult rv = NS_OK;

  // Enter the socket transport lock...
  nsAutoLock lock(mLock);

  PR_LOG(gSocketLog, PR_LOG_DEBUG, 
         ("+++ Entering nsSocketTransport::AsyncWrite() [this=%x]\t"
          "writeCount = %d.\n", 
          this, writeCount));

  // If a write is already in progress then fail...
  if (mWriteStream) {
    rv = NS_ERROR_IN_PROGRESS;
  }

  if (NS_SUCCEEDED(rv)) {
    mWriteStream = aFromStream;
    NS_ADDREF(aFromStream);

    NS_IF_RELEASE(mWriteContext);
    mWriteContext = aContext;
    NS_IF_ADDREF(mWriteContext);

    // Create a marshalling stream observer to receive notifications...
    NS_IF_RELEASE(mWriteObserver);
    if (aObserver) {
      nsCOMPtr<nsIEventQueue> eventQ;

      // Get the event queue of the current thread...
      NS_WITH_SERVICE(nsIEventQueueService, eventQService, kEventQueueService, &rv);
      if (NS_SUCCEEDED(rv)) {
        rv = eventQService->GetThreadEventQueue(PR_CurrentThread(), 
                                                getter_AddRefs(eventQ));
      }
      if (NS_SUCCEEDED(rv)) {
        rv = NS_NewAsyncStreamObserver(&mWriteObserver, eventQ, aObserver);
      }
    }

    mWriteCount = writeCount;
  }

  if (NS_SUCCEEDED(rv)) {
    mOperation = eSocketOperation_ReadWrite;
    rv = mService->AddToWorkQ(this);
  }

  PR_LOG(gSocketLog, PR_LOG_DEBUG, 
         ("--- Leaving nsSocketTransport::AsyncWrite() [this=%x]. rv = %x.\n",
          this, rv));

  return rv;
}


NS_IMETHODIMP
nsSocketTransport::OpenInputStream(PRUint32 startPosition, PRInt32 readCount, 
                                   nsIInputStream* *result)
{
  nsresult rv = NS_OK;
  NS_ASSERTION(startPosition == 0, "fix me");

  // Enter the socket transport lock...
  nsAutoLock lock(mLock);

  PR_LOG(gSocketLog, PR_LOG_DEBUG, 
         ("+++ Entering nsSocketTransport::OpenInputStream() [this=%x].\n", 
         this));

  // If a read is already in progress then fail...
  if (mReadListener) {
    rv = NS_ERROR_IN_PROGRESS;
  }

  // Create a new blocking input stream for reading data into...
  if (NS_SUCCEEDED(rv)) {
    NS_IF_RELEASE(mReadStream);
    // XXX: The buffer should be reused...
    NS_IF_RELEASE(mReadBuffer);
    NS_IF_RELEASE(mReadContext);

    rv = NS_NewBuffer(&mReadBuffer, MAX_IO_BUFFER_SIZE/2, 
                      2*MAX_IO_BUFFER_SIZE, this);

    if (NS_SUCCEEDED(rv)) {
      rv = NS_NewBufferInputStream(&mReadStream, mReadBuffer, PR_TRUE);
    }
  }

  if (NS_SUCCEEDED(rv)) {
    mOperation = eSocketOperation_ReadWrite;

    rv = mService->AddToWorkQ(this);
  }

  PR_LOG(gSocketLog, PR_LOG_DEBUG, 
         ("--- Leaving nsSocketTransport::OpenInputStream() [this=%x].\t"
          "rv = %x.\n",
          this, rv));

  return rv;
}


NS_IMETHODIMP
nsSocketTransport::OpenOutputStream(PRUint32 startPosition, nsIOutputStream* *result)
{
  nsresult rv = NS_OK;
  NS_ASSERTION(startPosition == 0, "fix me");

  // Enter the socket transport lock...
  nsAutoLock lock(mLock);

  PR_LOG(gSocketLog, PR_LOG_DEBUG, 
         ("+++ Entering nsSocketTransport::OpenOutputStream() [this=%x].\n", 
         this));

  // If a write is already in progress then fail...
  if (mWriteStream) {
    rv = NS_ERROR_IN_PROGRESS;
  }

  if (NS_SUCCEEDED(rv)) {
    NS_IF_RELEASE(mWriteObserver);
    NS_IF_RELEASE(mWriteContext);

  	// We want a pipe here so the caller can "write" into one end
  	// and the other end (aWriteStream) gets the data. This data
  	// is then written to the underlying socket when nsSocketTransport::doWrite()
  	// is called.

    // XXX not sure if this should be blocking (PR_TRUE) or non-blocking.
    nsIBufferOutputStream* out = nsnull;
    nsIBufferInputStream* in = nsnull;
    rv = NS_NewPipe(&in, &out,
                    MAX_IO_BUFFER_SIZE, MAX_IO_BUFFER_SIZE, PR_TRUE, nsnull);
    // No need to addref since NewPipe(...) already did!!!
    mWriteStream = in;
    *result = out;
  }

  if (NS_SUCCEEDED(rv)) {
    mOperation = eSocketOperation_ReadWrite;
    // Start the crank.
    rv = mService->AddToWorkQ(this);
  }

  PR_LOG(gSocketLog, PR_LOG_DEBUG, 
         ("--- Leaving nsSocketTransport::OpenOutputStream() [this=%x].\t"
          "rv = %x.\n",
          this, rv));

  return rv;
}

NS_IMETHODIMP
nsSocketTransport::GetLoadAttributes(PRUint32 *aLoadAttributes)
{
  *aLoadAttributes = mLoadAttributes;
  return NS_OK;
}

NS_IMETHODIMP
nsSocketTransport::SetLoadAttributes(PRUint32 aLoadAttributes)
{
  mLoadAttributes = aLoadAttributes;
  return NS_OK;
}

NS_IMETHODIMP
nsSocketTransport::GetContentType(char * *aContentType)
{
  *aContentType = nsnull;
  return NS_ERROR_FAILURE;    // XXX doesn't make sense for transports
}

