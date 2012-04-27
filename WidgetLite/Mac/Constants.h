#pragma once

#define SB_LINEUP           0
#define SB_LINELEFT         0
#define SB_LINEDOWN         1
#define SB_LINERIGHT        1
#define SB_PAGEUP           2
#define SB_PAGELEFT         2
#define SB_PAGEDOWN         3
#define SB_PAGERIGHT        3
#define SB_THUMBPOSITION    4
#define SB_THUMBTRACK       5
#define SB_TOP              6
#define SB_LEFT             6
#define SB_BOTTOM           7
#define SB_RIGHT            7
#define SB_ENDSCROLL        8

#define SW_HIDE             0
#define SW_MAXIMIZE         3
#define SW_SHOW             5
#define SW_MINIMIZE         6
#define SW_RESTORE          9
#define SW_SHOWMINIMIZED	10
#define SW_SHOWMINNOACTIVE	SW_SHOWMINIMIZED
#define SW_SHOWMAXIMIZED	11
#define SW_SHOWNOACTIVATE	SW_SHOW

#define VK_ESCAPE			1
#define VK_CONTROL			3
#define VK_ALT				4
#define VK_SHIFT			5
#define VK_INSERT			6
#define VK_LEFT				7
#define VK_RIGHT			8
#define VK_UP				9
#define VK_DOWN				10
#define VK_PRIOR			11
#define VK_NEXT				12
#define VK_BACK				13
#define VK_DELETE			14
#define VK_RETURN			15
#define VK_END				16
#define VK_HOME				17

#define VK_LBUTTON			20
#define VK_SPACE			21
#define VK_TAB				22
#define VK_F3				23

// Modifier Flags
#define MK_SHIFT			(1 << 17) // NSShiftKeyMask
#define MK_CONTROL			(1 << 18) // NSControlKeyMask


#define WHEEL_DELTA			50


#define MB_OK                       0x00000000L
#define MB_OKCANCEL                 0x00000001L
#define MB_ABORTRETRYIGNORE         0x00000002L
#define MB_YESNOCANCEL              0x00000003L
#define MB_YESNO                    0x00000004L
#define MB_RETRYCANCEL              0x00000005L

#define MB_ICONHAND                 0x00000010L
#define MB_ICONQUESTION             0x00000020L
#define MB_ICONEXCLAMATION          0x00000030L
#define MB_ICONASTERISK             0x00000040L
#define MB_ICONWARNING              MB_ICONEXCLAMATION
#define MB_ICONERROR                MB_ICONHAND
#define MB_ICONINFORMATION          MB_ICONASTERISK
#define MB_ICONSTOP                 MB_ICONHAND

#define IDOK                1
#define IDCANCEL            2
#define IDABORT             3
#define IDRETRY             4
#define IDIGNORE            5
#define IDYES               6
#define IDNO                7
#define IDCLOSE				8
#define IDHELP				9

#define MK_LBUTTON			1

