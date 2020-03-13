// SPDX-License-Identifier: GPL-3.0-only

#include <QAudioDeviceInfo>
#include <QAudioOutput>
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

namespace Invader::EditQt {
    void TagEditorSoundSubwindow::update() {
        QWidget *widget = new QWidget();
        QVBoxLayout *layout = new QVBoxLayout();
        layout->setSpacing(4);
        layout->setMargin(4);
        widget->setLayout(layout);
        this->setCentralWidget(widget);

        // Add options
        auto add_option = [&layout](const char *label_text, auto **set_to, double width_multiplier) -> QLayout * {
            auto *option_widget = new QWidget();
            auto *option_layout = new QHBoxLayout();
            option_layout->setMargin(0);
            option_layout->setSpacing(8);
            auto *option_label = new QLabel(label_text);
            option_layout->addWidget(option_label);
            option_widget->setLayout(option_layout);
            int width = option_label->fontMetrics().horizontalAdvance('M') * 13;
            option_label->setMinimumWidth(width);
            option_label->setMaximumWidth(width);
            *set_to = new QComboBox();
            (*set_to)->setMinimumWidth(width * width_multiplier);
            option_layout->addWidget(*set_to);
            layout->addWidget(option_widget);
            return option_layout;
        };

        // Generate our options
        add_option("Actual permutation:", &this->actual_permutation, 2.0);

        this->loop = new QCheckBox("Loop");
        add_option("Permutation:", &this->permutation, 1.0)->addWidget(this->loop);

        // Add playback stuff
        this->slider = new QSlider(Qt::Orientation::Horizontal);
        connect(this->slider, &QSlider::valueChanged, this, &TagEditorSoundSubwindow::change_sample);
        layout->addWidget(this->slider);

        // Add playback
        auto *playback_widget = new QWidget();
        auto *playback_layout = new QHBoxLayout();
        playback_layout->setMargin(0);
        playback_layout->setSpacing(4);
        playback_widget->setLayout(playback_layout);
        this->play_button = new QPushButton("Play");
        connect(this->play_button, &QPushButton::clicked, this, &TagEditorSoundSubwindow::play_sound);
        this->stop_button = new QPushButton("Stop");
        connect(this->stop_button, &QPushButton::clicked, this, &TagEditorSoundSubwindow::stop_sound);
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
        Parser::SoundPitchRange *pitch_ranges = this->get_pitch_range();
        if(!pitch_ranges) {
            this->actual_permutation->setEnabled(false);
            this->permutation->setEnabled(false);
            return;
        }

        // All right
        std::size_t actual_permutations = pitch_ranges->actual_permutation_count;
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
        this->update_permutation_list();
    }

    TagEditorSoundSubwindow::TagEditorSoundSubwindow(TagEditorWindow *parent_window) : TagEditorSubwindow(parent_window) {
        this->update();
        this->adjustSize();
        this->setMaximumHeight(this->height());
        this->setMinimumHeight(this->height());
        this->setMaximumWidth(this->width());
        this->setMinimumWidth(this->width());
        this->center_window();
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
        auto &bits_per_sample = this->bits_per_sample;

        auto add_to_pcm = [&pcm, &bits_per_sample](const SoundReader::Sound &what) {
            std::vector<std::byte> new_pcm;
            const auto *pcm_data = &what.pcm;
            if(!bits_per_sample.has_value()) {
                bits_per_sample = what.bits_per_sample;
                if(*bits_per_sample > 24) {
                    *bits_per_sample = 24;
                }
                else if(*bits_per_sample < 16) {
                    *bits_per_sample = 16;
                }
            }
            if(what.bits_per_sample != *bits_per_sample) {
                new_pcm = SoundEncoder::convert_int_to_int(what.pcm, what.bits_per_sample, *bits_per_sample);
                pcm_data = &new_pcm;
            }
            pcm.insert(pcm.end(), pcm_data->begin(), pcm_data->end());
        };

        // Do it!
        for(auto p : permutations_to_play) {
            auto &permutation = permutations[p];
            auto *sound_data = permutation.samples.data();
            auto sound_size = permutation.samples.size();
            switch(permutation.format) {
                case HEK::SoundFormat::SOUND_FORMAT_16_BIT_PCM:
                    add_to_pcm(SoundReader::sound_from_16_bit_pcm_big_endian(sound_data, sound_size, this->channel_count, this->sample_rate));
                    break;
                case HEK::SoundFormat::SOUND_FORMAT_FLAC:
                    add_to_pcm(SoundReader::sound_from_flac(sound_data, sound_size));
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

        // Set up the audio thing
        if(!this->output) {
            QAudioFormat format;
            format.setSampleRate(this->sample_rate);
            format.setChannelCount(this->channel_count);
            format.setSampleSize(*bits_per_sample);
            format.setCodec("audio/pcm");
            format.setByteOrder(QAudioFormat::LittleEndian);
            format.setSampleType(QAudioFormat::SignedInt);

            // Check if it works
            QAudioDeviceInfo info = QAudioDeviceInfo::defaultOutputDevice();
            if(!info.isFormatSupported(format)) {
                QAudioDeviceInfo device = QAudioDeviceInfo::defaultOutputDevice();
                oprintf("Querying %s...\n", device.deviceName().toLatin1().data());

                for(int i : device.supportedSampleRates()) {
                    oprintf("    %.03f kHz\n", i / 1000.0);
                }

                for(int i : device.supportedSampleSizes()) {
                    oprintf("    %i-bit\n", i);
                }

                for(auto i : device.supportedCodecs()) {
                    oprintf("    %s\n", i.toLatin1().data());
                }

                char message[256];
                std::snprintf(message, sizeof(message), "Your default output device does not support playback of this sound (%.03f kHz, %u ch, %u-bit)\n\nMake sure you're not using a dinosaur.", this->sample_rate / 1000.0, this->channel_count, *bits_per_sample);
                QMessageBox(QMessageBox::Icon::Critical, "Error", message, QMessageBox::Ok).exec();
                return;
            }

            this->output = new QAudioOutput(format, this);
            connect(this->output, &QAudioOutput::stateChanged, this, &TagEditorSoundSubwindow::state_changed);
            this->output->setNotifyInterval(1);
            connect(this->output, &QAudioOutput::notify, this, &TagEditorSoundSubwindow::play_sample);
            this->device = this->output->start();
        }

        this->playing = false;
        this->all_pcm = pcm;
        this->sample = 0;
        this->slider->blockSignals(true);
        this->slider->setValue(0);
        this->sample_granularity = this->channel_count * (*this->bits_per_sample / 8);
        this->slider->setMaximum(this->all_pcm.size() / this->sample_granularity);
        this->slider->blockSignals(false);
        this->update_time_label();

        this->stop_sound();
    }

    Parser::SoundPitchRange *TagEditorSoundSubwindow::get_pitch_range() noexcept {
        auto &sample_rate = this->sample_rate;
        auto &channel_count = this->channel_count;
        auto get_it = [&channel_count, &sample_rate](auto *what) -> Parser::SoundPitchRange * {
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

            if(what->pitch_ranges.size()) {
                return what->pitch_ranges.data();
            }
            else {
                return nullptr;
            }
        };

        // Depending on the class, get the thing
        auto *parser_data = this->get_parent_window()->get_parser_data();
        switch(this->get_parent_window()->get_file().tag_class_int) {
            case TagClassInt::TAG_CLASS_SOUND:
                return get_it(dynamic_cast<Parser::Sound *>(parser_data));
            default:
                std::terminate();
        }
    }

    void TagEditorSoundSubwindow::state_changed(QAudio::State state) {
        switch(state) {
            case QAudio::State::ActiveState:
                break;
            case QAudio::State::IdleState:
            case QAudio::State::InterruptedState:
            case QAudio::State::StoppedState:
            case QAudio::State::SuspendedState:
                this->stop_sound();
                break;
        }
    }

    void TagEditorSoundSubwindow::play_sound() {
        this->stop_button->setEnabled(true);
        this->play_button->setEnabled(false);
        this->playing = true;
        if(this->sample >= this->all_pcm.size()) {
            this->sample = 0;
        }
        this->update_time_label();
        this->play_sample();
    }

    void TagEditorSoundSubwindow::stop_sound() {
        this->play_button->setEnabled(true);
        this->stop_button->setEnabled(false);
        this->playing = false;
    }

    void TagEditorSoundSubwindow::play_sample() {
        // If we're done, stop
        if(!this->playing) {
            this->stop_sound();
            return;
        }

        // Get how much data is left
        std::size_t pcm_data_remaining = this->all_pcm.size() - this->sample;
        bool play_in_loop = this->loop->checkState() == Qt::Checked;
        if(pcm_data_remaining == 0) {
            if(play_in_loop) {
                this->sample = 0;
                pcm_data_remaining = this->all_pcm.size();
            }
            else {
                this->stop_sound();
                return;
            }
        }

        auto *data = reinterpret_cast<const char *>(this->all_pcm.data());
        std::size_t pcm_data_increment = this->device->write(data + this->sample, pcm_data_remaining);

        // Did we play everything? And are we looping? If so, loop back.
        this->sample += pcm_data_increment;
        if(this->sample == this->all_pcm.size() && play_in_loop) {
            this->sample = this->device->write(data, this->all_pcm.size());
        }

        this->slider->blockSignals(true);
        this->slider->setValue(this->sample / this->sample_granularity);
        this->slider->blockSignals(false);

        this->update_time_label();
    }

    void TagEditorSoundSubwindow::change_sample() {
        this->sample = this->slider->value() * this->sample_granularity;
        this->update_time_label();
    }

    void TagEditorSoundSubwindow::closeEvent(QCloseEvent *) {
        this->stop_sound();
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
}
