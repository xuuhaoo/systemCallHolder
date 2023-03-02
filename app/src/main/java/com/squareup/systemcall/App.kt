package com.squareup.systemcall

import android.app.Application
import android.os.Handler
import android.os.Looper
import com.example.syscallholder.SysCallHolder

class App : Application() {
  override fun onCreate() {
    super.onCreate()
    FileHelper.initFile(this)
    Handler(Looper.getMainLooper()).postDelayed({
      Thread {
        SysCallHolder.ptraceViewSvcCall()
      }.start()
    }, 1_0)
  }
}