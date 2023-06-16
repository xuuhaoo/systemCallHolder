package com.squareup.systemcall

import android.content.pm.PackageInfo
import android.content.pm.PackageManager
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import com.example.syscallholder.SysCallHolder
import com.squareup.systemcall.databinding.ActivityMainBinding
import dalvik.system.DexFile
import java.io.File


class MainActivity : AppCompatActivity() {

  private lateinit var binding: ActivityMainBinding

  override fun onCreate(savedInstanceState: Bundle?) {
    super.onCreate(savedInstanceState)
    binding = ActivityMainBinding.inflate(layoutInflater)
    setContentView(binding.root)
    binding.btn.setOnClickListener {
      binding.sampleText.text = File(FileHelper.file.path).readText().trim()
    }
  }
}