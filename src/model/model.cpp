// SPDX-License-Identifier: GPL-3.0-only

#include <vector>
#include <cstring>
#include <regex>

#include <invader/version.hpp>
#include <invader/printf.hpp>
#include <invader/file/file.hpp>
#include <invader/command_line_option.hpp>
#include <invader/model/jms.hpp>

int main(int argc, const char **argv) {
    using namespace Invader;
    using namespace Invader::HEK;

    std::vector<Invader::CommandLineOption> options;
    options.emplace_back("info", 'i', 0, "Show credits, source info, and other info.");

    static constexpr char DESCRIPTION[] = "Create a model tag.";
    static constexpr char USAGE[] = "[options] <model-tag>";

    auto remaining_arguments = Invader::CommandLineOption::parse_arguments<std::nullptr_t>(argc, argv, options, USAGE, DESCRIPTION, 1, 1, nullptr, [](char opt, const auto &, std::nullptr_t) {
        switch(opt) {
            case 'i':
                Invader::show_version_info();
                std::exit(EXIT_SUCCESS);
        }
    });
    
    auto file = Invader::File::open_file(remaining_arguments[0]).value();
    auto to_string = std::string(reinterpret_cast<const char *>(file.data()), file.size());
    auto jms = JMS::from_string(to_string.c_str(), nullptr);
    std::printf("%s\n", jms.string().c_str());
}
