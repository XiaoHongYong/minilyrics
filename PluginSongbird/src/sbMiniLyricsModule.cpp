
// Local imports.
#include "sbMiniLyricsService.h"

// Mozilla imports.
#include <nsICategoryManager.h>
#include <nsIGenericFactory.h>
#include <nsServiceManagerUtils.h>


//------------------------------------------------------------------------------
//
// Songbird album art service.
//
//------------------------------------------------------------------------------

// Construct the sbMiniLyricsService object and call its Initialize method.
NS_GENERIC_FACTORY_CONSTRUCTOR_INIT(sbMiniLyricsService, Initialize)


/**
 * Register the Songbird album art service component.
 */

static NS_METHOD
sbMiniLyricsServiceRegister(nsIComponentManager*         aCompMgr,
                          nsIFile*                     aPath,
                          const char*                  aLoaderStr,
                          const char*                  aType,
                          const nsModuleComponentInfo* aInfo)
{
  nsresult rv;

  // Get the category manager.
  nsCOMPtr<nsICategoryManager> categoryManager =
                                 do_GetService(NS_CATEGORYMANAGER_CONTRACTID,
                                               &rv);
  NS_ENSURE_SUCCESS(rv, rv);

  // Add self to the app-startup category.
  rv = categoryManager->AddCategoryEntry
                          ("app-startup",
                           SB_MINILYRICSSERVICE_CLASSNAME,
                           "service," SB_MINILYRICSSERVICE_CONTRACTID,
                           PR_TRUE,
                           PR_TRUE,
                           nsnull);
  NS_ENSURE_SUCCESS(rv, rv);

  return NS_OK;
}


/**
 * Unregister the Songbird album art service component.
 */

static NS_METHOD
sbMiniLyricsServiceUnregister(nsIComponentManager*         aCompMgr,
                            nsIFile*                     aPath,
                            const char*                  aLoaderStr,
                            const nsModuleComponentInfo* aInfo)
{
  nsresult rv;

  // Get the category manager.
  nsCOMPtr<nsICategoryManager> categoryManager =
                                 do_GetService(NS_CATEGORYMANAGER_CONTRACTID,
                                               &rv);
  NS_ENSURE_SUCCESS(rv, rv);

  // Delete self from the app-startup category.
  rv = categoryManager->DeleteCategoryEntry("app-startup",
                                            SB_MINILYRICSSERVICE_CLASSNAME,
                                            PR_TRUE);
  NS_ENSURE_SUCCESS(rv, rv);

  return NS_OK;
}


//------------------------------------------------------------------------------
//
// Songbird album art components module registration services.
//
//------------------------------------------------------------------------------

// Module component information.
static nsModuleComponentInfo sbMiniLyricsComponents[] =
{
  // Album art service component info.
  {
    SB_MINILYRICSSERVICE_CLASSNAME,
    SB_MINILYRICSSERVICE_CID,
    SB_MINILYRICSSERVICE_CONTRACTID,
    sbMiniLyricsServiceConstructor,
    sbMiniLyricsServiceRegister,
    sbMiniLyricsServiceUnregister
  },
};

// NSGetModule
NS_IMPL_NSGETMODULE(sbMiniLyricsModule, sbMiniLyricsComponents)

