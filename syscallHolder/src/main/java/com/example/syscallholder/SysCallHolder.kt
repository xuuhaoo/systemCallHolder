package com.example.syscallholder

import com.bytedance.android.bytehook.ByteHook

object SysCallHolder {
  init {
    ByteHook.init();
    System.loadLibrary("systemcall")
  }

  @JvmStatic
  @JvmOverloads
  external fun interceptOpenAt(targetFileName: String = "", replaceFilePath: String = "")

  @JvmStatic
  @JvmOverloads
  external fun hookPtrace()
}