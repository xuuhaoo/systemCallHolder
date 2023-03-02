package com.example.syscallholder

object SysCallHolder {
  init {
    System.loadLibrary("systemcall")
  }

  @JvmStatic
  external fun interceptOpenAt(targetFileName: String, replaceFilePath: String)
}