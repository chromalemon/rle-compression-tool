# RLE Compression Tool Overview

This is a tool for compressing and decompressing files, using the Run Length Encoding algorithm. This type of compression is most commonly used for bitmap images, which often involve long sequences of repeating colours.

> [!NOTE]
> For BMP compression and decompression, only V3 24 BPP is supported.

Example of compression:

```
Compression complete.

Original: 822 bytes
New: 62 bytes
Compression ratio: 7.54%
```

# Installation & Running

Clone the repository:

```bash
git clone https://github.com/chromalemon/rle-compression-tool
cd rle-compression-tool
```

Compile the required files:

```bash
make
```

Run the program:

```bash
./rle_tool [OPTION]... SOURCE DEST
```

**Options:**

- -c = compress
- -d = decompress
- -s = suppress size info (no "Original size: New size:" message)
- -b = BMP file format

**Example Usage:**

- ./rle_tool -c -b input.bmp output.bmp

## Creating BMP Files

To create your own BMP files for testing, use ImageMagick. Head to their website for installation and usage. 

**Example BMP File Creation:**

With ImageMagick installed, use this command to make an M by N BMP V3 image, with a full canvas of color C, stored in sample.bmp:

```bash
magick -size MxN xc:C BMP3:sample.bmp
```

To create a 16x16 solid blue image:

```bash
magick -size 16x16 xc:blue BMP3:sample.bmp
```

# Backstory

I wrote this program so I could explore file handling, streams, padding, buffers, strict formatting, error handling, memory management, and other principles/concepts.

Originally, I had pure RLE in mind for standard text files. I stored counts as unsigned chars, and the data to compress was always 8 bits per block. This was a bit boring so I looked into BMP files and how they are formatted. I realised that instead of focusing specifically on files, I should make a universal RLE tool. It should be compatible with any input stream: files, network packets, sensor data, live video, and more.
