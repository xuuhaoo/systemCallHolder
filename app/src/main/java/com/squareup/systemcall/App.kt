package com.squareup.systemcall

import android.app.Application
import android.os.Handler
import android.os.Looper

class App : Application() {
  override fun onCreate() {
    super.onCreate()
    FileHelper.initFile(this)
    Handler(Looper.getMainLooper()).postDelayed({
      Thread {
        Native.ptraceViewSvcCall()
      }.start()
    }, 1_0)
  }
}