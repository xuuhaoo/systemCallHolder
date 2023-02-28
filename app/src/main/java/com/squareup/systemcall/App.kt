package com.squareup.systemcall

import android.app.Application

class App : Application() {
    override fun onCreate() {
        super.onCreate()
        FileHelper.initFile(this)
        Native.ptraceViewSvcCall()
    }
}