/*
 * Copyright (C) 2012 The Android Open Source Project
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

#ifndef ANDROID_AUDIO_FAST_MIXER_H
#define ANDROID_AUDIO_FAST_MIXER_H

#include <utils/Thread.h>
extern "C" {
#include "../private/bionic_futex.h"
}
#include "StateQueue.h"
#include "FastMixerState.h"

namespace android {

typedef StateQueue<FastMixerState> FastMixerStateQueue;

class FastMixer : public Thread {

public:
            FastMixer() : Thread(false /*canCallJava*/) { }
    virtual ~FastMixer() { }

            FastMixerStateQueue* sq() { return &mSQ; }

private:
    virtual bool                threadLoop();
            FastMixerStateQueue mSQ;

};  // class FastMixer

// Represents the dump state of a fast track
struct FastTrackDump {
    FastTrackDump() : mUnderruns(0) { }
    /*virtual*/ ~FastTrackDump() { }
    uint32_t mUnderruns;        // Underrun status, represented as follows:
                                //   bit 0 == 0 means not currently in underrun
                                //   bit 0 == 1 means currently in underrun
                                //   bits 1 to 31 == total number of underruns
                                // Not reset to zero for new tracks or if track generation changes.
                                // This representation is used to keep the information atomic.
};

// The FastMixerDumpState keeps a cache of FastMixer statistics that can be logged by dumpsys.
// Each individual native word-sized field is accessed atomically.  But the
// overall structure is non-atomic, that is there may be an inconsistency between fields.
// No barriers or locks are used for either writing or reading.
// Only POD types are permitted, and the contents shouldn't be trusted (i.e. do range checks).
// It has a different lifetime than the FastMixer, and so it can't be a member of FastMixer.
struct FastMixerDumpState {
    FastMixerDumpState();
    /*virtual*/ ~FastMixerDumpState();

    void dump(int fd);

    FastMixerState::Command mCommand;   // current command
    uint32_t mWriteSequence;    // incremented before and after each write()
    uint32_t mFramesWritten;    // total number of frames written successfully
    uint32_t mNumTracks;        // total number of active fast tracks
    uint32_t mWriteErrors;      // total number of write() errors
    uint32_t mUnderruns;        // total number of underruns
    uint32_t mOverruns;         // total number of overruns
    uint32_t mSampleRate;
    size_t   mFrameCount;
    struct timespec mMeasuredWarmupTs;  // measured warmup time
    uint32_t mWarmupCycles;     // number of loop cycles required to warmup
    FastTrackDump   mTracks[FastMixerState::kMaxFastTracks];
#ifdef FAST_MIXER_STATISTICS
    // cycle times in seconds
    float    mMean;
    float    mMinimum;
    float    mMaximum;
    float    mStddev;
#endif
};

}   // namespace android

#endif  // ANDROID_AUDIO_FAST_MIXER_H