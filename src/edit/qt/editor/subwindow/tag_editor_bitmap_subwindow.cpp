// SPDX-License-Identifier: GPL-3.0-only

#include <QScrollArea>
#include <QHBoxLayout>
#include <QLabel>
#include <QImage>
#include <QComboBox>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QMessageBox>
#include "../tag_editor_window.hpp"
#include "tag_editor_bitmap_subwindow.hpp"
#include "tag_editor_subwindow.hpp"
#include "../../../../bitmap/color_plate_scanner.hpp"
#include "s3tc/s3tc.hpp"
#include <invader/tag/parser/parser.hpp>

namespace Invader::EditQt {
    void TagEditorBitmapSubwindow::set_values(TagEditorBitmapSubwindow *what, QComboBox *bitmaps, QComboBox *mipmaps, QComboBox *colors, QComboBox *scale, QComboBox *sequence, QComboBox *sprite, QScrollArea *images, std::vector<Parser::BitmapGroupSequence> *all_sequences) {
        what->mipmaps = mipmaps;
        what->colors = colors;
        what->bitmaps = bitmaps;
        what->scale = scale;
        what->sequence = sequence;
        what->sprite = sprite;
        what->images = images;
        what->all_sequences = all_sequences;

        connect(mipmaps, &QComboBox::currentTextChanged, what, &TagEditorBitmapSubwindow::reload_view);
        connect(colors, &QComboBox::currentTextChanged, what, &TagEditorBitmapSubwindow::reload_view);
        connect(scale, &QComboBox::currentTextChanged, what, &TagEditorBitmapSubwindow::reload_view);
        connect(bitmaps, &QComboBox::currentTextChanged, what, &TagEditorBitmapSubwindow::refresh_data);

        if(sequence) {
            connect(sequence, &QComboBox::currentTextChanged, what, &TagEditorBitmapSubwindow::refresh_sprite_list);
            connect(sprite, &QComboBox::currentTextChanged, what, &TagEditorBitmapSubwindow::select_sprite);
        }

        what->refresh_data();
    }

    static QWidget *generate_text_widget(const char *text, QComboBox **box) {
        // Create the widget
        QWidget *w = new QWidget();
        QHBoxLayout *l = new QHBoxLayout();
        l->setMargin(0);
        l->setSpacing(0);
        w->setLayout(l);

        // Set the label
        QLabel *label = new QLabel(text);
        int width = label->fontMetrics().horizontalAdvance('M') * 10;
        label->setMinimumWidth(width);
        label->setMaximumWidth(width);
        l->addWidget(label);
        label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

        // And the combo box
        *box = new QComboBox();
        l->addWidget(*box);
        l->addStretch();

        // Done
        return w;
    }

    template<typename T> static void generate_main_widget(TagEditorBitmapSubwindow *subwindow, T *bitmap_data, void (*set_values)(TagEditorBitmapSubwindow *, QComboBox *, QComboBox *, QComboBox *, QComboBox *, QComboBox *, QComboBox *, QScrollArea *, std::vector<Parser::BitmapGroupSequence> *)) {
        // Set up the main widget
        auto *main_widget = new QWidget();
        subwindow->setCentralWidget(main_widget);
        auto *main_layout = new QVBoxLayout();
        main_widget->setLayout(main_layout);

        // Add the header
        QComboBox *bitmaps, *mipmaps, *colors, *scale, *sequence = nullptr, *sprite = nullptr;
        main_layout->setSpacing(4);
        main_layout->setMargin(4);
        main_layout->addWidget(generate_text_widget("Bitmap:", &bitmaps));
        main_layout->addWidget(generate_text_widget("Mipmap:", &mipmaps));
        main_layout->addWidget(generate_text_widget("Channels:", &colors));
        BitmapType type = bitmap_data->type;
        main_layout->addWidget(generate_text_widget("Scale:", &scale));

        // Add this if we have sprites
        if(type == BitmapType::BITMAP_TYPE_SPRITES) {
            main_layout->addWidget(generate_text_widget("Sequence:", &sequence));
            main_layout->addWidget(generate_text_widget("Sprite:", &sprite));

            // Populate sequences
            sequence->addItem("None");
            sprite->setEnabled(false);
            auto sequence_count = bitmap_data->bitmap_group_sequence.size();
            for(std::size_t i = 0; i < sequence_count; i++) {
                sequence->addItem(QString::number(i));
            }
        }

        // Get the size
        auto bitmap_count = bitmap_data->bitmap_data.size();
        for(std::size_t i = 0; i < bitmap_count; i++) {
            bitmaps->addItem(QString::number(i));
        }

        // Colors
        colors->addItem("ARGB");
        colors->addItem("RGB only");
        colors->addItem("Alpha only");
        colors->addItem("Red only");
        colors->addItem("Green only");
        colors->addItem("Blue only");
        colors->setCurrentIndex(0);

        // Zoom stuff
        for(int z = 0; z < 4; z++) {
            char t[8];
            std::snprintf(t, sizeof(t), "%ix", 1 << z);
            scale->addItem(t);
        }
        scale->setCurrentIndex(0);

        // Set up the scroll view
        auto *scroll_view = new QScrollArea();
        main_layout->addWidget(scroll_view);
        scroll_view->setWidgetResizable(true);

        // Set the stuff we just got
        set_values(subwindow, bitmaps, mipmaps, colors, scale, sequence, sprite, scroll_view, &bitmap_data->bitmap_group_sequence);
    }

    void TagEditorBitmapSubwindow::TagEditorBitmapSubwindow::update() {
        auto *parent_window = this->get_parent_window();
        auto *data = parent_window->get_parser_data();
        switch(parent_window->get_file().tag_class_int) {
            case TagClassInt::TAG_CLASS_BITMAP:
                generate_main_widget(this, dynamic_cast<Parser::Bitmap *>(data), TagEditorBitmapSubwindow::set_values);
                break;
            case TagClassInt::TAG_CLASS_EXTENDED_BITMAP:
                generate_main_widget(this, dynamic_cast<Parser::ExtendedBitmap *>(data), TagEditorBitmapSubwindow::set_values);
                break;
            default:
                std::terminate();
        }
    }

    TagEditorBitmapSubwindow::TagEditorBitmapSubwindow(TagEditorWindow *parent_window) : TagEditorSubwindow(parent_window) {
        this->update();
        this->center_window();
    }

    void TagEditorBitmapSubwindow::refresh_data() {
        this->mipmaps->blockSignals(true);
        this->mipmaps->setUpdatesEnabled(false);

        // Set mipmaps
        this->mipmaps->clear();

        int index = this->bitmaps->currentIndex();
        if(index >= 0) {
            std::size_t index_unsigned = static_cast<std::size_t>(index);
            auto *parent_window = this->get_parent_window();
            Parser::BitmapData *bitmap_data;
            switch(parent_window->get_file().tag_class_int) {
                case TagClassInt::TAG_CLASS_BITMAP:
                    bitmap_data = &dynamic_cast<Parser::Bitmap *>(parent_window->get_parser_data())->bitmap_data[index_unsigned];
                    break;
                case TagClassInt::TAG_CLASS_EXTENDED_BITMAP:
                    bitmap_data = &dynamic_cast<Parser::ExtendedBitmap *>(parent_window->get_parser_data())->bitmap_data[index_unsigned];
                    break;
                default:
                    std::terminate();
            }
            std::size_t mipmap_count = bitmap_data->mipmap_count;
            this->mipmaps->addItem("All");
            std::size_t width = bitmap_data->width;
            std::size_t height = bitmap_data->height;
            std::size_t depth = bitmap_data->depth;
            for(std::size_t i = 0; i <= mipmap_count; i++) {
                // Based on the type, we could have two or three numbers here
                char resolution[128];
                auto resolution_ending = static_cast<std::size_t>(std::snprintf(resolution, sizeof(resolution), "%zu x %zu", width, height));
                switch(bitmap_data->type) {
                    case HEK::BitmapDataType::BITMAP_DATA_TYPE_3D_TEXTURE:
                        std::snprintf(resolution + resolution_ending, sizeof(resolution) - resolution_ending, " x %zu", depth);
                        break;
                    case HEK::BitmapDataType::BITMAP_DATA_TYPE_CUBE_MAP:
                        std::snprintf(resolution + resolution_ending, sizeof(resolution) - resolution_ending, " x 6");
                        break;
                    default:
                        break;
                }

                // Add item
                char name[256];
                if(i == 0) {
                    std::snprintf(name, sizeof(name), "Main image (%s)", resolution);
                }
                else {
                    std::snprintf(name, sizeof(name), "Mipmap #%zu (%s)", i, resolution);
                }
                this->mipmaps->addItem(name);

                width /= 2;
                height /= 2;
                depth /= 2;
                if(width == 0) {
                    width = 1;
                }
                if(height == 0) {
                    height = 1;
                }
                if(depth == 0) {
                    depth = 1;
                }
            }
            this->mipmaps->setCurrentIndex(0);
        }

        this->mipmaps->blockSignals(false);
        this->mipmaps->setUpdatesEnabled(true);

        this->reload_view();
    }

    QGraphicsView *TagEditorBitmapSubwindow::draw_bitmap_to_widget(Parser::BitmapData *bitmap_data, std::size_t mipmap, std::size_t index, Colors mode, int scale, const std::vector<std::byte> *pixel_data) {
        // Get the dimensions of the mipmap
        std::size_t width = static_cast<std::size_t>(bitmap_data->width);
        std::size_t height = static_cast<std::size_t>(bitmap_data->height);
        std::size_t depth = static_cast<std::size_t>(bitmap_data->depth);
        std::size_t real_width = width;
        std::size_t real_height = height;
        std::size_t offset = bitmap_data->pixel_data_offset;;
        std::size_t bits_per_pixel;
        bool compressed = false;

        switch(bitmap_data->format) {
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_A1R5G5B5:
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_R5G6B5:
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_A4R4G4B4:
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_A8Y8:
                bits_per_pixel = 16;
                break;

            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_A8R8G8B8:
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_X8R8G8B8:
                bits_per_pixel = 32;
                break;

            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_DXT5:
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_DXT3:
                compressed = true;
                // fallthrough
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_A8:
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_Y8:
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_P8_BUMP:
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_AY8:
                bits_per_pixel = 8;
                break;

            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_DXT1:
                bits_per_pixel = 4;
                compressed = true;
                break;

            default:
                return nullptr;
        }

        std::size_t stride = bitmap_data->type == HEK::BitmapDataType::BITMAP_DATA_TYPE_CUBE_MAP ? 6 : 1;

        // Find the offset
        std::size_t pixels_required = ((depth * width * height) * (bits_per_pixel) / 8);
        for(std::size_t m = 0; m < mipmap; m++) {
            offset += pixels_required * stride;
            real_width /= 2;
            real_height /= 2;
            width = real_width;
            height = real_height;
            depth /= 2;
            if(width < 1) {
                width = 1;
            }
            if(depth < 1) {
                depth = 1;
            }
            if(height < 1) {
                height = 1;
            }
            if(compressed) {
                if(width % 4) {
                    width += 4 - (width % 4);
                }
                if(height % 4) {
                    height += 4 - (height % 4);
                }
            }
            pixels_required = ((depth * width * height) * (bits_per_pixel) / 8);
        }

        // Recalculate pixels required with 1 depth since we're doing 1 bitmap at a time
        pixels_required = ((width * height) * (bits_per_pixel) / 8);
        offset += index * pixels_required;

        // Zero width/height
        if(real_width == 0) {
            real_width = 1;
        }
        if(real_height == 0) {
            real_height = 1;
        }

        std::size_t pixel_count = real_width * real_height;
        std::size_t data_remaining = pixel_data->size();
        if(offset >= data_remaining || data_remaining - offset < pixels_required) {
            eprintf_warn("Not enough data left for bitmap preview (%zu < %zu)", data_remaining, pixels_required);
            QMessageBox(QMessageBox::Icon::Critical, "Error", "Failed to load all data.\n\nThe tag may be corrupt.", QMessageBox::Ok).exec();
            return nullptr;
        }

        const auto *bytes = pixel_data->data() + offset;
        std::vector<std::uint32_t> data(real_width * real_height);

        auto decode_8_bit = [&pixels_required, &bytes, &data](ColorPlatePixel (*with_what)(std::uint8_t)) {
            auto pixels_left = pixels_required;
            auto *bytes_to_add = bytes;
            auto *bytes_to_write = data.data();
            while(pixels_left) {
                auto from_pixel = *reinterpret_cast<const std::uint8_t *>(bytes_to_add);
                auto to_pixel = with_what(from_pixel);
                *bytes_to_write = ((static_cast<std::uint32_t>(to_pixel.alpha) << 24) | (static_cast<std::uint32_t>(to_pixel.red) << 16) | (static_cast<std::uint32_t>(to_pixel.green) << 8) | static_cast<std::uint32_t>(to_pixel.blue));

                pixels_left -= sizeof(from_pixel);
                bytes_to_add += sizeof(from_pixel);
                bytes_to_write++;
            }
        };

        auto decode_16_bit = [&pixels_required, &bytes, &data](ColorPlatePixel (*with_what)(std::uint16_t)) {
            auto pixels_left = pixels_required;
            auto *bytes_to_add = bytes;
            auto *bytes_to_write = data.data();
            while(pixels_left) {
                auto from_pixel = *reinterpret_cast<const std::uint16_t *>(bytes_to_add);
                auto to_pixel = with_what(from_pixel);
                *bytes_to_write = ((static_cast<std::uint32_t>(to_pixel.alpha) << 24) | (static_cast<std::uint32_t>(to_pixel.red) << 16) | (static_cast<std::uint32_t>(to_pixel.green) << 8) | static_cast<std::uint32_t>(to_pixel.blue));

                pixels_left -= sizeof(from_pixel);
                bytes_to_add += sizeof(from_pixel);
                bytes_to_write++;
            }
        };

        auto copy_block = [&real_width, &real_height](const std::uint32_t *from, std::uint32_t *to, std::size_t to_x, std::size_t to_y, std::size_t width, std::size_t height) {
            for(std::uint32_t y = 0; y < 4 && y < height && (y + to_y < real_height); y++) {
                for(std::uint32_t x = 0; x < 4 && x < width && (x + to_x < real_width); x++) {
                    std::uint32_t color = from[x + 4 * y];
                    to[to_x + x + real_width * (to_y + y)] = ((color & 0xFF) << 24) | (((color >> 24) & 0xFF) << 16) | (((color >> 16) & 0xFF) << 8) | (((color >> 8) & 0xFF));
                }
            }
        };

        // Let's decode the thing
        std::size_t block_h = (real_height + 3) / 4;
        std::size_t block_w = (real_width + 3) / 4;
        const auto *block_input = reinterpret_cast<const std::uint8_t *>(bytes);

        switch(bitmap_data->format) {
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_DXT1:
                for(std::size_t y = 0; y < block_h; y++) {
                    for(std::size_t x = 0; x < block_w; x++) {
                        std::uint32_t output[4*4];
                        S3TCH::DecompressBlockDXT1(0, 0, 4, block_input + x * 8 + y * 8 * block_w, output);
                        copy_block(output, data.data(), x * 4, y * 4, width, height);
                    }
                }
                break;

            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_DXT3:
                for(std::size_t y = 0; y < block_h; y++) {
                    for(std::size_t x = 0; x < block_w; x++) {
                        std::uint32_t output[4*4];
                        // Decompress color
                        auto *input = block_input + x * 16 + y * 16 * block_w;
                        S3TCH::DecompressBlockDXT1(0, 0, 4, input + 8, output);

                        // Decompress alpha
                        auto input_alpha = *reinterpret_cast<const std::uint64_t *>(input);
                        for(std::uint32_t ya = 0; ya < 4; ya++) {
                            for(std::uint32_t xa = 0; xa < 4; xa++) {
                                auto &oa = output[xa + ya * 4];
                                oa &= 0xFFFFFF00;
                                oa |= static_cast<std::uint32_t>((input_alpha & 0b1111) / 15.0 * 0xFF);
                                input_alpha >>= 4;
                            }
                        }
                        copy_block(output, data.data(), x * 4, y * 4, width, height);
                    }
                }
                break;

            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_DXT5:
                for(std::size_t y = 0; y < block_h; y++) {
                    for(std::size_t x = 0; x < block_w; x++) {
                        std::uint32_t output[4*4];
                        S3TCH::DecompressBlockDXT5(0, 0, 4, block_input + x * 16 + y * 16 * block_w, output);
                        copy_block(output, data.data(), x * 4, y * 4, width, height);
                    }
                }
                break;

            // 16-bit color
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_A1R5G5B5:
                decode_16_bit(ColorPlatePixel::convert_from_16_bit<1,5,5,5>);
                break;
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_R5G6B5:
                decode_16_bit(ColorPlatePixel::convert_from_16_bit<0,5,6,5>);
                break;
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_A4R4G4B4:
                decode_16_bit(ColorPlatePixel::convert_from_16_bit<4,4,4,4>);
                break;

            // 32-bit color
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_A8R8G8B8:
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_X8R8G8B8:
                std::memcpy(data.data(), bytes, pixel_count * sizeof(*data.data()));
                break;

            // Monochrome
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_A8Y8:
                decode_16_bit(ColorPlatePixel::convert_from_a8y8);
                break;
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_A8:
                decode_8_bit(ColorPlatePixel::convert_from_a8);
                break;
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_Y8:
                decode_8_bit(ColorPlatePixel::convert_from_y8);
                break;
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_AY8:
                decode_8_bit(ColorPlatePixel::convert_from_ay8);
                break;

            // p8
            case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_P8_BUMP:
                decode_8_bit(ColorPlatePixel::convert_from_p8);
                break;

            default:
                break;
        }

        // Scale if needed
        if(scale > 1) {
            std::size_t new_height = real_height * scale;
            std::size_t new_width = real_width * scale;
            std::vector<std::uint32_t> scaled(new_height * new_width);
            auto scale_s = static_cast<std::size_t>(scale);
            for(std::size_t y = 0; y < real_height; y++) {
                for(std::size_t x = 0; x < real_width; x++) {
                    auto pixel = data[x + y * real_width];
                    auto base_x = x * scale_s;
                    auto base_y = y * scale_s;
                    for(std::size_t ys = 0; ys < scale_s; ys++) {
                        for(std::size_t xs = 0; xs < scale_s; xs++) {
                            scaled[xs + base_x + (ys + base_y) * new_width] = pixel;
                        }
                    }
                }
            }
            real_width = new_width;
            real_height = new_height;
            pixel_count *= scale * scale;
            data = std::move(scaled);
        }

        // Filter for mode
        auto set_to_channel = [](std::size_t channel, std::uint32_t &input) {
            std::uint32_t value = (input >> (24 - (channel * 8))) & 0xFF;
            input = (value) | (value << 8) | (value << 16) | (value << 24);
        };

        switch(mode) {
            case COLOR_ARGB:
                for(std::size_t y = 0; y < real_height; y++) {
                    for(std::size_t x = 0; x < real_width; x++) {
                        // Blend with checkerboard
                        auto luminosity = static_cast<std::uint8_t>(((x / 4) % 2) ^ !((y / 4) % 2) ? 0x5F : 0x3F);
                        ColorPlatePixel checkerboard = { luminosity, luminosity, luminosity, 0xFF };
                        auto &pixel_output = data[x + y * real_width];

                        ColorPlatePixel pixel = { static_cast<std::uint8_t>(pixel_output), static_cast<std::uint8_t>(pixel_output >> 8), static_cast<std::uint8_t>(pixel_output >> 16), static_cast<std::uint8_t>(pixel_output >> 24) };
                        auto resulting_pixel = checkerboard.alpha_blend(pixel);

                        pixel_output = ((static_cast<std::uint32_t>(resulting_pixel.alpha) << 24) & 0xFF000000) | ((static_cast<std::uint32_t>(resulting_pixel.red) << 16) & 0xFF0000) | ((static_cast<std::uint32_t>(resulting_pixel.green) << 8) & 0xFF00) | (static_cast<std::uint32_t>(resulting_pixel.blue) & 0xFF);
                    }
                }
                break;
            case COLOR_RGB:
                for(std::size_t p = 0; p < pixel_count; p++) {
                    data[p] |= 0xFF000000;
                }
                break;
            case COLOR_ALPHA:
                for(std::size_t p = 0; p < pixel_count; p++) {
                    set_to_channel(0, data[p]);
                    data[p] |= 0xFF000000;
                }
                break;
            case COLOR_RED:
                for(std::size_t p = 0; p < pixel_count; p++) {
                    set_to_channel(1, data[p]);
                    data[p] |= 0xFF000000;
                }
                break;
            case COLOR_GREEN:
                for(std::size_t p = 0; p < pixel_count; p++) {
                    set_to_channel(2, data[p]);
                    data[p] |= 0xFF000000;
                }
                break;
            case COLOR_BLUE:
                for(std::size_t p = 0; p < pixel_count; p++) {
                    set_to_channel(3, data[p]);
                    data[p] |= 0xFF000000;
                }
                break;
        }

        // Show sprite if selected
        if(this->sequence) {
            int current_sequence_index = this->sequence->currentIndex();
            int current_sprite_index = this->sprite->currentIndex();
            if(current_sequence_index > 0 && current_sprite_index >= 0) {
                std::size_t current_sequence_index_unsigned = static_cast<std::size_t>(current_sequence_index - 1);
                std::size_t current_sprite_index_unsigned = static_cast<std::size_t>(current_sprite_index);
                auto &sprite = (*this->all_sequences)[current_sequence_index_unsigned].sprites[current_sprite_index_unsigned];

                if(static_cast<int>(sprite.bitmap_index) == this->bitmaps->currentIndex()) {
                    int left = sprite.left * real_width;
                    int right = sprite.right * real_width - 1;
                    int width = right - left;

                    int top = sprite.top * real_height;
                    int bottom = sprite.bottom * real_height - 1;
                    int height = bottom - top;

                    int middle_x = sprite.registration_point.x * real_width + left;
                    int middle_y = sprite.registration_point.y * real_height + top;

                    #define CALCULATE_PIXEL(x,y) (y * real_width + x)

                    if(width > 0 && height > 0) {
                        if(left >= 0) {
                            // Draw the top and bottom borders
                            for(int i = left; i < right && i < static_cast<long>(real_width); i++) {
                                if(i < 0) {
                                    i = -1;
                                }
                                else {
                                    if(top >= 0 && static_cast<std::size_t>(top) < real_height) {
                                        data[CALCULATE_PIXEL(i, top)] |= 0x0000FF00;
                                    }
                                    if(bottom >= 0 && static_cast<std::size_t>(bottom) < real_height) {
                                        data[CALCULATE_PIXEL(i, bottom)] |= 0x0000FF00;
                                    }
                                }
                            }

                            // Draw the left and right borders
                            for(int i = top; i < bottom && i < static_cast<long>(real_height); i++) {
                                if(i < 0) {
                                    i = -1;
                                }
                                else {
                                    if(left >= 0 && static_cast<std::size_t>(left) < real_width) {
                                        data[CALCULATE_PIXEL(left, i)] |= 0x0000FF00;
                                    }
                                    if(right >= 0 && static_cast<std::size_t>(right) < real_width) {
                                        data[CALCULATE_PIXEL(right, i)] |= 0x0000FF00;
                                    }
                                }
                            }
                        }

                        if(top < 0) {
                            top = 0;
                        }
                        if(left < 0) {
                            left = 0;
                        }
                        if(right < 0) {
                            right = 0;
                        }
                        if(bottom < 0) {
                            bottom = 0;
                        }

                        if(top > static_cast<long>(real_height)) {
                            top = static_cast<int>(real_height);
                        }
                        if(bottom > static_cast<long>(real_height)) {
                            bottom = static_cast<int>(real_height);
                        }
                        if(left > static_cast<long>(real_width)) {
                            left = static_cast<int>(real_width);
                        }
                        if(right > static_cast<long>(real_width)) {
                            right = static_cast<int>(real_width);
                        }

                        static constexpr const ColorPlatePixel red = { 0, 0, 0xFF, 0x1F };
                        for(int y = top; y < bottom; y++) {
                            for(int x = left; x < right; x++) {
                                auto &pixel_output = data[CALCULATE_PIXEL(x,y)];

                                ColorPlatePixel pixel = { static_cast<std::uint8_t>(pixel_output), static_cast<std::uint8_t>(pixel_output >> 8), static_cast<std::uint8_t>(pixel_output >> 16), static_cast<std::uint8_t>(pixel_output >> 24) };
                                auto resulting_pixel = pixel.alpha_blend(red);

                                pixel_output = ((static_cast<std::uint32_t>(resulting_pixel.alpha) << 24) & 0xFF000000) | ((static_cast<std::uint32_t>(resulting_pixel.red) << 16) & 0xFF0000) | ((static_cast<std::uint32_t>(resulting_pixel.green) << 8) & 0xFF00) | (static_cast<std::uint32_t>(resulting_pixel.blue) & 0xFF);
                            }
                        }
                    }

                    for(int x = middle_x - 1; x <= middle_x + 1; x++) {
                        for(int y = middle_y - 1; y <= middle_y + 1; y++) {
                            if(x >= 0 && x < static_cast<long>(real_width) && y >= 0 && y < static_cast<long>(real_height)) {
                                data[CALCULATE_PIXEL(x,y)] = 0xFF00FFFF;
                            }
                        }
                    }

                    #undef CALCULATE_PIXEL
                }
            }
        }

        // Finish up
        QGraphicsView *view = new QGraphicsView();
        QGraphicsScene *scene = new QGraphicsScene(view);
        QPixmap map;
        map.convertFromImage(QImage(reinterpret_cast<const uchar *>(data.data()), static_cast<int>(real_width), static_cast<int>(real_height), QImage::Format_ARGB32));
        scene->addPixmap(map);
        view->setScene(scene);

        view->setFrameStyle(0);
        view->setMinimumSize(real_width,real_height);
        view->setMaximumSize(real_width,real_height);
        view->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
        view->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
        view->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);

        return view;
    }

    void TagEditorBitmapSubwindow::reload_view() {
        // Get the index if valid
        int index = this->bitmaps->currentIndex();
        int mip_index = this->mipmaps->currentIndex();
        if(index < 0 || mip_index < 0) {
            return;
        }
        std::size_t index_unsigned = static_cast<std::size_t>(index);
        std::size_t mip_index_unsigned = static_cast<std::size_t>(mip_index);

        // Get the data
        Parser::BitmapData *bitmap_data;
        std::vector<std::byte> *pixel_data;
        auto *parent_window = this->get_parent_window();
        switch(parent_window->get_file().tag_class_int) {
            case TagClassInt::TAG_CLASS_BITMAP:
                bitmap_data = dynamic_cast<Parser::Bitmap *>(parent_window->get_parser_data())->bitmap_data.data() + index_unsigned;
                pixel_data = &dynamic_cast<Parser::Bitmap *>(parent_window->get_parser_data())->processed_pixel_data;
                break;
            case TagClassInt::TAG_CLASS_EXTENDED_BITMAP:
                bitmap_data = dynamic_cast<Parser::ExtendedBitmap *>(parent_window->get_parser_data())->bitmap_data.data() + index_unsigned;
                pixel_data = &dynamic_cast<Parser::ExtendedBitmap *>(parent_window->get_parser_data())->processed_pixel_data;
                break;
            default:
                std::terminate();
        }

        auto *scroll_widget = new QWidget();
        auto *layout = new QVBoxLayout();
        auto color = static_cast<Colors>(this->colors->currentIndex());
        int scale = 1 << (this->scale->currentIndex());
        auto *what = this;

        auto make_widget = [&bitmap_data, &color, &scale, &pixel_data, &what](std::size_t mip, std::size_t index) {
            return what->draw_bitmap_to_widget(bitmap_data, mip, index, color, scale, pixel_data);
        };

        auto make_row = [&make_widget, &bitmap_data, &layout](std::size_t mip) {
            std::size_t elements;
            switch(bitmap_data->type) {
                case HEK::BitmapDataType::BITMAP_DATA_TYPE_3D_TEXTURE:
                    elements = bitmap_data->depth >> mip;
                    if(elements == 0) {
                        elements = 1;
                    }
                    break;
                case HEK::BitmapDataType::BITMAP_DATA_TYPE_CUBE_MAP:
                    elements = 6;
                    break;
                default:
                    elements = 1;
            }

            QWidget *row = new QWidget();
            auto *row_layout = new QHBoxLayout();
            row->setLayout(row_layout);
            for(std::size_t e = 0; e < elements; e++) {
                auto *widget = make_widget(mip, e);
                if(!widget) {
                    break;
                }
                row_layout->addWidget(widget);
            }
            row_layout->addStretch();
            row_layout->setMargin(4);
            row_layout->setSpacing(4);
            layout->addWidget(row);
        };

        // Draw the mips
        if(mip_index_unsigned == 0) {
            for(std::size_t i = 0; i <= bitmap_data->mipmap_count; i++) {
                make_row(i);
            }
        }
        else {
            make_row(mip_index_unsigned - 1);
        }

        layout->addStretch();
        layout->setMargin(0);
        layout->setSpacing(0);
        scroll_widget->setLayout(layout);

        // Replace it!
        auto *old_widget = this->images->takeWidget();
        if(old_widget) {
            old_widget->deleteLater();
        }
        this->images->setWidget(scroll_widget);
    }

    void TagEditorBitmapSubwindow::refresh_sprite_list() {
        // Clear everything. Yay
        this->sprite->blockSignals(true);
        this->sprite->clear();
        this->sprite->blockSignals(false);

        // Make sure we have a sprite selected
        int current_sequence_index = this->sequence->currentIndex();
        if(current_sequence_index > 0) {
            std::size_t current_sequence_index_unsigned = static_cast<std::size_t>(current_sequence_index - 1);
            this->sprite->blockSignals(true);
            this->sprite->clear();
            auto sprite_count = (*this->all_sequences)[current_sequence_index_unsigned].sprites.size();
            for(std::size_t i = 0; i < sprite_count; i++) {
                this->sprite->addItem(QString::number(i));
            }
            this->sprite->setCurrentIndex(0);
            this->sprite->blockSignals(false);
            this->sprite->setEnabled(true);
        }
        else {
            this->sprite->setEnabled(false);
        }

        // Done
        this->select_sprite();
    }

    void TagEditorBitmapSubwindow::select_sprite() {
        int current_sequence_index = this->sequence->currentIndex();
        int current_sprite_index = this->sprite->currentIndex();
        if(current_sequence_index > 0 && current_sprite_index >= 0) {
            std::size_t current_sequence_index_unsigned = static_cast<std::size_t>(current_sequence_index - 1);
            std::size_t current_sprite_index_unsigned = static_cast<std::size_t>(current_sprite_index);
            auto &sprite = (*this->all_sequences)[current_sequence_index_unsigned].sprites[current_sprite_index_unsigned];
            if(sprite.bitmap_index < this->bitmaps->count()) {
                this->bitmaps->blockSignals(true);
                this->bitmaps->setCurrentIndex(sprite.bitmap_index);
                this->bitmaps->blockSignals(false);
            }
        }

        this->reload_view();
    }
}
