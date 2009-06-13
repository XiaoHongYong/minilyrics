
#ifndef __SB_MINILYRICSSERVICE_H__
#define __SB_MINILYRICSSERVICE_H__

// Songbird imports.
#include <sbIMiniLyricsService.h>
#include <sbMemoryUtils.h>

// Mozilla imports.
#include <nsCOMPtr.h>
#include <nsIFileStreams.h>
#include <nsIIOService.h>
#include <nsIObserver.h>
#include <nsIObserverService.h>
#include <nsTArray.h>
#include <nsInterfaceHashtable.h>
#include <nsITimer.h>

//------------------------------------------------------------------------------
//
// Songbird album art service defs.
//
//------------------------------------------------------------------------------

//
// Songbird album art service component defs.
//

#define SB_MINILYRICSSERVICE_CLASSNAME "sbMiniLyricsService"
#define SB_MINILYRICSSERVICE_CID                                                 \
  /* {8DF4920B-89AC-49A3-8552-D74D313C62B6} */                                 \
  {                                                                            \
    0x8DF4920B,                                                                \
    0x89AC,                                                                    \
    0x49A3,                                                                    \
    { 0x85, 0x52, 0xD7, 0x4D, 0x31, 0x3C, 0x62, 0xB6 }                         \
  }

//------------------------------------------------------------------------------
//
// Songbird album art service classes.
//
//------------------------------------------------------------------------------

/**
 * This class implements the album art service component.
 */

class sbMiniLyricsService : public sbIMiniLyricsService,
                          public nsIObserver
{
public:

  //
  // Inherited interfaces.
  //

  NS_DECL_ISUPPORTS
  NS_DECL_SBIMINILYRICSSERVICE
  NS_DECL_NSIOBSERVER


  //
  // Public services.
  //

  sbMiniLyricsService();

  virtual ~sbMiniLyricsService();

  nsresult Initialize();

  void Finalize();

  //----------------------------------------------------------------------------
  //
  // Private interface.
  //
  //----------------------------------------------------------------------------

private:

  nsCOMPtr<nsIObserverService>  mObserverService;
  nsCOMPtr<nsIIOService>        mIOService;
  PRBool                        mInitialized;
  PRBool                        mPrefsAvailable;
  
  //
  // Internal services.
  //

  //
  // InitializeLibraryWatch
  // 

};

#endif // __SB_MINILYRICSSERVICE_H__

