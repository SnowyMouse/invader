// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__EDIT__QT__TAG_EDITOR_SOUND_SUBWINDOW_HPP
#define INVADER__EDIT__QT__TAG_EDITOR_SOUND_SUBWINDOW_HPP

#include <QAudio>

#include "tag_editor_subwindow.hpp"

class QComboBox;
class QSlider;
class QAudioOutput;
class QPushButton;
class QIODevice;
class QLabel;
class QCheckBox;

namespace Invader::Parser {
    struct SoundPitchRange;
}

namespace Invader::EditQt {
    class TagEditorWindow;

    class TagEditorSoundSubwindow : public TagEditorSubwindow {
        Q_OBJECT

    public:
        /**
         * Update the window
         */
        void update() override;

        /**
         * Instantiate a subwindow
         * @param parent parent window
         */
        TagEditorSoundSubwindow(TagEditorWindow *parent_window);

        ~TagEditorSoundSubwindow() = default;

    private:
        /**
         * Update the current permutation
         */
        void update_permutation_list();

        /**
         * Update the current sound
         */
        void update_sound();

        /**
         * Get the pitch range
         * @return pitch range
         */
        Parser::SoundPitchRange *get_pitch_range() noexcept;

        std::optional<std::vector<std::size_t>> actual_permutation_indices(std::size_t permutation);

        std::uint32_t sample_rate;
        std::uint32_t channel_count;
        std::optional<std::uint32_t> bits_per_sample;

        std::vector<std::byte> all_pcm;
        std::size_t sample = 0;
        std::uint32_t sample_granularity = 0;

        QPushButton *play_button;
        QPushButton *stop_button;
        QComboBox *actual_permutation;
        QComboBox *permutation;
        QSlider *slider;
        QLabel *time;
        QCheckBox *loop;
        QAudioOutput *output = nullptr;
        QIODevice *device = nullptr;

        bool playing = false;

        void state_changed(QAudio::State);
        void play_sound();
        void stop_sound();
        void play_sample();
        void change_sample();
        void update_time_label();

        void closeEvent(QCloseEvent *) override;
    };
}

#endif
