package com.example.syscallholder

object SysCallHolder {
    init {
        System.loadLibrary("systemcall")
    }

    /**
     * A native method that is implemented by the 'systemcall' native library,
     * which is packaged with this application.
     */
     external fun readFileSysCall(filePath: String): String

     external fun ptraceViewSvcCall()
}