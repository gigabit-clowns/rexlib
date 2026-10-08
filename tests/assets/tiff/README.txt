TIFF test assets

These files are here so that the TIFF reader is tested against bytes this
project did not write. They are synthetic: each one was encoded by the command
line tools of libtiff 4.7.2 from raw samples that follow a formula, so that a
test can state what a file holds without reading it by other means. Everything
the reader and the writer can produce themselves is built inside the tests
instead, where the bytes sit next to the assertion.

What they are not is the output of a detector: none carries the private tags
or the layout the acquisition software of a microscope chooses.


The samples

The raw samples are little-endian, and i counts them from zero, row after row
and then page after page.

  uint8     (7 * i + 3) mod 251    144 samples, cut into three pages of 48
  uint16    1000 + 257 * i          48 samples
  int16     37 * i - 300            48 samples
  float32   0.25 * i - 3.5         960 samples

  import numpy as np
  i = np.arange(144)
  u8 = ((7 * i + 3) % 251).astype('<u1')
  for p in range(3):
      u8[48 * p:48 * (p + 1)].tofile('uint8_page%d.raw' % p)
  i = np.arange(48)
  (1000 + 257 * i).astype('<u2').tofile('uint16.raw')
  (37 * i - 300).astype('<i2').tofile('int16.raw')
  i = np.arange(960)
  (0.25 * i - 3.5).astype('<f4').tofile('float32.raw')
  np.zeros(8 * 6 * 3, '<u1').tofile('rgb.raw')
  np.zeros(8 * 5, '<u1').tofile('uint8_short_page.raw')

Each was first wrapped into an uncompressed file, which the files below were
then encoded from.

  for p in 0 1 2; do
      raw2tiff -w 8 -l 6 -d byte uint8_page$p.raw uint8_page$p.tif
  done
  raw2tiff -w 8 -l 6 -d short uint16.raw uint16.tif
  raw2tiff -w 8 -l 6 -d sshort int16.raw int16.tif
  raw2tiff -w 40 -l 24 -d float float32.raw float32.tif
  raw2tiff -w 8 -l 6 -b 3 -p rgb rgb.raw rgb.tif
  raw2tiff -w 8 -l 5 -d byte uint8_short_page.raw uint8_short_page.tif


stack_uint8_lzw.tif
698 bytes
Three pages of 8 columns by 6 rows, uint8, compressed with LZW in strips of
four rows, so that every page is two strips and the second one is shorter.

  tiffcp -c lzw -r 4 uint8_page0.tif uint8_page1.tif uint8_page2.tif \
      stack_uint8_lzw.tif

The specimen of what a movie is: several pages, compressed, each cut into more
than one strip.


image_uint16_deflate_predictor.tif
214 bytes
One page of 8 columns by 6 rows, uint16, compressed with Deflate after
horizontal differencing.

  tiffcp -c zip:2 uint16.tif image_uint16_deflate_predictor.tif

Deflate is the codec that zlib provides, so this is the file that fails where
libtiff was built without it. The predictor stores each sample as its
difference from the one before, which the decoder has to undo.


image_float32_tiled.tif
1,142 bytes
One page of 40 columns by 24 rows, float32, cut into tiles of 16 by 16 and
compressed with Deflate after the floating point predictor.

  tiffcp -t -w 16 -l 16 -c zip:3 float32.tif image_float32_tiled.tif

Neither extent is a multiple of the tile, so the tiles of the last column and
of the last row reach past the page and only part of each is the image.


image_int16_big_endian.tif
266 bytes
One page of 8 columns by 6 rows, int16, uncompressed, in big-endian byte
order.

  tiffcp -B -c none int16.tif image_int16_big_endian.tif

It begins with "MM" rather than "II", and its samples are in the byte order
no machine the suite runs on has.


stack_uint8_bigtiff.tif
684 bytes
The first two pages of the uint8 samples, compressed with LZW, as a BigTIFF
file.

  tiffcp -8 -c lzw uint8_page0.tif uint8_page1.tif stack_uint8_bigtiff.tif

Its version is 43 rather than 42 and its offsets are 64 bits wide.


refused_rgb.tif
326 bytes
One page of 8 columns by 6 rows of three 8 bit samples per pixel.

  tiffcp -c none rgb.tif refused_rgb.tif

A well formed file this project does not read: it is recognized as TIFF and
refused when it is opened.


refused_pages_differ.tif
420 bytes
Two uint8 pages of 8 columns, the first of 6 rows and the second of 5.

  tiffcp -c none uint8_page0.tif uint8_short_page.tif \
      refused_pages_differ.tif

A well formed file that is not a stack, since its pages do not have one
size.
