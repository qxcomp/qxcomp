package org.envsh.jvkt;

import android.content.Context
import android.util.Log
import androidx.work.Worker
import androidx.work.WorkerParameters
import androidx.work.*
import java.util.concurrent.TimeUnit

import android.app.NotificationManager
import android.content.Context.NOTIFICATION_SERVICE

import android.os.Build
import yti.fedim.qmlui.R; // 这个包名是在哪儿设置的都忘记了
import androidx.core.app.NotificationCompat
import androidx.annotation.RequiresApi

class SendLogWorker(context: Context, userParameters: WorkerParameters) : Worker(context, userParameters) {

    override fun doWork(): Result {
        try {
            for (i in 0..500) {
                Log.i("furkanpasa", "Sending log $i")
            }
            return Result.success()
        } catch (e: Exception) {
            return Result.failure()
        }
    }

}

    public fun setupPeriodicallySendingLogs(wkman : WorkManager) {
        // val workManager = WorkManager.getInstance(this)
        val workManager = wkman

        val TAG_SEND_LOG = "tag.send.log"

        // OneTimeWorkRequestBuilder
        val sendingLog = PeriodicWorkRequestBuilder<SendLogWorker>(15, TimeUnit.MINUTES)
            .addTag(TAG_SEND_LOG)
            .build()

        workManager.enqueue(sendingLog)

        // workManager.getWorkInfoByIdLiveData(sendingLog.id)
        //     .observe(this, Observer { workInfo ->
        //         binding.textViewWorkState.text = workInfo.state.name
        //     })
    }

// see https://github.com/FPasaoglu/Work-Manager-Example/blob/master/app/src/main/java/com/example/workmanagerexample/SendLogWorker.kt


/////////
class DownloadWorker1(val context: Context, userParameters: WorkerParameters) :
    Worker(context, userParameters) {

    override fun doWork(): Result {
        try {

            val downloadTitle = inputData.getString("KEY_DOWNLOAD_TITLE") ?: return Result.failure()
            val downloadDesc = inputData.getString("KEY_DOWNLOAD_DESC") ?: return Result.failure()

            for (i in 0..300) {
                Log.i("furkanpasa", "Downloading image $i")
            }

            val notificationManager: NotificationManager =
                context.getSystemService(NOTIFICATION_SERVICE) as NotificationManager

            // notificationManager.sendNotification(context, downloadTitle, downloadDesc)

            return Result.success()
        } catch (e: Exception) {
            return Result.failure()
        }
    }
}


///////
// see https://developer.android.com/develop/background-work/background-tasks/persistent/how-to/long-running?hl=zh-cn
class DownloadWorker(context: Context, parameters: WorkerParameters) :
   CoroutineWorker(context, parameters) {

   private val notificationManager =
       context.getSystemService(Context.NOTIFICATION_SERVICE) as
               NotificationManager

   override suspend fun doWork(): Result {
       val inputUrl = inputData.getString(KEY_INPUT_URL)
                      ?: return Result.failure()
       val outputFile = inputData.getString(KEY_OUTPUT_FILE_NAME)
                      ?: return Result.failure()
       // Mark the Worker as important
       val progress = "Starting Download"
       setForeground(createForegroundInfo(progress))
       download(inputUrl, outputFile)
       return Result.success()
   }

   private fun download(inputUrl: String, outputFile: String) {
       // Downloads a file and updates bytes read
       // Calls setForeground() periodically when it needs to update
       // the ongoing Notification
   }
   // Creates an instance of ForegroundInfo which can be used to update the
   // ongoing notification.
   private fun createForegroundInfo(progress: String): ForegroundInfo {
    //    val id = applicationContext.getString(R.string.notification_channel_id)
    //    val title = applicationContext.getString(R.string.notification_title)
    //    val cancel = applicationContext.getString(R.string.cancel_download)
    val id = "notichanid12345"
    val title = "notichantitle"
    val cancel = "canceldl"

       // This PendingIntent can be used to cancel the worker
       val intent = WorkManager.getInstance(applicationContext)
               .createCancelPendingIntent(getId())

       // Create a Notification channel if necessary
       if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
           createChannel()
       }

       val notification = NotificationCompat.Builder(applicationContext, id)
           .setContentTitle(title)
           .setTicker(title)
           .setContentText(progress)
        //    .setSmallIcon(R.drawable.ic_work_notification)
        .setSmallIcon(R.drawable.ic_launcher)
           .setOngoing(true)
           // Add the cancel action to the notification which can
           // be used to cancel the worker
        //    .addAction(android.R.drawable.ic_delete, cancel, intent)
        .addAction(R.drawable.ic_launcher, cancel, intent)
           .build()

        val notificationId = 12345
       return ForegroundInfo(notificationId, notification)
   }

   @RequiresApi(Build.VERSION_CODES.O)
   private fun createChannel() {
       // Create a Notification channel
   }

   companion object {
       const val KEY_INPUT_URL = "KEY_INPUT_URL"
       const val KEY_OUTPUT_FILE_NAME = "KEY_OUTPUT_FILE_NAME"
   }
}

