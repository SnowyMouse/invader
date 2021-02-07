#include "bludgeoner.hpp"
#include <invader/tag/parser/parser.hpp>
#include <invader/tag/parser/compile/model.hpp>
#include <invader/tag/parser/compile/scenario.hpp>
#include <invader/tag/parser/compile/scenario_structure_bsp.hpp>
#include <invader/tag/parser/compile/string_list.hpp>
#include <invader/sound/sound_reader.hpp>

namespace Invader::Bludgeoner {
    bool broken_enums(Parser::ParserStruct *s, bool fix) {
        return s->check_for_broken_enums(fix);
    }
    
    bool broken_normals(Parser::ParserStruct *s, bool fix) {
        return s->check_for_nonnormal_vectors(fix);
    }
    
    bool missing_scripts(Parser::ParserStruct *s, bool fix) {
        auto *scenario = dynamic_cast<Parser::Scenario *>(s);
        if(scenario) {
            return fix_missing_script_source_data(*scenario, fix);
        }
        return false;
    }

    static bool broken_bsp_vertices(Parser::ScenarioStructureBSP *bsp, bool fix) {
        if(bsp) {
            return regenerate_missing_bsp_vertices(*bsp, fix);
        }
        return false;
    }

    static bool broken_model_vertices(Parser::GBXModel *model, bool fix) {
        if(model) {
            return regenerate_missing_model_vertices(*model, fix);
        }
        return false;
    }

    static bool broken_model_vertices(Parser::Model *model, bool fix) {
        if(model) {
            return regenerate_missing_model_vertices(*model, fix);
        }
        return false;
    }

    bool broken_vertices(Parser::ParserStruct *s, bool fix) {
        return broken_bsp_vertices(dynamic_cast<Parser::ScenarioStructureBSP *>(s), fix) || broken_model_vertices(dynamic_cast<Parser::GBXModel *>(s), fix) || broken_model_vertices(dynamic_cast<Parser::Model *>(s), fix);
    }

    bool invalid_model_markers(Parser::ParserStruct *s, bool fix) {
        auto attempt_fix = [&fix](auto *model) -> bool {
            if(!model) {
                return false;
            }
            return Parser::uncache_model_markers(*model, fix);
        };
        return attempt_fix(dynamic_cast<Invader::Parser::GBXModel *>(s)) || attempt_fix(dynamic_cast<Invader::Parser::Model *>(s));
    }
    
    #ifndef DISABLE_AUDIO
    
    static bool broken_sound_buffer(Parser::SoundPermutation &pe, bool fix) {
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
    
    static bool broken_sound_buffer(Parser::SoundPermutation &, bool) {
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
                    fucked = broken_sound_buffer(pe, fix) || fucked;
                    if(!fix && fucked) {
                        return fucked;   
                    }
                }
            }
            return fucked;
        };

        return attempt_fix(dynamic_cast<Invader::Parser::Sound *>(s)) || attempt_fix(dynamic_cast<Invader::Parser::InvaderSound *>(s));
    }
    
    bool broken_references(Parser::ParserStruct *s, bool fix) {
        return s->check_for_invalid_references(fix);
    }
    
    bool broken_range_fix(Parser::ParserStruct *s, bool fix) {
        return s->check_for_invalid_ranges(fix);
    }
    
    bool broken_indices_fix(Parser::ParserStruct *s, bool fix) {
        return s->check_for_invalid_indices(fix);
    }
    
    template <typename T> static bool broken_strings(T *s, bool fix) {
        if(s) {
            return fix_broken_strings(*s, fix);
        }
        return false;
    }
    
    bool broken_strings(Parser::ParserStruct *s, bool fix) {
        return broken_strings(dynamic_cast<Parser::StringList *>(s), fix) || broken_strings(dynamic_cast<Parser::UnicodeStringList *>(s), fix);
    }
    
    bool uppercase_references(Parser::ParserStruct *s, bool fix) {
        bool rval = false;
        
        // Go through all the values. Fix the stuff.
        for(auto &i : s->get_values()) {
            switch(i.get_type()) {
                case Parser::ParserStructValue::ValueType::VALUE_TYPE_REFLEXIVE: {
                    auto count = i.get_array_size();
                    for(std::size_t j = 0; j < count; j++) {
                        if((rval = rval || uppercase_references(&i.get_object_in_array(j), fix)) && !fix) {
                            return rval;
                        }
                    }
                    break;
                }
                case Parser::ParserStructValue::ValueType::VALUE_TYPE_DEPENDENCY: {
                    auto value = i.get_dependency();
                    for(auto &i : value.path) {
                        if(std::tolower(i) != i) {
                            i = std::tolower(i);
                            rval = true;
                        }
                    }
                    if(fix) {
                        i.get_dependency() = value;
                    }
                    else if(rval) {
                        return true;
                    }
                    break;
                }
                default:
                    break;
            }
        }
        
        return rval;
    }
    
    bool excessive_script_nodes(Parser::ParserStruct *s, bool fix) {
        auto attempt_fix = [&fix](auto *s) -> bool {
            if(s) {
                return fix_excessive_script_nodes(*s, fix);
            }
            return false;
        };
        return attempt_fix(dynamic_cast<Invader::Parser::Scenario *>(s));
    }
}
