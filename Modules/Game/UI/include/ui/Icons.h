#pragma once

/**
 * @file Icons.h
 * @brief Font icon definitions for the MusicMapMaker UI.
 *
 * This file contains UTF-8 encoded strings for FontAwesome 6 icons used in the
 * application. Using these constants instead of hardcoded hex strings ensures
 * consistency and easier maintenance.
 */

namespace MMM::UI
{

// --- General UI Icons ---
constexpr const char* ICON_MMM_DESKTOP = "\xef\x84\x88";  ///< \uf108 desktop
constexpr const char* ICON_MMM_EYE     = "\xef\x81\xae";  ///< \uf06e eye
constexpr const char* ICON_MMM_FOLDER  = "\xef\x81\xbb";  ///< \uf07b folder
constexpr const char* ICON_MMM_FOLDER_OPEN =
    "\xef\x81\xbc";                                      ///< \uf07c folder-open
constexpr const char* ICON_MMM_PEN    = "\xef\x8c\x84";  ///< \uf304 pen
constexpr const char* ICON_MMM_FILE   = "\xef\x85\x9b";  ///< \uf15b file
constexpr const char* ICON_MMM_MUSIC  = "\xef\x80\x81";  ///< \uf001 music
constexpr const char* ICON_MMM_COG    = "\xef\x80\x93";  ///< \uf013 cog
constexpr const char* ICON_MMM_SEARCH = "\xef\x80\x82";  ///< \uf002 search

// --- Window Control Icons ---
constexpr const char* ICON_MMM_MINIMIZE =
    "\xef\x8b\x91";  ///< \uf2d1 window-minimize (actually maximize icon, but
                     ///< used for minimize)
constexpr const char* ICON_MMM_MAXIMIZE =
    "\xef\x8b\x90";  ///< \uf2d0 window-maximize (actually minimize icon, but
                     ///< used for maximize)
constexpr const char* ICON_MMM_CLOSE =
    "\xef\x80\x8d";  ///< \uf00d xmark / close

}  // namespace MMM::UI
