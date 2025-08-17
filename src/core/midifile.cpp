// This file is part of seq192
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.


#include "midifile.h"
#include <iostream>
#include <cmath>

midifile::midifile(const string& a_name)
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

    unsigned short sig_nomiator;
    unsigned short sig_denomitor;
    bool has_timesig = false;

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

    if (Format > 2) {
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
        bool has_channel = false;

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

                        break;

                        /* one data item */
                    case EVENT_PROGRAM_CHANGE:
                    case EVENT_CHANNEL_PRESSURE:

                        data[0] = m_d[m_pos++];
                        //printf( "%02X\n", data[0] );

                        /* set data and add */
                        e.set_data (data[0]);
                        seq->add_event (&e);

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
                                        has_channel = true;
                                        len--;
                                    }

                                    else if (proprietary == c_timesig)
                                    {
                                        seq->undoable_lock(false);
                                        seq->set_bpm (m_d[m_pos++], false);
                                        seq->set_bw (m_d[m_pos++], false);
                                        seq->undoable_unlock();
                                        len -= 2;
                                    }

                                    else if (proprietary == c_resume)
                                    {
                                        seq->set_resume (m_d[m_pos++]);
                                        len--;
                                    }

                                    else if (proprietary == c_alt_cc)
                                    {
                                        seq->set_alt_cc (m_d[m_pos++] - 1);
                                        len--;
                                    }

                                    else if (proprietary == c_chase)
                                    {
                                        seq->set_chase (m_d[m_pos++]);
                                        len--;
                                    }
                                    else if (proprietary == c_snap_tick)
                                    {
                                        seq->set_snap_tick (m_d[m_pos++]);
                                        len--;
                                    }
                                    else if (proprietary == c_note_tick)
                                    {
                                        seq->set_note_tick (m_d[m_pos++]);
                                        len--;
                                    }
                                    break;

                                    /* Trk Done */
                                case 0x2f:
                                {
                                    // If delta is 0, then another event happened at the same time
                                    // as the track end.  the sequence class will discard the last
                                    // note.  This is a fix for that.   Native Seq24 file will always
                                    // have a Delta >= 1
                                    if ( Delta == 0 ){
                                        CurrentTime += 1;
                                    }

                                    seq->undoable_lock(false);
                                    long units = ((seq->get_bpm() * (c_ppqn * 4)) /  seq->get_bw() );
                                    long measures = CurrentTime / units;
                                    if (CurrentTime % units != 0) measures++;
                                    seq->set_measures(measures);
                                    seq->undoable_unlock();
                                    seq->zero_markers ();
                                    done = true;
                                    break;
                                }
                                    /* Track name */
                                case 0x03:
                                    for (i = 0; i < len; i++)
                                    {
                                        TrackName[i] = m_d[m_pos++];
                                    }

                                    TrackName[i] = '\0';

                                    //printf("[%s]\n", TrackName );
                                    len = 0;
                                    seq->set_name (TrackName);
                                    break;

                                    /* sequence number */
                                case 0x00:
                                    if (len == 0x00)
                                        perf = 0;
                                    else {
                                        perf = read_short ();
                                        len -= 2;
                                    }
                                    //printf ( "perf %d\n", perf );
                                    break;

                                    /* sequence time signature */
                                case 0x58:
                                    sig_nomiator = m_d[m_pos++];
                                    sig_denomitor = pow(2, m_d[m_pos++]);
                                    has_timesig = true;
                                    len -= 2;
                                    break;


                                default:
                                    // int c;
                                    // for (i = 0; i < len; i++)
                                    // {
                                    //     c = m_d[m_pos++];
                                        //printf( "%02X ", c  );
                                    // }
                                    //printf("\n");
                                    break;
                            }

                            /* eat the rest */
                            m_pos += len;

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

            if (!has_channel) {
                // if proprietary channel is not found, we're importing some generic midi
                // in which case note channel should not be kept to prevent conflict with note modes (slide, etc)
                seq->prune_event_channels();
            }

            /* the sequence has been filled, add it  */
            //printf ( "add_sequence( %d )\n", perf + (a_screen_set * c_seqs_in_set));
            a_perf->undoable_lock(false);


            if (has_timesig) {
                // if a standard midi time signature was found, use it
                seq->set_bpm (sig_nomiator, false);
                seq->set_bw (sig_denomitor, false);

                // in fomat 2 all tracks should have their own signature
                if (Format == 2) has_timesig = false;
            }

            a_perf->add_sequence (seq, perf + (a_screen_set * c_seqs_in_set));
            a_perf->undoable_unlock();
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
                a_perf->undoable_lock(false);
                a_perf->set_screen_set_notepad (x + a_screen_set, &notess);
                a_perf->undoable_unlock();
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
            double bpm = (double) read_long ();
            if(bpm > (c_bpm_scale_factor - 1.0))
                bpm /= c_bpm_scale_factor;

            if ( bpm < c_bpm_minimum ) bpm = c_bpm_minimum;
            if ( bpm > c_bpm_maximum ) bpm = c_bpm_maximum;
            a_perf->m_master_bus.set_bpm(bpm);
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

bool midifile::write (perform * a_perf, int a_screen_set, int a_sequence)
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

    /* get number of tracks */
    int numtracks = 0;
    int minTrack = 0;
    int maxTrack = 0;
    if (a_sequence != -1) {
        // sequence export
        numtracks = 1;
        minTrack = a_sequence;
        maxTrack = a_sequence + 1;
    } else {
        if (a_screen_set != -1) {
            // screenset export
            minTrack = a_screen_set * c_seqs_in_set;
            maxTrack = minTrack + c_seqs_in_set;
        } else {
            // session export
            minTrack = 0;
            maxTrack = c_max_sequence;
        }
        for (i = minTrack; i < maxTrack; i++)
        {
            if (a_perf->is_active (i))
                numtracks++;
        }
    }

    //printf ("numtracks[%d]\n", numtracks );

    /* write header */
    /* 'MThd' and length of 6 */
    write_long (0x4D546864);
    write_long (0x00000006);

    /* format 1, number of tracks, ppqn */
    if (a_sequence == -1) {
        // multiple sequences = format 1
        write_short (0x0001);
    } else {
        // sequence export = format 0
        write_short (0x0000);
    }
    write_short (numtracks);
    write_short (c_ppqn);

    /* We should be good to load now   */
    /* for each Track in the midi file */
    for (int curTrack = minTrack; curTrack < maxTrack; curTrack++)
    {

        if (a_perf->is_active (curTrack))
        {

            //printf ("track[%d]\n", curTrack );

            seq = a_perf->get_sequence (curTrack);
            seq->fill_list (&l, curTrack - minTrack);

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

    /* notepad data */
    if (a_screen_set != -1) {
        write_long (c_notes);
        write_short (1);
        string * note = a_perf->get_screen_set_notepad (a_screen_set);
        write_short (note->length ());

        for (unsigned int j = 0; j < note->length (); j++)
            m_l.push_front ((*note)[j]);
    } else {
        write_long (c_notes);
        write_short (c_max_sets);

        for (i = 0; i < c_max_sets; i++)
        {
            string * note = a_perf->get_screen_set_notepad (i);
            write_short (note->length ());

            for (unsigned int j = 0; j < note->length (); j++)
                m_l.push_front ((*note)[j]);
        }
    }


    /* bpm */
    write_long (c_bpmtag);
    /* From sequencer64
     * We now encode the Sequencer64-specific BPM value by multiplying it
     *  by 1000.0 first, to get more implicit precision in the number.
     */
    long scaled_bpm = long(a_perf->get_bpm() * c_bpm_scale_factor);
    write_long (scaled_bpm);

    int data_size = m_l.size ();
    m_d = (unsigned char *) new char[data_size];

    m_pos = 0;

    for (list < unsigned char >::reverse_iterator ri = m_l.rbegin ();
            ri != m_l.rend (); ri++)
    {
        m_d[m_pos++] = *ri;
    }

    m_l.clear ();

    file.write ((char *) m_d, data_size);
    file.close ();

    delete[]m_d;

    return true;
}
