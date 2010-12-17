/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2010 The XCSoar Project
  A detailed list of copyright holders can be found in the file "AUTHORS".

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

#include "InputParser.hpp"
#include "InputConfig.hpp"
#include "InputEvents.hpp"
#include "IO/LineReader.hpp"
#include "Util/StringUtil.hpp"
#include "UtilsText.hpp"
#include "LogFile.hpp"
#include "Compatibility/string.h" /* for _ttoi() */

#include <string.h>
#include <tchar.h>
#include <stdio.h>

static bool
parse_assignment(TCHAR *buffer, const TCHAR *&key, const TCHAR *&value)
{
  TCHAR *separator = _tcschr(buffer, '=');
  if (separator == NULL || separator == buffer)
    return false;

  *separator = _T('\0');

  key = buffer;
  value = separator + 1;

  return true;
}

void
ParseInputFile(InputConfig &config, TLineReader &reader)
{
  // TODO code - Safer sizes, strings etc - use C++ (can scanf restrict length?)

  // Multiple modes (so large string)
  TCHAR d_mode[1024] = _T("");
  TCHAR d_type[256] = _T("");
  TCHAR d_data[256] = _T("");
  unsigned event_id = 0;
  TCHAR d_label[256] = _T("");
  int d_location = 0;
  TCHAR d_event[256] = _T("");
  TCHAR d_misc[256] = _T("");

  int line = 0;

  // Read from the file
  TCHAR *buffer;
  while ((buffer = reader.read()) != NULL) {
    TrimRight(buffer);
    line++;

    const TCHAR *key, *value;

    // experimental: if the first line is "#CLEAR" then the whole default config is cleared
    //               and can be overwritten by file
    if (line == 1 && _tcscmp(buffer, _T("#CLEAR")) == 0) {
      config.clear_all_events();
    } else if (buffer[0] == _T('\0')) {
      // Check valid line? If not valid, assume next record (primative, but works ok!)
      // General checks before continue...
      if (!string_is_empty(d_mode)) {

        TCHAR *token;

        // For each mode
        token = _tcstok(d_mode, _T(" "));

        // General errors - these should be true
        assert(d_location >= 0);
        assert(d_location < 1024);

        const TCHAR *new_label = NULL;
        while (token != NULL) {

          // All modes are valid at this point
          int mode_id = config.make_mode(token);
          assert(mode_id >= 0);

          // Make label event
          // TODO code: Consider Reuse existing entries...
          if (d_location > 0) {
            // Only copy this once per object - save string space
            if (!new_label) {
              new_label = StringMallocParse(d_label);
            }

            config.append_menu(mode_id, new_label, d_location, event_id);
          }

          // Make key (Keyboard input)
          // key - Hardware key or keyboard
          if (_tcscmp(d_type, _T("key")) == 0) {
            // Get the int key (eg: APP1 vs 'a')
            unsigned key = InputEvents::findKey(d_data);
            if (key > 0)
              config.Key2Event[mode_id][key] = event_id;
            else
              LogStartUp(_T("Invalid key data: %s at %i"), d_data, line);

          // Make gce (Glide Computer Event)
          // GCE - Glide Computer Event
          } else if (_tcscmp(d_type, _T("gce")) == 0) {
            // Get the int key (eg: APP1 vs 'a')
            int key = InputEvents::findGCE(d_data);
            if (key >= 0)
              config.GC2Event[mode_id][key] = event_id;
            else
              LogStartUp(_T("Invalid GCE data: %s at %i"), d_data, line);

          // Make ne (NMEA Event)
          // NE - NMEA Event
          } else if (_tcscmp(d_type, _T("ne")) == 0) {
            // Get the int key (eg: APP1 vs 'a')
            int key = InputEvents::findNE(d_data);
            if (key >= 0)
              config.N2Event[mode_id][key] = event_id;
            else
              LogStartUp(_T("Invalid GCE data: %s at %i"), d_data, line);

          // label only - no key associated (label can still be touch screen)
          } else if (_tcscmp(d_type, _T("label")) == 0) {
            // Nothing to do here...

          } else {
            LogStartUp(_T("Invalid type: %s at %i"), d_type, line);
          }

          token = _tcstok(NULL, _T(" "));
        }
      }

      // Clear all data.
      _tcscpy(d_mode, _T(""));
      _tcscpy(d_type, _T(""));
      _tcscpy(d_data, _T(""));
      event_id = 0;
      _tcscpy(d_label, _T(""));
      d_location = 0;

    } else if (string_is_empty(buffer) || buffer[0] == _T('#')) {
      // Do nothing - we probably just have a comment line
      // NOTE: Do NOT display buffer to user as it may contain an invalid stirng !

    } else if (parse_assignment(buffer, key, value)) {
      if (_tcscmp(key, _T("mode")) == 0) {
        if (_tcslen(value) < 1024) {
          _tcscpy(d_mode, value);
        }
      } else if (_tcscmp(key, _T("type")) == 0) {
        if (_tcslen(value) < 256)
          _tcscpy(d_type, value);
      } else if (_tcscmp(key, _T("data")) == 0) {
        if (_tcslen(value) < 256)
          _tcscpy(d_data, value);
      } else if (_tcscmp(key, _T("event")) == 0) {
        if (_tcslen(value) < 256) {
          _tcscpy(d_event, _T(""));
          _tcscpy(d_misc, _T(""));
          int ef;

          #if defined(__BORLANDC__)
          memset(d_event, 0, sizeof(d_event));
          memset(d_misc, 0, sizeof(d_event));
          if (_tcschr(value, ' ') == NULL) {
            _tcscpy(d_event, value);
          } else {
          #endif

          ef = _stscanf(value, _T("%[^ ] %[A-Za-z0-9 \\/().,]"), d_event,
              d_misc);

          #if defined(__BORLANDC__)
          }
          #endif

          if ((ef == 1) || (ef == 2)) {

            // TODO code: Consider reusing existing identical events

            pt2Event event = InputEvents::findEvent(d_event);
            if (event) {
              TCHAR *allocated = StringMallocParse(d_misc);
              event_id = config.append_event(event, allocated, event_id);

              /* not freeing the string, because
                 InputConfig::append_event() stores the string point
                 without duplicating it; strictly speaking, this is a
                 memory leak, but the input file is only loaded once
                 at startup, so this is acceptable; in return, we
                 don't have to duplicate the hard-coded defaults,
                 which saves some memory */
              //free(allocated);

            } else {
              LogStartUp(_T("Invalid event type: %s at %i"), d_event, line);
            }
          } else {
            LogStartUp(_T("Invalid event type at %i"), line);
          }
        }
      } else if (_tcscmp(key, _T("label")) == 0) {
        _tcscpy(d_label, value);
      } else if (_tcscmp(key, _T("location")) == 0) {
        d_location = _ttoi(value);

      } else {
        LogStartUp(_T("Invalid key/value pair %s=%s at %i"), key, value, line);
      }
    } else  {
      LogStartUp(_T("Invalid line at %i"), line);
    }

  }
}
