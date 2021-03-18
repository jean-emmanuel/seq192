//----------------------------------------------------------------------------
//
//  This file is part of seq24.
//
//  seq24 is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  seq24 is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with seq24; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//-----------------------------------------------------------------------------

#ifndef SEQ24_FONT
#define SEQ24_FONT

#include <gtkmm/image.h>
#include <gtkmm/widget.h>
#include <gtkmm/drawingarea.h>


#include <string>

using namespace Gtk;

class font
{

private:

    Glib::RefPtr<Gdk::Pixmap>   m_pixmap;
    Glib::RefPtr<Gdk::Pixmap>   m_black_pixmap;
    Glib::RefPtr<Gdk::Pixmap>   m_white_pixmap;
    Glib::RefPtr<Gdk::Bitmap>   m_clip_mask;

public:

    enum Color {
        BLACK = 0,
        WHITE = 1
    };

    font( );

    void init( Glib::RefPtr<Gdk::Window> a_window );

    void render_string_on_drawable(
        Glib::RefPtr<Gdk::GC> m_gc,
        int x,
        int y,
        Glib::RefPtr<Gdk::Drawable> a_draw,
        const char *str,
        font::Color col );

};

extern font *p_font_renderer;

#endif
