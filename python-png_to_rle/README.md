# python-rle
Convert a PNG file into a human-readable RLE format by using Python 3 and Pillow.

<h3>If you don't have Pillow installed</h3>
Run the following command from the command prompt:
<pre>pip install pillow</pre>

<h3>What is RLE</h3>
According to Wikipedia, RLE stands for "Run-length encoding (RLE)". <br/><br/>
Run-length encoding is a very simple form of lossless data compression in which runs of data (that is, sequences in which the same data value occurs in many consecutive data elements) are stored as a single data value and count, rather than as the original run.
<br/><br/>
<i>For example, this string:</i><br/>
WWWWWWWWWWWWBWWWWWWWWWWWWBBBWWWWWWWWWWWWWWWWWWWWWWWWBWWWWWWWWWWWWWW<br/><br/>
<i>...becomes this after using RLE:</i><br/>
12W1B12W3B24W1B14W<br/><br/>
This script simply converts a binary PNG image into an RLE-type file format.
<h3>Example</h3>
<i>Original image</i><br/>
<img src="https://github.com/anwserman/python-rle/blob/master/input/golfcourse.png">
<i>Sample output</i><br/>
<pre>
#Image Dimensions
Width: 683 
Height: 384 


#Image Palette
255, 0, 128
0, 102, 0
0, 255, 0
66, 255, 66
0, 0, 255
255, 255, 0
128, 128, 128
0, 0, 0

#Pixel Count
0: 42670
1: 48
0: 631
1: 53
0: 627
1: 61
0: 55
2: 55
0: 509
1: 67
0: 48
2: 63

... truncated for brevity
</pre>
<h3>Included commands</h3>
<b>open_png(filename):</b> Open up a PNG file by file path, and read it into memory<br/>
<b>get_color_atpoint(point):</b> Get a tuple with RGB values at a given point in an image, by passing in a tuple with X,Y coordinates<br/>
<b>read_rle_fromstream(stream):</b> Read in RLE information from a file, by passing in a file stream <br/>
<b>write_memory_tofile(filename):</b> Write out the image in memory as a binary file <br/>
<b>write_rle_tostream(filename):</b> Write out RLE information to a file, by passing in a file stream<br/>
