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


#include "lfownd.h"
#include <string>
#include <math.h>
#include <sigc++/slot.h>
#include "seqedit.h"
using std::string;
using sigc::mem_fun;

#define PI (3.14159265359)


double lfownd::wave_func(double a_angle, int wave_type)
{
    double result = 0.0;
    switch (wave_type)
    {
    case WAVE_SINE:
        result = sin(a_angle * PI * 2.0);
        break;

    case WAVE_SAWTOOTH:
        result = (a_angle - int(a_angle)) * 2.0 - 1.0;
        break;

    case WAVE_REVERSE_SAWTOOTH:
        result = (a_angle - int(a_angle)) * -2.0 + 1.0;
        break;

    case WAVE_TRIANGLE:
    {
        double tmp = a_angle * 2.0;
        result = (tmp - int(tmp));
        if ((int(tmp)) % 2 == 1)
            result = 1.0 - result;

        result = result * 2.0 - 1.0;
        break;
    }
    default:
        break;
    }
    /*
     * printf("y[%s](%f)=%f\n", wave_type_name(wavetype).c_str(), angle, result);
     */
    return result;
}

lfownd::~lfownd()
{
    //printf("Delete destructor lfownd\n");
}

lfownd::lfownd(sequence *a_seq, seqdata *a_seqdata)
{
    m_seq = a_seq;
    m_seqdata = a_seqdata;
    m_hbox = manage(new HBox(true, 8));
    /* main window */
    string title = "seq24 - LFO Editor - ";
    title.append(m_seq->get_name());
    set_title(title);
    set_size_request(400, 300);

    m_scale_value = manage(new VScale(0, 127, .1));
    m_scale_range = manage(new VScale(0, 127, .1));
    m_scale_speed = manage(new VScale(0, 16, .01));
    m_scale_phase = manage(new VScale(0,1,.01));
    m_scale_wave = manage(new VScale(1,5,1));
    m_wave_name  = manage(new Gtk::Label("Sine"));

    m_scale_value->set_tooltip_text
    (
        "Value: a kind of DC offset for the data value. Starts at 64."
    );
    m_scale_range->set_tooltip_text
    (
        "Range: controls the depth of modulation. Starts at 64."
    );
    m_scale_speed->set_tooltip_text
    (
        "Speed: the number of periods per pattern (divided by beat width, "
        "normally 4).  For long patterns, this parameter needs to be set "
        "high in some cases.  Also subject to an 'anti-aliasing' effect in "
        "some parts of the range, especially for short patterns. "
        "Try it.  For short patterns, try a value of 1."
    );
    m_scale_phase->set_tooltip_text
    (
        "Phase: phase shift in a beat width (quarter note). "
        "A value of 1 is a phase shift of 360 degrees."
    );
    m_scale_wave->set_tooltip_text
    (
        "Wave type: 1 = sine; 2 = ramp sawtooth; 3 = decay sawtooth; "
        "4 = triangle."
    );

    m_scale_value->set_value(64);
    m_scale_range->set_value(64);
    m_scale_speed->set_value(0);
    m_scale_phase->set_value(0);
    m_scale_wave->set_value(1);
    m_scale_value->signal_value_changed().connect(mem_fun( *this, &lfownd::scale_lfo_change));
    m_scale_range->signal_value_changed().connect(mem_fun( *this, &lfownd::scale_lfo_change));
    m_scale_speed->signal_value_changed().connect(mem_fun( *this, &lfownd::scale_lfo_change));
    m_scale_phase->signal_value_changed().connect(mem_fun( *this, &lfownd::scale_lfo_change));
    m_scale_wave->signal_value_changed().connect(mem_fun( *this, &lfownd::scale_lfo_change));

    Gtk::VBox * vbox1 = manage(new Gtk::VBox(false, 2));
    Gtk::VBox * vbox2 = manage(new Gtk::VBox(false, 2));
    Gtk::VBox * vbox3 = manage(new Gtk::VBox(false, 2));
    Gtk::VBox * vbox4 = manage(new Gtk::VBox(false, 2));
    Gtk::VBox * vbox5 = manage(new Gtk::VBox(false, 2));
    Gtk::Label * label1 = manage(new Gtk::Label("Value"));
    Gtk::Label * label2 = manage(new Gtk::Label("Range"));
    Gtk::Label * label3 = manage(new Gtk::Label("Speed"));
    Gtk::Label * label4 = manage(new Gtk::Label("Phase"));
    Gtk::Label * label5 = manage(new Gtk::Label("Type"));
    m_wave_name->set_width_chars(12);
    vbox1->pack_start(*label1,  false, false, 8);
    vbox1->pack_start(*m_scale_value,  true, true, 0);
    vbox2->pack_start(*label2,  false, false, 8);
    vbox2->pack_start(*m_scale_range,  true, true, 0);
    vbox3->pack_start(*label3,  false, false, 8);
    vbox3->pack_start(*m_scale_speed,  true, true, 0);
    vbox4->pack_start(*label4,  false, false, 8);
    vbox4->pack_start(*m_scale_phase,  true, true, 0);
    vbox5->pack_start(*label5,  false, false, 8);
    vbox5->pack_start(*m_scale_wave,  true, true, 0);
    vbox5->pack_start(*m_wave_name, false, false, 0);
    vbox5->pack_start(*manage(new Gtk::Label(" ")), false, false, 0);
    m_hbox->pack_start(*vbox1);
    m_hbox->pack_start(*vbox2);
    m_hbox->pack_start(*vbox3);
    m_hbox->pack_start(*vbox4);
    m_hbox->pack_start(*vbox5, true, true, 4);
    add(*m_hbox);
}

void lfownd::toggle_visible()
{
    if(!get_visible())
    {
        show_all();
        raise();
    }
    else
        hide();
}

void lfownd::scale_lfo_change()
{
    int wtype = int(m_scale_wave->get_value());
    m_value = m_scale_value->get_value();
    m_range = m_scale_range->get_value();
    m_speed = m_scale_speed->get_value();
    m_phase = m_scale_phase->get_value();
    m_wave = wave_type_t(wtype);
    m_wave_name->set_text(wave_type_name(wave_type_t(wtype)));
    m_seq->change_event_data_lfo
    (
        m_value,
        m_range,
        m_speed,
        m_phase,
        m_wave,
        m_seqdata->m_status,
        m_seqdata->m_cc
    );
    m_seqdata->update_pixmap();
    m_seqdata->draw_pixmap_on_window();
}

bool lfownd::on_focus_out_event(GdkEventFocus* p0 )
{
    if(m_seq->get_hold_undo())
    {
        m_seq->push_undo(true);
        m_seq->set_hold_undo(false);
    }
    return true;
}

std::string lfownd::wave_type_name (wave_type_t wavetype)
{
    std::string result = "None";
    switch (wavetype)
    {
    case WAVE_SINE:
        result = "Sine";
        break;

    case WAVE_SAWTOOTH:
        result = "Ramp";
        break;

    case WAVE_REVERSE_SAWTOOTH:
        result = "Decay";
        break;

    case WAVE_TRIANGLE:
        result = "Triangle";
        break;

    default:
        break;
    }
    return result;
}
