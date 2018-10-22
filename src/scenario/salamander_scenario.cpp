/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#include <string>

#include "salamander_scenario.hpp"

namespace Invader {
    SalamanderScenario scenario_from_file(const std::vector<char> &file) {
        SalamanderScenario scenario;

        std::size_t offset = 0;

        auto seek_to_next = [&offset, file] () {
            const char *file_chars = file.data();
            while(offset < file.size() && (file_chars[offset] == ' ' || file_chars[offset] == '\n' || file_chars[offset] == '\r' || file_chars[offset] == '\t')) {
                offset++;
            }
        };

        auto read_int = [&offset, file, &seek_to_next] () -> int {
            seek_to_next();
            const char *file_chars = file.data();
            if(offset == file.size()) {
                throw std::exception();
            }

            std::size_t end;
            int i = std::stoi(file_chars + offset, &end, 10);
            offset += end;

            return i;
        };

        auto read_float = [&offset, file, &seek_to_next] () -> double {
            seek_to_next();
            const char *file_chars = file.data();
            if(offset == file.size()) {
                throw std::exception();
            }

            std::size_t end;
            double i = std::stod(file_chars + offset, &end);
            offset += end;

            return i;
        };

        auto read_string = [&offset, file, &seek_to_next] () -> std::string {
            seek_to_next();
            const char *file_chars = file.data();
            std::size_t file_size = file.size();

            if(offset + 1 >= file_size) {
                throw std::exception();
            }
            else if(file[offset] != '\"') {
                throw std::exception();
            }

            std::vector<char> string_to_add;

            offset++;

            while(file_chars[offset] != '\"') {
                if(file_chars[offset] == '\\') {
                    offset++;
                    if(offset >= file_size) {
                        throw std::exception();
                    }
                }
                string_to_add.push_back(file_chars[offset]);

                if(++offset >= file_size) {
                    throw std::exception();
                }
            }
            offset++;
            string_to_add.push_back(0);

            return string_to_add.data();
        };

        int version = read_int();
        if(version != 1) {
            throw std::exception();
        }

        return scenario;
    }
}
