/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2016 The XCSoar Project
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

#ifndef XCSOAR_SCREEN_CANVAS_HPP
#define XCSOAR_SCREEN_CANVAS_HPP

// IWYU pragma: begin_exports

#ifdef ENABLE_OPENGL
#include "Screen/OpenGL/Canvas.hpp"
#elif defined(USE_MEMORY_CANVAS)
#include "Screen/Memory/Canvas.hpp"
#elif defined(USE_GDI)
#include "Screen/GDI/Canvas.hpp"
#else
#error No Canvas implementation
#endif

// IWYU pragma: end_exports

#endif
