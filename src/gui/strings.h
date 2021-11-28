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


#ifndef SEQ192_STRINGS
#define SEQ192_STRINGS


std::map<unsigned char, std::string> status_strings = {
    {EVENT_NOTE_ON, "Note On"},
    {EVENT_NOTE_OFF, "Note Off"},
    {EVENT_AFTERTOUCH, "Aftertouch"},
    {EVENT_PROGRAM_CHANGE, "Aftertouch"},
    {EVENT_CHANNEL_PRESSURE, "Aftertouch"},
    {EVENT_PITCH_WHEEL, "Aftertouch"},
    {EVENT_CONTROL_CHANGE, "Control Change"}
};


#endif
