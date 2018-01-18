package com.saurik.substrate;

import java.io.File;
import java.io.FilenameFilter;

public class Unix {
    public static int EEXIST = 17;
    public static int ENOENT = 2;
    public static int SIGTERM = 15;

    public static native void chmod(String str, int i) throws PosixException;

    public static native void chown(String str, int i, int i2) throws PosixException;

    public static native int getppid();

    public static native boolean grep_F(String str, String str2) throws PosixException;

    public static native void kill(int i, int i2) throws PosixException;

    public static native void mkdir(String str, int i) throws PosixException;

    public static native String readlink(String str) throws PosixException;

    public static native void remount(String str, boolean z) throws PosixException;

    public static native void symlink(String str, String str2) throws PosixException;

    public static native void unlink(String str) throws PosixException;

    static {
        System.load(Utility.lib_ + "/libSubstrateJNI.so");
    }

    public static void ln_sf(String source, String target) throws PosixException {
        rm_f(target);
        symlink(source, target);
    }

    public static boolean mkdir_p(String path, int mode) throws PosixException {
        try {
            mkdir(path, mode);
            return true;
        } catch (PosixException e) {
            if (e.getCode() == EEXIST) {
                return false;
            }
            throw e;
        }
    }

    public static void rm_f(String path) throws PosixException {
        try {
            unlink(path);
        } catch (PosixException e) {
            if (e.getCode() != ENOENT) {
                throw e;
            }
        }
    }

    public static boolean exists(String path) {
        File child = new File(path);
        File parent = child.getParentFile();
        final String name = child.getName();
        String[] list = parent.list(new FilenameFilter() {
            public boolean accept(File directory, String filename) {
                return name.equals(filename);
            }
        });
        return (list == null || list.length == 0) ? false : true;
    }
}
