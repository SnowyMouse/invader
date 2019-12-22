// SPDX-License-Identifier: GPL-3.0-only

#include <filesystem>
#include <invader/command_line_option.hpp>
#include <invader/printf.hpp>
#include <invader/file/file.hpp>
#include <invader/tag/parser/parser.hpp>
#include <invader/sound/sound_reader.hpp>
#include <invader/version.hpp>
#include <vorbis/vorbisenc.h>

int main(int argc, const char **argv) {
    using namespace Invader;
    using namespace Invader::HEK;
    static constexpr std::size_t SPLIT_BUFFER_SIZE = 0x38E00;
    static constexpr std::size_t MAX_PERMUTATIONS = UINT16_MAX - 1;

    struct SoundOptions {
        const char *data = "data";
        const char *tags = "tags";
        std::optional<bool> split;
        std::optional<SoundFormat> format;
        bool fs_path = false;
        float vorbis_quality = 1.0F;
        std::optional<SoundClass> sound_class;
    } sound_options;

    std::vector<CommandLineOption> options;
    options.emplace_back("info", 'i', 0, "Show credits, source info, and other info.");
    options.emplace_back("tags", 't', 1, "Use the specified tags directory. Use multiple times to add more directories, ordered by precedence.", "<dir>");
    options.emplace_back("data", 'd', 1, "Use the specified data directory.", "<dir>");
    options.emplace_back("split", 's', 0, "Split permutations into 227.5 KiB chunks.");
    options.emplace_back("no-split", 'S', 0, "Do not split permutations.");
    options.emplace_back("format", 'F', 1, "Set the format. Can be: 16-bit-pcm, ogg-vorbis. Default (new tag): 16-bit-pcm");
    options.emplace_back("fs-path", 'P', 0, "Use a filesystem path for the data.");
    options.emplace_back("vorbis-quality", 'q', 1, "Set the Vorbis quality. This can be between -0.1 and 1.0. Default: 1.0");
    options.emplace_back("class", 'c', 1, "Set the class. This is required when generating new sounds. Can be: ambient-computers, ambient-machinery, ambient-nature, device-computers, device-door, device-force-field, device-machinery, device-nature, first-person-damage, game-event, music, object-impacts, particle-impacts, projectile-impact, projectile-detonation, scripted-dialog-force-unspatialized, scripted-dialog-other, scripted-dialog-player, scripted-effect, slow-particle-impacts, unit-dialog, unit-footsteps, vehicle-collision, vehicle-engine, weapon-charge, weapon-empty, weapon-fire, weapon-idle, weapon-overheat, weapon-ready, weapon-reload");

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

            case 'S':
                sound_options.split = false;
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

            case 'q':
                sound_options.vorbis_quality = std::atof(arguments[0]);
                if(sound_options.vorbis_quality > 1.0F || sound_options.vorbis_quality < -0.1F) {
                    eprintf_error("Vorbis quality is outside of the allowed range of -0.1 to 1.0");
                    std::exit(EXIT_FAILURE);
                }
                break;

            case 'P':
                sound_options.fs_path = true;
                break;

            case 'c':
                try {
                    sound_options.sound_class = SoundClass_from_string(arguments[0]);
                }
                catch(std::exception &) {
                    eprintf_error("Unknown sound class %s", arguments[0]);
                    std::exit(EXIT_FAILURE);
                }
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
        if(sound_options.sound_class.has_value()) {
            sound_tag.sound_class = *sound_options.sound_class;
        }
        else {
            sound_options.sound_class = sound_tag.sound_class;
        }
    }
    else {
        sound_tag.format = SoundFormat::SOUND_FORMAT_16_BIT_PCM;
        sound_tag.flags.split_long_sound_into_permutations = 0;

        if(!sound_options.sound_class.has_value()) {
            eprintf_error("A sound class is required when generating new sound tags");
            return EXIT_FAILURE;
        }
        sound_tag.sound_class = *sound_options.sound_class;
    }

    // Hold onto this
    auto sound_class = sound_tag.sound_class;

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
        if(sound.name.size() >= sizeof(TagString)) {
            eprintf_error("Permutation name %s exceeds the maximum permutation name size (%zu >= %zu)", sound.name.data(), sound.name.size(), sizeof(TagString));
            std::exit(EXIT_FAILURE);
        }

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

        // Make it small
        sound.pcm.shrink_to_fit();
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
    if(format == SoundFormat::SOUND_FORMAT_16_BIT_PCM) {
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
            permutation.pcm.shrink_to_fit();
            permutation.channel_count = 2;
        }

        // Sample rate doesn't match; this can be fixed with interpolation
        if(permutation.sample_rate != highest_sample_rate) {
            eprintf_error("Sample rate (%u) does not match the selected sample rate (%u)", permutation.sample_rate, highest_sample_rate);
            eprintf_warn("TODO: Interpolation");
            return EXIT_FAILURE;
        }
    }

    // Add a new permutation
    auto new_permutation = [&pitch_range, &format](SoundReader::Sound &permutation) -> Parser::SoundPermutation & {
        auto &p = pitch_range.permutations.emplace_back();
        std::memset(p.name.string, 0, sizeof(p.name.string));
        std::strncpy(p.name.string, permutation.name.data(), sizeof(p.name.string) - 1);
        p.format = format;
        return p;
    };

    // Add initial permutations
    for(auto &permutation : permutations) {
        new_permutation(permutation);
    }

    // Make the sound tag
    const char *output_name = nullptr;
    switch(format) {
        case SoundFormat::SOUND_FORMAT_16_BIT_PCM:
            output_name = "16-bit PCM";
            break;
        case SoundFormat::SOUND_FORMAT_IMA_ADPCM:
            output_name = "IMA ADPCM";
            break;
        case SoundFormat::SOUND_FORMAT_XBOX_ADPCM:
            output_name = "Xbox ADPCM";
            break;
        case SoundFormat::SOUND_FORMAT_OGG:
            output_name = "Ogg Vorbis";
            break;
    }
    oprintf("Output: %s, %s, %zu Hz%s, %s\n", output_name, highest_channel_count == 1 ? "mono" : "stereo", static_cast<std::size_t>(highest_sample_rate), split ? ", split" : "", SoundClass_to_string(sound_class));
    oprintf("Found %zu sound%s:\n", actual_permutation_count, actual_permutation_count == 1 ? "" : "s");
    for(std::size_t i = 0; i < actual_permutation_count; i++) {
        auto &permutation = permutations[i];

        // Calculate length
        double seconds = permutation.pcm.size() / static_cast<double>(static_cast<std::size_t>(permutation.sample_rate) * static_cast<std::size_t>(permutation.bits_per_sample / 8) * static_cast<std::size_t>(permutation.channel_count));
        std::size_t sample_size = highest_bytes_per_sample * highest_channel_count;
        std::size_t sample_size_16_bit = sizeof(std::int16_t) * highest_channel_count;

        // Encode a permutation
        auto encode_permutation = [&permutation, &format, &sound_options, &highest_channel_count, &highest_sample_rate, &highest_bytes_per_sample, &sample_size, &sample_size_16_bit](Parser::SoundPermutation &p, const std::vector<std::byte> &pcm) {
            switch(format) {
                // Basically, just make it big endian
                case SoundFormat::SOUND_FORMAT_16_BIT_PCM:
                    if(permutation.bits_per_sample == 16) {
                        p.buffer_size = pcm.size();
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
                // Encode to Vorbis in an Ogg container
                case SoundFormat::SOUND_FORMAT_OGG: {
                    // Begin
                    std::vector<std::byte> output_samples;
                    vorbis_info vi;
                    vorbis_info_init(&vi);
                    auto ret = vorbis_encode_init_vbr(&vi, highest_channel_count, highest_sample_rate, sound_options.vorbis_quality);
                    if(ret) {
                        eprintf_error("Failed to initialize vorbis encoder");
                        std::exit(EXIT_FAILURE);
                    }

                    // Set the comment
                    vorbis_comment vc;
                    vorbis_comment_init(&vc);
                    vorbis_comment_add_tag(&vc, "ENCODER", full_version());

                    // Start making a vorbis block
                    vorbis_dsp_state vd;
                    vorbis_block vb;
                    vorbis_analysis_init(&vd, &vi);
                    vorbis_block_init(&vd, &vb);

                    // Ogg packet stuff
                    ogg_packet op;
                    ogg_packet op_comm;
                    ogg_packet op_code;
                    ogg_stream_state os;
                    ogg_stream_init(&os, 0);
                    vorbis_analysis_headerout(&vd, &vc, &op, &op_comm, &op_code);
                    ogg_stream_packetin(&os, &op);
                    ogg_stream_packetin(&os, &op_comm);
                    ogg_stream_packetin(&os, &op_code);

                    // Analyze data
                    std::size_t sample_count = pcm.size() / sample_size;
                    p.buffer_size = static_cast<std::uint32_t>(sample_count * sample_size_16_bit);
                    std::size_t split_count = 1024;
                    std::size_t encoded_count = 0;
                    auto *pcm_data = pcm.data();
                    std::size_t s = 0;
                    while(true) {
                        std::size_t sample_count_to_encode = (sample_count - s);
                        float **buffer = vorbis_analysis_buffer(&vd, sample_count_to_encode);

                        if(sample_count_to_encode > split_count) {
                            sample_count_to_encode = split_count;
                        }
                        encoded_count += sample_count_to_encode;

                        auto divide_by = std::pow(256, highest_bytes_per_sample) / 2;

                        for(std::size_t i = 0; i < sample_count_to_encode; i++) {
                            auto *sample = pcm_data + (s + i) * sample_size;
                            for(std::size_t c = 0; c < highest_channel_count; c++) {
                                auto *channel_sample = sample + c * highest_bytes_per_sample;
                                auto &sample_data = buffer[c][i];
                                sample_data = 0.0F;

                                // Get the sample value
                                std::int64_t sample = 0;
                                for(std::size_t b = 0; b < highest_bytes_per_sample; b++) {
                                    std::int64_t significance = 8 * b;
                                    if(b + 1 == highest_bytes_per_sample) {
                                        sample |= static_cast<std::int64_t>(static_cast<std::int8_t>(channel_sample[b])) << significance;
                                    }
                                    else {
                                        sample |= static_cast<std::int64_t>(static_cast<std::uint8_t>(channel_sample[b])) << significance;
                                    }
                                }

                                // Do it
                                sample_data = sample / static_cast<float>(sample > 0 ? (divide_by - 1) : (divide_by));
                            }
                        }
                        ret = vorbis_analysis_wrote(&vd, sample_count_to_encode);
                        if(ret) {
                            eprintf_error("Failed to read samples");
                            std::exit(EXIT_FAILURE);
                        }

                        // Encode the blocks
                        int eos = 0;
                        while(vorbis_analysis_blockout(&vd, &vb) == 1) {
                            vorbis_analysis(&vb, nullptr);
                            vorbis_bitrate_addblock(&vb);
                            while(vorbis_bitrate_flushpacket(&vd, &op)) {
                                ogg_stream_packetin(&os, &op);
                                while(!eos) {
                                    // Write data
                                    ogg_page og;
                                    if(ogg_stream_pageout(&os, &og)) {
                                        output_samples.insert(output_samples.end(), reinterpret_cast<std::byte *>(og.header), reinterpret_cast<std::byte *>(og.header) + og.header_len);
                                        output_samples.insert(output_samples.end(), reinterpret_cast<std::byte *>(og.body), reinterpret_cast<std::byte *>(og.body) + og.body_len);
                                    }
                                    else {
                                        break;
                                    }

                                    // End
                                    if(ogg_page_eos(&og)) {
                                        eos = 1;
                                    }
                                }
                            }
                        }

                        // Increment; break if we did 0
                        if(sample_count_to_encode == 0) {
                            break;
                        }
                        s += sample_count_to_encode;
                    }

                    // Clean up
                    ogg_stream_clear(&os);
                    vorbis_block_clear(&vb);
                    vorbis_dsp_clear(&vd);
                    vorbis_comment_clear(&vc);
                    vorbis_info_clear(&vi);

                    output_samples.shrink_to_fit();
                    p.samples = std::move(output_samples);
                    break;
                }
                default:
                    eprintf_error("Unimplemented sound format");
                    std::exit(EXIT_FAILURE);
            }
        };

        // Split if requested
        if(split) {
            const std::size_t MAX_SPLIT_SIZE = SPLIT_BUFFER_SIZE - (SPLIT_BUFFER_SIZE % sample_size);
            std::size_t digested = 0;
            while(permutation.pcm.size() > 0) {
                // Basically, if we haven't encoded anything, use the i-th permutation, otherwise make a new one
                auto &p = digested == 0 ? pitch_range.permutations[i] : new_permutation(permutation);
                std::size_t remaining_size = permutation.pcm.size();
                std::size_t permutation_size = remaining_size > MAX_SPLIT_SIZE ? MAX_SPLIT_SIZE : remaining_size;

                // Encode it
                auto *sample_data_start = permutation.pcm.data();
                auto sample_data = std::vector<std::byte>(sample_data_start, sample_data_start + permutation_size);
                permutation.pcm = std::vector<std::byte>(permutation.pcm.begin() + permutation_size, permutation.pcm.end());
                encode_permutation(p, sample_data);
                p.samples.shrink_to_fit();
                permutation.pcm.shrink_to_fit();
                digested += permutation_size;

                if(permutation.pcm.size() == 0) {
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
            auto &p = pitch_range.permutations[i];
            p.next_permutation_index = NULL_INDEX;
            encode_permutation(p, permutation.pcm);
            p.samples.shrink_to_fit();
        }

        // Print sound info
        oprintf("    %-32s%2zu:%06.3f (%2zu-bit %6s %5zu Hz)\n", permutation.name.data(), static_cast<std::size_t>(seconds) / 60, std::fmod(seconds, 60.0), static_cast<std::size_t>(permutation.input_bits_per_sample), permutation.input_channel_count == 1 ? "mono" : "stereo", static_cast<std::size_t>(permutation.input_sample_rate));
        permutation.pcm = std::vector<std::byte>();
    }

    // Wrap it up
    std::memset(pitch_range.name.string, 0, sizeof(pitch_range.name));
    static constexpr char DEFAULT_NAME[] = "default";
    std::memcpy(pitch_range.name.string, DEFAULT_NAME, sizeof(DEFAULT_NAME));
    sound_tag.pitch_ranges[0] = std::move(pitch_range);
    auto sound_tag_data = sound_tag.generate_hek_tag_data(TagClassInt::TAG_CLASS_SOUND, true);
    oprintf("Total: %.03f MiB\n", sound_tag_data.size() / 1024.0 / 1024.0);

    // Create missing directories if needed
    std::filesystem::create_directories(tag_path.parent_path());

    // Save
    if(!Invader::File::save_file(tag_path.string().data(), sound_tag_data)) {
        eprintf_error("Failed to save %s", tag_path.string().data());
        return EXIT_FAILURE;
    }
}
