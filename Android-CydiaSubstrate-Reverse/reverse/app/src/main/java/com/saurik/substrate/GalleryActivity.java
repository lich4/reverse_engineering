package com.saurik.substrate;

/**
 * Created by lichao on 2016/5/9.
 */
import android.app.Activity;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.webkit.WebView;

public class GalleryActivity extends Activity {
    public int ID_WEBVIEW = R.id.ID_WEBVIEW;
    public final int MENU_RELOAD = 0x377A9303;

    protected void onCreate(Bundle state) {
        super.onCreate(state);
        WebView web = new WebView(this);
        web.setId(ID_WEBVIEW);
        setContentView(web);
        web.setBackgroundColor(MENU_RELOAD);
        web.getSettings().setJavaScriptEnabled(true);
        web.loadUrl("https://cydia.saurik.com/ui/android/0.8/");
    }

    private WebView getWebView() {
        return (WebView) findViewById(ID_WEBVIEW);
    }

    public boolean onCreateOptionsMenu(Menu menu) {
        menu.add(0, MENU_RELOAD, 0, "Reload");
        return true;
    }

    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case MENU_RELOAD:
                getWebView().reload();
                return true;
            default:
                return super.onOptionsItemSelected(item);
        }
    }
}