package com.example.syscallholder


object SysCallHolder {
  init {
    System.loadLibrary("systemcall")
  }

  @JvmStatic
  @JvmOverloads
  external fun interceptOpenAt(targetFileName: String = "", replaceFilePath: String = "")
}