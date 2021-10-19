// SPDX-License-Identifier: GPL-3.0-only

#include <QScrollArea>
#include <QHBoxLayout>
#include <QLabel>
#include <QImage>
#include <QComboBox>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QMessageBox>
#include "../tag_editor_window.hpp"
#include "tag_editor_bitmap_subwindow.hpp"
#include "tag_editor_subwindow.hpp"
#include <invader/bitmap/pixel.hpp>
#include <invader/tag/parser/parser.hpp>
#include <invader/bitmap/swizzle.hpp>
#include <invader/bitmap/bitmap_encode.hpp>
#include <invader/tag/hek/class/bitmap.hpp>

#define GET_PIXEL(x,y) (x + y * real_width)

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
        l->setContentsMargins(0, 0, 0, 0);
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

        // Done
        return w;
    }
    
    void TagEditorBitmapSubwindow::generate_colors_array(bool monochrome) {
        // Is this populated at all?
        if(this->colors->count() == 0) {
            this->monochrome = monochrome;
        }
        // Do we even need to generate a new array?
        else if(this->monochrome == monochrome) {
            return;
        }
        
        // Generate it!
        colors->blockSignals(true);
        colors->clear();
        if(monochrome) {
            colors->addItem("Alpha-Luminance");
            colors->addItem("Luminance only");
            colors->addItem("Alpha only");
        }
        else {
            colors->addItem("ARGB");
            colors->addItem("RGB only");
            colors->addItem("Alpha only");
            colors->addItem("Red only");
            colors->addItem("Green only");
            colors->addItem("Blue only");
        }
        colors->setCurrentIndex(0);
        colors->blockSignals(false);
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
        main_layout->setContentsMargins(4, 4, 4, 4);
        main_layout->addWidget(generate_text_widget("Bitmap:", &bitmaps));
        main_layout->addWidget(generate_text_widget("Mipmap:", &mipmaps));
        main_layout->addWidget(generate_text_widget("Channels:", &colors));
        HEK::BitmapType type = bitmap_data->type;
        main_layout->addWidget(generate_text_widget("Scale:", &scale));

        // Add this if we have sprites
        if(type == HEK::BitmapType::BITMAP_TYPE_SPRITES) {
            main_layout->addWidget(generate_text_widget("Sequence:", &sequence));
            main_layout->addWidget(generate_text_widget("Sprite:", &sprite));

            // Populate sequences
            sequence->addItem("None");
            sprite->setEnabled(false);
            auto sequence_count = bitmap_data->bitmap_group_sequence.size();
            for(std::size_t i = 0; i < sequence_count; i++) {
                char text[64];
                auto sprite_count = bitmap_data->bitmap_group_sequence[i].sprites.size();
                std::snprintf(text, sizeof(text), "%zu (%zu sprite%s)", i, sprite_count, sprite_count == 1 ? "" : "s");
                sequence->addItem(text);
            }
        }

        // Get the size
        auto bitmap_count = bitmap_data->bitmap_data.size();
        for(std::size_t i = 0; i < bitmap_count; i++) {
            bitmaps->addItem(QString::number(i), false);
        }
        if(!bitmap_data->compressed_color_plate_data.empty()) {
            bitmaps->addItem("Color plate", true);
        }
            
        // Scaling stuff
        for(int z = 3; z > 0; z--) {
            char t[8];
            std::snprintf(t, sizeof(t), "1/%ix", 1 << z);
            scale->addItem(t);
        }
        scale->addItem("Original");
        for(int z = 1; z < 4; z++) {
            char t[8];
            std::snprintf(t, sizeof(t), "%ix", 1 << z);
            scale->addItem(t);
        }
        scale->setCurrentIndex(3);

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
        switch(parent_window->get_file().tag_fourcc) {
            case TagFourCC::TAG_FOURCC_BITMAP:
                generate_main_widget(this, dynamic_cast<Parser::Bitmap *>(data), TagEditorBitmapSubwindow::set_values);
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
        
        // Are we viewing the color plate?
        if(this->bitmaps->currentData().toBool()) {
            // Add item
            char name[256];
            
            Parser::Bitmap *bitmap;
            auto *parent_window = this->get_parent_window();
            switch(parent_window->get_file().tag_fourcc) {
                case TagFourCC::TAG_FOURCC_BITMAP:
                    bitmap = dynamic_cast<Parser::Bitmap *>(parent_window->get_parser_data());
                    break;
                default:
                    std::terminate();
            }
            
            std::size_t width = 0, height = 0;
            if(bitmap->compressed_color_plate_data.size() != 0) {
                width = bitmap->color_plate_width;
                height = bitmap->color_plate_height;
            }
            
            std::snprintf(name, sizeof(name), "Color plate data (%zu x %zu)", width, height);
            this->mipmaps->addItem(name);
            this->sequence->setEnabled(false);
            this->sprite->setEnabled(false);
        }
        
        // Otherwise, let's go
        else if(index >= 0) {
            std::size_t index_unsigned = static_cast<std::size_t>(index);
            auto *parent_window = this->get_parent_window();
            Parser::BitmapData *bitmap_data;
            switch(parent_window->get_file().tag_fourcc) {
                case TagFourCC::TAG_FOURCC_BITMAP:
                    bitmap_data = &dynamic_cast<Parser::Bitmap *>(parent_window->get_parser_data())->bitmap_data[index_unsigned];
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
                    std::snprintf(name, sizeof(name), "Mipmap #%zu (%s)", i - 1, resolution);
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
            this->sequence->setEnabled(true);
            this->sprite->setEnabled(true);
        }

        this->mipmaps->blockSignals(false);
        this->mipmaps->setUpdatesEnabled(true);

        this->reload_view();
    }

    QGraphicsView *TagEditorBitmapSubwindow::draw_color_plate(Parser::Bitmap *bitmap_data, Colors colors, int scale) {
        QGraphicsView *view = new QGraphicsView();
        QGraphicsScene *scene = new QGraphicsScene(view);
        QPixmap map;
        
        std::vector<std::uint32_t> color_plate_data;
        int width = 0, height = 0;
        
        // Decompress
        try {
            auto c = HEK::decompress_color_plate_data(*bitmap_data);
            if(c.has_value()) {
                color_plate_data.reserve(c->size());
                for(auto &p : *c) {
                    color_plate_data.emplace_back(p.read());
                }
                width = bitmap_data->color_plate_width;
                height = bitmap_data->color_plate_height;
            }
        }
        catch (std::exception &e) {
            std::printf("Got %s when trying to decompress color plate data\n", e.what());
            color_plate_data.clear();
        }
        
        std::size_t real_width = width, real_height = height, pixel_count = color_plate_data.size();
        this->scale_bitmap(scale, real_width, real_height, pixel_count, color_plate_data);
        this->show_channel(color_plate_data.data(), real_width, real_height, colors);
        
        map.convertFromImage(QImage(reinterpret_cast<const uchar *>(color_plate_data.data()), real_width, real_height, QImage::Format_ARGB32));
        scene->addPixmap(map);
        view->setScene(scene);

        view->setFrameStyle(0);
        view->setMinimumSize(real_width, real_height);
        view->setMaximumSize(real_width, real_height);
        view->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
        view->setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
        view->setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);

        return view;
    }
    
    QGraphicsView *TagEditorBitmapSubwindow::draw_bitmap_to_widget(Parser::BitmapData *bitmap_data, std::size_t mipmap, std::size_t index, Colors mode, int scale, const std::vector<std::byte> *pixel_data) {
        // Get the dimensions of the mipmap
        std::size_t width = static_cast<std::size_t>(bitmap_data->width);
        std::size_t height = static_cast<std::size_t>(bitmap_data->height);
        std::size_t depth = static_cast<std::size_t>(bitmap_data->depth);
        std::size_t real_width = width;
        std::size_t real_height = height;
        std::size_t offset = bitmap_data->pixel_data_offset;
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

        // Decode bitmap
        const auto *bytes = pixel_data->data() + offset;
        std::vector<std::uint32_t> data(real_width * real_height);
        std::fill(data.begin(), data.end(), 0xFFFF00FF);
        BitmapEncode::encode_bitmap(bytes, bitmap_data->format, reinterpret_cast<std::byte *>(data.data()), HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_A8R8G8B8, real_width, real_height);

        // Scale if needed
        if(scale != 0) {
            this->scale_bitmap(scale, real_width, real_height, pixel_count, data);
        }

        // Filter for mode
        this->show_channel(data.data(), real_width, real_height, mode);

        // Show sprite if selected
        if(this->sequence) {
            this->highlight_sprite(data.data(), real_width, real_height);
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

    void TagEditorBitmapSubwindow::show_channel(std::uint32_t *data, std::size_t real_width, std::size_t real_height, Colors mode) {
        // Get the number of pixels we'll be dealing with
        std::size_t pixel_count = static_cast<std::size_t>(real_width) * real_height;

        // Only show one channel
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
                        Pixel checkerboard = { luminosity, luminosity, luminosity, 0xFF };
                        auto &pixel_output = data[x + y * real_width];
                        pixel_output = checkerboard.alpha_blend(Pixel::convert_from_32_bit(pixel_output)).convert_to_32_bit();
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
    }

    void TagEditorBitmapSubwindow::highlight_sprite(std::uint32_t *data, std::size_t real_width, std::size_t real_height) {
        if(this->bitmaps->currentData().toBool()) {
            return; // color plate data
        }
        
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

                if(width > 0 && height > 0) {
                    if(left >= 0) {
                        // Draw the top and bottom borders
                        for(int i = left; i < right && i < static_cast<long>(real_width); i++) {
                            if(i < 0) {
                                i = -1;
                            }
                            else {
                                if(top >= 0 && static_cast<std::size_t>(top) < real_height) {
                                    data[GET_PIXEL(i, top)] |= 0x0000FF00;
                                }
                                if(bottom >= 0 && static_cast<std::size_t>(bottom) < real_height) {
                                    data[GET_PIXEL(i, bottom)] |= 0x0000FF00;
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
                                    data[GET_PIXEL(left, i)] |= 0x0000FF00;
                                }
                                if(right >= 0 && static_cast<std::size_t>(right) < real_width) {
                                    data[GET_PIXEL(right, i)] |= 0x0000FF00;
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

                    // Blend the sprite with a transparent shade of red
                    static constexpr const Pixel red = { 0, 0, 0xFF, 0x1F };
                    for(int y = top; y < bottom; y++) {
                        for(int x = left; x < right; x++) {
                            auto &pixel_output = data[GET_PIXEL(x,y)];
                            pixel_output = Pixel::convert_from_32_bit(pixel_output).alpha_blend(red).convert_to_32_bit();
                        }
                    }
                }

                // Show a blue crosshair in the center
                for(int x = middle_x - 2; x <= middle_x + 2; x++) {
                    for(int y = middle_y - 2; y <= middle_y + 2; y++) {
                        if(x != middle_x && y != middle_y) {
                            continue;
                        }
                        if(x >= 0 && x < static_cast<long>(real_width) && y >= 0 && y < static_cast<long>(real_height)) {
                            data[GET_PIXEL(x,y)] = 0xFF00FFFF;
                        }
                    }
                }
            }
        }
    }

    void TagEditorBitmapSubwindow::scale_bitmap(int scale, std::size_t &real_width, std::size_t &real_height, std::size_t &pixel_count, std::vector<std::uint32_t> &data) {
        std::size_t new_height;
        std::size_t new_width;

        if(scale > 0) {
            new_height = real_height << scale;
            new_width = real_width << scale;
        }
        else {
            new_height = real_height >> -scale;
            new_width = real_width >> -scale;
        }

        std::vector<std::uint32_t> scaled(new_height * new_width);

        if(new_width * new_height > 0) {
            if(scale > 0) {
                std::size_t scale_s = 1 << scale;
                for(std::size_t y = 0; y < real_height; y++) {
                    for(std::size_t x = 0; x < real_width; x++) {
                        auto pixel = data[GET_PIXEL(x,y)];
                        auto base_x = x * scale_s;
                        auto base_y = y * scale_s;
                        for(std::size_t ys = 0; ys < scale_s; ys++) {
                            for(std::size_t xs = 0; xs < scale_s; xs++) {
                                scaled[xs + base_x + (ys + base_y) * new_width] = pixel;
                            }
                        }
                    }
                }
            }

            // Downscale it (linear filter)
            else {
                std::size_t scale_s = 1 << -scale;
                for(std::size_t ys = 0; ys < new_height; ys++) {
                    for(std::size_t xs = 0; xs < new_width; xs++) {
                        std::uint32_t total_red = 0;
                        std::uint32_t total_green = 0;
                        std::uint32_t total_blue = 0;
                        std::uint32_t total_alpha = 0;
                        std::size_t total_pixel = 0;

                        // Cap it to real_width / real_height in case we have a non Po2 bitmap
                        std::size_t y_start = ys * scale_s;
                        std::size_t y_end = (ys + 1) * scale_s;
                        if(y_end > real_height) {
                            y_end = real_height;
                        }
                        std::size_t x_start = xs * scale_s;
                        std::size_t x_end = (xs + 1) * scale_s;
                        if(x_end > real_width) {
                            x_end = real_width;
                        }

                        // Add all pixels together
                        for(std::size_t y = y_start; y < y_end; y++) {
                            for(std::size_t x = x_start; x < x_end; x++) {
                                total_pixel++;
                                auto pixel = data[GET_PIXEL(x,y)];
                                total_alpha += (pixel & 0xFF000000) >> 24;
                                total_red += (pixel & 0xFF0000) >> 16;
                                total_green += (pixel & 0xFF00) >> 8;
                                total_blue += (pixel & 0xFF);
                            }
                        }

                        // Average
                        if(total_pixel) {
                            std::uint8_t average_red = total_red / total_pixel;
                            std::uint8_t average_green = total_green / total_pixel;
                            std::uint8_t average_blue = total_blue / total_pixel;
                            std::uint8_t average_alpha = total_alpha / total_pixel;

                            scaled[xs + ys * new_width] = (Pixel { average_blue, average_green, average_red, average_alpha }).convert_to_32_bit();
                        }
                    }
                }
            }
        }

        real_width = new_width;
        real_height = new_height;
        if(scale > 0) {
            pixel_count <<= scale * 2;
        }
        else {
            pixel_count >>= scale * 2;
        }

        data = std::move(scaled);
    }

    void TagEditorBitmapSubwindow::reload_view() {
        // Get the index if valid
        int index = this->bitmaps->currentIndex();
        int mip_index = this->mipmaps->currentIndex();
        bool color_plate = this->bitmaps->currentData().toBool();
        
        if(mip_index < 0) {
            return;
        }
        std::size_t index_unsigned = static_cast<std::size_t>(index);
        std::size_t mip_index_unsigned = static_cast<std::size_t>(mip_index);

        // Get the data
        Parser::Bitmap *bitmap = dynamic_cast<Parser::Bitmap *>(this->get_parent_window()->get_parser_data());
        Parser::BitmapData *bitmap_data = nullptr;
        std::vector<std::byte> *pixel_data = nullptr;
        auto *parent_window = this->get_parent_window();
        
        if(color_plate) {
            this->generate_colors_array(false);
        }
        else {
            switch(parent_window->get_file().tag_fourcc) {
                case TagFourCC::TAG_FOURCC_BITMAP:
                    bitmap_data = bitmap->bitmap_data.data() + index_unsigned;
                    pixel_data = &bitmap->processed_pixel_data;
                    break;
                default:
                    std::terminate();
            }
            // Set the monochrome stuff depending on what we're doing
            switch(bitmap_data->format) {
                case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_A8:
                case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_Y8:
                case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_AY8:
                case HEK::BitmapDataFormat::BITMAP_DATA_FORMAT_A8Y8:
                    this->generate_colors_array(true);
                    break;
                default:
                    this->generate_colors_array(false);
                    break;
            }
        }

        auto *scroll_widget = new QWidget();
        auto *layout = new QVBoxLayout();
        auto color = static_cast<Colors>(this->colors->currentIndex());
        int scale = this->scale->currentIndex() - 3;
        auto *what = this;

        auto make_widget = [&bitmap, &bitmap_data, &color, &scale, &pixel_data, &what, &color_plate](std::size_t mip, std::size_t index) {
            if(color_plate) {
                return what->draw_color_plate(bitmap, color, scale);
            }
            else {
                return what->draw_bitmap_to_widget(bitmap_data, mip, index, color, scale, pixel_data);
            }
        };

        auto make_row = [&make_widget, &bitmap_data, &layout, &color_plate](std::size_t mip) {
            std::size_t elements;
            
            // Only one color plate
            if(color_plate) {
                elements = 1;
            }
            
            // For each mipmap?
            else {
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
            row_layout->setContentsMargins(4, 4, 4, 4);
            row_layout->setSpacing(4);
            layout->addWidget(row);
        };

        // Draw the mips
        if(mip_index_unsigned == 0 && !color_plate) {
            for(std::size_t i = 0; i <= bitmap_data->mipmap_count; i++) {
                make_row(i);
            }
        }
        else {
            make_row(mip_index_unsigned - 1);
        }

        layout->addStretch();
        layout->setContentsMargins(0, 0, 0, 0);
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
            auto &sequence = (*this->all_sequences)[current_sequence_index_unsigned];
            auto sprite_count = sequence.sprites.size();
            
            for(std::size_t i = 0; i < sprite_count; i++) {
                char sprite_info[64];
                
                // Determine height and width of sprite (requires getting its bitmap and multiplying that with memes)
                std::size_t height = 0;
                std::size_t width = 0;
                auto &sprite = sequence.sprites[i];
                std::vector<Parser::BitmapData> *bitmap_data;
                auto *parent_window = this->get_parent_window();
                switch(parent_window->get_file().tag_fourcc) {
                    case TagFourCC::TAG_FOURCC_BITMAP:
                        bitmap_data = &dynamic_cast<Parser::Bitmap *>(parent_window->get_parser_data())->bitmap_data;
                        break;
                    default:
                        std::terminate();
                }
                
                // If we have it, we have it
                if(sprite.bitmap_index < bitmap_data->size()) {
                    auto &bitmap = (*bitmap_data)[sprite.bitmap_index];
                    height = (sprite.bottom - sprite.top) * bitmap.height;
                    width = (sprite.right - sprite.left) * bitmap.width;
                    std::snprintf(sprite_info, sizeof(sprite_info), "%zu (%zu x %zu)", i, width, height);
                }
                
                // Otherwise oh well
                else {
                    std::snprintf(sprite_info, sizeof(sprite_info), "%zu (unknown size)", i);
                }
                
                
                this->sprite->addItem(sprite_info);
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
