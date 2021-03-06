#ifndef SEQ64_MASTERMIDIBUS_HPP
#define SEQ64_MASTERMIDIBUS_HPP

/*
 *  This file is part of seq24/sequencer64.
 *
 *  seq24 is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  seq24 is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with seq24; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/**
 * \file          mastermidibus.hpp
 *
 *  This module declares/defines the Master MIDI Bus.
 *
 * \library       sequencer64 application
 * \author        Seq24 team; modifications by Chris Ahlstrom
 * \date          2015-07-30
 * \updates       2016-08-20
 * \license       GNU GPLv2 or above
 *
 *  The mastermidibus module is the Linux version of the mastermidibus module.
 *  There's almost enough commonality to be worth creating a base class
 *  for both classes, and it might be nice to put the mastermidibus
 *  classes into their own modules.  This module is the latter.
 */

#include <vector>                       /* for channel-filtered recording   */

#include "midibus_common.hpp"
#include "mutex.hpp"
#include "user_midi_bus.hpp"

#if SEQ64_HAVE_LIBASOUND                /* covers this whole module         */

#include <alsa/asoundlib.h>
#include <alsa/seq_midi_event.h>

namespace seq64
{
    class event;
    class midibus;
    class sequence;

/**
 *  The class that "supervises" all of the midibus objects?
 */

class mastermidibus
{

private:

    /**
     *  The ALSA sequencer client handle.
     */

    snd_seq_t * m_alsa_seq;

    /**
     *  The maximum number of busses supported.  Set to c_max_busses for now.
     */

    int m_max_busses;

    /**
     *  The number of output busses.
     */

    int m_num_out_buses;

    /**
     *  The number of input busses.
     */

    int m_num_in_buses;

    /**
     *  Output MIDI busses.
     */

    midibus * m_buses_out[c_max_busses];

    /**
     *  Input MIDI busses.
     */

    midibus * m_buses_in[c_max_busses];

    /**
     *  MIDI buss announcer?
     */

    midibus * m_bus_announce;

    /**
     *  Active output MIDI busses.
     */

    bool m_buses_out_active[c_max_busses];

    /**
     *  Active input MIDI busses.
     */

    bool m_buses_in_active[c_max_busses];

    /**
     *  Output MIDI buss initialization.
     */

    bool m_buses_out_init[c_max_busses];

    /**
     *  Input MIDI buss initialization.
     */

    bool m_buses_in_init[c_max_busses];

    /**
     *  Clock initialization.
     */

    clock_e m_init_clock[c_max_busses];

    /**
     *  Input initialization?
     */

    bool m_init_input[c_max_busses];

    /**
     *  The ID of the MIDI queue.
     */

    int m_queue;

    /**
     *  Resolution in parts per quarter note.
     */

    int m_ppqn;

    /**
     *  BPM (beats per minute).  We had to lengthen this name; way too easy to
     *  confuse it with "bpm" for "beats per measure".
     */

    int m_beats_per_minute;

    /**
     *  The number of descriptors for polling.
     */

    int m_num_poll_descriptors;

    /**
     *  Points to the list of descriptors for polling.
     */

    struct pollfd * m_poll_descriptors;

    /**
     *  For dumping MIDI input to a sequence for recording.
     */

    bool m_dumping_input;

    /**
     *  Used for the new "stazed" feature of filtering MIDI channels so that
     *  a sequence gets only the channels meant for it.  We want to make this
     *  a run-time, non-legacy option.
     */

    std::vector<sequence *> m_vector_sequence;

    /**
     *  If true, the m_vector_sequence container is used to divert incoming
     *  data to the sequence that has the channel it is meant for.
     */

    bool m_filter_by_channel;

    /**
     *  Points to the sequence object.
     */

    sequence * m_seq;

    /**
     *  The locking mutex.  This object is passed to an automutex object that
     *  lends exception-safety to the mutex locking.
     */

    mutex m_mutex;

public:

    mastermidibus
    (
        int ppqn = SEQ64_USE_DEFAULT_PPQN,
        int bpm = c_beats_per_minute
    );
    ~mastermidibus ();

    void init (int ppqn);

    /**
     * \getter m_alsa_seq
     */

    snd_seq_t * get_alsa_seq () const
    {
        return m_alsa_seq;
    }

    /**
     * \getter m_num_out_buses
     */

    int get_num_out_buses () const
    {
        return m_num_out_buses;
    }

    /**
     * \getter m_num_in_buses
     */

    int get_num_in_buses () const
    {
        return m_num_in_buses;
    }

    void set_beats_per_minute (int bpm);
    void set_ppqn (int ppqn);

    /**
     * \getter m_filter_by_channel
     */

    bool filter_by_channel () const
    {
        return m_filter_by_channel;
    }

    /**
     * \setter m_filter_by_channel
     */

    void filter_by_channel (bool flag)
    {
        m_filter_by_channel = flag;
    }

    /**
     * \getter m_beats_per_minute
     */

    int get_beats_per_minute () const
    {
        return m_beats_per_minute;
    }

    /**
     * \getter m_ppqn
     */

    int get_ppqn () const
    {
        return m_ppqn;
    }

    std::string get_midi_out_bus_name (int bus);
    std::string get_midi_in_bus_name (int bus);
    void print ();
    void flush ();
    void start ();
    void stop ();
    void clock (midipulse tick);
    void continue_from (midipulse tick);
    void init_clock (midipulse tick);
    int poll_for_midi ();
    bool is_more_input ();
    bool get_midi_event (event * in);
    void set_sequence_input (bool state, sequence * seq);
    void dump_midi_input (event in);                    /* seq32 function */

    /**
     * \getter m_dumping_input
     */

    bool is_dumping () const
    {
        return m_dumping_input;
    }

    /**
     * \getter m_seq
     */

    sequence * get_sequence () const
    {
        return m_seq;
    }

    void sysex (event * event);
    void port_start (int client, int port);
    void port_exit (int client, int port);
    void play (bussbyte bus, event * e24, midibyte channel);
    void set_clock (bussbyte bus, clock_e clock_type);
    clock_e get_clock (bussbyte bus);
    void set_input (bussbyte bus, bool inputing);
    bool get_input (bussbyte bus);

};

#endif      // SEQ64_HAVE_LIBASOUND

}           // namespace seq64

#endif      // SEQ64_MASTERMIDIBUS_HPP

/*
 * mastermidibus.hpp
 *
 * vim: sw=4 ts=4 wm=4 et ft=cpp
 */

