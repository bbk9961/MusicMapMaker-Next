#pragma once

#include "log/colorful-log.h"
#include "mmm/beatmap/BeatMap.h"
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <set>

namespace MMM
{

using json = nlohmann::json;

/**
 * @brief 保存谱面为 mmm 格式 (JSON)
 * @param beatMap 谱面对象
 * @param path 保存路径
 * @return 是否保存成功
 */
inline bool saveMMMMap(const BeatMap& beatMap, const std::filesystem::path& path)
{
    json root;

    // 1. Metadata
    auto& metadata = root["metadata"];

    // Base metadata
    auto& base            = metadata["base"];
    base["name"]          = beatMap.m_baseMapMetadata.name;
    base["title"]         = beatMap.m_baseMapMetadata.title;
    base["title_unicode"] = beatMap.m_baseMapMetadata.title_unicode;
    base["artist"]         = beatMap.m_baseMapMetadata.artist;
    base["artist_unicode"] = beatMap.m_baseMapMetadata.artist_unicode;
    base["version"]        = beatMap.m_baseMapMetadata.version;
    base["author"]         = beatMap.m_baseMapMetadata.author;
    base["audio"]          = beatMap.m_baseMapMetadata.main_audio_path.string();
    base["cover"]          = beatMap.m_baseMapMetadata.main_cover_path.string();
    base["track_count"]    = beatMap.m_baseMapMetadata.track_count;
    base["bpm"]            = beatMap.m_baseMapMetadata.preference_bpm;
    base["duration"]       = beatMap.m_baseMapMetadata.map_length;

    // Extra metadata
    auto& extra = metadata["extra"];
    extra       = json::array();
    for ( const auto& [type, props] : beatMap.m_metadata.map_properties ) {
        json        sourceObj;
        std::string sourceName;
        if ( type == MapMetadataType::OSU )
            sourceName = "osu";
        else if ( type == MapMetadataType::MALODY )
            sourceName = "malody";
        else if ( type == MapMetadataType::RM )
            sourceName = "rm";

        if ( sourceName.empty() ) continue;

        json propObj;
        for ( const auto& [key, val] : props ) {
            propObj[key] = val;
        }
        sourceObj[sourceName] = propObj;
        extra.push_back(sourceObj);
    }

    // 2. Timing
    auto& timingArr = root["timing"];
    timingArr       = json::array();
    for ( const auto& timing : beatMap.m_timings ) {
        json t;
        t["timestamp"]   = timing.m_timestamp;
        t["bpm"]         = timing.m_bpm;
        t["beat_length"] = timing.m_beat_length;
        t["effect"] = timing.m_timingEffect == TimingEffect::BPM ? "bpm" : "scroll";
        t["param"]  = timing.m_timingEffectParameter;

        auto& tExtra = t["extra"];
        tExtra       = json::array();
        for ( const auto& [type, props] : timing.m_metadata.timing_properties ) {
            json        sourceObj;
            std::string sourceName;
            if ( type == TimingMetadataType::OSU )
                sourceName = "osu";
            else if ( type == TimingMetadataType::MALODY )
                sourceName = "malody";

            if ( sourceName.empty() ) continue;

            json propObj;
            for ( const auto& [key, val] : props ) {
                propObj[key] = val;
            }
            sourceObj[sourceName] = propObj;
            tExtra.push_back(sourceObj);
        }
        timingArr.push_back(t);
    }

    // 3. Note
    auto& noteArr = root["note"];
    noteArr       = json::array();

    // Helper to serialize a note
    auto serializeNote = [&](const Note& note) {
        json n;
        n["timestamp"] = note.m_timestamp;
        n["track"]     = note.m_track;

        switch ( note.m_type ) {
        case NoteType::NOTE: n["type"] = "note"; break;
        case NoteType::HOLD: {
            n["type"]     = "hold";
            n["duration"] = static_cast<const Hold&>(note).m_duration;
            break;
        }
        case NoteType::FLICK: {
            n["type"]   = "flick";
            n["dtrack"] = static_cast<const Flick&>(note).m_dtrack;
            break;
        }
        case NoteType::POLYLINE: {
            n["type"]          = "polyline";
            auto& poly         = static_cast<const Polyline&>(note);
            json  subNotesJson = json::array();
            for ( const auto& subNoteRef : poly.m_subNotes ) {
                const Note& subNote = subNoteRef.get();
                json        sn;
                sn["timestamp"] = subNote.m_timestamp;
                sn["track"]     = subNote.m_track;
                if ( subNote.m_type == NoteType::HOLD ) {
                    sn["type"]     = "hold";
                    sn["duration"] = static_cast<const Hold&>(subNote).m_duration;
                } else if ( subNote.m_type == NoteType::FLICK ) {
                    sn["type"]   = "flick";
                    sn["dtrack"] = static_cast<const Flick&>(subNote).m_dtrack;
                }
                subNotesJson.push_back(sn);
            }
            n["sub_notes"] = subNotesJson;
            break;
        }
        }

        auto& nExtra = n["extra"];
        nExtra       = json::array();
        for ( const auto& [type, props] : note.m_metadata.note_properties ) {
            json        sourceObj;
            std::string sourceName;
            if ( type == NoteMetadataType::OSU )
                sourceName = "osu";
            else if ( type == NoteMetadataType::MALODY )
                sourceName = "malody";

            if ( sourceName.empty() ) continue;

            json propObj;
            for ( const auto& [key, val] : props ) {
                propObj[key] = val;
            }
            sourceObj[sourceName] = propObj;
            nExtra.push_back(sourceObj);
        }
        return n;
    };

    // We only want to save top-level notes.
    // Sub-notes of Polylines are already serialized inside the Polyline.
    std::set<const Note*> subNotesSet;
    for ( const auto& poly : beatMap.m_noteData.polylines ) {
        for ( const auto& subNoteRef : poly.m_subNotes ) {
            subNotesSet.insert(&subNoteRef.get());
        }
    }

    std::vector<json> serializedNotes;
    for ( const auto& note : beatMap.m_noteData.notes ) {
        if ( subNotesSet.find(&note) == subNotesSet.end() )
            serializedNotes.push_back(serializeNote(note));
    }
    for ( const auto& hold : beatMap.m_noteData.holds ) {
        if ( subNotesSet.find(&hold) == subNotesSet.end() )
            serializedNotes.push_back(serializeNote(hold));
    }
    for ( const auto& flick : beatMap.m_noteData.flicks ) {
        if ( subNotesSet.find(&flick) == subNotesSet.end() )
            serializedNotes.push_back(serializeNote(flick));
    }
    for ( const auto& poly : beatMap.m_noteData.polylines ) {
        serializedNotes.push_back(serializeNote(poly));
    }

    // Sort by timestamp
    std::sort(serializedNotes.begin(), serializedNotes.end(),
              [](const json& a, const json& b) {
                  return a["timestamp"].get<double>() < b["timestamp"].get<double>();
              });

    for ( auto& n : serializedNotes ) {
        noteArr.push_back(n);
    }

    std::ofstream file(path);
    if ( !file.is_open() ) {
        XERROR("Failed to open file for saving mmm map: {}", path.string());
        return false;
    }

    file << root.dump(4);
    return true;
}

}  // namespace MMM
