package com.saurik.substrate;

import android.app.AlertDialog.Builder;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.os.Bundle;
import android.os.SystemProperties;
import android.support.v4.app.FragmentActivity;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.LinearLayout.LayoutParams;
import android.widget.TextView;

import java.lang.reflect.Constructor;
import java.lang.reflect.Method;
import java.util.Set;

public class SetupActivity extends FragmentActivity {
    static LayoutParams fill_ = new LayoutParams(-1, -2);
    private String ACTION_RESTART = "com.saurik.substrate.RESTART";

    protected Dialog onCreateDialog(int id, final Bundle bundle) {
        switch (id) {
            case 0:
                return new Builder(this).setTitle("Script Failure").setMessage(bundle.getString("error")).setPositiveButton("Retry?", new OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        dialog.dismiss();
                        SetupActivity.this.run(bundle.getString("action"));
                    }
                }).setNegativeButton("Cancel", new OnClickListener() {
                    public void onClick(DialogInterface dialog, int which) {
                        dialog.dismiss();
                    }
                }).create();
            case 1:
                return ProgressDialog.show(this, "", "Reloading Data");
            default:
                return null;
        }
    }

    private void run(String action) {
        String error = Utility.run(this, action);
        setContentView();
        if (error != null) {
            Bundle bundle = new Bundle();
            bundle.putString("action", action);
            bundle.putString("error", error);
            removeDialog(0);
            showDialog(0, bundle);
        }
    }

    private void help(ViewGroup group, String text) {
        TextView help = new TextView(this);
        group.addView(help);
        help.setText(text);
    }

    static {
        fill_.setMargins(0, 0, 0, 0);
    }

    private void button(ViewGroup group, String text, final Runnable click) {
        Button button = new Button(this);
        group.addView(button, fill_);
        button.setText(text);
        button.setTextSize(18.0f);
        button.setOnClickListener(new View.OnClickListener() {
            public void onClick(View view) {
                click.run();
            }
        });
    }

    private void action(ViewGroup group, final String action, String text) {
        final SetupActivity thiz = this;
        button(group, text, new Runnable() {
            public void run() {
                thiz.run("do_" + action);
            }
        });
    }

    private void intent(ViewGroup group, String text, final boolean activity, final Intent intent) {
        button(group, text, new Runnable() {
            public void run() {
                if (activity) {
                    SetupActivity.this.startActivity(intent);
                } else {
                    SetupActivity.this.sendBroadcast(intent);
                }
            }
        });
    }

    protected void onCreate(Bundle state) {
        super.onCreate(state);
        setContentView();
    }

    public void setContentView() {
        LinearLayout content = new LinearLayout(this);
        setContentView(content);
        content.setOrientation(1);
        Utility.updatePermittedList(this);
        boolean failed = false;
        if (Utility.isPhysicalVendor()) {
            help(content, "Note: you have a 'physical /vendor', which means that you will have to re-Link Substrate every time you reboot your phone.");
        }
        Set<String> missing = Utility.getMissingSymbols();
        if (missing == null) {
            help(content, "Note: something about your device made it impossible for Substrate to perform its internal safety check; can you please contact saurik via e-mail?");
            failed = true;
        } else if (missing.size() != 0) {
            StringBuilder builder = new StringBuilder();
            for (String symbol : missing) {
                builder.append("\n   ");
                builder.append(symbol);
            }
            help(content, "Note: your device is not fully compatible with this version of Substrate. Please contact saurik via e-mail and include the following symbols:\n" + builder.toString());
            failed = true;
        }
        if (Utility.isLinkerPatched()) {
            help(content, "Note: you have patched your system linker to not honor LD_LIBRARY_PATH; this may (unlikely) cause something to break, and may (likely) make OTA updates fail.");
            action(content, "unpatch", "Unpatch System Linker");
        } else if (Utility.isSystemPath()) {
            String paths = System.getenv("LD_LIBRARY_PATH");
            if (paths.equals("/system/lib")) {
                help(content, "Note: your device overrides the library path and hardcodes it to first examine /system/lib; one fix for this is to patch the system linker to ignore the override.");
                action(content, "patch", "Patch System Linker");
            } else {
                help(content, "Note: your device overrides the library path and moves /system/lib before /vendor/lib, adding extra things to the path; this breaks Substrate. Can you report this to saurik via e-mail?\n   " + paths);
                failed = true;
            }
        }
        if (Utility.isLinked()) {
            action(content, "unlink", "Unlink Substrate Files");
            intent(content, "Restart System (Soft)", false, new Intent(this.ACTION_RESTART));
            if (Utility.isInstalled()) {
                intent(content, "Open Cydia Gallery", true, new Intent(this, GalleryActivity.class));
            }
            else {
                help(content, "Substrate is installed and linked, but is not currently active. To browse the Gallery or for extensions to start working, you will need to restart.");
            }
            help(content, "Remember (really, this is the one actually important thing): if your device ever can't boot due to a broken Substrate extension, you can hold your volume-up key to temporarily disable Substrate, allowing your activity to complete.");
        } else if (!failed) {
            if (Utility.isRooted()) {
                action(content, "link", "Link Substrate Files");
                if (Utility.isInstalled()) {
                    intent(content, "Restart System (Soft)", false, new Intent(this.ACTION_RESTART));
                    return;
                }
                return;
            }
            help(content, "To link Substrate, you must have a rooted device with a copy of \"su\" (SuperUser) installed. Unfortunately, it is difficult to tell you exactly how to root your specific device; you may be able to find a guide online.");
        }
    }
}
