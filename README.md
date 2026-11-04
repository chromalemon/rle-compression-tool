[![Build and Test](https://github.com/chromalemon/rle-compression-tool/actions/workflows/main.yml/badge.svg)](https://github.com/chromalemon/rle-compression-tool/actions/workflows/main.yml)
[![Format and Lint](https://github.com/chromalemon/rle-compression-tool/actions/workflows/format-lint.yml/badge.svg)](https://github.com/chromalemon/rle-compression-tool/actions/workflows/format-lint.yml)

# RLE Compression Tool Overview

A modular lossless compression utility in C supporting generic binary streams and 24-bit BMP images through configurable run-length encoding.

**Key Features**

- Configurable word sizes
- BMP parsing
- Padding handling
- Automated tests
- Sanitizers

**Compression Example**

```
Compression complete.

Original: 822 bytes
New: 62 bytes
Compression ratio: 7.54%
```

# Installation & Running

Install dependencies:

```bash
sudo apt update
sudo apt install build-essential cmake -y
```

Clone the repository:

```bash
git clone https://github.com/chromalemon/rle-compression-tool
cd rle-compression-tool
```

Compile the required files:

```bash
cmake -S . -B build -D CMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

Run the program:

```bash
build/rle_tool [OPTION]... SOURCE DEST
```

Cleaning build artifacts:

```bash
rm -rf build
```

**Options:**

- -c = Compress
- -d = Decompress
- -s = Suppress/silence size info (no "Original size: New size:" message)
- -b = BMP file format
- -r = Regular binary file format

**Example Usage:**

```bash
# Compress BMP file.
- build/rle_tool -c -b input.bmp output.bmp

# Compress regular file with word size of 3 bytes, silenced.
- build/rle_tool -c -r3 -s input.txt output.rle
```

## Creating BMP Files

To create your own BMP files for testing, use ImageMagick. Head to their website for installation and usage. 

**Example BMP File Creation:**

> [!NOTE]
> For BMP compression and decompression, only V3 24 BPP is supported.

With ImageMagick installed, use this command to make an M by N BMP V3 image, with a full canvas of color C, stored in sample.bmp:

```bash
magick -size MxN xc:C BMP3:sample.bmp
```

To create a 16x16 solid blue image:

```bash
magick -size 16x16 xc:blue BMP3:sample.bmp
```

# Backstory

I started this project so I could explore file handling, streams, padding, buffers, strict formatting, error handling, memory management, and other principles/concepts.

Originally, I had pure RLE in mind for standard text files. I stored counts as unsigned chars, and the data to compress was always 8 bits per block. This was a bit boring so I looked into BMP files and how they are formatted. I realised that instead of focusing specifically on files, I should make a universal RLE tool. It should be compatible with any input stream: files, network packets, sensor data, live video, and more.
