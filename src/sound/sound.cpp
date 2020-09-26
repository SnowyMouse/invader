// SPDX-License-Identifier: GPL-3.0-only

#include <filesystem>
#include <invader/command_line_option.hpp>
#include <invader/printf.hpp>
#include <invader/file/file.hpp>
#include <invader/tag/parser/parser.hpp>
#include <invader/sound/sound_encoder.hpp>
#include <invader/sound/sound_reader.hpp>
#include <invader/version.hpp>
#include <vorbis/vorbisenc.h>
#include <samplerate.h>
#include <atomic>
#include <thread>

using namespace Invader;
using namespace Invader::HEK;

struct SoundOptions {
    const char *data = "data";
    const char *tags = "tags";
    std::optional<bool> split;
    std::optional<SoundFormat> format;
    bool fs_path = false;
    std::optional<float> compression_level;
    bool extended = false;
    std::optional<std::size_t> channel_count;
    std::optional<SoundClass> sound_class;
    std::optional<std::uint32_t> sample_rate;
    std::optional<std::uint16_t> bitrate;
    std::size_t max_threads = 1;
};

static void populate_pitch_range(std::vector<SoundReader::Sound> &permutations, const std::filesystem::path &directory, std::uint32_t &highest_sample_rate, std::uint16_t &highest_channel_count, std::size_t pitch_range_index, Parser::InvaderSound *invader_sound);
static void process_permutation_thread(SoundReader::Sound *permutation, std::uint16_t highest_sample_rate, SoundFormat format, std::uint16_t highest_channel_count, std::atomic<std::size_t> *thread_count);

template<typename T> static std::vector<std::byte> make_sound_tag(const std::filesystem::path &tag_path, const std::filesystem::path &data_path, SoundOptions &sound_options) {
    static constexpr std::size_t SPLIT_BUFFER_SIZE = 0x38E00;
    static constexpr std::size_t MAX_PERMUTATIONS = UINT16_MAX - 1;

    // Parse the sound tag
    T sound_tag = {};
    auto *invader_sound = sizeof(Parser::InvaderSound) == sizeof(T) ? reinterpret_cast<Parser::InvaderSound *>(&sound_tag) : nullptr;

    if(std::filesystem::exists(tag_path)) {
        if(std::filesystem::is_directory(tag_path)) {
            eprintf_error("A directory exists at %s where a file was expected", tag_path.string().c_str());
            std::exit(EXIT_FAILURE);
        }
        auto sound_file = File::open_file(tag_path.string().c_str());
        if(sound_file.has_value()) {
            auto &sound_data = *sound_file;
            try {
                sound_tag = T::parse_hek_tag_file(sound_data.data(), sound_data.size());
            }
            catch(std::exception &e) {
                eprintf_error("An error occurred while attempting to read %s", tag_path.string().c_str());
                std::exit(EXIT_FAILURE);
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
        sound_tag.random_pitch_bounds.from = 1.0F;
        sound_tag.random_pitch_bounds.to = 1.0F;
        sound_tag.outer_cone_gain = 1.0F;
        sound_tag.random_gain_modifier = 1.0F;
        sound_tag.inner_cone_angle = 2 * HALO_PI;
        sound_tag.outer_cone_angle = 2 * HALO_PI;

        if(!sound_options.sound_class.has_value()) {
            eprintf_error("A sound class is required when generating new sound tags");
            std::exit(EXIT_FAILURE);
        }
        sound_tag.sound_class = *sound_options.sound_class;
    }

    if(invader_sound) {
        // Set sample rate
        if(sound_options.sample_rate.has_value()) {
            switch(*sound_options.sample_rate) {
                case 22050:
                    invader_sound->encoding_sample_rate = HEK::InvaderSoundSampleRate::INVADER_SOUND_SAMPLE_RATE_22050_HZ;
                    break;
                case 44100:
                    invader_sound->encoding_sample_rate = HEK::InvaderSoundSampleRate::INVADER_SOUND_SAMPLE_RATE_44100_HZ;
                    break;
                default:
                    eprintf_error("Invalid sample rate given. What?");
                    std::terminate();
            }
        }
        else switch(invader_sound->encoding_sample_rate) {
            case HEK::InvaderSoundSampleRate::INVADER_SOUND_SAMPLE_RATE_AUTOMATIC:
                break;
            case HEK::InvaderSoundSampleRate::INVADER_SOUND_SAMPLE_RATE_22050_HZ:
                sound_options.sample_rate = 22050;
                break;
            case HEK::InvaderSoundSampleRate::INVADER_SOUND_SAMPLE_RATE_44100_HZ:
                sound_options.sample_rate = 44100;
                break;
            default:
                eprintf_error("Invalid sample rate read. What?");
                std::terminate();
                break;
        }

        // Set channel count
        if(sound_options.channel_count.has_value()) {
            switch(*sound_options.channel_count) {
                case 1:
                    invader_sound->encoding_channel_count = HEK::InvaderSoundChannelCount::INVADER_SOUND_CHANNEL_COUNT_MONO;
                    break;
                case 2:
                    invader_sound->encoding_channel_count = HEK::InvaderSoundChannelCount::INVADER_SOUND_CHANNEL_COUNT_STEREO;
                    break;
                default:
                    eprintf_error("Invalid channel count given. What?");
                    std::terminate();
            }
        }
        else switch(invader_sound->encoding_channel_count) {
            case HEK::InvaderSoundChannelCount::INVADER_SOUND_CHANNEL_COUNT_AUTOMATIC:
                break;
            case HEK::InvaderSoundChannelCount::INVADER_SOUND_CHANNEL_COUNT_MONO:
                sound_options.channel_count = 1;
                break;
            case HEK::InvaderSoundChannelCount::INVADER_SOUND_CHANNEL_COUNT_STEREO:
                sound_options.channel_count = 2;
                break;
            default:
                eprintf_error("Invalid channel count read. What?");
                std::terminate();
                break;
        }

        // Set format
        if(sound_options.format.has_value()) {
            invader_sound->encoding_format = *sound_options.format;
        }
        else {
            sound_options.format = invader_sound->encoding_format;
        }
        
        // If we have compression level set, then the sound option shouldn't have this set
        if(sound_options.compression_level.has_value()) {
            invader_sound->invader_sound_flags &= ~InvaderSoundFlagsFlag::INVADER_SOUND_FLAGS_FLAG_USE_CONSTANT_BITRATE_WHEN_POSSIBLE;
        }
        // If we don't have compression level set but we have a bitrate, set the flag
        else if(sound_options.bitrate.has_value()) {
            invader_sound->invader_sound_flags |= InvaderSoundFlagsFlag::INVADER_SOUND_FLAGS_FLAG_USE_CONSTANT_BITRATE_WHEN_POSSIBLE;
            invader_sound->compression_bitrate = *sound_options.bitrate;
        }
        // If we have neither but we have the bitrate flag, set the bitrate
        else if(invader_sound->invader_sound_flags & InvaderSoundFlagsFlag::INVADER_SOUND_FLAGS_FLAG_USE_CONSTANT_BITRATE_WHEN_POSSIBLE) {
            sound_options.bitrate = invader_sound->compression_bitrate;
        }

        // If we have a compression level set, set our compression level to the thing
        if(sound_options.compression_level.has_value()) {
            invader_sound->compression_level = *sound_options.compression_level;
        }
        // Otherwise, default to it
        else {
            sound_options.compression_level = invader_sound->compression_level;
        }

        invader_sound->source_FLACs.clear();
    }

    // Hold onto this
    auto sound_class = sound_tag.sound_class;

    // If a format is defined, use it
    if(sound_options.format.has_value()) {
        sound_tag.format = *sound_options.format;
    }
    else {
        sound_options.format = SoundFormat::SOUND_FORMAT_16_BIT_PCM;
    }

    // Make sure we support the format
    switch(*sound_options.format) {
        case SoundFormat::SOUND_FORMAT_16_BIT_PCM:
        case SoundFormat::SOUND_FORMAT_XBOX_ADPCM:
        case SoundFormat::SOUND_FORMAT_OGG_VORBIS:
            break;
        case SoundFormat::SOUND_FORMAT_FLAC:
            if(!invader_sound) {
                eprintf_error("FLAC cannot be used without --extended");
                std::exit(EXIT_FAILURE);
            }
            break;
        case SoundFormat::SOUND_FORMAT_IMA_ADPCM:
            eprintf_error("IMA ADPCM is unsupported");
            std::exit(EXIT_FAILURE);
        default:
            eprintf_error("Unsupported audio codec");
            std::exit(EXIT_FAILURE);
    }

    auto &format = sound_tag.format;

    // Set a default level
    if(!sound_options.compression_level.has_value()) {
        sound_options.compression_level = 1.0F;
    }

    // Error if bullshit compression levels were given
    if(sound_options.compression_level > 1.0F || sound_options.compression_level < 0.0F) {
        eprintf_error("Compression level (%.05f) is outside of the allowed range of 0.0 to 1.0", *sound_options.compression_level);
        std::exit(EXIT_FAILURE);
    }

    // Clear the old one
    for(auto &old_pitch_range : sound_tag.pitch_ranges) {
        for(auto &permutation : old_pitch_range.permutations) {
            permutation.samples = std::vector<std::byte>();
        }
    }
    if(invader_sound) {
        invader_sound->source_FLACs.clear();
    }

    // Same with whether to split
    if(sound_options.split.has_value()) {
        if(*sound_options.split) {
            sound_tag.flags |= HEK::SoundFlagsFlag::SOUND_FLAGS_FLAG_SPLIT_LONG_SOUND_INTO_PERMUTATIONS;
        }
        else {
            sound_tag.flags &= ~HEK::SoundFlagsFlag::SOUND_FLAGS_FLAG_SPLIT_LONG_SOUND_INTO_PERMUTATIONS;
        }
    }
    bool split = sound_tag.flags & HEK::SoundFlagsFlag::SOUND_FLAGS_FLAG_SPLIT_LONG_SOUND_INTO_PERMUTATIONS;
    
    // Check to see if we have either just directories (so multiple pitch ranges) or just files (one pitch range)
    bool contains_files = false;
    bool contains_directories = false;
    for(auto &f : std::filesystem::directory_iterator(data_path)) {
        if(f.is_directory()) {
            contains_directories = true;
        }
        if(f.is_regular_file()) {
            contains_files = true;
        }
    }
    
    // Is it bullshit?
    if(contains_files && contains_directories) {
        eprintf_error("Data directory must have only directories or only files");
        std::exit(EXIT_FAILURE);
    }
    if(!contains_files && !contains_directories) {
        eprintf_error("Data directory is empty");
        std::exit(EXIT_FAILURE);
    }

    std::uint16_t highest_channel_count = 0;
    std::uint32_t highest_sample_rate = 0;
    std::vector<std::pair<std::vector<SoundReader::Sound>, std::string>> pitch_ranges;

    oprintf("Loading sounds... ");
    oflush();
    
    // Load the sounds
    if(contains_files) {
        auto &pitch_range = pitch_ranges.emplace_back(std::vector<SoundReader::Sound>(), "default");
        populate_pitch_range(pitch_range.first, data_path, highest_sample_rate, highest_channel_count, 0, invader_sound);
    }
    else if(contains_directories) {
        std::size_t i = 0;
        for(auto &f : std::filesystem::directory_iterator(data_path)) {
            auto &path = f.path();
            if(!f.is_directory()) {
                eprintf_error("Unexpected file %s", path.string().c_str());
                std::exit(EXIT_FAILURE);
            }
            auto &pitch_range = pitch_ranges.emplace_back(std::vector<SoundReader::Sound>(), path.filename().string());
            populate_pitch_range(pitch_range.first, path, highest_sample_rate, highest_channel_count, i++, invader_sound);
            if(i == NULL_INDEX) {
                eprintf_error("%u or more pitch ranges are present", NULL_INDEX);
                std::exit(EXIT_FAILURE);
            }

            // Make sure we have stuff
            if(pitch_range.first.size() == 0) {
                eprintf_error("No permutations found in %s", path.string().c_str());
                std::exit(EXIT_FAILURE);
            }
        }
    }

    oprintf("done!\n");

    // Force channel count
    if(sound_options.channel_count.has_value()) {
        highest_channel_count = *sound_options.channel_count;
    }

    // Sound tags currently only support 22.05 kHz and 44.1 kHz
    if(!sound_options.sample_rate.has_value()) {
        if(highest_sample_rate > 22050) {
            highest_sample_rate = 44100;
        }
        else {
            highest_sample_rate = 22050;
        }
    }
    else {
        highest_sample_rate = *sound_options.sample_rate;
    }

    // Set the tag data
    if(highest_sample_rate == 22050) {
        sound_tag.sample_rate = SoundSampleRate::SOUND_SAMPLE_RATE_22050_HZ;
    }
    else if(highest_sample_rate == 44100) {
        sound_tag.sample_rate = SoundSampleRate::SOUND_SAMPLE_RATE_44100_HZ;
    }
    else {
        eprintf_error("Unsupported sample rate %u", highest_sample_rate);
        std::exit(EXIT_FAILURE);
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
        std::exit(EXIT_FAILURE);
    }

    // Resample permutations when needed
    oprintf("Processing sounds... ");
    oflush();
    std::size_t total_sound_count = 0;
    std::atomic<std::size_t> thread_count;
    
    // Wait until we have 0 threads working
    auto wait_until_threads_are_done = [&thread_count]() {
        while(thread_count > 0) {
            std::this_thread::yield();
        }
    };
    
    // Wait until we have room for more threads
    auto wait_until_threads_are_open = [&thread_count, &sound_options]() {
        while(thread_count >= sound_options.max_threads) {
            std::this_thread::yield();
        }
    };
    
    // Process things!
    for(auto &pitch_range : pitch_ranges) {
        for(auto &permutation : pitch_range.first) {
            total_sound_count++;
            wait_until_threads_are_open();
            thread_count++;
            std::thread(process_permutation_thread, &permutation, highest_sample_rate, format, highest_channel_count, &thread_count).detach();
        }
    }
    
    // Wait until done
    wait_until_threads_are_done();
    
    oprintf("done!\n");
    
    // Remove pitch ranges that are present in the tag but not in what we found
    while(true) {
        bool should_continue = false;
        std::size_t pitch_range_count = sound_tag.pitch_ranges.size();
        for(std::size_t p = 0; p < pitch_range_count; p++) {
            should_continue = true;
            auto &name = sound_tag.pitch_ranges[p].name;
            
            // Check the pitch ranges we got
            for(auto &pi : pitch_ranges) {
                if(pi.second == name.string) {
                    should_continue = false;
                    break;
                }
            }
            
            // If we didn't find a match, erase it and continue
            if(should_continue) {
                sound_tag.pitch_ranges.erase(sound_tag.pitch_ranges.begin() + p);
                break;
            }
        }
        
        if(!should_continue) {
            break;
        }
    }
    
    // Index read pitch ranges to output pitch range
    std::size_t pitch_range_count = pitch_ranges.size();
    std::vector<std::size_t> pitch_range_index(pitch_range_count);
    for(std::size_t i = 0; i < pitch_range_count; i++) {
        auto &index = pitch_range_index[i];
        std::size_t old_pitch_range_count = sound_tag.pitch_ranges.size();
        auto &name = pitch_ranges[i].second;
        
        index = NULL_INDEX;
        for(std::size_t p = 0; p < old_pitch_range_count; p++) {
            if(name == sound_tag.pitch_ranges[p].name.string) {
                index = p;
                break;
            }
        }
        
        // If no pitch range was found, add one
        if(index == NULL_INDEX) {
            index = old_pitch_range_count;
            auto &new_pitch_range = sound_tag.pitch_ranges.emplace_back();
            std::strncpy(new_pitch_range.name.string, name.c_str(), sizeof(new_pitch_range.name.string) - 1);
        }
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
        case SoundFormat::SOUND_FORMAT_OGG_VORBIS:
            output_name = "Ogg Vorbis";
            break;
        case SoundFormat::SOUND_FORMAT_FLAC:
            output_name = "Free Lossless Audio Codec";
            break;
        case SoundFormat::SOUND_FORMAT_ENUM_COUNT:
            eprintf_error("Invalid format output name. What?");
            std::terminate();
    }
    oprintf("Found %zu sound%s:\n", total_sound_count, total_sound_count == 1 ? "" : "s");

    // Check if this is dialogue
    bool is_dialogue;
    switch(sound_class) {
        case SoundClass::SOUND_CLASS_UNIT_DIALOG:
        case SoundClass::SOUND_CLASS_SCRIPTED_DIALOG_PLAYER:
        case SoundClass::SOUND_CLASS_SCRIPTED_DIALOG_OTHER:
        case SoundClass::SOUND_CLASS_SCRIPTED_DIALOG_FORCE_UNSPATIALIZED:
            is_dialogue = true;
            break;
        default:
            is_dialogue = false;
    }

    if(is_dialogue && split) {
        eprintf_error("Split dialogue is unsupported.");
        std::exit(EXIT_FAILURE);
    }
    
    // Make sure we don't completely blow things up
    std::mutex encoding_mutex;

    // Encode this
    for(std::size_t pr = 0; pr < pitch_range_count; pr++) {
        encoding_mutex.lock();
        auto &pitch_range = sound_tag.pitch_ranges[pitch_range_index[pr]];
        auto &permutations = pitch_ranges[pr].first;
        auto actual_permutation_count = permutations.size();
        pitch_range.actual_permutation_count = actual_permutation_count;
        pitch_range.permutations.resize(actual_permutation_count);
        encoding_mutex.unlock();
        
        for(auto &p : pitch_range.permutations) {
            p.format = sound_tag.format;
        }
        
        for(std::size_t i = 0; i < actual_permutation_count; i++) {
            // Get the permutation and set its name, too
            auto &permutation = permutations[i];
            std::strncpy(pitch_range.permutations[i].name.string, permutation.name.c_str(), sizeof(pitch_range.permutations[i].name.string) - 1);

            // Calculate length
            double seconds = permutation.pcm.size() / static_cast<double>(static_cast<std::size_t>(permutation.sample_rate) * static_cast<std::size_t>(permutation.bits_per_sample / 8) * static_cast<std::size_t>(permutation.channel_count));
            std::size_t bytes_per_sample_one_channel = permutation.bits_per_sample / 8;
            std::size_t bytes_per_sample_all_channels = bytes_per_sample_one_channel * permutation.channel_count;

            // Encode a permutation
            auto encode_permutation = [](auto *sound_tag, std::size_t pitch_range, std::size_t pitch_range_permutation, std::mutex *mutex, std::vector<std::byte> pcm, const SoundReader::Sound *permutation, bool is_dialogue, SoundFormat format, SoundOptions *sound_options, std::atomic<std::size_t> *thread_count) {
                auto generate_mouth_data = [&permutation](const std::vector<std::uint8_t> &pcm_8_bit) -> std::vector<std::byte> {
                    // Basically, take the sample rate, multiply by channel count, divide by tick rate (30 Hz), and round the result
                    std::size_t samples_per_tick = static_cast<std::size_t>((permutation->sample_rate * permutation->channel_count) / TICK_RATE + 0.5);
                    std::size_t sample_count = pcm_8_bit.size();

                    // Generate samples, adding an extra tick for incomplete ticks
                    std::size_t tick_count = (sample_count + samples_per_tick - 1) / samples_per_tick;
                    std::vector<std::byte> mouth_data = std::vector<std::byte>(tick_count);
                    auto *pcm_data = pcm_8_bit.data();

                    // Get max and total
                    std::uint8_t max = 0;
                    double mouth_total = 0;
                    for(std::size_t t = 0; t < tick_count; t++) {
                        // Get the sample range, accounting for when there aren't enough ticks
                        std::size_t first_sample = t * samples_per_tick;
                        std::size_t sample_count_to_check = sample_count - first_sample;
                        if(sample_count_to_check > samples_per_tick) {
                            sample_count_to_check = samples_per_tick;
                        }
                        std::size_t last_sample = first_sample + sample_count_to_check;
                        double total = 0;
                        for(std::size_t s = first_sample; s < last_sample; s++) {
                            total += pcm_data[s];
                        }

                        // Divide by samples per tick
                        double average = total / samples_per_tick;
                        mouth_total += average;
                        mouth_data[t] = static_cast<std::byte>(average);

                        if(average > max) {
                            max = average;
                        }
                    }

                    // Get average and min, clamping min to 0-255
                    double average = mouth_total / tick_count;
                    double min = 2.0 * average - max;
                    if(min > UINT8_MAX) {
                        min = UINT8_MAX;
                    }
                    else if(min < 0) {
                        min = 0;
                    }

                    // Get range
                    double range = static_cast<double>(max + average) / 2 - min;

                    // Do nothing if there's no range
                    if(range == 0) {
                        return mouth_data;
                    }

                    // Go through each sample
                    for(std::size_t t = 0; t < tick_count; t++) {
                        double sample = (static_cast<std::uint8_t>(mouth_data[t]) - min) / range;

                        // Clamp to 0 - 255
                        if(sample >= 1.0) {
                            mouth_data[t] = static_cast<std::byte>(UINT8_MAX);
                        }
                        else if(sample <= 0.0) {
                            mouth_data[t] = static_cast<std::byte>(0);
                        }
                        else {
                            mouth_data[t] = static_cast<std::byte>(sample * UINT8_MAX);
                        }
                    }
                    
                    return mouth_data;
                };

                // Generate mouth data if needed
                if(is_dialogue) {
                    // Convert samples to 8-bit unsigned so we can use it to generate mouth data
                    auto samples_float = SoundEncoder::convert_int_to_float(permutation->pcm, permutation->bits_per_sample);
                    std::vector<std::uint8_t> pcm_8_bit;
                    pcm_8_bit.reserve(samples_float.size());
                    for(auto &f : samples_float) {
                        float ff = f;
                        if(ff < 0.0F) {
                            ff *= -1.0F;
                        }
                        pcm_8_bit.emplace_back(static_cast<std::uint8_t>(ff * UINT8_MAX));
                    }
                    samples_float = {};
                    generate_mouth_data(pcm_8_bit);
                }
                
                // Do the encoding thing
                std::vector<std::byte> samples;
                std::size_t buffer_size = 0;

                switch(format) {
                    // Basically, just make it 16-bit big endian
                    case SoundFormat::SOUND_FORMAT_16_BIT_PCM:
                        samples = Invader::SoundEncoder::convert_to_16_bit_pcm_big_endian(pcm, permutation->bits_per_sample);
                        buffer_size = samples.size();
                        break;

                    // Encode to Vorbis in an Ogg container
                    case SoundFormat::SOUND_FORMAT_OGG_VORBIS: {
                        if(sound_options->compression_level > 1.0F) {
                            sound_options->compression_level = 1.0F;
                        }
                        else if(sound_options->compression_level < 0.0F) {
                            sound_options->compression_level = 0.0F;
                        }
                        if(sound_options->bitrate.has_value()) {
                            samples = Invader::SoundEncoder::encode_to_ogg_vorbis_cbr(pcm, permutation->bits_per_sample, permutation->channel_count, permutation->sample_rate, *sound_options->bitrate);
                        }
                        else {
                            samples = Invader::SoundEncoder::encode_to_ogg_vorbis_vbr(pcm, permutation->bits_per_sample, permutation->channel_count, permutation->sample_rate, *sound_options->compression_level);
                        }
                        buffer_size = pcm.size() / (permutation->bits_per_sample / 8) * sizeof(std::int16_t);
                        break;
                    }

                    // Encode to FLAC
                    case SoundFormat::SOUND_FORMAT_FLAC: {
                        // Clamp to 0.0 - 0.8
                        if(sound_options->compression_level > 0.8F) {
                            sound_options->compression_level = 0.8F;
                        }
                        else if(sound_options->compression_level < 0.0F) {
                            sound_options->compression_level = 0.0F;
                        }
                        // Convert to integer, rounding to the nearest level
                        int flac_level = static_cast<int>(*sound_options->compression_level * 10.0F + 0.5F);
                        samples = Invader::SoundEncoder::encode_to_flac(pcm, permutation->bits_per_sample, permutation->channel_count, permutation->sample_rate, flac_level);
                        break;
                    }

                    // Encode to Xbox ADPCMeme
                    case SoundFormat::SOUND_FORMAT_XBOX_ADPCM:
                        samples = Invader::SoundEncoder::encode_to_xbox_adpcm(pcm, permutation->bits_per_sample, permutation->channel_count);
                        break;

                    default:
                        eprintf_error("Invalid format. What?");
                        std::terminate();
                }
                
                // Set the default stuff
                mutex->lock();
                auto &p = sound_tag->pitch_ranges[pitch_range].permutations[pitch_range_permutation];
                p.gain = 1.0F;
                p.samples = samples;
                p.buffer_size = buffer_size;
                p.samples.shrink_to_fit();
                mutex->unlock();
                
                // Finish up
                (*thread_count)--;
            };

            // Split if requested
            if(split) {
                const std::size_t MAX_SPLIT_SIZE = SPLIT_BUFFER_SIZE - (SPLIT_BUFFER_SIZE % bytes_per_sample_all_channels);
                std::size_t digested = 0;
                while(permutation.pcm.size() > 0) {
                    // Basically, if we haven't encoded anything, use the i-th permutation, otherwise make a new one as a copy
                    encoding_mutex.lock();
                    auto &p = digested == 0 ? pitch_range.permutations[i] : pitch_range.permutations.emplace_back(pitch_range.permutations[i]);
                    encoding_mutex.unlock();
                    std::size_t remaining_size = permutation.pcm.size();
                    std::size_t permutation_size = remaining_size > MAX_SPLIT_SIZE ? MAX_SPLIT_SIZE : remaining_size;

                    // Encode it
                    auto *sample_data_start = permutation.pcm.data();
                    auto sample_data = std::vector<std::byte>(sample_data_start, sample_data_start + permutation_size);
                    permutation.pcm = std::vector<std::byte>(permutation.pcm.begin() + permutation_size, permutation.pcm.end());
                    permutation.pcm.shrink_to_fit();
                    digested += permutation_size;

                    if(permutation.pcm.size() == 0) {
                        p.next_permutation_index = NULL_INDEX;
                    }
                    else {
                        std::size_t next_permutation = pitch_range.permutations.size();
                        if(next_permutation > MAX_PERMUTATIONS) {
                            encoding_mutex.lock();
                            eprintf_error("Maximum number of total permutations (%zu > %zu) exceeded", next_permutation, MAX_PERMUTATIONS);
                            std::exit(EXIT_FAILURE);
                        }
                        p.next_permutation_index = static_cast<Index>(next_permutation);
                    }
                    
                    // Wait until we have threads cleared up
                    wait_until_threads_are_open();
                    
                    // Punch it
                    thread_count++;
                    std::thread(encode_permutation, &sound_tag, pr, &p - pitch_range.permutations.data(), &encoding_mutex, std::move(sample_data), &permutation, is_dialogue, format, &sound_options, &thread_count).join();
                }
            }
            else {
                // Wait until we have threads cleared up
                wait_until_threads_are_open();
                    
                // Punch it
                thread_count++;
                auto &p = pitch_range.permutations[i];
                p.next_permutation_index = NULL_INDEX;
                std::thread(encode_permutation, &sound_tag, pr, &p - pitch_range.permutations.data(), &encoding_mutex, std::move(permutation.pcm), &permutation, is_dialogue, format, &sound_options, &thread_count).join();
            }

            // Print sound info
            encoding_mutex.lock();
            oprintf("    %-32s%2zu:%06.3f (%2zu-bit %6s %5zu Hz)\n", permutation.name.c_str(), static_cast<std::size_t>(seconds) / 60, std::fmod(seconds, 60.0), static_cast<std::size_t>(permutation.input_bits_per_sample), permutation.input_channel_count == 1 ? "mono" : "stereo", static_cast<std::size_t>(permutation.input_sample_rate));
            permutation.pcm = std::vector<std::byte>();
            encoding_mutex.unlock();
        }
    }
    
    // Wait until we have 0 threads left
    wait_until_threads_are_done();

    auto sound_tag_data = sound_tag.generate_hek_tag_data(invader_sound == nullptr ? TagClassInt::TAG_CLASS_SOUND : TagClassInt::TAG_CLASS_INVADER_SOUND, true);
    oprintf("Output: %s, %s, %zu Hz%s, %s, %.03f MiB%s\n", output_name, highest_channel_count == 1 ? "mono" : "stereo", static_cast<std::size_t>(highest_sample_rate), split ? ", split" : "", SoundClass_to_string(sound_class), sound_tag_data.size() / 1024.0 / 1024.0, invader_sound == nullptr ? "" : " [--extended]");

    return sound_tag_data;
}

int main(int argc, const char **argv) {
    EXIT_IF_INVADER_EXTRACT_HIDDEN_VALUES

    SoundOptions sound_options;

    std::vector<CommandLineOption> options;
    options.emplace_back("info", 'i', 0, "Show credits, source info, and other info.");
    options.emplace_back("tags", 't', 1, "Use the specified tags directory. Use multiple times to add more directories, ordered by precedence.", "<dir>");
    options.emplace_back("data", 'd', 1, "Use the specified data directory.", "<dir>");
    options.emplace_back("split", 's', 0, "Split permutations into 227.5 KiB chunks. This is necessary for longer sounds (e.g. music) when being played in the original Halo engine.");
    options.emplace_back("no-split", 'S', 0, "Do not split permutations.");
    options.emplace_back("extended", 'x', 0, "Create an invader_sound tag (required for some features).");
    options.emplace_back("format", 'F', 1, "Set the format. Can be: 16-bit-pcm, ogg-vorbis, or xbox-adpcm. Using flac requires --extended. Setting this is required unless creating an extended tag, in which case it defaults to 16-bit-pcm.", "<fmt>");
    options.emplace_back("fs-path", 'P', 0, "Use a filesystem path for the data.");
    options.emplace_back("channel-count", 'C', 1, "[REQUIRES --extended] Set the channel count. Can be: mono, stereo. By default, this is determined based on the input audio.", "<#>");
    options.emplace_back("sample-rate", 'r', 1, "[REQUIRES --extended] Set the sample rate in Hz. Halo supports 22050 and 44100. By default, this is determined based on the input audio.", "<Hz>");
    options.emplace_back("compress-level", 'l', 1, "Set the compression level. This can be between 0.0 and 1.0. For Ogg Vorbis, higher levels result in better quality but worse sizes. For FLAC, higher levels result in better sizes but longer compression time, clamping from 0.0 to 0.8 (FLAC 0 to FLAC 8). Default: 1.0", "<lvl>");
    options.emplace_back("bitrate", 'b', 1, "Set the bitrate in kilobits per second. This only applies to vorbis.", "<br>");
    options.emplace_back("class", 'c', 1, "Set the class. This is required when generating new sounds. Can be: ambient-computers, ambient-machinery, ambient-nature, device-computers, device-door, device-force-field, device-machinery, device-nature, first-person-damage, game-event, music, object-impacts, particle-impacts, projectile-impact, projectile-detonation, scripted-dialog-force-unspatialized, scripted-dialog-other, scripted-dialog-player, scripted-effect, slow-particle-impacts, unit-dialog, unit-footsteps, vehicle-collision, vehicle-engine, weapon-charge, weapon-empty, weapon-fire, weapon-idle, weapon-overheat, weapon-ready, weapon-reload", "<class>");
    options.emplace_back("threads", 'j', 1, "Set the number of threads to use for parallel resampling and encoding. Default: 1");

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

            case 'j':
                try {
                    sound_options.max_threads = std::stoi(arguments[0]);
                    if(sound_options.max_threads < 1) {
                        throw std::exception();
                    }
                }
                catch(std::exception &) {
                    eprintf_error("Invalid number of threads %s\n", arguments[0]);
                    std::exit(EXIT_FAILURE);
                }
                break;

            case 'S':
                sound_options.split = false;
                break;

            case 'b':
                try {
                    sound_options.bitrate = static_cast<std::uint16_t>(std::stol(arguments[0]));
                }
                catch(std::exception &) {
                    eprintf_error("Invalid bitrate: %s", arguments[0]);
                    std::exit(EXIT_FAILURE);
                }
                break;

            case 'C':
                try {
                    sound_options.channel_count = SoundChannelCount_from_string(arguments[0]) == SoundChannelCount::SOUND_CHANNEL_COUNT_MONO ? 1 : 2;
                }
                catch(std::exception &) {
                    eprintf_error("Unknown channel count %s (should be \"mono\" or \"stereo\")", arguments[0]);
                    std::exit(EXIT_FAILURE);
                }
                break;

            case 'F':
                try {
                    sound_options.format = SoundFormat_from_string(arguments[0]);
                }
                catch(std::exception &) {
                    eprintf_error("Unknown sound format %s", arguments[0]);
                    std::exit(EXIT_FAILURE);
                }
                break;

            case 'l':
                sound_options.compression_level = std::atof(arguments[0]);
                break;

            case 'P':
                sound_options.fs_path = true;
                break;

            case 'x':
                sound_options.extended = true;
                break;

            case 'r':
                try {
                    sound_options.sample_rate = static_cast<std::uint32_t>(std::stol(arguments[0]));
                }
                catch(std::exception &) {
                    eprintf_error("Invalid sample rate: %s", arguments[0]);
                    std::exit(EXIT_FAILURE);
                }
                if(sound_options.sample_rate != 22050 && sound_options.sample_rate != 44100) {
                    eprintf_error("Only 22050 Hz and 44100 Hz sample rates are allowed");
                    std::exit(EXIT_FAILURE);
                }
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
    
    if(sound_options.bitrate.has_value() && sound_options.compression_level.has_value()) {
        eprintf_error("Bitrate and compression level are mutually exclusive.");
        return EXIT_FAILURE;
    }

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

    // Remove trailing slashes
    halo_tag_path = Invader::File::remove_trailing_slashes(halo_tag_path);
    auto data_path = std::filesystem::path(sound_options.data) / halo_tag_path;
    if(!std::filesystem::is_directory(data_path)) {
        eprintf_error("No directory exists at %s", data_path.string().c_str());
        return EXIT_FAILURE;
    }

    // Find it!
    auto tag_path = std::filesystem::path(sound_options.tags) / (halo_tag_path + ".invader_sound");
    if(std::filesystem::exists(tag_path)) {
        sound_options.extended = true;
    }

    // Generate sound tag
    std::vector<std::byte> sound_tag_data;
    try {
        if(!sound_options.extended) {
            tag_path = std::filesystem::path(sound_options.tags) / (halo_tag_path + ".sound");

            // Make sure format was set
            if(!sound_options.format.has_value()) {
                eprintf_error("No sound format set (required for .sound tags). Use -h for more information.");
                return EXIT_FAILURE;
            }

            sound_tag_data = make_sound_tag<Parser::Sound>(tag_path, data_path, sound_options);
        }
        else {
            sound_tag_data = make_sound_tag<Parser::InvaderSound>(tag_path, data_path, sound_options);
        }
    }
    catch(std::exception &e) {
        eprintf_error("Failed to create sound tag due to an exception error: %s", e.what());
        return EXIT_FAILURE;
    }

    // Create missing directories if needed
    try {
        if(!std::filesystem::exists(tag_path.parent_path())) {
            std::filesystem::create_directories(tag_path.parent_path());
        }
    }
    catch(std::exception &e) {
        eprintf_error("Error: Failed to create a directory: %s\n", e.what());
        return EXIT_FAILURE;
    }

    // Save
    if(!Invader::File::save_file(tag_path.string().c_str(), sound_tag_data)) {
        eprintf_error("Failed to save %s", tag_path.string().c_str());
        return EXIT_FAILURE;
    }
}

static void populate_pitch_range(std::vector<SoundReader::Sound> &permutations, const std::filesystem::path &directory, std::uint32_t &highest_sample_rate, std::uint16_t &highest_channel_count, std::size_t pitch_range_index, Parser::InvaderSound *invader_sound) {
    for(auto &wav : std::filesystem::directory_iterator(directory)) {
        // Skip directories
        auto path = wav.path();
        auto path_str = path.string();
        auto *path_str_cstr = path_str.c_str();
        if(wav.is_directory()) {
            eprintf_error("Unexpected directory %s", path_str_cstr);
            std::exit(EXIT_FAILURE);
        }
        auto extension = path.extension().string();

        // Get the sound
        SoundReader::Sound sound = {};
        try {
            if(extension == ".wav") {
                sound = SoundReader::sound_from_wav_file(path_str_cstr);
            }
            else if(extension == ".flac") {
                sound = SoundReader::sound_from_flac_file(path_str_cstr);
            }
            else {
                eprintf_error("Unknown file format for %s", path_str_cstr);
                std::exit(EXIT_FAILURE);
            }
        }
        catch(std::exception &e) {
            eprintf_error("Failed to load %s: %s", path_str_cstr, e.what());
            std::exit(EXIT_FAILURE);
        }

        // Get the permutation name
        auto filename = path.filename().string();
        sound.name = filename.substr(0, filename.size() - extension.size());
        if(sound.name.size() >= sizeof(HEK::TagString)) {
            eprintf_error("Permutation name %s exceeds the maximum permutation name size (%zu >= %zu)", sound.name.c_str(), sound.name.size(), sizeof(HEK::TagString));
            std::exit(EXIT_FAILURE);
        }

        // Lowercase it
        for(char &c : sound.name) {
            c = std::tolower(c);
        }

        // Add it to the source list
        if(invader_sound) {
            auto &source = invader_sound->source_FLACs.emplace_back();
            std::memset(source.file_name.string, 0, sizeof(source.file_name.string));
            std::strncpy(source.file_name.string, sound.name.c_str(), sizeof(source.file_name) - 1);
            if(extension == ".flac") {
                source.compressed_audio_data = *File::open_file(path_str_cstr);
            }
            else {
                source.compressed_audio_data = SoundEncoder::encode_to_flac(sound.pcm, sound.bits_per_sample, sound.channel_count, sound.sample_rate, 5);
            }
            source.pitch_range = pitch_range_index;
        }

        // Use the highest channel count and sample rate
        if(highest_channel_count < sound.channel_count) {
            highest_channel_count = sound.channel_count;
        }
        if(highest_sample_rate < sound.sample_rate) {
            highest_sample_rate = sound.sample_rate;
        }

        // Make sure we can actually work with this
        if(sound.channel_count > 2 || sound.channel_count < 1) {
            eprintf_error("Unsupported channel count %u in %s", static_cast<unsigned int>(sound.channel_count), path_str.c_str());
            std::exit(EXIT_FAILURE);
        }
        if(sound.bits_per_sample % 8 != 0 || sound.bits_per_sample < 8 || sound.bits_per_sample > 24) {
            eprintf_error("Bits per sample (%u) is not divisible by 8 in %s (or is too small or too big)", static_cast<unsigned int>(sound.bits_per_sample), path_str.data());
            std::exit(EXIT_FAILURE);
        }

        // Make it small
        sound.pcm.shrink_to_fit();

        // Add it
        std::size_t i;
        for(i = 0; i < permutations.size(); i++) {
            if(sound.name < permutations[i].name) {
                break;
            }
            else if(sound.name == permutations[i].name) {
                eprintf_error("Multiple permutations with the same name (%s) cannot be added", permutations[i].name.c_str());
                std::exit(EXIT_FAILURE);
            }
        }
        permutations.insert(permutations.begin() + i, std::move(sound));
    }
}

static void process_permutation_thread(SoundReader::Sound *permutation, std::uint16_t highest_sample_rate, SoundFormat format, std::uint16_t highest_channel_count, std::atomic<std::size_t> *thread_count) {
    // Calculate some stuff
    std::size_t bytes_per_sample = permutation->bits_per_sample / 8;
    std::size_t sample_count = permutation->pcm.size() / bytes_per_sample;
    
    // Mutex for errors
    static std::mutex error_mutex;

    // Sample rate doesn't match; this can be fixed with resampling
    if(permutation->sample_rate != highest_sample_rate) {
        double ratio = static_cast<double>(highest_sample_rate) / permutation->sample_rate;
        std::vector<float> float_samples = SoundEncoder::convert_int_to_float(permutation->pcm, permutation->bits_per_sample);
        std::vector<float> new_samples(float_samples.size() * ratio);
        permutation->sample_rate = highest_sample_rate;

        // Resample it
        SRC_DATA data = {};
        data.data_in = float_samples.data();
        data.data_out = new_samples.data();
        data.input_frames = float_samples.size() / permutation->channel_count;
        data.output_frames = new_samples.size() / permutation->channel_count;
        data.src_ratio = ratio;
        int res = src_simple(&data, SRC_SINC_BEST_QUALITY, permutation->channel_count);
        if(res) {
            error_mutex.lock();
            eprintf_error("Failed to resample: %s", src_strerror(res));
            std::exit(EXIT_FAILURE);
        }
        new_samples.resize(data.output_frames_gen * permutation->channel_count);

        // Set stuff
        if(format == SoundFormat::SOUND_FORMAT_16_BIT_PCM) {
            bytes_per_sample = sizeof(std::uint16_t);
        }
        permutation->sample_rate = highest_sample_rate;
        permutation->bits_per_sample = bytes_per_sample * 8;
        sample_count = new_samples.size();
        permutation->pcm = SoundEncoder::convert_float_to_int(new_samples, permutation->bits_per_sample);
    }

    // Bits per sample doesn't match; we can fix that though
    if(bytes_per_sample != sizeof(std::uint16_t) && format == SoundFormat::SOUND_FORMAT_16_BIT_PCM) {
        std::size_t new_bytes_per_sample = sizeof(std::uint16_t);
        permutation->pcm = SoundEncoder::convert_int_to_int(permutation->pcm, permutation->bits_per_sample, new_bytes_per_sample * 8);
        bytes_per_sample = new_bytes_per_sample;
        permutation->bits_per_sample = new_bytes_per_sample * 8;
    }

    // Mono -> Stereo (just duplicate the channels)
    if(permutation->channel_count == 1 && highest_channel_count == 2) {
        std::vector<std::byte> new_samples(sample_count * 2 * bytes_per_sample);
        const std::byte *old_sample = permutation->pcm.data();
        const std::byte *old_sample_end = permutation->pcm.data() + permutation->pcm.size();
        std::byte *new_sample = new_samples.data();

        while(old_sample < old_sample_end) {
            std::memcpy(new_sample, old_sample, bytes_per_sample);
            std::memcpy(new_sample + bytes_per_sample, old_sample, bytes_per_sample);
            old_sample += bytes_per_sample;
            new_sample += bytes_per_sample * 2;
        }

        permutation->pcm = std::move(new_samples);
        permutation->pcm.shrink_to_fit();
        permutation->channel_count = 2;
    }

    // Stereo -> Mono (mixdown)
    else if(permutation->channel_count == 2 && highest_channel_count == 1) {
        std::vector<std::byte> new_samples(sample_count * bytes_per_sample / 2);
        std::byte *new_sample = new_samples.data();
        const std::byte *old_sample = permutation->pcm.data();
        const std::byte *old_sample_end = permutation->pcm.data() + permutation->pcm.size();

        while(old_sample < old_sample_end) {
            std::int32_t a = Invader::SoundEncoder::read_sample(old_sample, permutation->bits_per_sample);
            std::int32_t b = Invader::SoundEncoder::read_sample(old_sample + bytes_per_sample, permutation->bits_per_sample);
            std::int64_t ab = a + b;
            Invader::SoundEncoder::write_sample(static_cast<std::int32_t>(ab / 2), new_sample, permutation->bits_per_sample);

            old_sample += bytes_per_sample * 2;
            new_sample += bytes_per_sample;
        }

        permutation->pcm = std::move(new_samples);
        permutation->pcm.shrink_to_fit();
        permutation->channel_count = 1;
    }
    
    (*thread_count)--;
}
