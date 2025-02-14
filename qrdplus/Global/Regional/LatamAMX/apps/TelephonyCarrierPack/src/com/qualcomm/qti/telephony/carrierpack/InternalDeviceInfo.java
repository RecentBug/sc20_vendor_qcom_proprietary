/*
 * Copyright (c) 2015 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Not a Contribution.
 * Apache license notifications and license are retained
 * for attribution purposes only.
 *
 * Copyright (C) 2008 The Android Open Source Project
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
package com.qualcomm.qti.telephony.carrierpack;

import android.content.Context;
import android.content.res.Resources;
import android.os.Build;
import android.os.Bundle;
import android.os.SystemProperties;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceScreen;
import android.text.TextUtils;
import android.util.Log;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.Date;

import java.text.Format;
import java.text.SimpleDateFormat;
/**
 * Display the following information # MODEL, BRAND, HW VERSION, SW VERSION
 */
public class InternalDeviceInfo extends PreferenceActivity {

    public static final String TAG = InternalDeviceInfo.class.getSimpleName();
    private static final boolean DBG = Log.isLoggable(InternalDeviceInfo.TAG, Log.DEBUG);

    private static final String SYS_PROP_HW_VERSION = "ro.hw_version";
    private static final String DEFAULT_HW_VERSION = "PVT2.0";
    private static final String MSV_ENGG = "(ENGINEERING)";

    private static final String KEY_DEVICE_BRANCH = "device_branch";
    private static final String KEY_BUILD = "build";
    private static final String KEY_BUILD_NUMBER = "build_number";
    private static final String KEY_SERIAL_NUMBER = "serial_number";
    private static final String KEY_BUILD_TIME = "build_time";

    private static String mUnknown = null;

    private static final String FILENAME_MSV = "/sys/board_properties/soc/msv";

    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);

        addPreferencesFromResource(R.xml.internal_device_info);

        if (mUnknown == null) {
            mUnknown = getResources().getString(R.string.device_info_unknown);
        }
        setPreferenceSummary();
    }

    private void setPreferenceSummary() {
        // get device branch
        setSummaryText(KEY_DEVICE_BRANCH,Build.DISPLAY);
        // get build
        setSummaryText(KEY_BUILD, SystemProperties.get("ro.build.description"));
        // get build number
        setSummaryText(KEY_BUILD_NUMBER, Build.DISPLAY);
        // get Serial number
        setSummaryText(KEY_SERIAL_NUMBER, Build.SERIAL);
       // get device Build time
        setSummaryText(KEY_BUILD_TIME, Long.toString(Build.TIME));
    }
    private void setSummaryText(String preferenceKey, String value) {
        Preference preference = findPreference(preferenceKey);
        if (preference == null) {
            return;
        }

        if (TextUtils.isEmpty(value)) {
            preference.setSummary(mUnknown);
        } else {
            preference.setSummary(value);
        }
    }
    /**
     * Returns " (ENGINEERING)" if the msv file has a zero value, else returns "".
     *
     * @return a string to append to the model number description.
     */
    private static String getMsvSuffix() {
        // Production devices should have a non-zero value. If we can't read it, assume it's a
        // production device so that we don't accidentally show that it's an
        // ENGINEERING device.
        try {
            String msv = readLine(FILENAME_MSV);
            // Parse as a hex number. If it evaluates to a zero, then it's an
            // engineering build.
            if (Long.parseLong(msv, 16) == 0) {
                return " " + MSV_ENGG;
            }
        } catch (IOException ioe) {
            // Fail quietly, as the file may not exist on some devices.
        } catch (NumberFormatException nfe) {
            // Fail quietly, returning empty string should be sufficient
        }
        return "";
    }

    /**
     * Reads a line from the specified file.
     *
     * @param filename the file to read from
     * @return the first line, if any.
     * @throws IOException if the file couldn't be read
     */
    private static String readLine(String filename) throws IOException {
        BufferedReader reader = new BufferedReader(new FileReader(filename),
                256);
        try {
            return reader.readLine();
        } finally {
            reader.close();
        }
    }

    public void log(String str) {
        if (DBG) {
            Log.d(TAG, str);
        }
    }
}
