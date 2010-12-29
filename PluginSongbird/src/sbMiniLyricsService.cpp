
// Self imports.
#include "sbMiniLyricsService.h"

// Songbird imports.
#include <sbILibrary.h>
#include <sbILibraryManager.h>
#include <sbIPropertyArray.h>
#include <sbStandardProperties.h>
#include <sbVariantUtils.h>

#include <prprf.h>
#include <Windows.h>

//------------------------------------------------------------------------------
//
// nsISupports implementation.
//
//------------------------------------------------------------------------------

NS_IMPL_ISUPPORTS2(sbMiniLyricsService,
                   sbIMiniLyricsService,
                   nsIObserver)

//------------------------------------------------------------------------------
//
// nsIObserver implementation.
//
//------------------------------------------------------------------------------

/**
 * Observe will be called when there is a notification for the
 * topic |aTopic|.  This assumes that the object implementing
 * this interface has been registered with an observer service
 * such as the nsIObserverService.
 *
 * If you expect multiple topics/subjects, the impl is
 * responsible for filtering.
 *
 * You should not modify, add, remove, or enumerate
 * notifications in the implemention of observe.
 *
 * @param aSubject : Notification specific interface pointer.
 * @param aTopic   : The notification topic or subject.
 * @param aData    : Notification specific wide string.
 *                    subject event.
 */

NS_IMETHODIMP
sbMiniLyricsService::Observe(nsISupports*     aSubject,
                           const char*      aTopic,
                           const PRUnichar* aData)
{
  nsresult rv;

  // Dispatch processing of event.
  if (!strcmp(aTopic, "profile-after-change")) {
    // Mark preferences as available and continue with initialization.
    mPrefsAvailable = PR_TRUE;
    rv = Initialize();
    NS_ENSURE_SUCCESS(rv, rv);
  } else if (!strcmp(aTopic, SB_LIBRARY_MANAGER_BEFORE_SHUTDOWN_TOPIC)) {
    // Finalize the album art service.
    Finalize();
  }

  return NS_OK;
}

#include "PlayerImp.h"


HANDLE		g_hThread = NULL;
DWORD		g_dwThreadID = 0;

void InitSB();

DWORD WINAPI ThreadML(LPVOID lpParam)
{
	InitSB();

	MSG			msg;

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	g_MLService.OnDisconnected();

	return 0;
}

//------------------------------------------------------------------------------
//
// Public services.
//
//------------------------------------------------------------------------------

/**
 * Construct an album art service instance.
 */

sbMiniLyricsService::sbMiniLyricsService() :
  mInitialized(PR_FALSE),
  mPrefsAvailable(PR_FALSE)
{
}


/**
 * Destroy an album art service instance.
 */

sbMiniLyricsService::~sbMiniLyricsService()
{
  Finalize();
}


/**
 * Initialize the album art service.
 */

nsresult
sbMiniLyricsService::Initialize()
{
  nsresult rv;

  // Do nothing if already initialized.
  if (mInitialized)
    return NS_OK;

  // Add observers.
  if (!mObserverService) {
    // Get the observer service.
    mObserverService = do_GetService("@mozilla.org/observer-service;1", &rv);
    NS_ENSURE_SUCCESS(rv, rv);

    // Add observers.
    rv = mObserverService->AddObserver(this,
                                       "profile-after-change",
                                       PR_FALSE);
    NS_ENSURE_SUCCESS(rv, rv);
    rv = mObserverService->AddObserver(this,
                                       SB_LIBRARY_MANAGER_BEFORE_SHUTDOWN_TOPIC,
                                       PR_FALSE);
    NS_ENSURE_SUCCESS(rv, rv);
  }

  // Do nothing more if preferences are not available.
  if (!mPrefsAvailable)
    return NS_OK;


  // Mark component as initialized.
  mInitialized = PR_TRUE;

  g_hThread = CreateThread(NULL, 0, ThreadML, NULL, 0, &g_dwThreadID);


  return NS_OK;
}


//------------------------------------------------------------------------------
//
// Internal services.
//
//------------------------------------------------------------------------------

/**
 * Finalize the album art service.
 */

void
sbMiniLyricsService::Finalize()
{

	PostThreadMessage(g_dwThreadID, WM_QUIT, 0, 0);
	WaitForSingleObject(g_hThread, INFINITE);
	CloseHandle(g_hThread);
	g_hThread = NULL;

  // Remove observers.
  if (mObserverService) {
    mObserverService->RemoveObserver(this,
                                     "profile-after-change");
    mObserverService->RemoveObserver(this,
                                     SB_LIBRARY_MANAGER_BEFORE_SHUTDOWN_TOPIC);
    mObserverService = nsnull;
  }
}
