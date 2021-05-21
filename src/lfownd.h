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

#pragma once

#include <gtkmm.h>
#include <sigc++/bind.h>
#include "globals.h"
#include "sequence.h"
#include "seqdata.h"

using namespace Gtk;

enum wave_type_t
{
    WAVE_NONE               = 0,    /**< No waveform, never used.           */
    WAVE_SINE               = 1,    /**< Sine wave modulation.              */
    WAVE_SAWTOOTH           = 2,    /**< Saw-tooth (ramp) modulation.       */
    WAVE_REVERSE_SAWTOOTH   = 3,    /**< Reverse saw-tooth (decay).         */
    WAVE_TRIANGLE           = 4     /**< No waveform, never used.           */
};

class lfownd: public Gtk::Window
{
private:

    VScale *m_scale_value;
    VScale *m_scale_range;
    VScale *m_scale_speed;
    VScale *m_scale_phase;
    VScale *m_scale_wave;
    Gtk::Label * m_wave_name;       /**< Human readable name for wave type. */

    double m_value;
    double m_range;
    double m_speed;
    double m_phase;
    wave_type_t m_wave;

    HBox *m_hbox;
    sequence *m_seq;
    seqdata *m_seqdata;

    void scale_lfo_change();
    bool on_focus_out_event(GdkEventFocus* p0 );
    std::string wave_type_name (wave_type_t wv);

public:

    lfownd (sequence *a_seq, seqdata *a_seqdata);
    virtual ~lfownd();

    static double wave_func(double a_angle, int wave_type);
    void toggle_visible();

};
