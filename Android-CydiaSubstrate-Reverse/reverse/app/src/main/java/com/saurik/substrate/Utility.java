package com.saurik.substrate;

import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.os.Build.VERSION;
import android.os.Bundle;
import android.util.Log;

import com.saurik.substrate.Manifest.permission;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.io.PrintWriter;
import java.io.StringWriter;
import java.util.HashSet;
import java.util.Set;

public class Utility {
    public static String data_ = "/data/data/com.saurik.substrate";
    public static String lib_ = (data_ + "/lib");
    private static final String PermittedList_ = (data_ + "/permitted.list");

    public static boolean isInstalled() {
        return false;
    }

    public static boolean checkPermittedPackage(String packageName) {
        boolean find = false;
        synchronized(Utility.class){
            FileReader fr = null;
            BufferedReader br = null;
            try{
                fr = new java.io.FileReader(PermittedList_);
                br = new java.io.BufferedReader(fr);
                String line = br.readLine();
                if(line != null){
                    if(line.equals("0")){
                        while((line = br.readLine()) != null){
                            String[] splitter = line.split(" ");
                            if(splitter[0].equals(packageName))
                                find = true;
                        }
                    }
                }
            }
            catch(FileNotFoundException e){
            }
            catch (IOException e) {
            }
            if(br != null)
            {
                try{
                    br.close();
                }
                catch(IOException e){
                }
            }
            if(fr != null)
                try{
                    fr.close();
                }
                catch(IOException e){
                }
        }
        return find;
    }

    public static boolean unsafeField(String value) {
        return value.length() == 0 || value.contains("\n") || value.contains(" ");
    }

    public static synchronized void updatePermittedList(Context context) {
        synchronized (Utility.class) {
            File file = new File(PermittedList_);
            PackageManager packager = context.getPackageManager();
            try {
                PrintWriter out = new PrintWriter(file);
                out.println("0");
                for (PackageInfo pinfo : packager.getInstalledPackages(0)) {
                    String pname = pinfo.packageName;
                    if (!unsafeField(pname) && packager.checkPermission(permission.SUBSTRATE, pname) == 0) {
                        ApplicationInfo ainfo;
                        try {
                            ainfo = packager.getApplicationInfo(pname, 128);
                        } catch (NameNotFoundException e) {
                            ainfo = null;
                        }
                        Bundle meta = ainfo == null ? null : ainfo.metaData;
                        try {
                            out.print(pname);
                            out.print(" ");
                            out.print(ainfo.sourceDir);
                            if (meta != null) {
                                String main = meta.getString("com.saurik.substrate.main");
                                if (main != null) {
                                    if (main.startsWith(".")) {
                                        main = pname + main;
                                    }
                                    if (!unsafeField(main)) {
                                        out.print(" ");
                                        out.print(main);
                                    }
                                }
                            }
                            out.println();
                        } catch (Throwable th) {
                            out.close();
                        }
                    }
                }
                out.close();
                file.setReadable(true, false);
            } catch (IOException e2) {
                Log.e("CydiaSubstrate", "unable to update substrate extensions");
            }
        }
        return;
    }

    public static String su(String... args) {
        StringWriter console = new StringWriter();
        try {
            int status = Shell.exec("su", console, console, args);
            if (status == 0) {
                return null;
            }
            return "shell status " + status + ":\n" + console.toString();
        } catch (IOException e) {
            return e.toString();
        }
    }

    public static String getSource(Context context) {
        ApplicationInfo ainfo;
        try {
            ainfo = context.getPackageManager().getApplicationInfo("com.saurik.substrate", 0);
        } catch (NameNotFoundException e) {
            ainfo = null;
        }
        return ainfo.sourceDir;
    }

    public static synchronized String run(Context context, String name) {
        String su;
        synchronized (Utility.class) {
            su = su(lib_ + "/libSubstrateRun.so", name);
        }
        return su;
    }

    public static boolean isRooted() {
        return new File("/system/bin/su").exists() || new File("/system/xbin/su").exists();
    }

    public static boolean isLinked() {
        return new File("/vendor/lib/liblog!.so").exists() && new File("/vendor/lib/liblog.so").exists() && new File("/system/lib/libsubstrate.so").exists() && new File("/system/lib/libsubstrate-dvm.so").exists();
    }

    public static boolean isSystemPath() {
        String paths = System.getenv("LD_LIBRARY_PATH");
        if (paths == null) {
            return false;
        }
        for (String path : paths.split(":")) {
            if (path.equals("/system/lib")) {
                return true;
            }
            if (path.equals("/vendor/lib")) {
                return false;
            }
        }
        return false;
    }

    public static String getCanonicalPath(String path) {
        try {
            return new File(path).getCanonicalPath();
        } catch (IOException e) {
            return null;
        }
    }

    public static boolean isPhysicalVendor() {
        String vendor = getCanonicalPath("/vendor");
        if (vendor == null) {
            return true;
        }
        if (!vendor.equals("/vendor")) {
            return false;
        }
        String lib = getCanonicalPath("/vendor/lib");
        if (lib == null || lib.equals("/vendor/lib")) {
            return true;
        }
        return false;
    }

    public static boolean isLinkerPatched() {
        try {
            return Unix.grep_F("/system/bin/linker", "CY_LIBRARY_PATH");
        } catch (PosixException e) {
            return false;
        }
    }

    public static Set<String> getSymbols(String library) {
        Set<String> set = null;
        StringWriter data = new StringWriter();
        try {
            if (Shell.exec("sh", data, null, lib_ + "/libSubstrateRun.so", "nm", library) == 0) {
                set = new HashSet();
                for (String symbol : data.toString().split("\n")) {
                    set.add(symbol);
                }
                set.remove("");
            }
        } catch (IOException e) {
        }
        return set;
    }

    public static Set<String> getMissingSymbols() {
        Set<String> required = getSymbols("/system/lib/liblog.so");
        if (required == null) {
            return null;
        }
        Set<String> provided = getSymbols(lib_ + "/libAndroidBootstrap0.so");
        if (provided == null) {
            return null;
        }
        Set<String> missing = new HashSet();
        missing.addAll(required);
        missing.removeAll(provided);
        missing.remove("__on_dlclose");
        return missing;
    }

    public static synchronized String restart() {
        String su;
        synchronized (Utility.class) {
            if (Integer.valueOf(VERSION.SDK).intValue() >= 11) {
                su = su("/system/bin/setprop", "ctl.restart", "zygote");
            } else {
                su = su("/system/bin/kill", "-" + Unix.SIGTERM, "-" + Unix.getppid());
            }
        }
        return su;
    }
}
