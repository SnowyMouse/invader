// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__COMMAND_LINE_OPTION_HPP
#define INVADER__COMMAND_LINE_OPTION_HPP

#include <cstring>
#include <string>
#include <vector>

namespace Invader {
    /**
     * Header only command line option handler
     */
    class CommandLineOption {
    private:
        /** Full name of the option */
        std::string full_name;

        /** Char name of the option */
        char char_name;

        /** Number of arguments expected */
        int argument_count;
    public:
        /**
         * Initialize a command line option
         * @param full_name      full name of the command line option
         * @param char_name      char name of the command line option; this will be passed into the argument handler if this is correctly used
         * @param argument_count number of arguments expected for the command line option
         */
        CommandLineOption(const char *full_name, const char char_name, int argument_count) : full_name(full_name), char_name(char_name), argument_count(argument_count) {}

        /**
         * Get the full name of the command line option
         * @return full name
         */
        const std::string &get_full_name() const noexcept {
            return this->full_name;
        }

        /**
         * Get the single char name of the command line option
         * @return char name
         */
        char get_char_name() const noexcept {
            return this->char_name;
        }

        /**
         * Get the argument count of the command line option
         * @return argument count
         */
        int get_argument_count() const noexcept {
            return this->argument_count;
        }

        /**
         * Parse the arguments using the given options with the first argument as the path to the program. When enough arguments are found for the given option, the char_name and all options found will be passed to handler. If an error occurs, error_char_name with 0 arguments will be passed.
         * @param argc            number of arguments
         * @param argv            pointer to the first argument
         * @param options         options to use
         * @param error_char_name char_name to use for errors
         * @param user_data       user data passed into the arguments
         * @param handler         handler function to use
         * @return                a vector of all unhandled arguments
         */
        template<typename DataType>
        static std::vector<const char *> parse_arguments(int argc, const char * const *argv, const std::vector<CommandLineOption> &options, char error_char_name, DataType user_data, void (*handler)(char char_name, const std::vector<const char *> &arguments, DataType user_data)) {
            // Hold all unhandled arguments
            std::vector<const char *> unhandled_arguments;

            // Hold the options to be handled
            std::vector<const CommandLineOption *> currently_handled_options;
            std::vector<const char *> currently_handled_option_arguments_handled;

            static const char INVALID_OPTION_ERROR[] = "-%s is not a valid option.\n";

            for(int i = 1; i < argc; i++) {
                // If we don't have any options being worked on, check if this is an option
                if(currently_handled_options.size() == 0) {
                    // If it starts with a '-', then we've got something to do
                    const char *argument = argv[i];
                    std::size_t argument_length = std::strlen(argument);
                    if(argument_length >= 1 && argument[0] == '-') {
                        // It was just a hyphen
                        if(argument_length == 1) {
                            std::fprintf(stderr, INVALID_OPTION_ERROR, "");
                            handler(error_char_name, std::vector<const char *>(), user_data);
                        }

                        // It's a long option
                        else if(argument[1] == '-') {
                            const char *long_option = argument + 2;
                            bool got_it = false;

                            // Find the option. If we have it, add it.
                            for(const auto &option : options) {
                                if(option.get_full_name() == long_option) {
                                    currently_handled_options.push_back(&option);
                                    got_it = true;
                                    break;
                                }
                            }

                            // If we didn't get it, then darn
                            if(!got_it) {
                                std::fprintf(stderr, INVALID_OPTION_ERROR, argument + 1);
                                handler(error_char_name, std::vector<const char *>(), user_data);
                            }
                        }

                        // It's a short option; possibly multiple of them
                        else {
                            for(const char *o = argument + 1; *o; o++) {
                                // Find the option. If we have it, add it.
                                bool got_it = false;
                                for(const auto &option : options) {
                                    if(option.get_char_name() == *o) {
                                        currently_handled_options.push_back(&option);
                                        got_it = true;
                                        break;
                                    }
                                }

                                // If we didn't get it, then darn
                                if(!got_it) {
                                    char option_text[2] = { *o, 0 };
                                    std::fprintf(stderr, INVALID_OPTION_ERROR, option_text);
                                    handler(error_char_name, std::vector<const char *>(), user_data);
                                }
                            }
                        }
                    }

                    // Otherwise, it wasn't a handled argument.
                    else {
                        unhandled_arguments.push_back(argv[i]);
                    }
                }

                // If we do have options being handled, add them to the handled list
                else {
                    currently_handled_option_arguments_handled.push_back(argv[i]);
                }

                // Check to see if we have any options that can be handled now
                while(currently_handled_options.size() > 0) {
                    auto &topmost_option = currently_handled_options[0];

                    // If we don't have enough arguments for the option, break and try again later
                    if(currently_handled_option_arguments_handled.size() < static_cast<std::size_t>(topmost_option->get_argument_count())) {
                        break;
                    }

                    // If we do, handle it
                    handler(topmost_option->get_char_name(), currently_handled_option_arguments_handled, user_data);

                    // Clear the array up
                    currently_handled_option_arguments_handled.clear();

                    // Remove the first item
                    currently_handled_options.erase(currently_handled_options.begin());
                }
            }

            // If we have unfinished business, do it
            std::size_t remaining_options = currently_handled_options.size();
            if(remaining_options != 0) {
                auto &arg = currently_handled_options[0];
                int expected_argument_count = arg->get_argument_count();
                std::fprintf(stderr, "-%c takes %i argument%s, but only %zu %s given.\n", arg->get_char_name(), expected_argument_count, expected_argument_count == 1 ? "" : "s", remaining_options, remaining_options == 1 ? "was" : "were");
                handler(error_char_name, std::vector<const char *>(), user_data);
            }

            return unhandled_arguments;
        }
    };
}

#endif
