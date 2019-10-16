/* Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Not a Contribution.
 * Apache license notifications and license are retained
 * for attribution purposes only.
 *
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <OmaDrmMimeTypeUtil.h>
#include <utils/Log.h>

namespace android {

#undef LOG_TAG
#define LOG_TAG "OmaDrmMimeTypeUtil"

#ifdef DRM_OMA_FL_ENGINE_DEBUG
#define LOG_NDEBUG 0
#define LOG_DEBUG(...) ALOGD(__VA_ARGS__)
#else
#define LOG_DEBUG(...)
#endif

enum {
    MIMETYPE_AUDIO       = 0,
    MIMETYPE_APPLICATION = 1,
    MIMETYPE_IMAGE       = 2,
    MIMETYPE_VIDEO       = 3,
    MIMETYPE_LAST        = -1,
};

struct MimeGroup{
    int         type;     // Audio, video,.. use the enum values
    const char* pGroup;   // "audio/", "video/",.. should contain the last "/"
    int         size;     // Number of bytes. e.g. "audio/" = 6 bytes
};

struct MimeTypeList{
    int         type;
    const char* pMimeExt;  // Everything after the '/' e.g. audio/x-mpeg -> "x-mpeg"
    int         size;      // Number of bytes. e.g. "x-mpeg" = 6 bytes
    const char* pMimeType; // Mimetype that should be returned
};


// Known mimetypes by android
static const char mime_type_audio_mpeg[]  = "audio/mpeg";
static const char mime_type_audio_3gpp[]  = "audio/3gpp";
static const char mime_type_audio_amr[]   = "audio/amr-wb";
static const char mime_type_audio_aac[]   = "audio/mp4a-latm";
static const char mime_type_audio_wav[]   = "audio/x-wav";
static const char mime_type_audio_midi[]   = "audio/midi";
static const char mime_type_audio_ogg[]   = "application/ogg";

static const char mime_type_video_mpeg4[] = "video/mpeg4";
static const char mime_type_video_3gpp[]  = "video/3gpp";
static const char mime_type_video_3gpp2[] = "video/3gpp2";
static const char mime_type_video_mp4[]   = "video/mp4";
static const char mime_type_video_asf[]   = "video/x-ms-asf";
static const char mime_type_video_matro[] = "video/x-matroska";
static const char mime_type_video_avi[]   = "video/avi";

// Known mimetype groups
static const char mime_group_audio[]       = "audio/";
static const char mime_group_application[] = "application/";
static const char mime_group_image[]       = "image/";
static const char mime_group_video[]       = "video/";
static const char mime_type_unsupported[]  = "unsupported/drm.mimetype";

static struct MimeGroup mimeGroup[] = {
    {MIMETYPE_AUDIO,       mime_group_audio,        sizeof(mime_group_audio)-1},
    {MIMETYPE_APPLICATION, mime_group_application,  sizeof(mime_group_application)-1},
    {MIMETYPE_IMAGE,       mime_group_image,        sizeof(mime_group_image)-1},
    {MIMETYPE_VIDEO,       mime_group_video,        sizeof(mime_group_video)-1},
    {MIMETYPE_LAST,        NULL,                    0} // Must be last entry
};

// List of all mimetypes that should be converted.
static struct MimeTypeList mimeTypeList[] = {
    // Mp3 mime types
    {MIMETYPE_AUDIO, "mp3",          sizeof("mp3")-1,         mime_type_audio_mpeg},
    {MIMETYPE_AUDIO, "mp3d",         sizeof("mp3d")-1,        mime_type_audio_mpeg},
    {MIMETYPE_AUDIO, "x-mpeg",       sizeof("x-mpeg")-1,      mime_type_audio_mpeg},
    {MIMETYPE_AUDIO, "x-mp3",        sizeof("x-mp3")-1,       mime_type_audio_mpeg},
    {MIMETYPE_AUDIO, "mpg",          sizeof("mpg")-1,         mime_type_audio_mpeg},
    {MIMETYPE_AUDIO, "mpg3",         sizeof("mpg")-1,         mime_type_audio_mpeg},
    {MIMETYPE_AUDIO, "x-mpg",        sizeof("x-mpg")-1,       mime_type_audio_mpeg},
    {MIMETYPE_AUDIO, "x-mpegaudio",  sizeof("x-mpegaudio")-1, mime_type_audio_mpeg},

    // 3gpp audio mime types
    {MIMETYPE_AUDIO, "3gp",          sizeof("3gp")-1,          mime_type_video_mp4},
    {MIMETYPE_AUDIO, "3gpp",         sizeof("3gpp")-1,         mime_type_video_mp4},
    {MIMETYPE_AUDIO, "3g2",          sizeof("3g2")-1,          mime_type_video_mp4},
    {MIMETYPE_AUDIO, "m4a",          sizeof("m4a")-1,          mime_type_video_mp4},
    {MIMETYPE_AUDIO, "mp4a-latm",    sizeof("mp4a-latm")-1,    mime_type_video_mp4},


    // Amr audio mime types
    {MIMETYPE_AUDIO, "amr",          sizeof("amr")-1,         mime_type_audio_3gpp},
    {MIMETYPE_AUDIO, "amr-nb",       sizeof("amr-nb")-1,      mime_type_audio_3gpp},
    {MIMETYPE_AUDIO, "amr-wb",       sizeof("amr-wb")-1,      mime_type_audio_3gpp},

    // Aac audio mime types
    {MIMETYPE_AUDIO, "aac",          sizeof("aac")-1,         mime_type_audio_aac},

    // Mpeg audio mime types
    {MIMETYPE_AUDIO, "mpeg",         sizeof("mpeg")-1,        mime_type_audio_mpeg},
    {MIMETYPE_AUDIO, "mp4",          sizeof("mp4")-1,         mime_type_video_mp4},

    // Wav audio mime types
    {MIMETYPE_AUDIO, "x-ms-wav",     sizeof("x-ms-wav")-1,    mime_type_audio_wav},
    {MIMETYPE_AUDIO, "wav",          sizeof("wav")-1,         mime_type_audio_wav},

    // Midi mimetype
    {MIMETYPE_AUDIO, "x-sp-midi",    sizeof("x-sp-midi")-1,   mime_type_audio_midi},
    {MIMETYPE_AUDIO, "sp-midi",      sizeof("sp-midi")-1,     mime_type_audio_midi},
    {MIMETYPE_AUDIO, "x-midi",       sizeof("x-midi")-1,      mime_type_audio_midi},
    {MIMETYPE_AUDIO, "imelody",      sizeof("imelody")-1,     mime_type_audio_midi},
    {MIMETYPE_AUDIO, "imy",          sizeof("imy")-1,         mime_type_audio_midi},
    {MIMETYPE_AUDIO, "ogg",          sizeof("ogg")-1,         mime_type_audio_ogg},

    // Mpeg4 video mime types
    {MIMETYPE_VIDEO, "mpg4",         sizeof("mpg4")-1,        mime_type_video_mpeg4},
    {MIMETYPE_VIDEO, "mp4v-es",      sizeof("mp4v-es")-1,     mime_type_video_mpeg4},

    // 3gpp video mime types
    {MIMETYPE_VIDEO, "3gp",          sizeof("3gp")-1,         mime_type_video_mp4},
    {MIMETYPE_VIDEO, "3gpp",         sizeof("3gpp")-1,        mime_type_video_mp4},

    {MIMETYPE_VIDEO, "x-ms-asf",     sizeof("x-ms-asf")-1,    mime_type_video_asf},
    {MIMETYPE_VIDEO, "webm",         sizeof("webm")-1,        mime_type_video_matro},
    {MIMETYPE_VIDEO, "x-msvideo",    sizeof("x-msvideo")-1,   mime_type_video_avi},

    // Must be last entry
    {MIMETYPE_LAST,  NULL,           0,                       NULL}
};

/**
 * May convert the mimetype if there is a well known
 * replacement mimetype otherwise the original mimetype
 * is returned.
 *
 * If the mimetype is of unsupported group i.e. application/*
 * then "unsupported/drm.mimetype" will be returned.
 *
 * @param mimeType - mimetype in lower case to convert.
 *
 * @return mimetype or "unsupported/drm.mimetype".
 */
String8 OmaDrmMimeTypeUtil::convertMimeType(String8& mimeType) {
    String8 result = mimeType;
    const char* pMimeType;
    struct MimeGroup* pGroup;
    struct MimeTypeList* pMimeItem;
    int len;
    pMimeType = mimeType.string();
    ALOGI("OmaDrmMimeTypeUtil::convertMimeType");
    if (NULL != pMimeType) {
        if ((0 == strncmp(pMimeType, mime_group_audio, (sizeof mime_group_audio) - 1)) ||
            (0 == strncmp(pMimeType, "application/ogg", 15))                           ||
            (0 == strncmp(pMimeType, mime_group_image, (sizeof mime_group_image) - 1)) ||
            (0 == strncmp(pMimeType, mime_group_video, (sizeof mime_group_video) - 1))) {
            /* Check which group the mimetype is */
            pGroup = mimeGroup;
            while (MIMETYPE_LAST != pGroup->type) {
                if (0 == strncmp(pMimeType, pGroup->pGroup, pGroup->size)) {
                    break;
                }
                pGroup++;
            }

            /* Go through the mimetype list. Only check items of the correct group */
            if (MIMETYPE_LAST != pGroup->type) {
                pMimeItem = mimeTypeList;
                len = strlen (pMimeType+pGroup->size);
                while (MIMETYPE_LAST != pMimeItem->type) {
                    if ((pGroup->type == pMimeItem->type) &&
                        (len == pMimeItem->size) &&
                        (0 == strcmp(pMimeType+pGroup->size, pMimeItem->pMimeExt))) {
                        result = String8(pMimeItem->pMimeType);
                        break;
                    }
                    pMimeItem++;
                }
            }
            if (0 == strncmp(pMimeType, mime_group_image, (sizeof mime_group_image) - 1)) {
                // Change the DRM image mime, so, only Drm Image viewer can show DRM images.
                result = String8("drm+container_based+") + result;
            }
        } else {
            result = String8(mime_type_unsupported);
        }
        ALOGI("convertMimeType got mimetype %s, converted into mimetype %s",
                pMimeType, result.string());
    }
    return result;
}
};
