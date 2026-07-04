# Custom RLE Compression Suite Overview

This is a tool for compressing and decompressing files, using the Run Length Encoding algorithm. This type of compression is most commonly used for bitmap images, which often involve long sequences of repeating colours.

# Installation & Running

Clone the repository:

```bash
git clone https://github.com/chromalemon/Custom-RLE-Compression-Suite RLE_Suite
cd RLE_Suite
```

Compile the files:

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

- ./rle_tool -c -b input.bmp output.rle

# Backstory

I wrote this program so I could explore file handling, streams, padding, buffers, strict formatting, error handling, memory management, and other principles/concepts.

Originally, I had pure RLE in mind for standard text files. I stored counts as unsigned chars, and the data to compress was always 8 bits per block. This was a bit boring so I looked into BMP files and how they are formatted. I realised that instead of focusing specifically on files, I should make a universal RLE tool. It should be compatible with any input stream: files, network packets, sensor data, live video, and more.
