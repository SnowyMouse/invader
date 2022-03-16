// SPDX-License-Identifier: GPL-3.0-only

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QFontDatabase>
#include <QCheckBox>
#include <invader/printf.hpp>
#include "../tag_editor_window.hpp"
#include <invader/tag/parser/parser.hpp>
#include "tag_editor_sound_subwindow.hpp"
#include <invader/sound/sound_encoder.hpp>
#include <invader/sound/sound_reader.hpp>

#undef LittleEndian
#undef BigEndian

#define CONVERSION_BITS_PER_SAMPLE 16

namespace Invader::EditQt {
    void TagEditorSoundSubwindow::update() {
        QWidget *widget = new QWidget();
        QVBoxLayout *layout = new QVBoxLayout();
        layout->setSpacing(4);
        layout->setContentsMargins(4, 4, 4, 4);
        widget->setLayout(layout);
        this->setCentralWidget(widget);

        // Add options
        auto add_option = [&layout](const char *label_text, auto *what, double width_multiplier) -> QLayout * {
            auto *option_widget = new QWidget();
            auto *option_layout = new QHBoxLayout();
            option_layout->setContentsMargins(0, 0, 0, 0);
            option_layout->setSpacing(8);
            auto *option_label = new QLabel(label_text);
            option_layout->addWidget(option_label);
            option_widget->setLayout(option_layout);
            int width = option_label->fontMetrics().horizontalAdvance('M') * 13;
            option_label->setMinimumWidth(width);
            option_label->setMaximumWidth(width);
            what->setMinimumWidth(width * width_multiplier);
            option_layout->addWidget(what);
            layout->addWidget(option_widget);
            return option_layout;
        };

        auto *pitch_range = new QComboBox();
        add_option("Pitch range:", pitch_range, 2.0);
        this->pitch_range = pitch_range;
        auto *parser_data = this->get_parent_window()->get_parser_data();
        
        for(auto &i : dynamic_cast<Parser::Sound &>(*parser_data).pitch_ranges) {
            this->pitch_range->addItem(i.name.string);
        }
        
        connect(this->pitch_range, &QComboBox::currentTextChanged, this, &TagEditorSoundSubwindow::update_pitch_range_permutations);

        // Generate our options
        this->actual_permutation = new QComboBox();
        add_option("Actual permutation:", this->actual_permutation, 2.0);

        this->loop = new QCheckBox("Loop");
        this->permutation = new QComboBox();
        add_option("Permutation:", this->permutation, 1.0)->addWidget(this->loop);
        
        // Add playback stuff
        this->slider = new QSlider(Qt::Orientation::Horizontal);
        connect(this->slider, &QSlider::valueChanged, this, &TagEditorSoundSubwindow::change_sample);
        layout->addWidget(this->slider);

        // Add playback
        auto *playback_widget = new QWidget();
        auto *playback_layout = new QHBoxLayout();
        playback_layout->setContentsMargins(0, 0, 0, 0);
        playback_layout->setSpacing(4);
        playback_widget->setLayout(playback_layout);
        this->play_button = new QPushButton("Play");
        connect(this->play_button, &QPushButton::clicked, this, &TagEditorSoundSubwindow::play_sound);
        this->stop_button = new QPushButton("Stop");
        connect(this->stop_button, &QPushButton::clicked, this, &TagEditorSoundSubwindow::stop_sound);
        this->stop_button->setEnabled(false);
        playback_layout->addWidget(this->play_button);
        playback_layout->addWidget(this->stop_button);

        // Show the time
        this->time = new QLabel();
        this->time->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
        this->time->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
        playback_layout->addWidget(this->time);
        layout->addWidget(playback_widget);

        // Done
        layout->addStretch();

        // Whoop!
        this->update_pitch_range_permutations();

        // Gray out everything if no pitch ranges exist
        widget->setEnabled(this->pitch_range->count() > 0);
    }
    
    void TagEditorSoundSubwindow::update_pitch_range_permutations() {
        this->actual_permutation->blockSignals(true);
        this->actual_permutation->clear();
        
        Parser::SoundPitchRange *pitch_ranges = this->get_pitch_range();
        std::size_t actual_permutations = 0;
        
        if(pitch_ranges != nullptr) {
            // All right
            actual_permutations = pitch_ranges->actual_permutation_count;
            if(actual_permutations == 0) {
                actual_permutations = pitch_ranges->permutations.size();
            }
            if(actual_permutations <= pitch_ranges->permutations.size()) {
                for(std::size_t p = 0; p < actual_permutations; p++) {
                    char permutation_name[256];
                    std::snprintf(permutation_name, sizeof(permutation_name), "%zu (%s)", p, pitch_ranges->permutations[p].name.string);
                    this->actual_permutation->addItem(permutation_name);
                }
            }
            else {
                this->actual_permutation->setEnabled(false);
                this->permutation->setEnabled(false);
                return;
            }

            connect(this->actual_permutation, &QComboBox::currentTextChanged, this, &TagEditorSoundSubwindow::update_permutation_list);
            connect(this->permutation, &QComboBox::currentTextChanged, this, &TagEditorSoundSubwindow::update_sound);
            this->actual_permutation->blockSignals(false);
            this->update_permutation_list();
        }

        this->play_button->setEnabled(actual_permutations > 0 && this->sdl_audio_device_id != 0);
        this->actual_permutation->setEnabled(actual_permutations > 0);
        this->permutation->setEnabled(actual_permutations > 0);
        this->loop->setEnabled(actual_permutations > 0);
        this->slider->setEnabled(actual_permutations > 0);
    }

    TagEditorSoundSubwindow::TagEditorSoundSubwindow(TagEditorWindow *parent_window) : TagEditorSubwindow(parent_window) {
        unsigned int channel_count;
        std::uint32_t sample_rate;
        
        auto get_it = [&channel_count, &sample_rate](auto *what) {
            switch(what->sample_rate) {
                case HEK::SoundSampleRate::SOUND_SAMPLE_RATE_44100_HZ:
                    sample_rate = 44100;
                    break;
                case HEK::SoundSampleRate::SOUND_SAMPLE_RATE_22050_HZ:
                    sample_rate = 22050;
                    break;
                default:
                    std::terminate();
            }
            switch(what->channel_count) {
                case HEK::SoundChannelCount::SOUND_CHANNEL_COUNT_MONO:
                    channel_count = 1;
                    break;
                case HEK::SoundChannelCount::SOUND_CHANNEL_COUNT_STEREO:
                    channel_count = 2;
                    break;
                default:
                    std::terminate();
            }
        };
        
        // Depending on the class, get the thing
        auto *parser_data = this->get_parent_window()->get_parser_data();
        switch(this->get_parent_window()->get_file().tag_fourcc) {
            case TagFourCC::TAG_FOURCC_SOUND:
                get_it(dynamic_cast<Parser::Sound *>(parser_data));
                break;
            default:
                std::terminate();
        }
        
        this->channel_count = channel_count;
        this->sample_rate = sample_rate;
        
        SDL_AudioSpec request = {}, result = {}, preferred = {};
        request.userdata = this;

        int q = SDL_GetAudioDeviceSpec(0, 0, &preferred);
        request.freq = preferred.freq;
        request.samples = preferred.samples;
        request.channels = preferred.channels;
        
        if(q != 0) {
            eprintf_error("Got an error with SDL_GetAudioDeviceSpec(): %s", SDL_GetError());
        }
        else {
            this->sdl_audio_device_id = SDL_OpenAudioDevice(nullptr, 0, &request, &result, 0);
            if(this->sdl_audio_device_id == 0) {
                eprintf_error("Got an error with SDL_OpenAudioDevice(): %s", SDL_GetError());
            }
        }
        
        if(this->sdl_audio_device_id != 0) {
            // Create a stream (required for resampling)
            this->stream = SDL_NewAudioStream(AUDIO_S16SYS, this->channel_count, this->sample_rate, result.format, result.channels, result.freq);
            if(this->stream == nullptr) {
                eprintf_error("SDL_NewAudioStream() fail: %s", SDL_GetError());
                SDL_CloseAudioDevice(this->sdl_audio_device_id);
                this->sdl_audio_device_id = 0;
            }
        }
        
        if(this->sdl_audio_device_id == 0) {
            QMessageBox(QMessageBox::Icon::Critical, "Error", "SDL audio could not be initialized. Playback will be disabled!", QMessageBox::Ok).exec();
        }
        
        this->update();
        this->adjustSize();
        this->setMaximumHeight(this->height());
        this->setMinimumHeight(this->height());
        this->setMaximumWidth(this->width());
        this->setMinimumWidth(this->width());
        this->center_window();
        
        this->sample_timer.callOnTimeout(this, &TagEditorSoundSubwindow::play_sample);
    }

    void TagEditorSoundSubwindow::update_permutation_list() {
        int selection = this->actual_permutation->currentIndex();
        if(selection < 0) {
            return;
        }

        // Do it
        auto actual_permutation_unsigned = static_cast<std::size_t>(selection);
        this->permutation->blockSignals(true);
        this->permutation->clear();
        auto all_permutations = this->actual_permutation_indices(actual_permutation_unsigned);
        if(all_permutations.has_value()) {
            this->permutation->addItem("All");
            for(auto &i : *all_permutations) {
                this->permutation->addItem(QString::number(i));
            }
        }
        this->permutation->setCurrentIndex(0);
        this->permutation->blockSignals(false);

        this->update_sound();
    }

    std::optional<std::vector<std::size_t>> TagEditorSoundSubwindow::actual_permutation_indices(std::size_t permutation) {
        std::vector<std::size_t> indices;
        auto &permutations = this->get_pitch_range()->permutations;

        do {
            // Make sure we're not out of bounds or we're not at an infinite loop
            if(permutation >= permutations.size()) {
                return std::nullopt;
            }
            for(auto &i : indices) {
                if(i == permutation) {
                    return std::nullopt;
                }
            }
            indices.emplace_back(permutation);
            permutation = permutations[permutation].next_permutation_index;
        }
        while(permutation != NULL_INDEX && permutation != 0);

        return indices;
    }

    void TagEditorSoundSubwindow::update_sound() {
        int selection = this->actual_permutation->currentIndex();
        if(selection < 0) {
            return;
        }
        auto actual_permutation_unsigned = static_cast<std::size_t>(selection);

        int permutation_selection = this->permutation->currentIndex();
        if(permutation_selection < 0) {
            return;
        }
        auto permutation_unsigned = static_cast<std::size_t>(permutation_selection);

        // Figure out what permutation(s) we're playing
        auto all_permutations = this->actual_permutation_indices(actual_permutation_unsigned);
        std::vector<std::size_t> permutations_to_play;
        if(permutation_unsigned == 0) {
            permutations_to_play = all_permutations.value();
        }
        else {
            permutations_to_play.emplace_back(all_permutations.value()[permutation_unsigned - 1]);
        }

        auto &permutations = this->get_pitch_range()->permutations;
        std::vector<std::byte> pcm;

        auto add_to_pcm = [&pcm](const SoundReader::Sound &what) {
            std::vector<std::byte> new_pcm;
            const auto *pcm_data = &what.pcm;
            if(what.bits_per_sample != CONVERSION_BITS_PER_SAMPLE) {
                new_pcm = SoundEncoder::convert_int_to_int(what.pcm, what.bits_per_sample, CONVERSION_BITS_PER_SAMPLE);
                pcm_data = &new_pcm;
            }
            pcm.insert(pcm.end(), pcm_data->begin(), pcm_data->end());
        };

        // Do it!
        for(auto p : permutations_to_play) {
            auto &permutation = permutations[p];
            auto *sound_data = permutation.samples.data();
            auto sound_size = permutation.samples.size();
            try {
                switch(permutation.format) {
                    case HEK::SoundFormat::SOUND_FORMAT_16_BIT_PCM:
                        add_to_pcm(SoundReader::sound_from_16_bit_pcm_big_endian(sound_data, sound_size, this->channel_count, this->sample_rate));
                        break;
                    case HEK::SoundFormat::SOUND_FORMAT_OGG_VORBIS:
                        add_to_pcm(SoundReader::sound_from_ogg(sound_data, sound_size));
                        break;
                    case HEK::SoundFormat::SOUND_FORMAT_XBOX_ADPCM:
                        add_to_pcm(SoundReader::sound_from_xbox_adpcm(sound_data, sound_size, this->channel_count, this->sample_rate));
                        break;
                    default:
                        return;
                }
            }
            catch(Invader::InvalidInputSoundException &) {
                pcm.clear();
                QMessageBox(QMessageBox::Icon::Critical, "Error", "Failed to load all data.\n\nThe tag may be corrupt.", QMessageBox::Ok).exec();
            }
        }
        
        this->all_pcm = pcm;
        this->sample = 0;
        this->slider->blockSignals(true);
        this->slider->setValue(0);
        this->sample_granularity = this->channel_count * (CONVERSION_BITS_PER_SAMPLE / 8);
        this->slider->setMaximum(this->all_pcm.size() / this->sample_granularity);
        this->slider->blockSignals(false);
        this->update_time_label();

        this->stop_sound();
    }

    Parser::SoundPitchRange *TagEditorSoundSubwindow::get_pitch_range() noexcept {
        int index = this->pitch_range->currentIndex();
        if(index < 0) {
            return nullptr;
        }
        
        auto get_it = [&index](auto *what) -> Parser::SoundPitchRange * {
            if(what->pitch_ranges.size()) {
                return what->pitch_ranges.data() + index;
            }
            else {
                return nullptr;
            }
        };
        
        // Depending on the class, get the thing
        auto *parser_data = this->get_parent_window()->get_parser_data();
        switch(this->get_parent_window()->get_file().tag_fourcc) {
            case TagFourCC::TAG_FOURCC_SOUND:
                return get_it(dynamic_cast<Parser::Sound *>(parser_data));
            default:
                std::terminate();
        }
    }

    void TagEditorSoundSubwindow::play_sound() {
        this->stop_button->setEnabled(true);
        this->play_button->setEnabled(false);
        if(this->sample >= this->all_pcm.size()) {
            this->sample = 0;
        }
        this->update_time_label();
        SDL_PauseAudioDevice(this->sdl_audio_device_id, 0);
        this->sample_timer.start(1);
    }

    void TagEditorSoundSubwindow::stop_sound() {
        this->play_button->setEnabled(this->sdl_audio_device_id != 0);
        this->stop_button->setEnabled(false);
        this->sample_timer.stop();
    }

    void TagEditorSoundSubwindow::play_sample() {
        const auto *data = this->all_pcm.data();
        auto end = this->all_pcm.size();
        
        // If we're done, stop
        if(end == 0) {
            this->stop_sound();
            return;
        }
        
        auto *this_window = this;
        auto update_info = [&this_window]() {
            this_window->slider->blockSignals(true);
            this_window->slider->setValue(this_window->sample / this_window->sample_granularity);
            this_window->slider->blockSignals(false);
            this_window->update_time_label();
        };
        
        // Play in a loop?
        bool play_in_loop = this->loop->isChecked();
        
        // Are we running low on samples?
        while(true) {
            auto queued_size = SDL_GetQueuedAudioSize(this->sdl_audio_device_id);
            if(queued_size > 4096*8) {
                break;
            }
            
            // Have we reached the end?
            if(this->sample == end) {
                // Loop!
                if(play_in_loop) {
                    this->sample = 0;
                }
                
                // No looping. Okay, anything queued?
                else if(queued_size != 0) {
                    SDL_AudioStreamFlush(this->stream);
                }
                
                // No looping. Nothing is queued.
                else {
                    update_info();
                    return;
                }
            }
            
            // Get the resampled audio
            std::byte audio_buffer[4096] = {};
            auto amount = SDL_AudioStreamGet(this->stream, audio_buffer, sizeof(audio_buffer));
            
            // Error
            if(amount < 0) {
                eprintf_error("SDL_AudioStreamGet() error: %s", SDL_GetError());
                update_info();
                this->stop_sound();
                return;
            }
            
            // Queue the audio
            if(amount != 0) {
                auto result = SDL_QueueAudio(this->sdl_audio_device_id, audio_buffer, amount);
                if(result != 0) {
                    eprintf_error("SDL_QueueAudio() error: %s", SDL_GetError());
                    update_info();
                    this->stop_sound();
                    return;
                }
            }
            
            // No audio left. We have to get more
            else {
                auto *cursor = data + this->sample;
                auto remainder = std::min(end - this->sample, static_cast<std::size_t>(512));
                
                int result = SDL_AudioStreamPut(this->stream, cursor, remainder);
                if(result != 0) {
                    std::printf("%zu %zu\n", this->sample, remainder);
                    
                    eprintf_error("SDL_AudioStreamPut() error: %s", SDL_GetError());
                    update_info();
                    this->stop_sound();
                    return;
                }
                
                this->sample += remainder;
            }
        }
        
        // Update everything
        update_info();
    }

    void TagEditorSoundSubwindow::change_sample() {
        this->sample = this->slider->value() * this->sample_granularity;
        this->update_time_label();
    }

    void TagEditorSoundSubwindow::closeEvent(QCloseEvent *) {
        this->stop_sound();
        
        // If we failed, delete us
        if(this->sdl_audio_device_id == 0) {
            this->get_parent_window()->subwindow = nullptr;
            this->deleteLater();
        }
    }

    void TagEditorSoundSubwindow::update_time_label() {
        char format[64] = {};

        std::size_t centiseconds = (this->sample / this->sample_granularity * 100) / this->sample_rate;
        std::size_t seconds = centiseconds / 100;
        std::size_t minutes = seconds / 60;

        std::size_t total_centiseconds = (this->all_pcm.size() / this->sample_granularity * 100) / this->sample_rate;
        std::size_t total_seconds = total_centiseconds / 100;
        std::size_t total_minutes = total_seconds / 60;

        std::snprintf(format, sizeof(format), "%02zu:%02zu.%02zu / %02zu:%02zu.%02zu", minutes % 100, seconds % 60, centiseconds % 100, total_minutes % 100, total_seconds % 60, total_centiseconds % 100);
        this->time->setText(format);
    }
    
    TagEditorSoundSubwindow::~TagEditorSoundSubwindow() {
        if(this->stream) {
            SDL_FreeAudioStream(this->stream);
            this->stream = nullptr;
        }
        
        if(this->sdl_audio_device_id != 0) {
            SDL_CloseAudioDevice(this->sdl_audio_device_id);
        }
    }
}
