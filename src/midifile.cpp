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

#include "midifile.h"
#include <iostream>

midifile::midifile(const Glib::ustring& a_name)
{
    m_name = a_name;
    m_pos = 0;
}

midifile::~midifile ()
{
}


unsigned long
midifile::read_long ()
{
    unsigned long ret = 0;

    ret += (m_d[m_pos++] << 24);
    ret += (m_d[m_pos++] << 16);
    ret += (m_d[m_pos++] << 8);
    ret += (m_d[m_pos++]);

    return ret;
}

unsigned short
midifile::read_short ()
{
    unsigned short ret = 0;

    ret += (m_d[m_pos++] << 8);
    ret += (m_d[m_pos++]);

    //printf( "read_short 0x%4x\n", ret );
    return ret;
}

unsigned long
midifile::read_var ()
{
    unsigned long ret = 0;
    unsigned char c;

    /* while bit #7 is set */
    while (((c = m_d[m_pos++]) & 0x80) != 0x00)
    {
        /* shift ret 7 bits */
        ret <<= 7;
        /* add bits 0-6 */
        ret += (c & 0x7F);
    }

    /* bit was clear */
    ret <<= 7;
    ret += (c & 0x7F);

    return ret;
}


bool midifile::parse (perform * a_perf, int a_screen_set)
{
    /* open binary file */
    ifstream file(m_name.c_str(), ios::in | ios::binary | ios::ate);

    if (!file.is_open ()) {
        fprintf(stderr, "Error opening MIDI file\n");
        return false;
    }

    int file_size = file.tellg ();

    /* run to start */
    file.seekg (0, ios::beg);

    /* alloc data */
    m_d = (unsigned char *) new char[file_size];
    if (m_d == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return false;
    }
    file.read ((char *) m_d, file_size);
    file.close ();

    /* set position to 0 */
    m_pos = 0;

    /* chunk info */
    unsigned long ID;
    unsigned long TrackLength;

    /* time */
    unsigned long Delta;
    unsigned long RunningTime;
    unsigned long CurrentTime;

    unsigned short Format;			/* 0,1,2 */
    unsigned short NumTracks;
    unsigned short ppqn;
    unsigned short perf;
    int c;

    /* track name from file */
    char TrackName[256];

    /* used in small loops */
    int i;

    /* sequence pointer */
    sequence * seq;
    event e;

    /* read in header */
    ID = read_long ();
    TrackLength = read_long ();
    Format = read_short ();
    NumTracks = read_short ();
    ppqn = read_short ();

    //printf( "[%8lX] len[%ld] fmt[%d] num[%d] ppqn[%d]\n",
    //      ID, TrackLength, Format, NumTracks, ppqn );

    /* magic number 'MThd' */
    if (ID != 0x4D546864) {
        fprintf(stderr, "Invalid MIDI header detected: %8lX\n", ID);
        delete[]m_d;
        return false;
    }

    /* we are only supporting format 1 for now */
    if (Format != 1) {
        fprintf(stderr, "Unsupported MIDI format detected: %d\n", Format);
        delete[]m_d;
        return false;
    }

    /* We should be good to load now   */
    /* for each Track in the midi file */
    for (int curTrack = 0; curTrack < NumTracks; curTrack++)
    {
        /* done for each track */
        bool done = false;
        perf = 0;

        /* events */
        unsigned char status = 0, type, data[2], laststatus;
        long len;
        unsigned long proprietary = 0;

        /* Get ID + Length */
        ID = read_long ();
        TrackLength = read_long ();
        //printf( "[%8lX] len[%8lX]\n", ID,  TrackLength );


        /* magic number 'MTrk' */
        if (ID == 0x4D54726B)
        {
            /* we know we have a good track, so we can create
               a new sequence to dump it to */
            seq = new sequence ();
            if (seq == NULL) {
                fprintf(stderr, "Memory allocation failed\n");
                delete[]m_d;
                return false;
            }
            seq->set_master_midi_bus (&a_perf->m_master_bus);

            /* reset time */
            RunningTime = 0;

            /* this gets each event in the Trk */
            while (!done)
            {
                /* get time delta */
                Delta = read_var ();

                /* get status */
                laststatus = status;
                status = m_d[m_pos];

                /* is it a status bit ? */
                if ((status & 0x80) == 0x00)
                {
                    /* no, its a running status */
                    status = laststatus;
                }
                else
                {
                    /* its a status, increment */
                    m_pos++;
                }

                /* set the members in event */
                e.set_status (status);

                // ORL rajoute ici le BITWISE AND pour le truc cu channelless
                //fprintf(stderr,"status %d | 0xF0 %d | status & 0xF0 %d\n",status,0xF0,status & 0xF0);

                RunningTime += Delta;
                /* current time is ppqn according to the file,
                   we have to adjust it to our own ppqn.
                   PPQN / ppqn gives us the ratio */
                CurrentTime = (RunningTime * c_ppqn) / ppqn;

                //printf( "D[%6ld] [%6ld] %02X\n", Delta, CurrentTime, status);
                e.set_timestamp (CurrentTime);

                /* switch on the channelless status */
                switch (status & 0xF0)
                {
                    /* case for those with 2 data bytes */
                    case EVENT_NOTE_OFF:
                    case EVENT_NOTE_ON:
                    case EVENT_AFTERTOUCH:
                    case EVENT_CONTROL_CHANGE:
                    case EVENT_PITCH_WHEEL:

                        data[0] = m_d[m_pos++];
                        data[1] = m_d[m_pos++];

                   //     fprintf(stderr,"data[0] : %d | data[1] %d\n",data[0],data[1]);

                        // some files have vel=0 as note off
                        if ((status & 0xF0) == EVENT_NOTE_ON && data[1] == 0)
                        {
                            e.set_status (EVENT_NOTE_OFF);
                        }

                        //printf( "%02X %02X\n", data[0], data[1] );

                        /* set data and add */
                        e.set_data (data[0], data[1]);
                        seq->add_event (&e);

                        /* set midi channel */
                        seq->set_midi_channel (status & 0x0F);
//                        fprintf(stderr,"Midi Channel : %d | 0x0F %d\n",status & 0x0F, 0x0F);
                        break;

                        /* one data item */
                    case EVENT_PROGRAM_CHANGE:
                    case EVENT_CHANNEL_PRESSURE:

                        data[0] = m_d[m_pos++];
                        //printf( "%02X\n", data[0] );

                        /* set data and add */
                        e.set_data (data[0]);
                        seq->add_event (&e);

                        /* set midi channel */
                        seq->set_midi_channel (status & 0x0F);
                        break;

                        /* meta midi events ---  this should be FF !!!!!  */
                    case 0xF0:

                        if (status == 0xFF)
                        {
                            /* get meta type */
                            type = m_d[m_pos++];
                            len = read_var ();

                            //printf( "%02X %08X ", type, (int) len );

                            switch (type)
                            {
                                /* proprietary */
                                case 0x7f:

                                    /* FF 7F len data  */
                                    if (len > 4)
                                    {
                                        proprietary = read_long ();
                                        len -= 4;
                                    }

                                    if (proprietary == c_midibus)
                                    {
                                        seq->set_midi_bus (m_d[m_pos++]);
                                        len--;
                                    }

                                    else if (proprietary == c_midich)
                                    {
                                        seq->set_midi_channel (m_d[m_pos++]);
                                        len--;
                                    }

                                    else if (proprietary == c_timesig)
                                    {
                                        seq->set_bpm (m_d[m_pos++]);
                                        seq->set_bw (m_d[m_pos++]);
                                        len -= 2;
                                    }

                                    else if (proprietary == c_triggers)
                                    {
                                        int num_triggers = len / 4;

                                        for (int i = 0; i < num_triggers; i += 2)
                                        {
                                            unsigned long on = read_long ();
                                            unsigned long length = (read_long () - on);
                                            len -= 8;
                                            seq->add_trigger(on, length, 0, false);
                                        }
                                    }

                                    else if (proprietary == c_triggers_new)
                                    {
                                        int num_triggers = len / 12;

                                        //printf( "num_triggers[%d]\n", num_triggers );
                                        for (int i = 0; i < num_triggers; i++)
                                        {
                                            unsigned long on = read_long ();
                                            unsigned long off = read_long ();
                                            unsigned long length = off - on + 1;
                                            unsigned long offset = read_long ();

                                            //printf( "< start[%d] end[%d] offset[%d]\n",
                                            //        on, off, offset );

                                            len -= 12;
                                            seq->add_trigger (on, length, offset, false);
                                        }
                                    }

                                    /* eat the rest */
                                    m_pos += len;
                                    break;

                                    /* Trk Done */
                                case 0x2f:

                                    // If delta is 0, then another event happened at the same time 
                                    // as the track end.  the sequence class will discard the last 
                                    // note.  This is a fix for that.   Native Seq24 file will always
                                    // have a Delta >= 1
                                    if ( Delta == 0 ){
                                        CurrentTime += 1;
                                    }

                                    seq->set_length (CurrentTime, false);
                                    seq->zero_markers ();
                                    done = true;
                                    break;

                                    /* Track name */
                                case 0x03:
                                    for (i = 0; i < len; i++)
                                    {
                                        TrackName[i] = m_d[m_pos++];
                                    }

                                    TrackName[i] = '\0';

                                    //printf("[%s]\n", TrackName );
                                    seq->set_name (TrackName);
                                    break;

                                    /* sequence number */
                                case 0x00:
                                    if (len == 0x00)
                                        perf = 0;
                                    else
                                        perf = read_short ();

                                    //printf ( "perf %d\n", perf );
                                    break;

                                default:
                                    for (i = 0; i < len; i++)
                                    {
                                        c = m_d[m_pos++];
                                        //printf( "%02X ", c  );  
                                    }
                                    //printf("\n");
                                    break;
                            }
                        }
                        else if(status == 0xF0)
                        {
                            /* sysex */
                            len = read_var ();

                            /* skip it */
                            m_pos += len;

                            fprintf(stderr, "Warning, no support for SYSEX messages, discarding.\n");
                        }
                        else
                        {
                            fprintf(stderr, "Unexpected system event : 0x%.2X", status);
                            delete[]m_d;
                            return false;
                        }

                        break;

                    default:
                        fprintf(stderr, "Unsupported MIDI event: %hhu\n", status);
                        delete[]m_d;
                        return false;
                        break;
                }

            }			/* while ( !done loading Trk chunk */

            /* the sequence has been filled, add it  */
            //printf ( "add_sequence( %d )\n", perf + (a_screen_set * c_seqs_in_set)); 
            a_perf->add_sequence (seq, perf + (a_screen_set * c_seqs_in_set));
        }

        /* dont know what kind of chunk */
        else
        {
            /* its not a MTrk, we dont know how to deal with it, 
               so we just eat it */
            fprintf(stderr, "Unsupported MIDI header detected: %8lX\n", ID);
            m_pos += TrackLength;
            done = true;
        }

    }				/* for(eachtrack) */

    //printf ( "file_size[%d] m_pos[%d]\n", file_size, m_pos );

    if ((file_size - m_pos) > (int) sizeof (unsigned long))
    {
        ID = read_long ();
        if (ID == c_midictrl)
        {
            unsigned long seqs = read_long ();

            for (unsigned int i = 0; i < seqs; i++)
            {

                a_perf->get_midi_control_toggle (i)->m_active = m_d[m_pos++];
                a_perf->get_midi_control_toggle (i)->m_inverse_active =
                    m_d[m_pos++];
                a_perf->get_midi_control_toggle (i)->m_status = m_d[m_pos++];
                a_perf->get_midi_control_toggle (i)->m_data = m_d[m_pos++];
                a_perf->get_midi_control_toggle (i)->m_min_value = m_d[m_pos++];
                a_perf->get_midi_control_toggle (i)->m_max_value = m_d[m_pos++];

                a_perf->get_midi_control_on (i)->m_active = m_d[m_pos++];
                a_perf->get_midi_control_on (i)->m_inverse_active =
                    m_d[m_pos++];
                a_perf->get_midi_control_on (i)->m_status = m_d[m_pos++];
                a_perf->get_midi_control_on (i)->m_data = m_d[m_pos++];
                a_perf->get_midi_control_on (i)->m_min_value = m_d[m_pos++];
                a_perf->get_midi_control_on (i)->m_max_value = m_d[m_pos++];

                a_perf->get_midi_control_off (i)->m_active = m_d[m_pos++];
                a_perf->get_midi_control_off (i)->m_inverse_active =
                    m_d[m_pos++];
                a_perf->get_midi_control_off (i)->m_status = m_d[m_pos++];
                a_perf->get_midi_control_off (i)->m_data = m_d[m_pos++];
                a_perf->get_midi_control_off (i)->m_min_value = m_d[m_pos++];
                a_perf->get_midi_control_off (i)->m_max_value = m_d[m_pos++];
            }
        }

        /* Get ID + Length */
        ID = read_long ();
        if (ID == c_midiclocks)
        {
            TrackLength = read_long ();
            /* TrackLength is nyumber of buses */
            for (unsigned int x = 0; x < TrackLength; x++)
            {
                int bus_on = m_d[m_pos++];
                a_perf->get_master_midi_bus ()->set_clock (x, (clock_e) bus_on);
            }
        }
    }

    if ((file_size - m_pos) > (int) sizeof (unsigned long))
    {
        /* Get ID + Length */
        ID = read_long ();
        if (ID == c_notes)
        {
            unsigned int screen_sets = read_short ();

            for (unsigned int x = 0; x < screen_sets; x++)
            {
                /* get the length of the string */
                unsigned int len = read_short ();
                char * notes = new char[len + 1];

                for (unsigned int i = 0; i < len; i++)
                    notes[i] = m_d[m_pos++];

                notes[len] = '\0';
                string notess (notes);
                a_perf->set_screen_set_notepad (x, &notess);
                delete[]notes;
            }
        }
    }

    if ((file_size - m_pos) > (int) sizeof (unsigned int))
    {
        /* Get ID + Length */
        ID = read_long ();
        if (ID == c_bpmtag)
        {
            long bpm = read_long ();
            a_perf->set_bpm (bpm);
        }
    }

    // read in the mute group info.
    if ((file_size - m_pos) > (int) sizeof (unsigned long))
    {
        ID = read_long ();
        if (ID == c_mutegroups)
        {
            long length = read_long ();
            if (c_gmute_tracks != length)
            {
                printf( "corrupt data in mutegroup section\n" );
            }
            for (int i = 0; i < c_seqs_in_set; i++)
            {
                a_perf->select_group_mute(read_long ());
                for (int k = 0; k < c_seqs_in_set; ++k) {
                    a_perf->set_group_mute_state(k, read_long ());
                }
            }
        }
    }

    // *** ADD NEW TAGS AT END **************/

    delete[]m_d;
    return true;
    //printf ( "done\n");
}


void
midifile::write_long (unsigned long a_x)
{
    m_l.push_front ((a_x & 0xFF000000) >> 24);
    m_l.push_front ((a_x & 0x00FF0000) >> 16);
    m_l.push_front ((a_x & 0x0000FF00) >> 8);
    m_l.push_front ((a_x & 0x000000FF));
}


void
midifile::write_short (unsigned short a_x)
{
    m_l.push_front ((a_x & 0xFF00) >> 8);
    m_l.push_front ((a_x & 0x00FF));
}

bool midifile::write (perform * a_perf)
{
    /* open binary file */
    ofstream file (m_name.c_str (), ios::out | ios::binary | ios::trunc);

    if (!file.is_open ())
        return false;

    /* used in small loops */
    int i;

    /* sequence pointer */
    sequence * seq;
    event e;
    list<char> l;

    int numtracks = 0;

    /* get number of tracks */
    for (i = 0; i < c_max_sequence; i++)
    {
        if (a_perf->is_active (i))
            numtracks++;
    }

    //printf ("numtracks[%d]\n", numtracks );

    /* write header */
    /* 'MThd' and length of 6 */
    write_long (0x4D546864);
    write_long (0x00000006);

    /* format 1, number of tracks, ppqn */
    write_short (0x0001);
    write_short (numtracks);
    write_short (c_ppqn);

    /* We should be good to load now   */
    /* for each Track in the midi file */
    for (int curTrack = 0; curTrack < c_max_sequence; curTrack++)
    {

        if (a_perf->is_active (curTrack))
        {

            //printf ("track[%d]\n", curTrack );

            seq = a_perf->get_sequence (curTrack);
            seq->fill_list (&l, curTrack);

            /* magic number 'MTrk' */
            write_long (0x4D54726B);
            write_long (l.size ());

            //printf("MTrk len[%d]\n", l.size());

            while (l.size () > 0)
            {
                m_l.push_front (l.back ());
                l.pop_back ();
            }
        }
    }


    /* midi control */
    write_long (c_midictrl);
    write_long (0);


    /* bus mute/unmute data */
    write_long (c_midiclocks);
    write_long (0);


    /* notepad data */
    write_long (c_notes);
    write_short (c_max_sets);

    for (i = 0; i < c_max_sets; i++)
    {
        string * note = a_perf->get_screen_set_notepad (i);
        write_short (note->length ());

        for (unsigned int j = 0; j < note->length (); j++)
            m_l.push_front ((*note)[j]);
    }


    /* bpm */
    write_long (c_bpmtag);
    write_long (a_perf->get_bpm ());

    /* write out the mute groups */
    write_long (c_mutegroups);
    write_long (c_gmute_tracks);
    for (int j=0; j < c_seqs_in_set; ++j){
        a_perf->select_group_mute(j);
        write_long(j);
        for (int i=0; i < c_seqs_in_set; ++i) {
            write_long( a_perf->get_group_mute_state(i) );
        }
    }
    
    int data_size = m_l.size ();
    m_d = (unsigned char *) new char[data_size];

    m_pos = 0;

    for (list < unsigned char >::reverse_iterator ri = m_l.rbegin ();
            ri != m_l.rend (); ri++)
    {
        m_d[m_pos++] = *ri;
    }

    m_l.clear ();

    // super slow
    //while ( m_l.size() > 0 ){
    //m_d[m_pos++] =  m_l.back();
    //  m_l.pop_back();
    //}

    file.write ((char *) m_d, data_size);
    file.close ();

    delete[]m_d;

    return true;
}
