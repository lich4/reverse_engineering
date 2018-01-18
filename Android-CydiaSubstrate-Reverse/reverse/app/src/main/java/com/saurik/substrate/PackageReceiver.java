package com.saurik.substrate;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.util.Log;
import com.saurik.substrate.Manifest.permission;

public class PackageReceiver extends BroadcastReceiver {
    public void onReceive(Context context, Intent intent) {
        Log.v("CydiaSubstrate", "onReceive(..., " + intent + "):" + intent.getBooleanExtra("android.intent.extra.REPLACING", false));
        String action = intent.getAction();
        boolean replacing = intent.getBooleanExtra("android.intent.extra.REPLACING", false);
        String name = intent.getData().getSchemeSpecificPart();
        if (action.equals("android.intent.action.PACKAGE_ADDED")) {
            action = "android.intent.action.PACKAGE_ADDED";
        } else if (action.equals("android.intent.action.PACKAGE_REMOVED")) {
            action = "android.intent.action.PACKAGE_REMOVED";
        } else if (action.equals("android.intent.action.PACKAGE_REPLACED")) {
            action = "android.intent.action.PACKAGE_REPLACED";
        } else {
            return;
        }
        if ((action != "android.intent.action.PACKAGE_ADDED" && action != "android.intent.action.PACKAGE_REMOVED") || !replacing) {
            if (action == "android.intent.action.PACKAGE_REMOVED") {
                if (!Utility.checkPermittedPackage(name)) {
                    return;
                }
            } else if (context.getPackageManager().checkPermission(permission.SUBSTRATE, name) != 0) {
                return;
            }
            Utility.updatePermittedList(context);
            String message = "Substrate extensions updated";
            Notification notification = new Notification.Builder(context)
                    .setSmallIcon(R.drawable.status)
                    .setContentTitle(message)
                    .setContentText("Touch to restart services or select features.")
                    .setContentIntent(PendingIntent.getActivity(context, 0, new Intent(context, SetupActivity.class), 0))
                    .build();
            notification.flags = 48;

            ((NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE)).notify(0, notification);
        }
    }
}
