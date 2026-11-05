# SPDX-License-Identifier: GPL-3.0-only

# Copies PLUGIN into ROOT/plugins-ñ, a directory whose name is not ASCII.
#
#   cmake -DPLUGIN=<file> -DROOT=<directory>
#   	-P copy_to_non_ascii_directory.cmake
file(COPY ${PLUGIN} DESTINATION "${ROOT}/plugins-ñ")
