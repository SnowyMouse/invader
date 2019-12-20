// SPDX-License-Identifier: GPL-3.0-only

#include <filesystem>
#include <invader/command_line_option.hpp>
#include <invader/printf.hpp>
#include <invader/file/file.hpp>
#include <invader/tag/parser/parser.hpp>
#include <invader/sound/sound_reader.hpp>
#include <invader/version.hpp>

int main(int argc, const char **argv) {
    using namespace Invader;
    using namespace Invader::HEK;
    static constexpr std::size_t SPLIT_SAMPLE_COUNT = 116480;
    static constexpr std::size_t MAX_PERMUTATIONS = UINT16_MAX - 1;

    struct SoundOptions {
        const char *data = "data";
        const char *tags = "tags";
        std::optional<bool> split;
        std::optional<SoundFormat> format;
        bool fs_path = false;
    } sound_options;

    std::vector<CommandLineOption> options;
    options.emplace_back("info", 'i', 0, "Show credits, source info, and other info.");
    options.emplace_back("tags", 't', 1, "Use the specified tags directory. Use multiple times to add more directories, ordered by precedence.", "<dir>");
    options.emplace_back("data", 'd', 1, "Use the specified data directory.", "<dir>");
    options.emplace_back("split", 's', 0, "Split permutations into 64 KiB chunks.");
    options.emplace_back("format", 'F', 1, "Set the format. Can be: 16-bit-pcm. Default (new tag): 16-bit-pcm");
    options.emplace_back("fs-path", 'P', 0, "Use a filesystem path for the data.");

    static constexpr char DESCRIPTION[] = "Create or modify a sound tag.";
    static constexpr char USAGE[] = "[options] <sound-tag>";

    auto remaining_arguments = CommandLineOption::parse_arguments<SoundOptions &>(argc, argv, options, USAGE, DESCRIPTION, 1, 1, sound_options, [](char opt, const std::vector<const char *> &arguments, auto &sound_options) {
        switch(opt) {
            case 'd':
                sound_options.data = arguments[0];
                break;

            case 't':
                sound_options.tags = arguments[0];
                break;

            case 'i':
                show_version_info();
                std::exit(EXIT_SUCCESS);

            case 's':
                sound_options.split = true;
                break;

            case 'F':
                if(std::strcmp(arguments[0], "ogg-vorbis") == 0) {
                    sound_options.format = SoundFormat::SOUND_FORMAT_OGG;
                }
                else if(std::strcmp(arguments[0], "16-bit-pcm") == 0) {
                    sound_options.format = SoundFormat::SOUND_FORMAT_16_BIT_PCM;
                }
                else if(std::strcmp(arguments[0], "xbox-adpcm") == 0) {
                    sound_options.format = SoundFormat::SOUND_FORMAT_XBOX_ADPCM;
                }
                else {
                    eprintf_error("Unknown sound format %s", arguments[0]);
                    std::exit(EXIT_FAILURE);
                }
                break;

            case 'P':
                sound_options.fs_path = true;
                break;
        }
    });

    // Get our paths and make sure a data directory exists
    std::string halo_tag_path;
    if(sound_options.fs_path) {
        std::vector<std::string> data;
        data.emplace_back(std::string(sound_options.data));
        try {
            halo_tag_path = Invader::File::file_path_to_tag_path(remaining_arguments[0], data, false).value();
        }
        catch(std::exception &) {
            eprintf_error("Cannot find %s in %s", remaining_arguments[0], sound_options.data);
            return EXIT_FAILURE;
        }
    }
    else {
        halo_tag_path = remaining_arguments[0];
    }

    auto tag_path = std::filesystem::path(sound_options.tags) / (halo_tag_path + ".sound");
    auto data_path = std::filesystem::path(sound_options.data) / halo_tag_path;
    if(!std::filesystem::is_directory(data_path)) {
        eprintf_error("No directory exists at %s", data_path.string().data());
        return EXIT_FAILURE;
    }

    // Parse the sound tag
    Parser::Sound sound_tag = {};
    if(std::filesystem::exists(tag_path)) {
        if(std::filesystem::is_directory(tag_path)) {
            eprintf_error("A directory exists at %s where a file was expected", tag_path.string().data());
            return EXIT_FAILURE;
        }
        auto sound_file = File::open_file(tag_path.string().data());
        if(sound_file.has_value()) {
            auto &sound_data = *sound_file;
            try {
                sound_tag = Parser::Sound::parse_hek_tag_file(sound_data.data(), sound_data.size());
            }
            catch(std::exception &e) {
                eprintf_error("An error occurred while attempting to read %s", tag_path.string().data());
                return EXIT_FAILURE;
            }
        }
    }
    else {
        sound_tag.format = SoundFormat::SOUND_FORMAT_16_BIT_PCM;
        sound_tag.flags.split_long_sound_into_permutations = 0;
    }

    // If a format is defined, use it
    if(sound_options.format.has_value()) {
        sound_tag.format = *sound_options.format;
    }
    // Same with whether to split
    if(sound_options.split.has_value()) {
        sound_tag.flags.split_long_sound_into_permutations = *sound_options.split;
    }

    auto &format = sound_tag.format;
    bool split = sound_tag.flags.split_long_sound_into_permutations;

    // If we don't have pitch ranges, add one
    if(sound_tag.pitch_ranges.size() == 0) {
        sound_tag.pitch_ranges.resize(1);
    }

    // Initialize a pitch range
    auto &old_pitch_range = sound_tag.pitch_ranges[0];
    Parser::SoundPitchRange pitch_range = old_pitch_range;
    pitch_range.permutations.clear();
    pitch_range.actual_permutation_count = 0;

    auto wav_iterator = std::filesystem::directory_iterator(data_path);

    std::uint16_t highest_channel_count = 0;
    std::uint32_t highest_sample_rate = 0;
    std::uint16_t highest_bits_per_sample = 0;

    std::vector<SoundReader::Sound> permutations;

    for(auto &wav : wav_iterator) {
        // Skip directories
        auto path = wav.path();
        auto path_str = path.string();
        if(wav.is_directory()) {
            eprintf_error("Unexpected directory %s", path_str.data());
            return EXIT_FAILURE;
        }
        auto extension = path.extension().string();

        // Open the file
        auto file = Invader::File::open_file(path_str.data());
        if(!file.has_value()) {
            eprintf_error("Unable to open %s", path_str.data());
            return EXIT_FAILURE;
        }
        auto &file_data = *file;

        // Get the sound
        auto &sound = permutations.emplace_back();
        if(extension == ".wav") {
            sound = SoundReader::sound_from_wav(file_data.data(), file_data.size());
        }
        else if(extension == ".flac") {
            eprintf_error("Unexpected FLAC file %s", path_str.data());
            eprintf_warn("FLAC files are not implemented");
            return EXIT_FAILURE;
        }
        else {
            eprintf_error("Unknown file format for %s", path_str.data());
            return EXIT_FAILURE;
        }

        // Use the highest channel count and sample rate
        if(highest_channel_count < sound.channel_count) {
            highest_channel_count = sound.channel_count;
        }
        if(highest_sample_rate < sound.sample_rate) {
            highest_sample_rate = sound.sample_rate;
        }
        if(highest_bits_per_sample < sound.bits_per_sample) {
            highest_bits_per_sample = sound.bits_per_sample;
        }

        // Make sure we can actually work with this
        if(sound.channel_count > 2 || sound.channel_count < 1) {
            eprintf_error("Unsupported channel count %u in %s", static_cast<unsigned int>(sound.channel_count), path_str.data());
            return EXIT_FAILURE;
        }
        if(sound.bits_per_sample % 8 != 0 || sound.bits_per_sample < 16 || sound.bits_per_sample > 64) {
            eprintf_error("Bits per sample (%u) is not divisible by 8 in %s (or is too small or too big)", static_cast<unsigned int>(sound.bits_per_sample), path_str.data());
            return EXIT_FAILURE;
        }
        auto filename = path.filename().string();
        sound.name = filename.substr(0, filename.size() - extension.size());

        // Check for duplicates
        std::size_t times_appeared = 0;
        for(auto &p : permutations) {
            if(p.name == sound.name) {
                times_appeared++;
            }
        }
        if(times_appeared != 1) {
            eprintf_error("Multiple permutations with the same name (%s) cannot be added", sound.name.data());
            return EXIT_FAILURE;
        }
    }

    // Make sure we have stuff
    if(permutations.size() == 0) {
        eprintf_error("No permutations found in %s", data_path.string().data());
        return EXIT_FAILURE;
    }

    // Set the actual permutation count
    auto actual_permutation_count = permutations.size();
    if(actual_permutation_count > MAX_PERMUTATIONS) {
        eprintf_error("Maximum number of actual permutations (%zu > %zu) exceeded", actual_permutation_count, MAX_PERMUTATIONS);
        return EXIT_FAILURE;
    }
    pitch_range.actual_permutation_count = static_cast<std::uint16_t>(actual_permutation_count);

    // If doing 16-bit PCM, set to 16-bit PCM regardless
    if(format == SoundFormat::SOUND_FORMAT_16_BIT_PCM && highest_bits_per_sample != 16) {
        highest_bits_per_sample = 16;
    }

    // Sound tags currently only support 22.05 kHz and 44.1 kHz
    if(highest_sample_rate < 22050) {
        highest_sample_rate = 22050;
    }
    else if(highest_sample_rate > 44100) {
        highest_sample_rate = 44100;
    }
    if(highest_sample_rate == 22050) {
        sound_tag.sample_rate = SoundSampleRate::SOUND_SAMPLE_RATE_22050_HZ;
    }
    else if(highest_sample_rate == 44100) {
        sound_tag.sample_rate = SoundSampleRate::SOUND_SAMPLE_RATE_44100_HZ;
    }
    else {
        eprintf_error("Unsupported sample rate %u", highest_sample_rate);
        return EXIT_FAILURE;
    }

    // Sound tags currently only support single and dual channels
    if(highest_channel_count == 1) {
        sound_tag.channel_count = SoundChannelCount::SOUND_CHANNEL_COUNT_MONO;
    }
    else if(highest_channel_count == 2) {
        sound_tag.channel_count = SoundChannelCount::SOUND_CHANNEL_COUNT_STEREO;
    }
    else {
        eprintf_error("Unsupported channel count %u", highest_channel_count);
        return EXIT_FAILURE;
    }

    std::size_t highest_bytes_per_sample = highest_bits_per_sample / 8;

    // Resample permutations when needed
    for(auto &permutation : permutations) {
        std::size_t bytes_per_sample = permutation.bits_per_sample / 8;
        std::size_t sample_count = permutation.pcm.size() / bytes_per_sample;

        // Bits per sample doesn't match; we can fix that though
        if(bytes_per_sample != highest_bytes_per_sample) {
            // Allocate stuff
            double divide_by = std::pow(256, bytes_per_sample);
            std::size_t new_bytes_per_sample = highest_bytes_per_sample;
            double multiply_by = std::pow(256, highest_bytes_per_sample);

            std::vector<std::byte> new_samples(sample_count * new_bytes_per_sample);
            const std::byte *old_sample = permutation.pcm.data();
            const std::byte *old_sample_end = permutation.pcm.data() + permutation.pcm.size();
            std::byte *new_sample = new_samples.data();

            // Go through each sample
            while(old_sample < old_sample_end) {
                std::int64_t sample = 0;

                // Get the sample value
                for(std::size_t b = 0; b < bytes_per_sample; b++) {
                    std::int64_t significance = 8 * b;
                    if(b + 1 == bytes_per_sample) {
                        sample |= static_cast<std::int64_t>(static_cast<std::int8_t>(old_sample[b])) << significance;
                    }
                    else {
                        sample |= static_cast<std::int64_t>(static_cast<std::uint8_t>(old_sample[b])) << significance;
                    }
                }

                // Write the new sample value
                std::int64_t new_sample_data = sample / divide_by * multiply_by;
                for(std::size_t b = 0; b < new_bytes_per_sample; b++) {
                    std::int64_t significance = 8 * b;
                    new_sample[b] = static_cast<std::byte>(new_sample_data >> significance);
                }

                old_sample += bytes_per_sample;
                new_sample += new_bytes_per_sample;
            }

            bytes_per_sample = new_bytes_per_sample;

            permutation.pcm = std::move(new_samples);
            permutation.bits_per_sample = highest_bits_per_sample;
        }

        // Channel count doesn't match; we can fix that though
        if(permutation.channel_count == 1 && highest_channel_count == 2) {
            std::vector<std::byte> new_samples(sample_count * 2 * bytes_per_sample);
            const std::byte *old_sample = permutation.pcm.data();
            const std::byte *old_sample_end = permutation.pcm.data() + permutation.pcm.size();
            std::byte *new_sample = new_samples.data();

            while(old_sample < old_sample_end) {
                std::memcpy(new_sample, old_sample, bytes_per_sample);
                std::memcpy(new_sample + bytes_per_sample, old_sample, bytes_per_sample);
                old_sample += bytes_per_sample;
                new_sample += bytes_per_sample * 2;
            }

            permutation.pcm = std::move(new_samples);
            permutation.channel_count = 2;
        }

        // Sample rate doesn't match; this can be fixed with interpolation
        if(permutation.sample_rate != highest_sample_rate) {
            eprintf_error("Sample rate (%u) does not match the selected sample rate (%u)", permutation.sample_rate, highest_sample_rate);
            eprintf_warn("TODO: Interpolation");
            return EXIT_FAILURE;
        }
    }

    // Make the sound tag
    for(auto &permutation : permutations) {
        // Add a new permutation
        auto new_permutation = [&pitch_range, &permutation, &format]() -> Parser::SoundPermutation & {
            auto &p = pitch_range.permutations.emplace_back();
            if(permutation.name.size() >= sizeof(TagString)) {
                eprintf_error("Permutation name %s has at least %zu characters", permutation.name.data(), permutation.name.size());;
            }
            std::memset(p.name.string, 0, sizeof(p.name.string));
            std::strncpy(p.name.string, permutation.name.data(), sizeof(p.name.string) - 1);
            p.format = format;
            return p;
        };

        // Encode a permutation
        auto encode_permutation = [&permutation, &format](Parser::SoundPermutation &p, const std::vector<std::byte> &pcm) {
            p.vorbis_sample_count = pcm.size();

            switch(format) {
                case SoundFormat::SOUND_FORMAT_16_BIT_PCM:
                    if(permutation.bits_per_sample == 16) {
                        const auto *data = reinterpret_cast<const LittleEndian<std::uint16_t> *>(pcm.data());
                        std::vector<std::byte> samples = std::vector<std::byte>(pcm.size());
                        auto *new_data = reinterpret_cast<BigEndian<std::uint16_t> *>(samples.data());
                        std::size_t sample_count = pcm.size() / sizeof(*data);
                        for(std::size_t s = 0; s < sample_count; s++) {
                            new_data[s] = data[s];
                        }
                        p.samples = samples;
                        break;
                    }
                    else {
                        eprintf_error("Needs to be 16-bit PCM, first");
                        std::exit(EXIT_FAILURE);
                    }
                default:
                    eprintf_error("Unimplemented sound format");
                    std::exit(EXIT_FAILURE);
            }
        };

        // Split based on the sample count
        if(split) {
            const std::size_t MAX_SPLIT_SIZE = SPLIT_SAMPLE_COUNT * highest_bytes_per_sample;
            std::size_t digested = 0;
            auto *samples_data = permutation.pcm.data();
            std::size_t total_size = permutation.pcm.size();
            while(digested < total_size) {
                auto &p = new_permutation();
                std::size_t remaining_size = total_size - digested;
                std::size_t permutation_size = remaining_size > MAX_SPLIT_SIZE ? MAX_SPLIT_SIZE : remaining_size;

                auto *sample_data_start = samples_data + digested;
                encode_permutation(p, std::vector<std::byte>(sample_data_start, sample_data_start + permutation_size));
                digested += permutation_size;

                if(digested == total_size) {
                    p.next_permutation_index = NULL_INDEX;
                }
                else {
                    std::size_t next_permutation = pitch_range.permutations.size();
                    if(next_permutation > MAX_PERMUTATIONS) {
                        eprintf_error("Maximum number of total permutations (%zu > %zu) exceeded", next_permutation, MAX_PERMUTATIONS);
                        return EXIT_FAILURE;
                    }
                    p.next_permutation_index = static_cast<Index>(next_permutation);
                }
            }
        }
        else {
            auto &p = new_permutation();
            p.next_permutation_index = NULL_INDEX;
            encode_permutation(p, permutation.pcm);
        }
    }

    // Wrap it up
    std::memset(pitch_range.name.string, 0, sizeof(pitch_range.name));
    static constexpr char DEFAULT_NAME[] = "default";
    std::memcpy(pitch_range.name.string, DEFAULT_NAME, sizeof(DEFAULT_NAME));
    sound_tag.pitch_ranges[0] = pitch_range;
    auto sound_tag_data = sound_tag.generate_hek_tag_data(TagClassInt::TAG_CLASS_SOUND);

    // Create missing directories if needed
    std::filesystem::create_directories(tag_path.parent_path());

    // Save
    if(!Invader::File::save_file(tag_path.string().data(), sound_tag_data)) {
        eprintf_error("Failed to save %s", tag_path.string().data());
        return EXIT_FAILURE;
    }
}
