/*          ___
          /    )   ,   /                      ,          /  /
          \           /          _     _                / /     _
           \     /   /   | /   /__)  /  )   /   /  /   / (    /__)
   ___ (____/___/___/____|/___(__ __/______/___(__(___/___\__(__ _____
  |                                       /                           |
  |  Silverjuke(R) SDK                (_ /                            |
  |  Version 2.10                                                     |
  |  Copyright (C) Bjoern Petersen Software Design and Development    |
  |  The last version of this file is availabe at www.silverjuke.net  |
  |___________________________________________________________________|
  
*/


#ifndef SJ_API_H
#define SJ_API_H
#ifdef __cplusplus
extern "C" {
#endif



/* Includes, Basic types */
#ifdef WIN32
#include <windows.h>
#else
#define CALLBACK
#define LPARAM long
#define UINT unsigned int
#endif



/* Plugin initialization */
typedef LPARAM (CALLBACK SJPROC)(struct SjInterface*, UINT msg,
    LPARAM param1, LPARAM param2, LPARAM param3);

typedef struct SjInterface
{
    SJPROC*  CallPlugin;
    SJPROC*  CallMaster;
    LPARAM   rsvd;
    LPARAM   user;
} SjInterface;

SjInterface* SjGetInterface(void);



/* Messages Silverjuke -> Plugin */
#define SJ_PLUGIN_INIT              10000
#define SJ_PLUGIN_EXIT              10001
#define SJ_PLUGIN_CALL              10010



/* Messages Plugin -> Silverjuke */
#define SJ_GET_VERSION              20000
#define SJ_EXECUTE                  20040



#ifdef __cplusplus
}
#endif
#endif /* #ifndef SJ_API_H */
