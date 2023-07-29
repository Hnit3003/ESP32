# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "D:/Solfware/ESP_IDF/Espressif/frameworks/esp-idf-v4.4.4/components/bootloader/subproject"
  "D:/Solfware/ESP_IDF/Wifi_Driver/build/bootloader"
  "D:/Solfware/ESP_IDF/Wifi_Driver/build/bootloader-prefix"
  "D:/Solfware/ESP_IDF/Wifi_Driver/build/bootloader-prefix/tmp"
  "D:/Solfware/ESP_IDF/Wifi_Driver/build/bootloader-prefix/src/bootloader-stamp"
  "D:/Solfware/ESP_IDF/Wifi_Driver/build/bootloader-prefix/src"
  "D:/Solfware/ESP_IDF/Wifi_Driver/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "D:/Solfware/ESP_IDF/Wifi_Driver/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
