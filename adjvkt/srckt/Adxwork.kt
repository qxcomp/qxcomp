package org.envsh.jvkt;

import android.content.Context
import android.util.Log
import androidx.work.Worker
import androidx.work.WorkerParameters
import androidx.work.*
import java.util.concurrent.TimeUnit

import android.widget.Toast;
import android.app.Activity;
import android.app.NotificationManager
import android.content.Context.NOTIFICATION_SERVICE

import android.os.SystemClock
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
    //    val inputUrl = inputData.getString(KEY_INPUT_URL)
                    //   ?: return Result.failure()
    //    val outputFile = inputData.getString(KEY_OUTPUT_FILE_NAME)
                    //   ?: return Result.failure()
        var inputUrl = "https://dlwk.bar/foo.gz";
        var outputFile = "savefoo.gz"
        log.info(inputUrl, outputFile)
       // Mark the Worker as important
       val progress = "Starting Download"
       setForeground(createForegroundInfo(progress))
       download(inputUrl, outputFile)
       return Result.success()
   }

   private suspend fun download(inputUrl: String, outputFile: String) {
       // Downloads a file and updates bytes read
       // Calls setForeground() periodically when it needs to update
       // the ongoing Notification
       var zit = 0
       for (i in 0..500000000) {
            zit ++
            // Thread.sleep(12345)
            var cnt = 0
            for (j in 0..10) {
            SystemClock.sleep(1234)
            cnt ++
            }
            // SystemClock.sleep(12345)
            // SystemClock.sleep(milliseconds) is a utility function very similar to Thread.sleep(milliseconds), but it ignores InterruptedException. 
            // 这一行重复调用是怎么回事，
            // 注意不同的线程！！！
            /*
08-15 11:32:30.039 19577 19649 I System.out: 18[I](CKS) indl(CKS) 1(CKI) 12(CKI)
08-15 11:32:30.061 19577 19643 I System.out: 19[I](CKS) indl(CKS) 1(CKI) 12(CKI)
08-15 11:32:30.072 19577 19646 I System.out: 20[I](CKS) indl(CKS) 1(CKI) 12(CKI)
08-15 11:32:30.078 19577 19650 I System.out: 21[I](CKS) indl(CKS) 1(CKI) 12(CKI)
08-15 11:32:30.089 19577 19644 I System.out: 22[I](CKS) indl(CKS) 1(CKI) 12(CKI)
08-15 11:32:30.093 19577 19645 I System.out: 23[I](CKS) indl(CKS) 1(CKI) 12(CKI)
08-15 11:32:30.101 19577 19648 I System.out: 24[I](CKS) indl(CKS) 1(CKI) 12(CKI)
 */
            log.info("indl", i, i*12, zit, cnt)
            // System.out.println("indl "+ i +" "+ i*12)
            // Toast.makeText(applicationContext , "Service is still running", Toast.LENGTH_LONG).show();
            val progress = "Starting Download $i,$cnt"
            // setForeground(createForegroundInfo(progress))
       }
       log.info("dlfn done")
   }
   // Creates an instance of ForegroundInfo which can be used to update the
   // ongoing notification.
   private fun createForegroundInfo(progress: String): ForegroundInfo {
    //    val id = applicationContext.getString(R.string.notification_channel_id)
    //    val title = applicationContext.getString(R.string.notification_title)
    //    val cancel = applicationContext.getString(R.string.cancel_download)
    // val id = "notichanid12345"
    val id = "12345" // must already created one
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
           .setSilent(true)
           // NotificationManager.IMPORTANCE_DEFAULT
           // Add the cancel action to the notification which can
           // be used to cancel the worker
        //    .addAction(android.R.drawable.ic_delete, cancel, intent)
        .addAction(R.drawable.ic_launcher, cancel, intent)
           .build()

        val notificationId = 123456
        log.info("fi", notificationId, progress)
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

    public fun setupLongtimeDownload(context: Activity) {

        val KEY_USER_COMMENT_TEXT = "key.user.comment.text"
        val KEY_DOWNLOAD_TITLE = "key.download.title"
        val KEY_DOWNLOAD_DESC = "key.download.desc"
        val KEY_UPLOAD_TITLE = "key.upload.title"
        val KEY_UPLOAD_DESC = "key.upload.desc"
        // val TAG_SEND_LOG = "tag.send.log"
        val CHANNEL_ID = "4747"

        // val workManager = wkman
        // val workManager = WorkManager.getInstance(this)
        val workManager = WorkManager.getInstance(context)

        val TAG_SEND_LOG = "tag.send.log"
        val downloadData = workDataOf(
            KEY_DOWNLOAD_TITLE to "Download Worker",
            KEY_DOWNLOAD_DESC to "Downlading Image is successfully"
        )

        // OneTimeWorkRequestBuilder
        val sendingLog = OneTimeWorkRequestBuilder<DownloadWorker>()
            .addTag("TAG_download_task")
            .setInputData(downloadData)
            .setInitialDelay(3, TimeUnit.SECONDS)
            .build()


        log.info("enqueue fgwork", CHANNEL_ID)
        workManager.enqueue(sendingLog)
        
        // workManager.getWorkInfoByIdLiveData(sendingLog.id)
        //     .observe(this, Observer { workInfo ->
        //         binding.textViewWorkState.text = workInfo.state.name
        //     })
    }

