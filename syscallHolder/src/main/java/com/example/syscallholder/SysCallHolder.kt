package com.example.syscallholder

object SysCallHolder {
  init {
    System.loadLibrary("systemcall")
  }

  external fun ptraceViewSvcCall(targetFileName: String, replaceFilePath: String)
}