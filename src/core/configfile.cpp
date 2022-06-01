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


#include <iostream>
#include <nlohmann/json.hpp>

#include "configfile.h"
#include "globals.h"

using json = nlohmann::json;

extern string last_used_dir;

ConfigFile::ConfigFile(const std::string& a_name) :
    m_name(a_name)
{
}

ConfigFile::~ConfigFile()
{
}

bool
ConfigFile::parse()
{

    /* open binary file */
    ifstream file (m_name.c_str());

    if (!file.is_open()) return false;

    json j;

    try {
        file >> j;
    } catch (json::exception& ex) {
        cerr << "Failed to load " + m_name + ":\n";
        cerr << ex.what();
        cerr << "\n";
        return false;
    }

    auto buses = j["buses"];
    if (buses.is_object())
    {
        for (json::iterator it = buses.begin(); it != buses.end(); ++it) {

            int bus_number = stoi(it.key());
            auto bus_data = it.value();

            auto bus_name = bus_data["name"];
            if (bus_name.is_string()) {
                global_user_midi_bus_definitions[bus_number].alias = bus_name;
            }

            auto channels = bus_data["channels"];
            if (channels.is_object()) {

                for (json::iterator it = channels.begin(); it != channels.end(); ++it) {

                    int channel_number = stoi(it.key());
                    auto channel_data = it.value();

                    int instrument_number = channel_number + bus_number * 16;
                    global_user_midi_bus_definitions[bus_number].instrument[channel_number] = instrument_number;

                    auto channel_name = channel_data["name"];
                    if (channel_name.is_string()) {
                        global_user_instrument_definitions[instrument_number].instrument = channel_name;
                    }

                    auto controls = channel_data["controls"];
                    if (controls.is_object()) {
                        for (json::iterator it = controls.begin(); it != controls.end(); ++it) {
                            int cc_number = stoi(it.key());
                            auto cc_data = it.value();
                            global_user_instrument_definitions[instrument_number].controllers[cc_number] = cc_data;
                            global_user_instrument_definitions[instrument_number].controllers_active[cc_number] = true;
                        }
                    }

                    auto notes = channel_data["notes"];
                    if (notes.is_object()) {
                        global_user_midi_bus_definitions[bus_number].keymap[channel_number] = instrument_number;
                        for (json::iterator it = notes.begin(); it != notes.end(); ++it) {
                            int note_number = stoi(it.key());
                            auto note_data = it.value();
                            global_user_keymap_definitions[instrument_number].keys[note_number] = note_data;
                            global_user_keymap_definitions[instrument_number].keys_active[note_number] = true;
                        }
                    }

                }

            }

        }
    }

    file.close();

    return true;
}
