/*
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef AVUTIL_WCHAR_FILENAME_H
#define AVUTIL_WCHAR_FILENAME_H

#ifdef _WIN32
#include <windows.h>
#include "mem.h"

av_warn_unused_result
static inline int utf8towchar(const char *filename_utf8, wchar_t **filename_w)
{
    int num_chars;
    num_chars = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, filename_utf8, -1, NULL, 0);
    if (num_chars <= 0) {
        *filename_w = NULL;
        return 0;
    }
    *filename_w = (wchar_t *)av_calloc(num_chars, sizeof(wchar_t));
    if (!*filename_w) {
        errno = ENOMEM;
        return -1;
    }
    MultiByteToWideChar(CP_UTF8, 0, filename_utf8, -1, *filename_w, num_chars);
    return 0;
}

/**
 * Checks for extended path prefixes for which normalization needs to be skipped.
 * see .NET6: PathInternal.IsExtended()
 */
static inline int path_is_extended(const wchar_t *path)
{
    size_t len = wcslen(path);
    if (len >= 4  && path[0] == L'\\' && (path[1] == L'\\' || path[1] == L'?') && path[2] == L'?' && path[3] == L'\\')
        return 1;

    return 0;
}

/**
 * Performs path normalization by calling GetFullPathNameW().
 * see .NET6: PathHelper.GetFullPathName()
 */
static inline int get_full_path_name(wchar_t **ppath_w)
{
    int num_chars;
    wchar_t *temp_w;

    num_chars = GetFullPathNameW(*ppath_w, 0, NULL, NULL);
    if (num_chars <= 0) {
        errno = EINVAL;
        return -1;
    }

    temp_w = (wchar_t *)av_calloc(num_chars, sizeof(wchar_t));
    if (!temp_w) {
        errno = ENOMEM;
        return -1;
    }

    num_chars = GetFullPathNameW(*ppath_w, num_chars, temp_w, NULL);
    if (num_chars <= 0) {
        errno = EINVAL;
        return -1;
    }

    av_freep(ppath_w);
    *ppath_w = temp_w;

    return 0;
}

/**
 * Normalizes a Windows file or folder path.
 * Expansion of short paths (with 8.3 path components) is currently omitted
 * as it is not required for accessing long paths.
 * see .NET6: PathHelper.Normalize().
 */
static inline int path_normalize(wchar_t **ppath_w)
{
    int ret;

    if ((ret = get_full_path_name(ppath_w)) < 0)
        return ret;

    /* What .NET does at this point is to call PathHelper.TryExpandShortFileName()
     * in case the path contains a '~' character.
     * We don't need to do this as we don't need to normalize the file name
     * for presentation, and the extended path prefix works with 8.3 path
     * components as well
     */
    return 0;
}

/**
 * Adds an extended path or UNC prefix to longs paths or paths ending
 * with a space or a dot. (' ' or '.').
 * This function expects that the path has been normalized before by
 * calling path_normalize().
 * see .NET6: PathInternal.EnsureExtendedPrefix() *
 */
static inline int add_extended_prefix(wchar_t **ppath_w)
{
    const wchar_t *unc_prefix           = L"\\\\?\\UNC\\";
    const wchar_t *extended_path_prefix = L"\\\\?\\";
    const wchar_t *path_w               = *ppath_w;
    const size_t len                    = wcslen(path_w);
    wchar_t *temp_w;

    if (len < 2)
        return 0;

    /* We're skipping the check IsPartiallyQualified() because
     * we know we have called GetFullPathNameW() already, also
     * we don't check IsDevice() because device paths are not
     * allowed to be long paths and we're calling this only
     * for long paths.
     */
    if (path_w[0] == L'\\' && path_w[1] == L'\\') {
        // The length of unc_prefix is 6 plus 1 for terminating zeros
        temp_w = (wchar_t *)av_calloc(len + 6 + 1, sizeof(wchar_t));
        if (!temp_w) {
            errno = ENOMEM;
            return -1;
        }
        wcscpy(temp_w, unc_prefix);
        wcscat(temp_w, path_w + 2);
    } else {
        // The length of extended_path_prefix is 4 plus 1 for terminating zeros
        temp_w = (wchar_t *)av_calloc(len + 4 + 1, sizeof(wchar_t));
        if (!temp_w) {
            errno = ENOMEM;
            return -1;
        }
        wcscpy(temp_w, extended_path_prefix);
        wcscat(temp_w, path_w);
    }

    av_freep(ppath_w);
    *ppath_w = temp_w;

    return 0;
}

/**
 * Converts a file or folder path to wchar_t for use with Windows file
 * APIs. Paths with extended path prefix (either '\\?\' or \??\') are
 * left unchanged.
 * All other paths are normalized and converted to absolute paths.
 * Longs paths (>= 260) are prefixed with the extended path or extended
 * UNC path prefix.
 * see .NET6: Path.GetFullPath() and Path.GetFullPathInternal()
 */
static inline int get_extended_win32_path(const char *path, wchar_t **ppath_w)
{
    int ret;
    size_t len;

    if ((ret = utf8towchar(path, ppath_w)) < 0)
        return ret;

    if (path_is_extended(*ppath_w)) {
        /* Paths prefixed with '\\?\' or \??\' are considered normalized by definition.
         * Windows doesn't normalize those paths and neither should we.
         */
        return 0;
    }

    if ((ret = path_normalize(ppath_w)) < 0)
        return ret;

    // see .NET6: PathInternal.EnsureExtendedPrefixIfNeeded()
    len = wcslen(*ppath_w);
    if (len >= 260 || (*ppath_w)[len - 1] == L' ' || (*ppath_w)[len - 1] == L'.') {
        if ((ret = add_extended_prefix(ppath_w)) < 0)
            return ret;
    }

    return 0;
}

#endif

#endif /* AVUTIL_WCHAR_FILENAME_H */
