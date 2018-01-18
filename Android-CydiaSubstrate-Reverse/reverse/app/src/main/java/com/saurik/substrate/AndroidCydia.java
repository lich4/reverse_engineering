package com.saurik.substrate;

import android.content.Context;
import android.content.Intent;
import android.content.pm.FeatureInfo;
import android.content.pm.PackageInfo;
import android.content.pm.PermissionInfo;
import android.net.Uri;
import android.os.Build;
import android.os.SystemProperties;
import android.provider.Settings;
import android.webkit.BrowserFrame;
import android.webkit.WebViewCore;

import java.net.MalformedURLException;
import java.net.URL;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

class AndroidCydia {
    private static CydiaObject cydia_;

    private static class CydiaObject {
        private Context context_;

        private class CydiaPackage {
            static final int FLAGS = 4096;
            private PackageInfo info_;

            CydiaPackage(PackageInfo info) {
                this.info_ = info;
            }

            public String getPackageName() {
                return this.info_.packageName;
            }

            public int getVersionCode() {
                return this.info_.versionCode;
            }

            public String getVersionName() {
                return this.info_.versionName;
            }

            public List<CydiaPermission> getPermissions() {
                if (this.info_.permissions == null) {
                    return null;
                }
                List<CydiaPermission> value = new ArrayList();
                for (PermissionInfo permission : this.info_.permissions) {
                    value.add(new CydiaPermission(permission));
                }
                return value;
            }

            public List<String> getRequestedPermissions() {
                if (this.info_.requestedPermissions == null) {
                    return null;
                }
                return Arrays.asList(this.info_.requestedPermissions);
            }

            public void remove() {
                CydiaObject.this.context_.startActivity(new Intent("android.intent.action.DELETE", Uri.parse("package:" + getPackageName())));
            }
        }

        private class CydiaPermission {
            private PermissionInfo info_;

            CydiaPermission(PermissionInfo info) {
                this.info_ = info;
            }

            public String getDescription() {
                CharSequence description = this.info_.loadDescription(CydiaObject.this.context_.getPackageManager());
                if (description == null) {
                    return null;
                }
                return description.toString();
            }

            public String getGroup() {
                return this.info_.group;
            }

            public String getName() {
                return this.info_.name;
            }

            public String getNonLocalizedDescription() {
                if (this.info_.nonLocalizedDescription == null) {
                    return null;
                }
                return this.info_.nonLocalizedDescription.toString();
            }

            public String getNonLocalizedLabel() {
                if (this.info_.nonLocalizedLabel == null) {
                    return null;
                }
                return this.info_.nonLocalizedLabel.toString();
            }

            public String getPackageName() {
                return this.info_.packageName;
            }

            public int getProtectionLevel() {
                return this.info_.protectionLevel;
            }
        }

        private CydiaObject(Context context) {
            this.context_ = context;
        }

        public int getVersion() {
            return 1;
        }

        public String getPlatform() {
            return "android";
        }

        public int getVersionCode() {
            return Version.CODE;
        }

        public String getVersionName() {
            return Version.NAME;
        }

        public String getAndroidBuild(String name) {
            try {
                String value = Build.class.getField(name).get(null).toString();
                if (value == null || value.equals("unknown")) {
                    return null;
                }
                return value;
            } catch (Exception e) {
                return null;
            }
        }

        public List<String> getAndroidFeatures() {
            FeatureInfo[] features = this.context_.getPackageManager().getSystemAvailableFeatures();
            if (features == null) {
                return null;
            }
            List<String> value = new ArrayList();
            for (FeatureInfo feature : features) {
                value.add(feature.name);
            }
            return value;
        }

        public boolean hasAndroidFeature(String name) {
            return this.context_.getPackageManager().hasSystemFeature(name);
        }

        public String getAndroidInstaller(String name) {
            return this.context_.getPackageManager().getInstallerPackageName(name);
        }

        public CydiaPackage getAndroidPackage(String name) {
            try {
                PackageInfo _package = this.context_.getPackageManager().getPackageInfo(name, 4096);
                if (_package == null) {
                    return null;
                }
                return new CydiaPackage(_package);
            } catch (Exception e) {
                return null;
            }
        }

        public List<CydiaPackage> getAndroidPackages() {
            List<PackageInfo> packages = this.context_.getPackageManager().getInstalledPackages(4096);
            if (packages == null) {
                return null;
            }
            List<CydiaPackage> value = new ArrayList();
            for (PackageInfo _package : packages) {
                value.add(new CydiaPackage(_package));
            }
            return value;
        }

        public CydiaPermission getAndroidPermission(String name) {
            try {
                PermissionInfo permission = this.context_.getPackageManager().getPermissionInfo(name, 0);
                if (permission == null) {
                    return null;
                }
                return new CydiaPermission(permission);
            } catch (Exception e) {
                return null;
            }
        }

        public String getAndroidProperty(String name) {
            return System.getProperty(name);
        }

        public String getAndroidSecure(String name) {
            return Settings.Secure.getString(this.context_.getContentResolver(), name);
        }

        public String getAndroidSystem(String name) {
            String value = SystemProperties.get(name);
            return (value == null || value.length() == 0 || value.equals("unknown")) ? null : value;
        }
    }

    AndroidCydia() {
    }


    public static Object windowObjectCleared(BrowserFrame frame) {
        if (!frame.mIsMainFrame) {
            return null;
        }
        String uri;
        WebViewCore core = frame.mWebViewCore;
        try {
            uri = core.mWebView.getUrl();
        } catch (NoSuchFieldError e) {
            try {
                uri = core.mWebViewClassic.getUrl();
            } catch (Exception e2) {
                return null;
            }
        }
        try {
            URL url = new URL(uri);
            if (!url.getProtocol().equals("https")) {
                return null;
            }
            boolean cydia = false;
            String host = url.getHost();
            for (String domain : new String[]{"cydia.com", "www.cydia.com", "cydia.saurik.com"}) {
                if (host.equals(domain)) {
                    cydia = true;
                    break;
                }
            }
            if (!cydia) {
                return null;
            }
            if (cydia_ == null) {
                cydia_ = new CydiaObject(frame.mContext);
            }
            return cydia_;
        } catch (MalformedURLException e3) {
            return null;
        }
    }

}
