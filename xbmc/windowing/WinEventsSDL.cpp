/*
 *      Copyright (C) 2005-2013 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "system.h"
#ifdef HAS_SDL_WIN_EVENTS

#include "WinEvents.h"
#include "WinEventsSDL.h"
#include "Application.h"
#include "messaging/ApplicationMessenger.h"
#include "GUIUserMessages.h"
#include "settings/DisplaySettings.h"
#include "guilib/GUIWindowManager.h"
#include "input/Key.h"
#include "input/InputManager.h"
#include "input/MouseStat.h"
#include "WindowingFactory.h"
#if defined(TARGET_DARWIN)
#include "platform/darwin/osx/CocoaInterface.h"
#endif

#ifdef HAVE_X11
#include <X11/Xlib.h>
#include <X11/XKBlib.h>
#include "input/XBMC_keysym.h"
#include "utils/log.h"
#endif

using namespace KODI::MESSAGING;

#ifdef HAVE_X11
// The following chunk of code is Linux specific. For keys that have
// with keysym.sym set to zero it checks the scan code, and sets the sym
// for some known scan codes. This is mostly the multimedia keys.
// Note that the scan code to sym mapping is different with and without
// the evdev driver so we need to check if evdev is loaded.

// m_bEvdev == true if evdev is loaded
static bool m_bEvdev = true;
// We need to initialise some local storage once. Set m_bEvdevInit to true
// once the initialisation has been done
static bool m_bEvdevInit = false;

// Mappings of scancodes to syms for the evdev driver
static uint16_t SymMappingsEvdev[][2] =
{ { 121, XBMCK_VOLUME_MUTE }         // Volume mute
, { 122, XBMCK_VOLUME_DOWN }         // Volume down
, { 123, XBMCK_VOLUME_UP }           // Volume up
, { 124, XBMCK_POWER }               // Power button on PC case
, { 127, XBMCK_SPACE }               // Pause
, { 135, XBMCK_MENU }                // Right click
, { 136, XBMCK_MEDIA_STOP }          // Stop
, { 138, 0x69 /* 'i' */}             // Info
, { 147, 0x6d /* 'm' */}             // Menu
, { 148, XBMCK_LAUNCH_APP2 }         // Launch app 2
, { 150, XBMCK_SLEEP }               // Sleep
, { 152, XBMCK_LAUNCH_APP1 }         // Launch app 1
, { 163, XBMCK_LAUNCH_MAIL }         // Launch Mail
, { 164, XBMCK_BROWSER_FAVORITES }   // Browser favorites
, { 166, XBMCK_BROWSER_BACK }        // Back
, { 167, XBMCK_BROWSER_FORWARD }     // Browser forward
, { 171, XBMCK_MEDIA_NEXT_TRACK }    // Next track
, { 172, XBMCK_MEDIA_PLAY_PAUSE }    // Play_Pause
, { 173, XBMCK_MEDIA_PREV_TRACK }    // Prev track
, { 174, XBMCK_MEDIA_STOP }          // Stop
, { 176, 0x72 /* 'r' */}             // Rewind
, { 179, XBMCK_LAUNCH_MEDIA_SELECT } // Launch media select
, { 180, XBMCK_BROWSER_HOME }        // Browser home
, { 181, XBMCK_BROWSER_REFRESH }     // Browser refresh
, { 208, XBMCK_MEDIA_PLAY_PAUSE }    // Play_Pause
, { 209, XBMCK_MEDIA_PLAY_PAUSE }    // Play_Pause
, { 214, XBMCK_ESCAPE }              // Close
, { 215, XBMCK_MEDIA_PLAY_PAUSE }    // Play_Pause
, { 216, 0x66 /* 'f' */}             // Forward
//, {167, 0xb3 } // Record
};

// The non-evdev mappings need to be checked. At the moment XBMC will never
// use them anyway.
static uint16_t SymMappingsUbuntu[][2] =
{ { 0xf5, XBMCK_F13 } // F13 Launch help browser
, { 0x87, XBMCK_F14 } // F14 undo
, { 0x8a, XBMCK_F15 } // F15 redo
, { 0x89, 0x7f } // F16 new
, { 0xbf, 0x80 } // F17 open
, { 0xaf, 0x81 } // F18 close
, { 0xe4, 0x82 } // F19 reply
, { 0x8e, 0x83 } // F20 forward
, { 0xda, 0x84 } // F21 send
//, { 0x, 0x85 } // F22 check spell (doesn't work for me with ubuntu)
, { 0xd5, 0x86 } // F23 save
, { 0xb9, 0x87 } // 0x2a?? F24 print
        // end of special keys above F1 till F12

, { 234,  XBMCK_BROWSER_BACK } // Browser back
, { 233,  XBMCK_BROWSER_FORWARD } // Browser forward
, { 231,  XBMCK_BROWSER_REFRESH } // Browser refresh
//, { , XBMCK_BROWSER_STOP } // Browser stop
, { 122,  XBMCK_BROWSER_SEARCH } // Browser search
, { 0xe5, XBMCK_BROWSER_SEARCH } // Browser search
, { 230,  XBMCK_BROWSER_FAVORITES } // Browser favorites
, { 130,  XBMCK_BROWSER_HOME } // Browser home
, { 0xa0, XBMCK_VOLUME_MUTE } // Volume mute
, { 0xae, XBMCK_VOLUME_DOWN } // Volume down
, { 0xb0, XBMCK_VOLUME_UP } // Volume up
, { 0x99, XBMCK_MEDIA_NEXT_TRACK } // Next track
, { 0x90, XBMCK_MEDIA_PREV_TRACK } // Prev track
, { 0xa4, XBMCK_MEDIA_STOP } // Stop
, { 0xa2, XBMCK_MEDIA_PLAY_PAUSE } // Play_Pause
, { 0xec, XBMCK_LAUNCH_MAIL } // Launch mail
, { 129,  XBMCK_LAUNCH_MEDIA_SELECT } // Launch media_select
, { 198,  XBMCK_LAUNCH_APP1 } // Launch App1/PC icon
, { 0xa1, XBMCK_LAUNCH_APP2 } // Launch App2/Calculator
, { 34, 0x3b /* vkey 0xba */} // OEM 1: [ on us keyboard
, { 51, 0x2f /* vkey 0xbf */} // OEM 2: additional key on european keyboards between enter and ' on us keyboards
, { 47, 0x60 /* vkey 0xc0 */} // OEM 3: ; on us keyboards
, { 20, 0xdb } // OEM 4: - on us keyboards (between 0 and =)
, { 49, 0xdc } // OEM 5: ` on us keyboards (below ESC)
, { 21, 0xdd } // OEM 6: =??? on us keyboards (between - and backspace)
, { 48, 0xde } // OEM 7: ' on us keyboards (on right side of ;)
//, { 0, 0xdf } // OEM 8
, { 94, 0xe2 } // OEM 102: additional key on european keyboards between left shift and z on us keyboards
//, { 0xb2, 0x } // Ubuntu default setting for launch browser
//, { 0x76, 0x } // Ubuntu default setting for launch music player
//, { 0xcc, 0x } // Ubuntu default setting for eject
, { 117, 0x5d } // right click
};

// Called once. Checks whether evdev is loaded and sets m_bEvdev accordingly.
static void InitEvdev(void)
{
  // Set m_bEvdevInit to indicate we have been initialised
  m_bEvdevInit = true;

  Display* dpy = XOpenDisplay(NULL);
  if (!dpy)
  {
    CLog::Log(LOGERROR, "CWinEventsSDL::CWinEventsSDL - XOpenDisplay failed");
    return;
  }

  XkbDescPtr desc;
  char* symbols;

  desc = XkbGetKeyboard(dpy, XkbAllComponentsMask, XkbUseCoreKbd);
  if(!desc)
  {
    XCloseDisplay(dpy);
    CLog::Log(LOGERROR, "CWinEventsSDL::CWinEventsSDL - XkbGetKeyboard failed");
    return;
  }

  symbols = XGetAtomName(dpy, desc->names->symbols);
  if(symbols)
  {
    CLog::Log(LOGDEBUG, "CWinEventsSDL::CWinEventsSDL - XKb symbols %s", symbols);
    if(strstr(symbols, "(evdev)"))
      m_bEvdev = true;
    else
      m_bEvdev = false;
  }

  XFree(symbols);
  XkbFreeKeyboard(desc, XkbAllComponentsMask, True);
  XCloseDisplay(dpy);

  CLog::Log(LOGDEBUG, "CWinEventsSDL::CWinEventsSDL - m_bEvdev = %d", m_bEvdev);
}

// Check the scan code and return the matching sym, or zero if the scan code
// is unknown.
static uint16_t SymFromScancode(uint16_t scancode)
{
  unsigned int i;

  // We need to initialise m_bEvdev once
  if (!m_bEvdevInit)
    InitEvdev();

  // If evdev is loaded look up the scan code in SymMappingsEvdev
  if (m_bEvdev)
  {
    for (i = 0; i < sizeof(SymMappingsEvdev)/4; i++)
      if (scancode == SymMappingsEvdev[i][0])
        return SymMappingsEvdev[i][1];
  }

  // If evdev is not loaded look up the scan code in SymMappingsUbuntu
  else
  {
    for (i = 0; i < sizeof(SymMappingsUbuntu)/4; i++)
      if (scancode == SymMappingsUbuntu[i][0])
        return SymMappingsUbuntu[i][1];
  }

  // Scan code wasn't found, return zero
  return 0;
}
#endif // End of checks for keysym.sym == 0

#if HAVE_SDL_VERSION == 2
// The keysyms changed from SDL 1 to SDL 2, so create a mapping
static std::map<SDL_Keycode, XBMCKey> s_keysymTable;

static void InitializeKeysymLookup()
{
    s_keysymTable[SDLK_RETURN] = XBMCK_RETURN;
    s_keysymTable[SDLK_ESCAPE] = XBMCK_ESCAPE;
    s_keysymTable[SDLK_BACKSPACE] = XBMCK_BACKSPACE;
    s_keysymTable[SDLK_TAB] = XBMCK_TAB;
    s_keysymTable[SDLK_SPACE] = XBMCK_SPACE;
    s_keysymTable[SDLK_EXCLAIM] = XBMCK_EXCLAIM;
    s_keysymTable[SDLK_QUOTEDBL] = XBMCK_QUOTEDBL;
    s_keysymTable[SDLK_HASH] = XBMCK_HASH;
    s_keysymTable[SDLK_PERCENT] = XBMCK_PERCENT;
    s_keysymTable[SDLK_DOLLAR] = XBMCK_DOLLAR;
    s_keysymTable[SDLK_AMPERSAND] = XBMCK_AMPERSAND;
    s_keysymTable[SDLK_QUOTE] = XBMCK_QUOTE;
    s_keysymTable[SDLK_LEFTPAREN] = XBMCK_LEFTPAREN;
    s_keysymTable[SDLK_RIGHTPAREN] = XBMCK_RIGHTPAREN;
    s_keysymTable[SDLK_ASTERISK] = XBMCK_ASTERISK;
    s_keysymTable[SDLK_PLUS] = XBMCK_PLUS;
    s_keysymTable[SDLK_COMMA] = XBMCK_COMMA;
    s_keysymTable[SDLK_MINUS] = XBMCK_MINUS;
    s_keysymTable[SDLK_PERIOD] = XBMCK_PERIOD;
    s_keysymTable[SDLK_SLASH] = XBMCK_SLASH;
    s_keysymTable[SDLK_0] = XBMCK_0;
    s_keysymTable[SDLK_1] = XBMCK_1;
    s_keysymTable[SDLK_2] = XBMCK_2;
    s_keysymTable[SDLK_3] = XBMCK_3;
    s_keysymTable[SDLK_4] = XBMCK_4;
    s_keysymTable[SDLK_5] = XBMCK_5;
    s_keysymTable[SDLK_6] = XBMCK_6;
    s_keysymTable[SDLK_7] = XBMCK_7;
    s_keysymTable[SDLK_8] = XBMCK_8;
    s_keysymTable[SDLK_9] = XBMCK_9;
    s_keysymTable[SDLK_COLON] = XBMCK_COLON;
    s_keysymTable[SDLK_SEMICOLON] = XBMCK_SEMICOLON;
    s_keysymTable[SDLK_LESS] = XBMCK_LESS;
    s_keysymTable[SDLK_EQUALS] = XBMCK_EQUALS;
    s_keysymTable[SDLK_GREATER] = XBMCK_GREATER;
    s_keysymTable[SDLK_QUESTION] = XBMCK_QUESTION;
    s_keysymTable[SDLK_AT] = XBMCK_AT;
    s_keysymTable[SDLK_LEFTBRACKET] = XBMCK_LEFTBRACKET;
    s_keysymTable[SDLK_BACKSLASH] = XBMCK_BACKSLASH;
    s_keysymTable[SDLK_RIGHTBRACKET] = XBMCK_RIGHTBRACKET;
    s_keysymTable[SDLK_CARET] = XBMCK_CARET;
    s_keysymTable[SDLK_UNDERSCORE] = XBMCK_UNDERSCORE;
    s_keysymTable[SDLK_BACKQUOTE] = XBMCK_BACKQUOTE;
    s_keysymTable[SDLK_a] = XBMCK_a;
    s_keysymTable[SDLK_b] = XBMCK_b;
    s_keysymTable[SDLK_c] = XBMCK_c;
    s_keysymTable[SDLK_d] = XBMCK_d;
    s_keysymTable[SDLK_e] = XBMCK_e;
    s_keysymTable[SDLK_f] = XBMCK_f;
    s_keysymTable[SDLK_g] = XBMCK_g;
    s_keysymTable[SDLK_h] = XBMCK_h;
    s_keysymTable[SDLK_i] = XBMCK_i;
    s_keysymTable[SDLK_j] = XBMCK_j;
    s_keysymTable[SDLK_k] = XBMCK_k;
    s_keysymTable[SDLK_l] = XBMCK_l;
    s_keysymTable[SDLK_m] = XBMCK_m;
    s_keysymTable[SDLK_n] = XBMCK_n;
    s_keysymTable[SDLK_o] = XBMCK_o;
    s_keysymTable[SDLK_p] = XBMCK_p;
    s_keysymTable[SDLK_q] = XBMCK_q;
    s_keysymTable[SDLK_r] = XBMCK_r;
    s_keysymTable[SDLK_s] = XBMCK_s;
    s_keysymTable[SDLK_t] = XBMCK_t;
    s_keysymTable[SDLK_u] = XBMCK_u;
    s_keysymTable[SDLK_v] = XBMCK_v;
    s_keysymTable[SDLK_w] = XBMCK_w;
    s_keysymTable[SDLK_x] = XBMCK_x;
    s_keysymTable[SDLK_y] = XBMCK_y;
    s_keysymTable[SDLK_z] = XBMCK_z;
    s_keysymTable[SDLK_CAPSLOCK] = XBMCK_CAPSLOCK;
    s_keysymTable[SDLK_F1] = XBMCK_F1;
    s_keysymTable[SDLK_F2] = XBMCK_F2;
    s_keysymTable[SDLK_F3] = XBMCK_F3;
    s_keysymTable[SDLK_F4] = XBMCK_F4;
    s_keysymTable[SDLK_F5] = XBMCK_F5;
    s_keysymTable[SDLK_F6] = XBMCK_F6;
    s_keysymTable[SDLK_F7] = XBMCK_F7;
    s_keysymTable[SDLK_F8] = XBMCK_F8;
    s_keysymTable[SDLK_F9] = XBMCK_F9;
    s_keysymTable[SDLK_F10] = XBMCK_F10;
    s_keysymTable[SDLK_F11] = XBMCK_F11;
    s_keysymTable[SDLK_F12] = XBMCK_F12;
    s_keysymTable[SDLK_PRINTSCREEN] = XBMCK_PRINT;
    s_keysymTable[SDLK_SCROLLLOCK] = XBMCK_SCROLLOCK;
    s_keysymTable[SDLK_PAUSE] = XBMCK_PAUSE;
    s_keysymTable[SDLK_INSERT] = XBMCK_INSERT;
    s_keysymTable[SDLK_HOME] = XBMCK_HOME;
    s_keysymTable[SDLK_PAGEUP] = XBMCK_PAGEUP;
    s_keysymTable[SDLK_DELETE] = XBMCK_DELETE;
    s_keysymTable[SDLK_END] = XBMCK_END;
    s_keysymTable[SDLK_PAGEDOWN] = XBMCK_PAGEDOWN;
    s_keysymTable[SDLK_RIGHT] = XBMCK_RIGHT;
    s_keysymTable[SDLK_LEFT] = XBMCK_LEFT;
    s_keysymTable[SDLK_DOWN] = XBMCK_DOWN;
    s_keysymTable[SDLK_UP] = XBMCK_UP;
    s_keysymTable[SDLK_NUMLOCKCLEAR] = XBMCK_NUMLOCK;
    s_keysymTable[SDLK_KP_DIVIDE] = XBMCK_KP_DIVIDE;
    s_keysymTable[SDLK_KP_MULTIPLY] = XBMCK_KP_MULTIPLY;
    s_keysymTable[SDLK_KP_MINUS] = XBMCK_KP_MINUS;
    s_keysymTable[SDLK_KP_PLUS] = XBMCK_KP_PLUS;
    s_keysymTable[SDLK_KP_ENTER] = XBMCK_KP_ENTER;
    s_keysymTable[SDLK_KP_1] = XBMCK_KP1;
    s_keysymTable[SDLK_KP_2] = XBMCK_KP2;
    s_keysymTable[SDLK_KP_3] = XBMCK_KP3;
    s_keysymTable[SDLK_KP_4] = XBMCK_KP4;
    s_keysymTable[SDLK_KP_5] = XBMCK_KP5;
    s_keysymTable[SDLK_KP_6] = XBMCK_KP6;
    s_keysymTable[SDLK_KP_7] = XBMCK_KP7;
    s_keysymTable[SDLK_KP_8] = XBMCK_KP8;
    s_keysymTable[SDLK_KP_9] = XBMCK_KP9;
    s_keysymTable[SDLK_KP_0] = XBMCK_KP0;
    s_keysymTable[SDLK_KP_PERIOD] = XBMCK_KP_PERIOD;
    s_keysymTable[SDLK_APPLICATION] = XBMCK_LAUNCH_APP1;
    s_keysymTable[SDLK_POWER] = XBMCK_POWER;
    s_keysymTable[SDLK_KP_EQUALS] = XBMCK_KP_EQUALS;
    s_keysymTable[SDLK_F13] = XBMCK_F13;
    s_keysymTable[SDLK_F14] = XBMCK_F14;
    s_keysymTable[SDLK_F15] = XBMCK_F15;
/*
    s_keysymTable[SDLK_F16] = XBMCK_F16;
    s_keysymTable[SDLK_F17] = XBMCK_F17;
    s_keysymTable[SDLK_F18] = XBMCK_F18;
    s_keysymTable[SDLK_F19] = XBMCK_F19;
    s_keysymTable[SDLK_F20] = XBMCK_F20;
    s_keysymTable[SDLK_F21] = XBMCK_F21;
    s_keysymTable[SDLK_F22] = XBMCK_F22;
    s_keysymTable[SDLK_F23] = XBMCK_F23;
    s_keysymTable[SDLK_F24] = XBMCK_F24;
    s_keysymTable[SDLK_EXECUTE] = XBMCK_EXECUTE;
*/
    s_keysymTable[SDLK_HELP] = XBMCK_HELP;
    s_keysymTable[SDLK_MENU] = XBMCK_MENU;
/*
    s_keysymTable[SDLK_SELECT] = XBMCK_SELECT;
    s_keysymTable[SDLK_STOP] = XBMCK_STOP;
    s_keysymTable[SDLK_AGAIN] = XBMCK_AGAIN;
    s_keysymTable[SDLK_UNDO] = XBMCK_UNDO;
    s_keysymTable[SDLK_CUT] = XBMCK_CUT;
    s_keysymTable[SDLK_COPY] = XBMCK_COPY;
    s_keysymTable[SDLK_PASTE] = XBMCK_PASTE;
    s_keysymTable[SDLK_FIND] = XBMCK_FIND;
*/
    s_keysymTable[SDLK_MUTE] = XBMCK_VOLUME_MUTE;
    s_keysymTable[SDLK_VOLUMEUP] = XBMCK_VOLUME_UP;
    s_keysymTable[SDLK_VOLUMEDOWN] = XBMCK_VOLUME_DOWN;
/*
    s_keysymTable[SDLK_KP_COMMA] = XBMCK_KP_COMMA;
    s_keysymTable[SDLK_KP_EQUALSAS400] = XBMCK_KP_EQUALSAS400;
    s_keysymTable[SDLK_ALTERASE] = XBMCK_ALTERASE;
*/
    s_keysymTable[SDLK_SYSREQ] = XBMCK_SYSREQ;
/*
    s_keysymTable[SDLK_CANCEL] = XBMCK_CANCEL;
*/
    s_keysymTable[SDLK_CLEAR] = XBMCK_CLEAR;
/*
    s_keysymTable[SDLK_PRIOR] = XBMCK_PRIOR;
    s_keysymTable[SDLK_RETURN2] = XBMCK_RETURN2;
    s_keysymTable[SDLK_SEPARATOR] = XBMCK_SEPARATOR;
    s_keysymTable[SDLK_OUT] = XBMCK_OUT;
    s_keysymTable[SDLK_OPER] = XBMCK_OPER;
    s_keysymTable[SDLK_CLEARAGAIN] = XBMCK_CLEARAGAIN;
    s_keysymTable[SDLK_CRSEL] = XBMCK_CRSEL;
    s_keysymTable[SDLK_EXSEL] = XBMCK_EXSEL;
    s_keysymTable[SDLK_KP_00] = XBMCK_KP_00;
    s_keysymTable[SDLK_KP_000] = XBMCK_KP_000;
    s_keysymTable[SDLK_THOUSANDSSEPARATOR] = XBMCK_THOUSANDSSEPARATOR;
    s_keysymTable[SDLK_DECIMALSEPARATOR] = XBMCK_DECIMALSEPARATOR;
    s_keysymTable[SDLK_CURRENCYUNIT] = XBMCK_CURRENCYUNIT;
    s_keysymTable[SDLK_CURRENCYSUBUNIT] = XBMCK_CURRENCYSUBUNIT;
    s_keysymTable[SDLK_KP_LEFTPAREN] = XBMCK_KP_LEFTPAREN;
    s_keysymTable[SDLK_KP_RIGHTPAREN] = XBMCK_KP_RIGHTPAREN;
    s_keysymTable[SDLK_KP_LEFTBRACE] = XBMCK_KP_LEFTBRACE;
    s_keysymTable[SDLK_KP_RIGHTBRACE] = XBMCK_KP_RIGHTBRACE;
    s_keysymTable[SDLK_KP_TAB] = XBMCK_KP_TAB;
    s_keysymTable[SDLK_KP_BACKSPACE] = XBMCK_KP_BACKSPACE;
    s_keysymTable[SDLK_KP_A] = XBMCK_KP_A;
    s_keysymTable[SDLK_KP_B] = XBMCK_KP_B;
    s_keysymTable[SDLK_KP_C] = XBMCK_KP_C;
    s_keysymTable[SDLK_KP_D] = XBMCK_KP_D;
    s_keysymTable[SDLK_KP_E] = XBMCK_KP_E;
    s_keysymTable[SDLK_KP_F] = XBMCK_KP_F;
    s_keysymTable[SDLK_KP_XOR] = XBMCK_KP_XOR;
    s_keysymTable[SDLK_KP_POWER] = XBMCK_KP_POWER;
    s_keysymTable[SDLK_KP_PERCENT] = XBMCK_KP_PERCENT;
    s_keysymTable[SDLK_KP_LESS] = XBMCK_KP_LESS;
    s_keysymTable[SDLK_KP_GREATER] = XBMCK_KP_GREATER;
    s_keysymTable[SDLK_KP_AMPERSAND] = XBMCK_KP_AMPERSAND;
    s_keysymTable[SDLK_KP_DBLAMPERSAND] = XBMCK_KP_DBLAMPERSAND;
    s_keysymTable[SDLK_KP_VERTICALBAR] = XBMCK_KP_VERTICALBAR;
    s_keysymTable[SDLK_KP_DBLVERTICALBAR] = XBMCK_KP_DBLVERTICALBAR;
    s_keysymTable[SDLK_KP_COLON] = XBMCK_KP_COLON;
    s_keysymTable[SDLK_KP_HASH] = XBMCK_KP_HASH;
    s_keysymTable[SDLK_KP_SPACE] = XBMCK_KP_SPACE;
    s_keysymTable[SDLK_KP_AT] = XBMCK_KP_AT;
    s_keysymTable[SDLK_KP_EXCLAM] = XBMCK_KP_EXCLAM;
    s_keysymTable[SDLK_KP_MEMSTORE] = XBMCK_KP_MEMSTORE;
    s_keysymTable[SDLK_KP_MEMRECALL] = XBMCK_KP_MEMRECALL;
    s_keysymTable[SDLK_KP_MEMCLEAR] = XBMCK_KP_MEMCLEAR;
    s_keysymTable[SDLK_KP_MEMADD] = XBMCK_KP_MEMADD;
    s_keysymTable[SDLK_KP_MEMSUBTRACT] = XBMCK_KP_MEMSUBTRACT;
    s_keysymTable[SDLK_KP_MEMMULTIPLY] = XBMCK_KP_MEMMULTIPLY;
    s_keysymTable[SDLK_KP_MEMDIVIDE] = XBMCK_KP_MEMDIVIDE;
    s_keysymTable[SDLK_KP_PLUSMINUS] = XBMCK_KP_PLUSMINUS;
    s_keysymTable[SDLK_KP_CLEAR] = XBMCK_KP_CLEAR;
    s_keysymTable[SDLK_KP_CLEARENTRY] = XBMCK_KP_CLEARENTRY;
    s_keysymTable[SDLK_KP_BINARY] = XBMCK_KP_BINARY;
    s_keysymTable[SDLK_KP_OCTAL] = XBMCK_KP_OCTAL;
    s_keysymTable[SDLK_KP_DECIMAL] = XBMCK_KP_DECIMAL;
    s_keysymTable[SDLK_KP_HEXADECIMAL] = XBMCK_KP_HEXADECIMAL;
*/
    s_keysymTable[SDLK_LCTRL] = XBMCK_LCTRL;
    s_keysymTable[SDLK_LSHIFT] = XBMCK_LSHIFT;
    s_keysymTable[SDLK_LALT] = XBMCK_LALT;
    s_keysymTable[SDLK_LGUI] = XBMCK_LSUPER;
    s_keysymTable[SDLK_RCTRL] = XBMCK_RCTRL;
    s_keysymTable[SDLK_RSHIFT] = XBMCK_RSHIFT;
    s_keysymTable[SDLK_RALT] = XBMCK_RALT;
    s_keysymTable[SDLK_RGUI] = XBMCK_RSUPER;
    s_keysymTable[SDLK_MODE] = XBMCK_MODE;
    s_keysymTable[SDLK_AUDIONEXT] = XBMCK_MEDIA_NEXT_TRACK;
    s_keysymTable[SDLK_AUDIOPREV] = XBMCK_MEDIA_PREV_TRACK;
    s_keysymTable[SDLK_AUDIOSTOP] = XBMCK_MEDIA_STOP;
    s_keysymTable[SDLK_AUDIOPLAY] = XBMCK_MEDIA_PLAY_PAUSE;
    s_keysymTable[SDLK_AUDIOMUTE] = XBMCK_VOLUME_MUTE;
    s_keysymTable[SDLK_MEDIASELECT] = XBMCK_LAUNCH_MEDIA_SELECT;
    s_keysymTable[SDLK_WWW] = XBMCK_BROWSER_HOME;
    s_keysymTable[SDLK_MAIL] = XBMCK_LAUNCH_MAIL;
    s_keysymTable[SDLK_CALCULATOR] = XBMCK_LAUNCH_APP1;
    s_keysymTable[SDLK_COMPUTER] = XBMCK_LAUNCH_FILE_BROWSER;
    s_keysymTable[SDLK_AC_SEARCH] = XBMCK_BROWSER_SEARCH;
    s_keysymTable[SDLK_AC_HOME] = XBMCK_BROWSER_HOME;
    s_keysymTable[SDLK_AC_BACK] = XBMCK_BROWSER_BACK;
    s_keysymTable[SDLK_AC_FORWARD] = XBMCK_BROWSER_FORWARD;
    s_keysymTable[SDLK_AC_STOP] = XBMCK_BROWSER_STOP;
    s_keysymTable[SDLK_AC_REFRESH] = XBMCK_BROWSER_REFRESH;
    s_keysymTable[SDLK_AC_BOOKMARKS] = XBMCK_BROWSER_FAVORITES;
/*
    s_keysymTable[SDLK_BRIGHTNESSDOWN] = XBMCK_BRIGHTNESSDOWN;
    s_keysymTable[SDLK_BRIGHTNESSUP] = XBMCK_BRIGHTNESSUP;
    s_keysymTable[SDLK_DISPLAYSWITCH] = XBMCK_DISPLAYSWITCH;
    s_keysymTable[SDLK_KBDILLUMTOGGLE] = XBMCK_KBDILLUMTOGGLE;
    s_keysymTable[SDLK_KBDILLUMDOWN] = XBMCK_KBDILLUMDOWN;
    s_keysymTable[SDLK_KBDILLUMUP] = XBMCK_KBDILLUMUP;
*/
    s_keysymTable[SDLK_EJECT] = XBMCK_EJECT;
    s_keysymTable[SDLK_SLEEP] = XBMCK_SLEEP;
}

static XBMCKey LookupKeysym(SDL_Keycode keycode)
{
	if (s_keysymTable.size() == 0) {
		InitializeKeysymLookup();
	}

	std::map<SDL_Keycode, XBMCKey>::iterator it = s_keysymTable.find(keycode);
	if (it != s_keysymTable.end()) {
		return it->second;
	}
	return XBMCK_UNKNOWN;
}

#endif // SDL 2.0

bool CWinEventsSDL::MessagePump()
{
  SDL_Event event;
  bool ret = false;

  while (SDL_PollEvent(&event))
  {
    switch(event.type)
    {
      case SDL_QUIT:
        if (!g_application.m_bStop) 
          CApplicationMessenger::GetInstance().PostMsg(TMSG_QUIT);
        break;

#if HAVE_SDL_VERSION == 1
      case SDL_ACTIVEEVENT:
        //If the window was inconified or restored
        if( event.active.state & SDL_APPACTIVE )
        {
          g_application.SetRenderGUI(event.active.gain != 0);
          g_Windowing.NotifyAppActiveChange(g_application.GetRenderGUI());
        }
        else if (event.active.state & SDL_APPINPUTFOCUS)
      {
        g_application.m_AppFocused = event.active.gain != 0;
        g_Windowing.NotifyAppFocusChange(g_application.m_AppFocused);
      }
      break;

      case SDL_KEYDOWN:
      {
        // process any platform specific shortcuts before handing off to XBMC
#ifdef TARGET_DARWIN_OSX
        if (ProcessOSXShortcuts(event))
        {
          ret = true;
          break;
        }
#endif

        XBMC_Event newEvent;
        newEvent.type = XBMC_KEYDOWN;
        newEvent.key.keysym.scancode = event.key.keysym.scancode;
        newEvent.key.keysym.sym = (XBMCKey) event.key.keysym.sym;
        newEvent.key.keysym.unicode = event.key.keysym.unicode;
        newEvent.key.state = event.key.state;
        newEvent.key.type = event.key.type;
        newEvent.key.which = event.key.which;

        // Check if the Windows keys are down because SDL doesn't flag this.
        uint16_t mod = event.key.keysym.mod;
        uint8_t* keystate = SDL_GetKeyState(NULL);
        if (keystate[SDLK_LSUPER] || keystate[SDLK_RSUPER])
          mod |= XBMCKMOD_LSUPER;
        newEvent.key.keysym.mod = (XBMCMod) mod;

#ifdef HAVE_X11
        // If the keysym.sym is zero try to get it from the scan code
        if (newEvent.key.keysym.sym == 0)
          newEvent.key.keysym.sym = (XBMCKey) SymFromScancode(newEvent.key.keysym.scancode);
#endif

        // don't handle any more messages in the queue until we've handled keydown,
        // if a keyup is in the queue it will reset the keypress before it is handled.
        ret |= g_application.OnEvent(newEvent);
        break;
      }

      case SDL_KEYUP:
      {
        XBMC_Event newEvent;
        newEvent.type = XBMC_KEYUP;
        newEvent.key.keysym.scancode = event.key.keysym.scancode;
        newEvent.key.keysym.sym = (XBMCKey) event.key.keysym.sym;
        newEvent.key.keysym.mod =(XBMCMod) event.key.keysym.mod;
        newEvent.key.keysym.unicode = event.key.keysym.unicode;
        newEvent.key.state = event.key.state;
        newEvent.key.type = event.key.type;
        newEvent.key.which = event.key.which;

        ret |= g_application.OnEvent(newEvent);
        break;
      }

      case SDL_MOUSEBUTTONDOWN:
      {
        XBMC_Event newEvent;
        newEvent.type = XBMC_MOUSEBUTTONDOWN;
        newEvent.button.button = event.button.button;
        newEvent.button.state = event.button.state;
        newEvent.button.type = event.button.type;
        newEvent.button.which = event.button.which;
        newEvent.button.x = event.button.x;
        newEvent.button.y = event.button.y;

        ret |= g_application.OnEvent(newEvent);
        break;
      }

      case SDL_MOUSEBUTTONUP:
      {
        XBMC_Event newEvent;
        newEvent.type = XBMC_MOUSEBUTTONUP;
        newEvent.button.button = event.button.button;
        newEvent.button.state = event.button.state;
        newEvent.button.type = event.button.type;
        newEvent.button.which = event.button.which;
        newEvent.button.x = event.button.x;
        newEvent.button.y = event.button.y;

        ret |= g_application.OnEvent(newEvent);
        break;
      }

      case SDL_MOUSEMOTION:
      {
        if (0 == (SDL_GetAppState() & SDL_APPMOUSEFOCUS))
        {
          CInputManager::GetInstance().SetMouseActive(false);
#if defined(TARGET_DARWIN_OSX)
          // See CApplication::ProcessSlow() for a description as to why we call Cocoa_HideMouse.
          // this is here to restore the pointer when toggling back to window mode from fullscreen.
          Cocoa_ShowMouse();
#endif
          break;
        }
        XBMC_Event newEvent;
        newEvent.type = XBMC_MOUSEMOTION;
        newEvent.motion.xrel = event.motion.xrel;
        newEvent.motion.yrel = event.motion.yrel;
        newEvent.motion.state = event.motion.state;
        newEvent.motion.type = event.motion.type;
        newEvent.motion.which = event.motion.which;
        newEvent.motion.x = event.motion.x;
        newEvent.motion.y = event.motion.y;

        ret |= g_application.OnEvent(newEvent);
        break;
      }

      case SDL_VIDEORESIZE:
      {
        // Under newer osx versions sdl is so fucked up that it even fires resize events
        // that exceed the screen size (maybe some HiDP incompatibility in old SDL?)
        // ensure to ignore those events because it will mess with windowed size
        int RES_SCREEN = g_Windowing.DesktopResolution(g_Windowing.GetCurrentScreen());
        if((event.resize.w > CDisplaySettings::GetInstance().GetResolutionInfo(RES_SCREEN).iWidth) ||
           (event.resize.h > CDisplaySettings::GetInstance().GetResolutionInfo(RES_SCREEN).iHeight))
        {
          break;
        }
        XBMC_Event newEvent;
        newEvent.type = XBMC_VIDEORESIZE;
        newEvent.resize.w = event.resize.w;
        newEvent.resize.h = event.resize.h;
        ret |= g_application.OnEvent(newEvent);
        g_windowManager.MarkDirty();
        break;
      }

      case SDL_VIDEOEXPOSE:
      {
        g_windowManager.MarkDirty();
        break;
      }
#endif // SDL 1.2

#if HAVE_SDL_VERSION == 2
      case SDL_WINDOWEVENT:
      {
        switch( event.window.event )
        {
        case SDL_WINDOWEVENT_MINIMIZED:
          if (g_application.GetRenderGUI())
          {
            g_application.SetRenderGUI(false);
            g_Windowing.NotifyAppActiveChange(g_application.GetRenderGUI());
          }
          break;
        case SDL_WINDOWEVENT_RESTORED:
          if (!g_application.GetRenderGUI())
          {
            g_application.SetRenderGUI(true);
            g_Windowing.NotifyAppActiveChange(g_application.GetRenderGUI());
          }
          break;
        case SDL_WINDOWEVENT_FOCUS_GAINED:
          g_application.m_AppFocused = true;
          g_Windowing.NotifyAppFocusChange(g_application.m_AppFocused);
          break;
        case SDL_WINDOWEVENT_FOCUS_LOST:
          g_application.m_AppFocused = false;
          g_Windowing.NotifyAppFocusChange(g_application.m_AppFocused);
          break;
        }
        break;
      }

      case SDL_KEYDOWN:
      {
        // process any platform specific shortcuts before handing off to XBMC
#ifdef TARGET_DARWIN_OSX
        if (ProcessOSXShortcuts(event))
        {
          ret = true;
          break;
        }
#endif

        // See if there's a text event associated with this keypress
        // This won't catch multi-character composed input, but it's better than nothing
        uint16_t unicode = 0;
        SDL_Event next_event;
        if (SDL_PeepEvents(&next_event, 1, SDL_PEEKEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT) > 0 &&
            next_event.type == SDL_TEXTINPUT)
        {
            SDL_PeepEvents(&next_event, 1, SDL_GETEVENT, SDL_TEXTINPUT, SDL_TEXTINPUT);
            if (next_event.text.text[0] < 0xC0)
                unicode = next_event.text.text[0];
            else
            {
                Uint16 *text16 = SDL_iconv_utf8_ucs2(next_event.text.text);
                if (text16)
                {
                    unicode = text16[0];
                    SDL_free(text16);
                }
            }
        }

        XBMC_Event newEvent;
        newEvent.type = XBMC_KEYDOWN;
        newEvent.key.keysym.scancode = event.key.keysym.scancode;
        newEvent.key.keysym.sym = LookupKeysym(event.key.keysym.sym);
        newEvent.key.keysym.unicode = unicode;
        newEvent.key.state = event.key.state;

        // Check if the Windows keys are down because SDL doesn't flag this.
        uint16_t mod = event.key.keysym.mod;
        const uint8_t* keystate = SDL_GetKeyboardState(NULL);
        if (keystate[SDL_SCANCODE_LGUI] || keystate[SDL_SCANCODE_RGUI])
          mod |= XBMCKMOD_LSUPER;
        newEvent.key.keysym.mod = (XBMCMod) mod;

#ifdef HAVE_X11
        // If the keysym.sym is zero try to get it from the scan code
        if (newEvent.key.keysym.sym == 0)
          newEvent.key.keysym.sym = (XBMCKey) SymFromScancode(newEvent.key.keysym.scancode);
#endif

        // don't handle any more messages in the queue until we've handled keydown,
        // if a keyup is in the queue it will reset the keypress before it is handled.
        ret |= g_application.OnEvent(newEvent);
        break;
      }

      case SDL_KEYUP:
      {
        XBMC_Event newEvent;
        newEvent.type = XBMC_KEYUP;
        newEvent.key.keysym.scancode = event.key.keysym.scancode;
        newEvent.key.keysym.sym = LookupKeysym(event.key.keysym.sym);
        newEvent.key.keysym.mod =(XBMCMod) event.key.keysym.mod;
        newEvent.key.keysym.unicode = 0;
        newEvent.key.state = event.key.state;

        ret |= g_application.OnEvent(newEvent);
        break;
      }

      case SDL_MOUSEBUTTONDOWN:
      {
        XBMC_Event newEvent;
        newEvent.type = XBMC_MOUSEBUTTONDOWN;
        newEvent.button.button = event.button.button;
        newEvent.button.state = event.button.state;
        newEvent.button.which = event.button.which;
        newEvent.button.x = event.button.x;
        newEvent.button.y = event.button.y;

        ret |= g_application.OnEvent(newEvent);
        break;
      }

      case SDL_MOUSEBUTTONUP:
      {
        XBMC_Event newEvent;
        newEvent.type = XBMC_MOUSEBUTTONUP;
        newEvent.button.button = event.button.button;
        newEvent.button.state = event.button.state;
        newEvent.button.which = event.button.which;
        newEvent.button.x = event.button.x;
        newEvent.button.y = event.button.y;

        ret |= g_application.OnEvent(newEvent);
        break;
      }

      case SDL_MOUSEMOTION:
      {
        XBMC_Event newEvent;
        newEvent.type = XBMC_MOUSEMOTION;
        newEvent.motion.xrel = event.motion.xrel;
        newEvent.motion.yrel = event.motion.yrel;
        newEvent.motion.state = event.motion.state;
        newEvent.motion.which = event.motion.which;
        newEvent.motion.x = event.motion.x;
        newEvent.motion.y = event.motion.y;

        ret |= g_application.OnEvent(newEvent);
        break;
      }
#endif // SDL 2.0

      case SDL_USEREVENT:
      {
        XBMC_Event newEvent;
        newEvent.type = XBMC_USEREVENT;
        newEvent.user.code = event.user.code;
        ret |= g_application.OnEvent(newEvent);
        break;
      }
    }
    memset(&event, 0, sizeof(SDL_Event));
  }

  return ret;
}

size_t CWinEventsSDL::GetQueueSize()
{
  int ret = 0;

#if HAVE_SDL_VERSION == 1
  if (-1 == (ret = SDL_PeepEvents(NULL, 0, SDL_PEEKEVENT, ~0)))
    ret = 0;
#endif
#if HAVE_SDL_VERSION == 2
  if (-1 == (ret = SDL_PeepEvents(NULL, 0, SDL_PEEKEVENT, SDL_FIRSTEVENT, SDL_LASTEVENT)))
    ret = 0;
#endif

  return ret;
}

#ifdef TARGET_DARWIN_OSX
bool CWinEventsSDL::ProcessOSXShortcuts(SDL_Event& event)
{
  static bool shift = false, cmd = false;

  cmd   = !!(SDL_GetModState() & (KMOD_LMETA  | KMOD_RMETA ));
  shift = !!(SDL_GetModState() & (KMOD_LSHIFT | KMOD_RSHIFT));

  if (cmd && event.key.type == SDL_KEYDOWN)
  {
    char keysymbol = event.key.keysym.sym;

    // if the unicode is in the ascii range
    // use this instead for getting the real
    // character based on the used keyboard layout
    // see http://lists.libsdl.org/pipermail/sdl-libsdl.org/2004-May/043716.html
    if (!(event.key.keysym.unicode & 0xff80))
      keysymbol = event.key.keysym.unicode;

    switch(keysymbol)
    {
    case SDLK_q:  // CMD-q to quit
      if (!g_application.m_bStop)
        CApplicationMessenger::GetInstance().PostMsg(TMSG_QUIT);
      return true;

    case SDLK_f: // CMD-f to toggle fullscreen
      g_application.OnAction(CAction(ACTION_TOGGLE_FULLSCREEN));
      return true;

    case SDLK_s: // CMD-3 to take a screenshot
      g_application.OnAction(CAction(ACTION_TAKE_SCREENSHOT));
      return true;

    case SDLK_h: // CMD-h to hide
      g_Windowing.Hide();
      return true;

    case SDLK_m: // CMD-m to minimize
      CApplicationMessenger::GetInstance().PostMsg(TMSG_MINIMIZE);
      return true;

    default:
      return false;
    }
  }

  return false;
}

#elif defined(TARGET_POSIX)

bool CWinEventsSDL::ProcessLinuxShortcuts(SDL_Event& event)
{
  bool alt = false;

  alt = !!(SDL_GetModState() & (XBMCKMOD_LALT  | XBMCKMOD_RALT));

  if (alt && event.key.type == SDL_KEYDOWN)
  {
    switch(event.key.keysym.sym)
    {
      case SDLK_TAB:  // ALT+TAB to minimize/hide
        g_application.Minimize();
        return true;
      default:
        break;
    }
  }
  return false;
}
#endif

#endif
