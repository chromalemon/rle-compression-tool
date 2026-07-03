# Custom RLE Compression Suite

This is a tool for compressing and decompressing files, using the Run Length Encoding algorithm. This type of compression is most commonly used for bitmap images, which often involve long sequences of repeating colours.



# How to run it

The different modes supported are:
<br>
-cB (compress bmp)
<br>
-dB (decompress bmp)

<p>$ git clone https://github.com/chromalemon/Custom-RLE-Compression-Suite</p>
<br>
<p>$ make</p>
<br>
<p>$ ./rle_tool mode input_path output_path</p>









# a

I decided to write this program to explore file handling and precise stream positioning.

The most difficult part for me was handling sequences of length > 255. I stored length as an unsigned char to restrict its size to 1 byte, improving compression ratio compared to using types like int. It was hard for me to keep track of where the stream position should stop / move to and what the counter should be set to once a sequence of length 255 was encountered, but after some rough sketching and careful thinking I figured it out.