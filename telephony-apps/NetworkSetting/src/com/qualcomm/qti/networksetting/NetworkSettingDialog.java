/*
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
 */
/*
 * Copyright (C) 2006 The Android Open Source Project
 * Copyright (c) 2011-2013 The Linux Foundation. All rights reserved.
 *
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

package com.qualcomm.qti.networksetting;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;

import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.ComponentName;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.RemoteException;
import android.preference.Preference;
import android.preference.Preference.OnPreferenceClickListener;
import android.preference.PreferenceActivity;
import android.telephony.ServiceState;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.Toast;
import java.util.Timer;
import android.content.SharedPreferences;


import com.android.internal.telephony.CommandException;
import com.android.internal.telephony.OperatorInfo;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.TelephonyIntents;

import com.qualcomm.qti.networksetting.INetworkQueryService;
import com.qualcomm.qti.networksetting.INetworkQueryServiceCallback;
import com.qualcomm.qti.networksetting.NetworkQueryService;
import com.qualcomm.qti.networksetting.NetworkSettingDataManager;

public class NetworkSettingDialog extends PreferenceActivity implements OnPreferenceClickListener {

    private static final String TAG = "NetworkSettingDialog";

    private static final int MENU_ID_SCAN = 1;

    private static final String BUTTON_AUTO_SELECT_KEY = "button_auto_select_key";
    private static final String LIST_AVAILABLE_NETWORKS = "network_list";
    public static final String EXTRA_SUB_ID = "sub_id";

    private ProgressPrefCategory mProgressPref;
    private MenuItem mSearchButton;

    private static final String ACTION_INCREMENTAL_NW_SCAN_IND
        = "qualcomm.intent.action.ACTION_INCREMENTAL_NW_SCAN_IND";

    private static final int EVENT_AUTO_SELECT_DONE = 1;
    private static final int EVENT_NETWORK_SELECTION_DONE = 2;
    private static final int EVENT_QUERY_AVAILABLE_NETWORKS = 3;
    private static final int EVENT_NETWORK_DATA_MANAGER_DONE = 4;
    private static final int EVENT_INCREMENTAL_MANUAL_SCAN_RESULTS = 5;

    private static final int DIALOG_NETWORK_SELECTION = 1;
    private static final int DIALOG_NETWORK_AUTO_SELECT = 2;
    private static final int DIALOG_NETWORK_QUIT = 3;

    // error statuses that will be retured in the callback.
    private static final int QUERY_EXCEPTION = -1;
    private static final int QUERY_OK = 0;
    private static final int QUERY_PARTIAL = 1;
    private static final int QUERY_ABORT = 2;
    private static final int QUERY_REJ_IN_RLF = 3;

    // this value should match with ALREADY_IN_AUTO_SELECTION of framework Phone.java
    private static final int ALREADY_IN_AUTO_SELECTION = 1;

    private Phone mPhone;
    private int mSubId;
    NetworkSettingDataManager mDataManager = null;

    private String mNetworkSelectMsg;
    //map of RAT type values to user understandable strings
    private HashMap<String, String> mRatMap;

    private SharedPreferences mSharedPreferences;
    private final static String TRI_KEY = "com.qualcomm.qti.networksetting.trinetworksetting";

    private enum State{
        IDLE,
        SCAN
    }
    private State mState = State.IDLE;

    // map of network controls to the network data.
    private Map<Preference, OperatorInfo> mNetworkMap;

    private boolean mNoAction = true;

    Handler mScheduleHandler = new Handler();
    Runnable r = new Runnable() {
        @Override
        public void run() {
            logd("mNoAction = " + mNoAction);
            if (mNoAction){
                logd("no action during 5s, destory activity");
                resetTriggerFlag();
                finish();
            }
        }
    };


    private final Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            AsyncResult ar = null;
            switch (msg.what) {
                case EVENT_AUTO_SELECT_DONE:
                    ar = (AsyncResult) msg.obj;
                    logd("EVENT_AUTO_SELECT_DONE msg.arg1 = " + msg.arg1);
                    onAutomaticResult(ar.exception, msg.arg1);
                    break;
                case EVENT_NETWORK_DATA_MANAGER_DONE:
                    logd("EVENT_NETWORK_DATA_MANAGER_DONE: " + msg.arg1);
                    if (msg.arg1 == 1) {
                        loadNetworksList();
                    } else {
                        NetworkSettingDialog.this.finish();
                    }
                    if (mSearchButton != null) mSearchButton.setEnabled(true);
                    break;
                case EVENT_NETWORK_SELECTION_DONE:
                    ar = (AsyncResult) msg.obj;
                    onNetworkManuallyResult(ar.exception);
                    break;
                case EVENT_QUERY_AVAILABLE_NETWORKS:
                    ArrayList<OperatorInfo> operatorInfoList =
                            (ArrayList<OperatorInfo>) msg.obj;
                    int exception = msg.arg1;
                    if (exception == -1) {
                        logd("query available networks fail");
                        onNetworksListLoadResult(null, QUERY_EXCEPTION);
                    } else {

                        logd("query available networks number is: " + operatorInfoList.size());
                        if (operatorInfoList.size() > 0) {
                            for (OperatorInfo operatorInfo: operatorInfoList) {
                                setNetworkList(operatorInfo);
                            }
                        }
                        mState = State.IDLE;
                        updateUIState();
                        if (mDataManager != null) {
                            mDataManager.updateDataState(true, null);
                        }
                    }
                    break;
                case EVENT_INCREMENTAL_MANUAL_SCAN_RESULTS:
                    try {
                        mNetworkQueryService.unregisterCallback(mCallback);
                    } catch (RemoteException e) {
                        logd("EVENT_INCREMENTAL_MANUAL_SCAN_RESULTS: exception " + e);
                    }
                    onNetworksListLoadResult((String[])msg.obj, msg.arg1);
                    break;
                default:
                    logd("unknown event!");
            }
            return;
        }
    };

    /**
     * This implementation of INetworkQueryServiceCallback is used to receive
     * callback notifications from the network query service.
     */
    private final INetworkQueryServiceCallback mCallback = new INetworkQueryServiceCallback.Stub() {

        /** place the message on the looper queue upon query completion. */
        public void onQueryComplete(List<OperatorInfo> networkInfoArray, int status) {
            logd("notifying message loop of query completion.");
            Message msg = mHandler.obtainMessage(EVENT_QUERY_AVAILABLE_NETWORKS,
                    status, 0, networkInfoArray);
            msg.sendToTarget();
        }

        public void  onIncrementalManualScanResult(String[] scanInfo, int result){
            logd("notifying onincrementalmanualscan ." + result);
            Message msg = mHandler.obtainMessage(EVENT_INCREMENTAL_MANUAL_SCAN_RESULTS,
                    result, 0, scanInfo);
            msg.sendToTarget();
        }
    };

    private BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (TelephonyIntents.ACTION_SIM_STATE_CHANGED.equals(intent.getAction())) {
                int simState = TelephonyManager.getDefault().getSimState(
                                SubscriptionManager.getSlotId(mSubId));
                if (simState != TelephonyManager.SIM_STATE_READY && (mState == State.SCAN)) {
                    logd("SIM state is not ready: sim_state = " + simState + "slot = "
                                    + SubscriptionManager.getSlotId(mSubId));
                    onNetworksListLoadResult(null, QUERY_EXCEPTION);
                }
            }
        }
    };

    /**
     * Service connection code for the NetworkQueryService.
     * Handles the work of binding to a local object so that we can make
     * the appropriate service calls.
     */

    /** Local service interface */
    private INetworkQueryService mNetworkQueryService = null;

    /** Service connection */
    private final ServiceConnection mNetworkQueryServiceConnection = new ServiceConnection() {

        /** Handle the task of binding the local object to the service */
        public void onServiceConnected(ComponentName className, IBinder service) {
            logd("connection created, binding local service.");
            mNetworkQueryService = ((NetworkQueryService.LocalBinder) service).getService();
            // as soon as it is bound, run a query.
            if (isDataDisableRequired()) {
                Message onCompleteMsg = mHandler.obtainMessage(EVENT_NETWORK_DATA_MANAGER_DONE);
                mDataManager.updateDataState(false, onCompleteMsg);
            } else {
                loadNetworksList();
            }
        }

        /** Handle the task of cleaning up the local binding */
        public void onServiceDisconnected(ComponentName className) {
            logd("connection disconnected, cleaning local binding.");
            mNetworkQueryService = null;
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
		moveTaskToBack(true);

        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(TelephonyIntents.ACTION_SIM_STATE_CHANGED);
        registerReceiver(mBroadcastReceiver, intentFilter);

        mSharedPreferences = getSharedPreferences(TRI_KEY, MODE_PRIVATE);
        Log.d(TAG, "triggernetworksetting : " + mSharedPreferences.getInt("triggernetworksetting",0));

        if (mSharedPreferences.getInt("triggernetworksetting", 0) != 1){
            Log.d(TAG, "no need to set network, exit");
            finish();
        } else {
            addPreferencesFromResource(R.xml.network_select_dialog);
            mNetworkMap = new LinkedHashMap<Preference, OperatorInfo>();
            mProgressPref = (ProgressPrefCategory) getPreferenceScreen().findPreference(
					LIST_AVAILABLE_NETWORKS);
            mSubId = getIntent().getIntExtra(EXTRA_SUB_ID,
							SubscriptionManager.getDefaultSubscriptionId());
            mPhone = PhoneFactory.getPhone(SubscriptionManager.getPhoneId(mSubId));
            mProgressPref.setOnPreferenceClickListener(this);
            mRatMap = new HashMap<String, String>();
            initRatMap();
            startService (new Intent(this, NetworkQueryService.class));
            bindService (new Intent(this, NetworkQueryService.class), mNetworkQueryServiceConnection,
					Context.BIND_AUTO_CREATE);
            if (isDataDisableRequired()) {
                mDataManager = new NetworkSettingDataManager(this);
            }
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        try {
            unregisterReceiver(mBroadcastReceiver);
            // used to un-register callback
            if (mNetworkQueryService != null) {
                mNetworkQueryService.unregisterCallback(mCallback);
                unbindService(mNetworkQueryServiceConnection);
            }
        } catch (RemoteException e) {
            logd("onDestroy: exception from unregisterCallback " + e);
        }
        if (mDataManager != null) {
            mDataManager.updateDataState(true, null);
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        mSearchButton = menu.add(Menu.NONE, MENU_ID_SCAN, 0, R.string.search_networks).setEnabled(
                true);
        mSearchButton.setShowAsAction(MenuItem.SHOW_AS_ACTION_IF_ROOM);
        return super.onCreateOptionsMenu(menu);
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        updateUIState();
        if (isDataDisableRequired()) {
            if (mSearchButton != null) mSearchButton.setEnabled(false);
        }
        return super.onPrepareOptionsMenu(menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case MENU_ID_SCAN:
                if (isDataDisableRequired()) {
                    if (mSearchButton != null)
                        mSearchButton.setEnabled(false);
                    Message onCompleteMsg = mHandler.obtainMessage(EVENT_NETWORK_DATA_MANAGER_DONE);
                    mDataManager.updateDataState(false, onCompleteMsg);
                } else {
                    loadNetworksList();
                }
                break;
        }
        return super.onOptionsItemSelected(item);
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        Dialog dialog = null;
        if ((id == DIALOG_NETWORK_SELECTION) || id == DIALOG_NETWORK_AUTO_SELECT
                || id == DIALOG_NETWORK_QUIT) {
            switch (id) {
                case DIALOG_NETWORK_SELECTION:
                    dialog = new ProgressDialog(this);
                    dialog.setCancelable(false);
                    ((ProgressDialog) dialog).setIndeterminate(true);
                    ((ProgressDialog) dialog).setMessage(mNetworkSelectMsg);
                    break;
                case DIALOG_NETWORK_AUTO_SELECT:
                    dialog = new ProgressDialog(this);
                    dialog.setCancelable(false);
                    ((ProgressDialog) dialog)
                            .setMessage(getString(R.string.register_automatically));
                    ((ProgressDialog) dialog).setIndeterminate(true);
                    break;
                case DIALOG_NETWORK_QUIT:
                    dialog = new AlertDialog.Builder(NetworkSettingDialog.this)
                            .setTitle(R.string.quit_mention).setMessage(R.string.dialog_message)
                            .setPositiveButton(android.R.string.ok,
                                    new DialogInterface.OnClickListener() {
                                        public void onClick(DialogInterface dialog,
                                                int whichButton) {
                                            try {
                                                mNetworkQueryService.stopNetworkQuery(mCallback);
                                            } catch (RemoteException ex) {
                                                logd("stopnetworkquery remote exception " + ex);
                                            }
                                            NetworkSettingDialog.this.finish();
                                        }
                                    }).setNeutralButton(android.R.string.cancel,
                                    new DialogInterface.OnClickListener() {
                                        public void onClick(DialogInterface dialog,
                                                int whichButton) {
                                            dialog.dismiss();
                                        }
                                    }).create();
                    break;
            }
        }
        return dialog;
    }

    @Override
    public boolean onPreferenceClick(Preference preference) {
        boolean handled = false;
        mNoAction = false;
		logd("onPreferenceClick  mNoAction = " + mNoAction);
        OperatorInfo operatorInfo = mNetworkMap.get(preference);
        mPhone.selectNetworkManually(operatorInfo, true,
                    mHandler.obtainMessage(EVENT_NETWORK_SELECTION_DONE));
        onNetworkManuallySelect(operatorInfo);
        handled = true;

        return handled;
    }

    private void onNetworkManuallySelect(final OperatorInfo operatorInfo) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mNetworkSelectMsg = getString(R.string.register_on_network,
                        getNetworkTitle(operatorInfo));
                showDialog(DIALOG_NETWORK_SELECTION);
            }
        });
    }

    private int getErrorResId(final Throwable ex) {
        if (ex == null) {
            return 0;
        } else if ((ex instanceof CommandException)
                && ((CommandException) ex).getCommandError()
                    == CommandException.Error.ILLEGAL_SIM_OR_ME) {
            return R.string.not_allowed;
        } else {
            return R.string.connect_later;
        }
    }

    private void onNetworkManuallyResult(final Throwable ex) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                removeDialog(DIALOG_NETWORK_SELECTION);
                onResult(getErrorResId(ex));
            }
        });
    }

    private void loadNetworksList() {
        logd("loadnetworklist");
        if (mState == State.IDLE) {
            try {
                logd("loadnetworklist idle");
                mState = State.SCAN;
                clearNetworkList();
                updateUIState();
                mNetworkQueryService.startNetworkQuery(mCallback, mSubId);
            } catch (RemoteException e) {
                logd("loadnetworklist exception");
                mState = State.IDLE;
                updateUIState();
            }
        }

    }

    private void clearNetworkList() {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                for (Preference p : mNetworkMap.keySet()) {
                    mProgressPref.removePreference(p);
                }
                mNetworkMap.clear();
            }
        });
    }

    private String getLocalString(String originalString) {
        String[] origNames = getResources().getStringArray(R.array.original_carrier_names);
        String[] localNames = getResources().getStringArray(R.array.locale_carrier_names);
        for (int i = 0; i < origNames.length; i++) {
            if (origNames[i].equalsIgnoreCase(originalString)) {
                return getString(getResources().getIdentifier(localNames[i],
                        "string", getPackageName()));
            }
        }
        return originalString;
    }

    private String getNetworkTitle(OperatorInfo ni) {
        String title;
        String numericOnly = "";
        String radioTech = "";
        String operatorNumeric = ni.getOperatorNumeric();

        /* operatorNumeric format: PLMN+RAT or PLMN */
        if (null != operatorNumeric) {
            String values[] = operatorNumeric.split("\\+");
            numericOnly = values[0];
            if (values.length > 1)
                radioTech = values[1];
        }

        if (!TextUtils.isEmpty(ni.getOperatorAlphaLong())) {
            title = getLocalString(ni.getOperatorAlphaLong());
        } else if (!TextUtils.isEmpty(ni.getOperatorAlphaShort())) {
            title = getLocalString(ni.getOperatorAlphaShort());
        } else
            title = numericOnly;

        if (!TextUtils.isEmpty(radioTech))
            title += " " + mRatMap.get(radioTech);

        if (ni.getState() == OperatorInfo.State.FORBIDDEN) {
            title += getString(R.string.network_forbidden);
        }

        return title;
    }

    private void setNetworkList(final OperatorInfo operatorInfo) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                Preference carrier = new Preference(NetworkSettingDialog.this, null);
                carrier.setTitle(getNetworkTitle(operatorInfo));
                carrier.setPersistent(false);
                mProgressPref.addPreference(carrier);
                carrier.setOnPreferenceClickListener(NetworkSettingDialog.this);
                mNetworkMap.put(carrier, operatorInfo);
            }
        });
    }

    private void onNetworksListLoadResult(final String[] result, final int status) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                logd("result: " + status);
                switch (status) {
                    case QUERY_REJ_IN_RLF:
                        // Network scan did not complete due to a radio link
                        // failure recovery in progress
                        mState = State.IDLE;
                        clearNetworkList();
                        Toast.makeText(NetworkSettingDialog.this, R.string.network_query_error,
                                Toast.LENGTH_SHORT).show();
                    case QUERY_ABORT:
                        // Network searching time out
                        // show the nw info if it bundled with this query
                    case QUERY_OK:
                        mState = State.IDLE;
                        updateUIState();
                        if (mDataManager != null) {
                            mDataManager.updateDataState(true, null);
                        }
                    case QUERY_PARTIAL:
                        if (result != null) {
                            if (result.length >= 4 && (result.length % 4 == 0)) {
                                //if receive the whole results with QUERY_OK at one time, will
                                //split with every four to show on UI
                                for (int i = 0; i < result.length / 4; i++) {
                                    int j = 4 * i;
                                    setNetworkList(new OperatorInfo(result[0 + j], result[1 + j],
                                            result[2 + j], result[3 + j]));
                                }
                            } else {
                                logd("result.length is: " + result.length);
                            }
                            moveTasktoTop();
                            mScheduleHandler.postDelayed(r, 5*1000);
                        }
                        break;
                    default:
                        mState = State.IDLE;
                        Toast.makeText(NetworkSettingDialog.this, R.string.network_query_error,
                                Toast.LENGTH_SHORT).show();
                        clearNetworkList();
                        updateUIState();
                        if (mDataManager != null) {
                            mDataManager.updateDataState(true, null);
                        }
                }
            }
        });
    }

    private void updateUIState() {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                switch (mState) {
                    case IDLE:
                        mProgressPref.setProgress(false);
                        if (mSearchButton != null) {
                            mSearchButton.setEnabled(true);
                            mSearchButton.setTitle(R.string.search_networks);
                        }
                        break;
                    case SCAN:
                        mProgressPref.setProgress(true);
                        if (mSearchButton != null) {
                            mSearchButton.setEnabled(false);
                            mSearchButton.setTitle(R.string.progress_scanning);
                        }
                        break;
                }
            }
        });
    }

    private void onAutomaticSelect() {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                showDialog(DIALOG_NETWORK_AUTO_SELECT);
                Message msg = mHandler.obtainMessage(EVENT_AUTO_SELECT_DONE);
                mPhone.setNetworkSelectionModeAutomatic(msg);
            }
        });
    }

    private void onAutomaticResult(final Throwable ex, int msgArg1) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                removeDialog(DIALOG_NETWORK_AUTO_SELECT);
                if (msgArg1 == ALREADY_IN_AUTO_SELECTION) {
                    // This will be true if automatic selection is requested while the device
                    // is already in automatic selection mode. We dont want to show registered to
                    // network here since the device may be OOS. Show an alternate toast instead.
                    Toast.makeText(NetworkSettingDialog.this, R.string.already_auto,
                            Toast.LENGTH_SHORT).show();
                    mHandler.postDelayed(new Runnable() {
                        public void run() {
                            finish();
                        }
                    }, 3000);
                    return;
                }
                onResult(getErrorResId(ex));
            }
        });
    }

    private void onResult(final int errorMsg) {
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (errorMsg != 0) {
                    Toast.makeText(NetworkSettingDialog.this, errorMsg, Toast.LENGTH_SHORT).show();
                } else {
                    Toast.makeText(NetworkSettingDialog.this, R.string.registration_done,
                            Toast.LENGTH_SHORT).show();
                    mHandler.postDelayed(new Runnable() {
                        public void run() {
                            resetTriggerFlag();
                            finish();
                        }
                    }, 3000);
                }
            }
        });
    }

    public void onBackPressed() {
        if (mState == State.SCAN) {
            showDialog(DIALOG_NETWORK_QUIT);
        } else {
            super.onBackPressed();
        }
    }

    private void initRatMap() {
        mRatMap.put(String.valueOf(ServiceState.RIL_RADIO_TECHNOLOGY_UNKNOWN), "Unknown");
        mRatMap.put(String.valueOf(ServiceState.RIL_RADIO_TECHNOLOGY_GPRS), "2G");
        mRatMap.put(String.valueOf(ServiceState.RIL_RADIO_TECHNOLOGY_EDGE), "2G");
        mRatMap.put(String.valueOf(ServiceState.RIL_RADIO_TECHNOLOGY_UMTS), "3G");
        mRatMap.put(String.valueOf(ServiceState.RIL_RADIO_TECHNOLOGY_IS95A), "2G");
        mRatMap.put(String.valueOf(ServiceState.RIL_RADIO_TECHNOLOGY_IS95B), "2G");
        mRatMap.put(String.valueOf(ServiceState.RIL_RADIO_TECHNOLOGY_1xRTT), "2G");
        mRatMap.put(String.valueOf(ServiceState.RIL_RADIO_TECHNOLOGY_EVDO_0), "3G");
        mRatMap.put(String.valueOf(ServiceState.RIL_RADIO_TECHNOLOGY_EVDO_A), "3G");
        mRatMap.put(String.valueOf(ServiceState.RIL_RADIO_TECHNOLOGY_HSDPA), "3G");
        mRatMap.put(String.valueOf(ServiceState.RIL_RADIO_TECHNOLOGY_HSUPA), "3G");
        mRatMap.put(String.valueOf(ServiceState.RIL_RADIO_TECHNOLOGY_HSPA), "3G");
        mRatMap.put(String.valueOf(ServiceState.RIL_RADIO_TECHNOLOGY_EVDO_B), "3G");
        mRatMap.put(String.valueOf(ServiceState.RIL_RADIO_TECHNOLOGY_EHRPD), "3G");
        mRatMap.put(String.valueOf(ServiceState.RIL_RADIO_TECHNOLOGY_LTE), "4G");
        //mRatMap.put(String.valueOf(ServiceState.RIL_RADIO_TECHNOLOGY_LTE_CA), "4G");
        mRatMap.put(String.valueOf(ServiceState.RIL_RADIO_TECHNOLOGY_HSPAP), "3G");
        mRatMap.put(String.valueOf(ServiceState.RIL_RADIO_TECHNOLOGY_GSM), "2G");
        mRatMap.put(String.valueOf(ServiceState.RIL_RADIO_TECHNOLOGY_TD_SCDMA), "3G");
    }

    @Override
    public void onResume() {
        super.onResume();
        Log.d(TAG, "onResume");

    }

    void moveTasktoTop(){
        Log.d(TAG, "moveTasktoTop");

        Intent start = new Intent(getApplicationContext(),NetworkSettingDialog.class);
        start.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_REORDER_TO_FRONT);
        startActivity(start);
    }


    private boolean isDataDisableRequired() {
        boolean isRequired = getApplicationContext().getResources().getBoolean(
                         R.bool.config_disable_data_manual_plmn);
        if((TelephonyManager.getDefault().getMultiSimConfiguration()
                == TelephonyManager.MultiSimVariants.DSDA) &&
               (SubscriptionManager.getDefaultDataSubscriptionId() != mPhone.getSubId())){
                isRequired = false;
        }
        return false/*isRequired*/;
    }

    void resetTriggerFlag(){
        logd("resetTriggerFlag settrigger");
        SharedPreferences.Editor editor = mSharedPreferences.edit();
        editor.putInt("triggernetworksetting",0);
        editor.commit();
	}

    private void logd(String msg) {
        Log.d(TAG, msg);
    }
}
