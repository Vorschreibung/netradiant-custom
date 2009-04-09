/*
Copyright (C) 2001-2006, William Joseph.
All Rights Reserved.

This file is part of GtkRadiant.

GtkRadiant is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

GtkRadiant is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GtkRadiant; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#if !defined(INCLUDED_GTKUTIL_GLFONT_H)
#define INCLUDED_GTKUTIL_GLFONT_H

typedef unsigned int GLuint;

class GLFont
{
  GLuint m_displayList;
  int m_pixelHeight;
  int m_pixelAscent;
  int m_pixelDescent;
public:
  GLFont(GLuint displayList, int asc, int desc, int pixelHeight) : m_displayList(displayList), m_pixelHeight(pixelHeight), m_pixelAscent(asc), m_pixelDescent(desc)
  {
  }
  GLuint getDisplayList() const
  {
    return m_displayList;
  }
  int getPixelHeight() const
  {
    return m_pixelHeight;
  }
  int getPixelAscent() const
  {
    return m_pixelAscent;
  }
  int getPixelDescent() const
  {
    return m_pixelDescent;
  }
};

GLFont glfont_create(const char* font_string);
void glfont_release(GLFont& font);

#endif
