#include "bludgeoner.hpp"
#include <invader/tag/parser/parser.hpp>
#include <invader/tag/parser/compile/bitmap.hpp>
#include <invader/tag/parser/compile/gbxmodel.hpp>
#include <invader/tag/parser/compile/scenario.hpp>
#include <invader/tag/parser/compile/scenario_structure_bsp.hpp>
#include <invader/sound/sound_reader.hpp>

namespace Invader::Bludgeoner {
    bool bullshit_enums(Parser::ParserStruct *s, bool fix) {
        return s->check_for_broken_enums(fix);
    }
    
    bool fucked_normals(Parser::ParserStruct *s, bool fix) {
        return s->check_for_nonnormal_vectors(fix);
    }
    
    bool where_the_fuck_are_the_scripts(Parser::ParserStruct *s, bool fix) {
        auto *scenario = dynamic_cast<Parser::Scenario *>(s);
        if(scenario) {
            return fix_missing_script_source_data(*scenario, fix);
        }
        return false;
    }

    static bool fucked_bsp_vertices(Parser::ScenarioStructureBSP *bsp, bool fix) {
        if(bsp) {
            return regenerate_missing_bsp_vertices(*bsp, fix);
        }
        return false;
    }

    static bool fucked_model_vertices(Parser::GBXModel *model, bool fix) {
        if(model) {
            return regenerate_missing_model_vertices(*model, fix);
        }
        return false;
    }

    bool fucked_vertices(Parser::ParserStruct *s, bool fix) {
        return fucked_bsp_vertices(dynamic_cast<Parser::ScenarioStructureBSP *>(s), fix) || fucked_model_vertices(dynamic_cast<Parser::GBXModel *>(s), fix);
    }

    bool refinery_model_markers(Parser::ParserStruct *s, bool fix) {
        auto attempt_fix = [&fix](auto *model) -> bool {
            if(!model) {
                return false;
            }
            return Parser::uncache_model_markers(*model, fix);
        };
        return attempt_fix(dynamic_cast<Invader::Parser::GBXModel *>(s));
    }
    
    #ifndef DISABLE_AUDIO
    
    static bool fucked_sound_buffer(Parser::SoundPermutation &pe, bool fix) {
        using SoundFormat = Invader::HEK::SoundFormat;
        auto &samples = pe.samples;
        auto &buffer_size = pe.buffer_size;
        
        bool fucked = false;

        // Fix depending on how we need to do it
        switch(pe.format) {
            // This one's simple
            case SoundFormat::SOUND_FORMAT_16_BIT_PCM: {
                if(buffer_size != samples.size()) {
                    fucked = true;
                    if(fix) {
                        buffer_size = samples.size();
                    }
                    else {
                        return fucked;
                    }
                }
                break;
            }

            // We need to decode this first
            case SoundFormat::SOUND_FORMAT_OGG_VORBIS: {
                try {
                    auto decoded = Invader::SoundReader::sound_from_ogg(samples.data(), samples.size());
                    auto decoded_size_16_bit = decoded.pcm.size() / (decoded.bits_per_sample / 8) * 2;
                    if(decoded_size_16_bit != buffer_size) {
                        fucked = true;
                        if(fix) {
                            buffer_size = decoded_size_16_bit;
                        }
                        else {
                            return fucked;
                        }
                    }
                }
                catch(std::exception &e) {
                    eprintf_error("Failed to decode: %s", e.what());
                    throw;
                }
                break;
            }

            default:
                break;
        }
        
        return fucked;
    }
    
    #else
    
    static bool fucked_sound_buffer(Parser::SoundPermutation &, bool) {
        return false;
    }
    
    #endif

    bool sound_buffer(Invader::Parser::ParserStruct *s, bool fix) {
        auto attempt_fix = [&fix](auto *s) -> bool {
            if(s == nullptr) {
                return false;
            }
            bool fucked = false;
            for(Invader::Parser::SoundPitchRange &pr : s->pitch_ranges) {
                for(auto &pe : pr.permutations) {
                    fucked = fucked_sound_buffer(pe, fix) || fucked;
                    if(!fix && fucked) {
                        return fucked;   
                    }
                }
            }
            return fucked;
        };

        return attempt_fix(dynamic_cast<Invader::Parser::Sound *>(s)) || attempt_fix(dynamic_cast<Invader::Parser::ExtendedSound *>(s));
    }
    
    bool bullshit_references(Parser::ParserStruct *s, bool fix) {
        return s->check_for_invalid_references(fix);
    }
    
    bool power_of_two_fix(Parser::ParserStruct *s, bool fix) {
        auto attempt_fix = [&fix](auto *s) {
            if(s) {
                return fix_power_of_two(*s, fix);
            }
            else {
                return false;
            }
        };
        return attempt_fix(dynamic_cast<Parser::Bitmap *>(s)) || attempt_fix(dynamic_cast<Parser::ExtendedBitmap *>(s));
    }
    
    bool bullshit_range_fix(Parser::ParserStruct *s, bool fix) {
        return s->check_for_invalid_ranges(fix);
    }
    
    bool fucked_indices_fix(Parser::ParserStruct *s, bool fix) {
        return s->check_for_invalid_indices(fix);
    }
}
