package psycho.euphoria.svg;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.IBinder;

import android.os.Process;
import android.util.Log;

public class AppService extends Service {

    public static final String ACTION_DISMISS = "psycho.euphoria.app.ServerService.ACTION_DISMISS";
    public static final String ACTION_APP = "psycho.euphoria.app.ServerService.ACTION_APP";
    public static final String KP_NOTIFICATION_CHANNEL_ID = "notes_notification_channel";

    public static void createNotification(AppService context) {
        Notification notification = new Notification.Builder(context, KP_NOTIFICATION_CHANNEL_ID).setContentTitle("笔记")
                .setSmallIcon(android.R.drawable.stat_sys_download)
                .addAction(getAction("关闭", getPendingIntent(context, ACTION_DISMISS)))
                .setContentIntent(getPendingIntent(context, ACTION_APP))
                .build();
        context.startForeground(1, notification);
    }

    public static void createNotificationChannel(Context context) {
        NotificationChannel notificationChannel = new NotificationChannel(KP_NOTIFICATION_CHANNEL_ID, "SVG", NotificationManager.IMPORTANCE_LOW);
        ((NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE)).createNotificationChannel(notificationChannel);
    }

    public static Notification.Action getAction(String name, PendingIntent piDismiss) {
        return new Notification.Action.Builder(null, name, piDismiss).build();
    }

    public static PendingIntent getPendingIntent(Context context, String action) {
        Intent dismissIntent = new Intent(context, AppService.class);
        dismissIntent.setAction(action);
        return PendingIntent.getService(context, 0, dismissIntent, PendingIntent.FLAG_IMMUTABLE);
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        createNotificationChannel(this);
        String host = Shared.getDeviceIP(this);
        new Thread(() -> {
            MainActivity.startServer(this, getAssets(), host, MainActivity.DEFAULT_PORT);
        }).start();
        createNotification(this);

    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        if (intent != null && intent.getAction() != null) {
            if (intent.getAction().equals(ACTION_DISMISS)) {
                stopForeground(true);
                stopSelf();
                Process.killProcess(Process.myPid());
                return START_NOT_STICKY;
            } else if (intent.getAction().equals(ACTION_APP)) {
                Intent app = new Intent(this, MainActivity.class);
                app.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_SINGLE_TOP);
                startActivity(app);
            }

        }
        return super.onStartCommand(intent, flags, startId);
    }
}