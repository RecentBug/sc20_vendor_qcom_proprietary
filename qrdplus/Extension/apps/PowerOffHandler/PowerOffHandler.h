/*
 * Copyright (c) 2016 Qualcomm Technologies, Inc. 
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Not a Contribution.
 * Apache license notifications and license are retained
 * for attribution purposes only.
 */
/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef POWEROFFHANDLER_H
#define POWEROFFHANDLER_H

#include <stdint.h>
#include <sys/types.h>

#include <androidfw/AssetManager.h>
#include <utils/Thread.h>

#include <EGL/egl.h>
#include <GLES/gl.h>

class SkBitmap;

namespace android {

class Surface;
class SurfaceComposerClient;
class SurfaceControl;

// ---------------------------------------------------------------------------

class PowerOffHandler: public Thread, public IBinder::DeathRecipient {
public:
    PowerOffHandler();
    virtual ~PowerOffHandler();

    sp<SurfaceComposerClient> session() const;

private:
    virtual bool threadLoop();
    virtual status_t readyToRun();
    virtual void onFirstRef();
    virtual void binderDied(const wp<IBinder>& who);

    enum {
        eOrientationDefault = 0,
        eOrientation90 = 1,
        eOrientation180 = 2,
        eOrientation270 = 3,
    };
    struct Texture {
        GLint w;
        GLint h;
        GLuint name;
    };

    struct Animation {
        struct Frame {
            String8 name;
            FileMap* map;
            mutable GLuint tid;
            bool operator <(const Frame& rhs) const {
                return name < rhs.name;
            }
        };
        struct Part {
            int count;
            int pause;
            String8 path;
            SortedVector<Frame> frames;
            bool playUntilComplete;
            float backgroundColor[3];
            FileMap* audioFile;
        };
        int fps;
        int width;
        int height;
        Vector<Part> parts;
    };

    status_t initTexture(Texture* texture, AssetManager& asset,
            const char* name);
    status_t initTexture(const Animation::Frame& frame);
    bool android();
    bool readFile(const char* name, String8& outString);
    bool movie();

    enum ImageID {
        IMG_DATA = 0, IMG_SYS = 1, IMG_ENC = 2
    };
    const char *getShutDownAnimationFileName(ImageID image);
    const char *getShutDownRingtoneFileName(ImageID image);
    void playBackgroundMusic();
    void checkExit();
    void checkShowAndroid();

    sp<SurfaceComposerClient> mSession;
    AssetManager mAssets;
    Texture mAndroid[3];
    int mWidth;
    int mHeight;
    EGLDisplay mDisplay;
    EGLDisplay mContext;
    EGLDisplay mSurface;
    sp<SurfaceControl> mFlingerSurfaceControl;
    sp<Surface> mFlingerSurface;
    ZipFileRO *mZip;
};

// ---------------------------------------------------------------------------

}
;
// namespace android

#endif // POWEROFFHANDLER_H
