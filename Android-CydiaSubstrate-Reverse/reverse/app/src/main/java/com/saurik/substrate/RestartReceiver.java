package com.saurik.substrate;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

public class RestartReceiver extends BroadcastReceiver {
    public void onReceive(Context context, Intent intent) {
        new Thread(new Runnable() {
            public void run() {
                Utility.restart();
            }
        }).start();
    }
}
