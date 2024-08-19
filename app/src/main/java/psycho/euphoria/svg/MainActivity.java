package psycho.euphoria.svg;

import android.Manifest.permission;
import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.DownloadManager;
import android.app.NotificationManager;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.res.AssetManager;
import android.net.Uri;
import android.os.Build;
import android.os.Build.VERSION;
import android.os.Build.VERSION_CODES;
import android.os.Bundle;
import android.os.Environment;
import android.os.ParcelFileDescriptor;
import android.os.StrictMode;
import android.preference.Preference;
import android.preference.PreferenceManager;
import android.provider.Settings;
import android.util.Log;
import android.view.ContextMenu;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.webkit.DownloadListener;
import android.webkit.WebChromeClient;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.widget.FrameLayout;
import android.widget.FrameLayout.LayoutParams;
import android.widget.Toast;


import com.tom_roush.pdfbox.android.PDFBoxResourceLoader;
import com.tom_roush.pdfbox.cos.COSName;
import com.tom_roush.pdfbox.pdmodel.PDDocument;
import com.tom_roush.pdfbox.pdmodel.PDPage;
import com.tom_roush.pdfbox.pdmodel.PDResources;
import com.tom_roush.pdfbox.pdmodel.graphics.PDXObject;
import com.tom_roush.pdfbox.pdmodel.graphics.form.PDFormXObject;
import com.tom_roush.pdfbox.pdmodel.graphics.image.PDImageXObject;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Set;

import static psycho.euphoria.svg.CustomWebChromeClient.FILE_CHOOSER_REQUEST_CODE;


public class MainActivity extends Activity {
    public static final int DEFAULT_PORT = 8090;
    public static final String POST_NOTIFICATIONS = "android.permission.POST_NOTIFICATIONS";

    static {
/*
加载编译Rust代码后得到共享库。它完整的名称为librust.so
  */
        System.loadLibrary("nativelib");
    }

    private WebView mWebView1;
    private WebView mWebView2;
    private WebView mWebView3;
    private WebView mWebView4;
    CustomWebChromeClient mCustomWebChromeClient1;
    CustomWebChromeClient mCustomWebChromeClient2;
    CustomWebChromeClient mCustomWebChromeClient3;
    CustomWebChromeClient mCustomWebChromeClient4;
    private FrameLayout mFrameLayout;

    public static void aroundFileUriExposedException() {
        StrictMode.VmPolicy.Builder builder = new StrictMode.VmPolicy.Builder();
        StrictMode.setVmPolicy(builder.build());
        // AroundFileUriExposedException.aroundFileUriExposedException(MainActivity.this);
    }

    public static void enableNotification(Context context) {
        try {
            Intent intent = new Intent();
            intent.setAction(Settings.ACTION_APP_NOTIFICATION_SETTINGS);
            intent.putExtra(Settings.EXTRA_APP_PACKAGE, context.getPackageName());
            intent.putExtra(Settings.EXTRA_CHANNEL_ID, context.getApplicationInfo().uid);
            intent.putExtra("app_package", context.getPackageName());
            intent.putExtra("app_uid", context.getApplicationInfo().uid);
            context.startActivity(intent);
        } catch (Exception e) {
            e.printStackTrace();
            Intent intent = new Intent();
            intent.setAction(Settings.ACTION_APPLICATION_DETAILS_SETTINGS);
            Uri uri = Uri.fromParts("package", context.getPackageName(), null);
            intent.setData(uri);
            context.startActivity(intent);
        }
    }

    public static String getAddress(Context context) {
        String host = Shared.getDeviceIP(context);
        return String.format("http://%s:%d", host, DEFAULT_PORT);
    }

    public void getAllImageFromFile(String pathToFile, int start, int end, String dir) {
        try (PDDocument document = PDDocument.load(new File(pathToFile))) {
            int j = 1;
            for (int i = start; i < Math.min(document.getNumberOfPages(), end); i++) {
                PDPage page = document.getPage(i);
                List<PDImageXObject> pageImages = getImagesFromResources(page.getResources());
                for (PDImageXObject pageImage : pageImages) {
                    try (FileOutputStream outputStream = new FileOutputStream(dir + "/" + (j++) + ".png")) {
                        //Bitmap bitmap =
                        //Bitmap bmp = new JP2Decoder(jp2data).decode();
                        //b.compress(CompressFormat.PNG, 100, outputStream);
                        //b.recycle();
                        Shared.copyStreams(pageImage.createInputStream(), outputStream);
                    }
                }
                //images.put(i, pageImages.isEmpty() ? new ArrayList<>() : pageImages);
            }
        } catch (IOException e) {
            throw new RuntimeException("Can't get images from file: " + e.toString());
        }
    }

    public static WebView initializeWebView(MainActivity context) {
        WebView webView = new WebView(context);
        webView.addJavascriptInterface(new WebAppInterface(context), "NativeAndroid");
        webView.setWebViewClient(new CustomWebViewClient(context));
        return webView;
    }

    public static void launchServer(MainActivity context) {
        Intent intent = new Intent(context, AppService.class);
        context.startService(intent);
    }

    public static void requestNotificationPermission(Activity activity) {
        if (Build.VERSION.SDK_INT >= 33) {
            if (activity.checkSelfPermission(POST_NOTIFICATIONS) == PackageManager.PERMISSION_DENIED) {
                if (!activity.shouldShowRequestPermissionRationale(POST_NOTIFICATIONS)) {
                    enableNotification(activity);
                } else {
                    activity.requestPermissions(new String[]{POST_NOTIFICATIONS}, 100);
                }
            }
        } else {
            boolean enabled = activity.getSystemService(NotificationManager.class).areNotificationsEnabled();
            if (!enabled) {
                enableNotification(activity);
            }
        }
    }

    public static void requestStorageManagerPermission(Activity context) {
        // RequestStorageManagerPermission.requestStorageManagerPermission(MainActivity.this);
        if (VERSION.SDK_INT >= VERSION_CODES.R) {
            // 测试是否已获取所有文件访问权限 Manifest.permission.MANAGE_EXTERNAL_STORAGE
            // 该权限允许程序访问储存中的大部分文件
            // 但不包括 Android/data 目录下程序的私有数据目录
            if (!Environment.isExternalStorageManager()) {
                try {
                    Uri uri = Uri.parse("package:" + context.getPackageName());
                    Intent intent = new Intent(Settings.ACTION_MANAGE_APP_ALL_FILES_ACCESS_PERMISSION, uri);
                    context.startActivity(intent);
                } catch (Exception ex) {
                    Intent intent = new Intent();
                    intent.setAction(Settings.ACTION_MANAGE_ALL_FILES_ACCESS_PERMISSION);
                    context.startActivity(intent);
                }
            }
        }
    }

    @SuppressLint("SetJavaScriptEnabled")
    public void setWebView(WebView webView) {
        WebSettings settings = webView.getSettings();
        settings.setJavaScriptEnabled(true);
        settings.setDomStorageEnabled(true);
        settings.setCacheMode(WebSettings.LOAD_DEFAULT);
        settings.setUserAgentString("Mozilla/5.0 (Linux; Android 9; SM-G950N) AppleWebKit/537.36 (KHTML, like Gecko) Version/4.0 Chrome/88.0.4324.93 Mobile Safari/537.36");
        //settings.setUserAgentString(USER_AGENT);
        settings.setSupportZoom(false);
        webView.setDownloadListener(new DownloadListener() {

            @Override
            public void onDownloadStart(String url, String userAgent,
                                        String contentDisposition, String mimetype,
                                        long contentLength) {
                //Log.e("B5aOx2", String.format("onDownloadStart, %s\n%s", url,contentDisposition));
                /*DownloadManager.Request request = new DownloadManager.Request(
                        Uri.parse(url));
                request.allowScanningByMediaScanner();
                request.setNotificationVisibility(DownloadManager.Request.VISIBILITY_VISIBLE_NOTIFY_COMPLETED); //Notify client once download is completed!
                request.setDestinationInExternalPublicDir(Environment.DIRECTORY_DOWNLOADS, "Name of your downloadble file goes here, example: Mathematics II ");
                DownloadManager dm = (DownloadManager) getSystemService(DOWNLOAD_SERVICE);
                dm.enqueue(request);
                Toast.makeText(getApplicationContext(), "Downloading File", //To notify the Client that the file is being downloaded
                        Toast.LENGTH_LONG).show();
*/
            }
        });
        registerForContextMenu(webView);
    }

    public static native String startServer(Context context, AssetManager assetManager, String host, int port);

    private List<PDImageXObject> getImagesFromResources(PDResources resources) throws IOException {
        List<PDImageXObject> images = new ArrayList<>();
        for (COSName xObjectName : resources.getXObjectNames()) {
            PDXObject xObject = resources.getXObject(xObjectName);
            if (xObject instanceof PDFormXObject) {
                images.addAll(getImagesFromResources(((PDFormXObject) xObject).getResources()));
            } else if (xObject instanceof PDImageXObject) {
                PDImageXObject image = ((PDImageXObject) xObject);
                images.add(image);
            }
        }
        return images;
    }

    private void initialize() {
        requestNotificationPermission(this);
        List<String> permissions = new ArrayList<>();
        if (checkSelfPermission(permission.CAMERA) != PackageManager.PERMISSION_GRANTED) {
            permissions.add(permission.CAMERA);
        }
        if (checkSelfPermission(permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
            permissions.add(permission.WRITE_EXTERNAL_STORAGE);
        }
        if (VERSION.SDK_INT < VERSION_CODES.R && checkSelfPermission(permission.WRITE_EXTERNAL_STORAGE) != PackageManager.PERMISSION_GRANTED) {
            permissions.add(permission.WRITE_EXTERNAL_STORAGE);
        }
        if (!permissions.isEmpty()) {
            requestPermissions(permissions.toArray(new String[0]), 0);
        }
        aroundFileUriExposedException();
        requestStorageManagerPermission(this);
        File dir = new File(Environment.getExternalStorageDirectory(), ".editor");
        if (!dir.isDirectory())
            dir.mkdir();
        launchServer(this);
        mWebView1 = initializeWebView(this);
        mCustomWebChromeClient1 = new CustomWebChromeClient(this);
        mWebView1.setWebChromeClient(mCustomWebChromeClient1);
        setWebView(mWebView1);
        mWebView2 = initializeWebView(this);
        mCustomWebChromeClient2 = new CustomWebChromeClient(this);
        mWebView2.setWebChromeClient(mCustomWebChromeClient2);
        mWebView3 = initializeWebView(this);
        mCustomWebChromeClient3 = new CustomWebChromeClient(this);
        mWebView3.setWebChromeClient(mCustomWebChromeClient3);
        mWebView4 = initializeWebView(this);
        mCustomWebChromeClient4 = new CustomWebChromeClient(this);
        mWebView4.setWebChromeClient(mCustomWebChromeClient2);
        mFrameLayout = new FrameLayout(this);
        mFrameLayout.addView(mWebView1);
        mWebView2.setVisibility(View.INVISIBLE);
        mFrameLayout.addView(mWebView2);
        mWebView3.setVisibility(View.INVISIBLE);
        mFrameLayout.addView(mWebView3);
        mWebView4.setVisibility(View.INVISIBLE);
        mFrameLayout.addView(mWebView4);
        setWebView(mWebView2);
        setContentView(mFrameLayout, new LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT));
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        CustomWebChromeClient customWebChromeClient = mWebView1.getVisibility() == View.VISIBLE ?
                mCustomWebChromeClient1 : mCustomWebChromeClient2;
        if (requestCode == FILE_CHOOSER_REQUEST_CODE && resultCode == RESULT_OK) {
            if (customWebChromeClient.ValueCallback == null) {
                super.onActivityResult(requestCode, resultCode, data);
                return;
            }
            Uri[] results = WebChromeClient.FileChooserParams.parseResult(resultCode, data);
            if (customWebChromeClient.ValueCallback != null)
                customWebChromeClient.ValueCallback.onReceiveValue(results);
            customWebChromeClient.ValueCallback = null;
        } else
            super.onActivityResult(requestCode, resultCode, data);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        initialize();
    }

    @Override
    public void onBackPressed() {
        WebView webView = mWebView1.getVisibility() == View.VISIBLE ? mWebView1 : mWebView2;
        if (webView != null && webView.canGoBack()) {
            webView.goBack();
            return;
        }
        super.onBackPressed();
    }

    @Override
    public void onCreateContextMenu(ContextMenu menu, View v, ContextMenuInfo menuInfo) {
        super.onCreateContextMenu(menu, v, menuInfo);
        WebView webView = mWebView1.getVisibility() == View.VISIBLE ? mWebView1 : mWebView2;
        final WebView.HitTestResult webViewHitTestResult = webView.getHitTestResult();
        if (webViewHitTestResult.getType() == WebView.HitTestResult.IMAGE_TYPE ||
                webViewHitTestResult.getType() == WebView.HitTestResult.SRC_IMAGE_ANCHOR_TYPE) {
            Shared.setText(this, webViewHitTestResult.getExtra());
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        menu.add(0, 1, 0, "刷新");
        menu.add(0, 2, 0, "首页").setShowAsAction(MenuItem.SHOW_AS_ACTION_ALWAYS);
        menu.add(0, 3, 0, "复制");
        MenuItem menuItem = menu.add(0, 4, 0, "阅读");
        menuItem.setShowAsAction(MenuItem.SHOW_AS_ACTION_ALWAYS);
        MenuItem menuItem1 = menu.add(0, 8, 0, "搜索");
        menuItem1.setShowAsAction(MenuItem.SHOW_AS_ACTION_ALWAYS);
        MenuItem menuItem2 = menu.add(0, 9, 0, "智能");
        menuItem2.setShowAsAction(MenuItem.SHOW_AS_ACTION_ALWAYS);
        menu.add(0, 5, 0, "打开");
        menu.add(0, 6, 0, "收藏");
        menu.add(0, 7, 0, "历史");
        return super.onCreateOptionsMenu(menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        WebView webView = mWebView1.getVisibility() == View.VISIBLE ? mWebView1 : mWebView2;
        switch (item.getItemId()) {
            case 1:
                webView.reload();
                break;
            case 2:
                mWebView2.setVisibility(View.INVISIBLE);
                mWebView3.setVisibility(View.INVISIBLE);
                mWebView4.setVisibility(View.INVISIBLE);
                mWebView1.setVisibility(View.VISIBLE);
                if (mWebView1.getUrl() == null || !mWebView1.getUrl().startsWith("http://0.0.0.0:8090"))
                    mWebView1.loadUrl("http://0.0.0.0:8090");
                break;
            case 3:
                Shared.setText(this, webView.getUrl());
                break;
            case 4:
                mWebView1.setVisibility(View.INVISIBLE);
                mWebView3.setVisibility(View.INVISIBLE);
                mWebView4.setVisibility(View.INVISIBLE);
                mWebView2.setVisibility(View.VISIBLE);
                if (mWebView2.getUrl() == null || !mWebView2.getUrl().startsWith("http://0.0.0.0:8500"))
                    mWebView2.loadUrl("http://0.0.0.0:8500/app.html");
                break;
            case 5:
                webView.loadUrl("http://0.0.0.0:8500/app.html");
                break;
            case 6:
                PreferenceManager.getDefaultSharedPreferences(this)
                        .edit().putString("uri", webView.getUrl())
                        .apply();
                break;
            case 7:
                webView.loadUrl(PreferenceManager.getDefaultSharedPreferences(this)
                        .getString("uri", "http://0.0.0.0:8500/app.html"))
                ;
                break;
            case 8:
                mWebView1.setVisibility(View.INVISIBLE);
                mWebView2.setVisibility(View.INVISIBLE);
                mWebView4.setVisibility(View.INVISIBLE);
                mWebView3.setVisibility(View.VISIBLE);
                if (mWebView3.getUrl() == null || !mWebView3.getUrl().startsWith("https://www.google.com"))
                    mWebView3.loadUrl("https://www.google.com/search?q=");
                break;
            case 9:
                mWebView1.setVisibility(View.INVISIBLE);
                mWebView2.setVisibility(View.INVISIBLE);
                mWebView3.setVisibility(View.INVISIBLE);
                mWebView4.setVisibility(View.VISIBLE);
                if (mWebView4.getUrl() == null || !mWebView4.getUrl().startsWith("https://gemini.google.com"))
                    mWebView4.loadUrl("https://gemini.google.com/");
                break;
        }
        return super.onOptionsItemSelected(item);
    }
}