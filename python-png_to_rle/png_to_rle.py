import os, sys
from PIL import Image

class RLEBitmap:
    width = 0
    height = 0
    pixels = None
    image = None
    
    #basic constructor
    def __init__(self):
        self.image = None
        self.pixels = None
        self.height = 0
        self.width = 0
    
    def open_png(self, filename):
        #open up the image
        self.image = Image.open(filename)
        #get the pixel data
        self.pixels = self.image.load()
        #get the width and height 
        self.width, self.height = self.image.size
        print ("width=",self.width)
        print ("height=",self.height)
    
    def get_color_atpoint(self, point):
        #return the pixel color as a tuple, at the given point
        return (self.pixels[point[0], point[1]])
    
    #read an image from a file stream
    def read_rle_fromstream(self, stream):
        #colors used within the image, this can be a list and not a dictionary
        colors = []
        colorCount = 0
        colorIndex = 0
        
        #iterator data
        i = 0
        x = 0
        y = 0
        
        #reset bitmap data
        self.image = None
        self.pixels = None
        
        #read in and skip the first line, it's the header description
        
        self.line1 = stream.readline()
        print("line1=", self.line1)
        #get the image width and height
        self.line2 = stream.readline()
        print("line2=", self.line2)
        self.width = int(self.line2.split(':')[1])
        self.height = int(stream.readline().split(':')[1])
        #skip the new line
        stream.readline()
        
        #set up our bitmap in memory
        self.image = Image.new("RGB", (self.width, self.height))
        
        #read in the image palette, and skip the first line as it's the palette description
        stream.readline()
        
        #interate through until we hit whitespace
        sI = stream.readline()
        while not sI.isspace():
            #split the line into rgb components
            sISplit = sI.split(',')
            #read in the values as an RGB tuple 
            colors.append((int(sISplit[0]), int(sISplit[1]), int(sISplit[2])))
            #read in the next new line
            sI = stream.readline()
        
        #now we read in the pixel count, and skip the first line as it's the header description
        stream.readline()
        
        #iterate through until we hit whitespae
        #first line
        sI = stream.readline()
        while not sI.isspace():
            #split the line into index/count components
            sISplit = sI.split(':')
            #get the RGB index value that we need based on index
            colorIndex = int(sISplit[0])
            #get the count of how many times we need to loop through this color
            colorCount = int(sISplit[1])
            
            i = 0
            for i in range(0, colorCount):
                self.image.putpixel((x, y), colors[colorIndex])
                x += 1
                
                if (x == (self.width)):
                    x = 0
                    y += 1
                    
            #read in the next new line    
            sI = stream.readline()
        
        #once the image has been constructed in memory, dump the pixel data into a table
        self.pixels = self.image.load()
        
    #print rle conversion
    def write_rle_tostream(self):
        #colors used within the image
        colors = {}
        #store the RLE data
        pixels = []
        pixelsTFE = [] # this is the format required by TFTe_SPI
        #store the color in use
        currentColor = None
        currentColorCount = 0
        
        #iterator data
        x = 0
        y = 0
        
        #iterate through image
        #row by row
        for y in range(0, self.height):
            #column by column
            for x in range(0, self.width):
                #get the current pixel
                newColor = self.pixels[x, y] 

                #compare new color versus existing color
                if newColor != currentColor:
                    #we don't want to do this if currentColor is nothing
                    if currentColor != None:
                        #add current (existing) color to our dictionary
                        #and give it an index value (lookup value)
                        #this is rudimentary lookup table for both saving the color data below and for saving/reading the file later
                        colors.setdefault(currentColor, len(colors.keys()))
                        #return the index value
                        colorIndex = colors[currentColor]
                        #add the color and pixel count to our list
                        pixels.append((colorIndex, currentColorCount))
                        while currentColorCount > 128:
                            if currentColor == 0:
                                pixelsTFE.append(127)
                            else :     
                                pixelsTFE.append(255)
                            currentColorCount = currentColorCount - 128    
                        if currentColor != 0:
                                currentColorCount = currentColorCount+128
                        pixelsTFE.append(currentColorCount-1)
                        #set the new color to our currentcolor 
                        currentColor = newColor
                        #set the count equal to 1, as we need to count it as part of the new run
                        currentColorCount = 1
                    else:
                        currentColor = newColor
                        currentColorCount = 1   
                else:
                    currentColorCount += 1
                
        #flush out the last of the colors we were working on, into the array
        colors.setdefault(currentColor, len(colors.keys()))
        colorIndex = colors[currentColor]
        pixels.append((colorIndex, currentColorCount))
        while currentColorCount > 128:
            if currentColor == 0:
                pixelsTFE.append(127)
            else :     
                pixelsTFE.append(255)
            currentColorCount = currentColorCount - 128
        if currentColor != 0:
                currentColorCount = currentColorCount+128
        pixelsTFE.append(currentColorCount-1)
                   
        print("colors =", colors)
        print("pixels (on/off , amount) =" , pixels)
        print("rle = " ,pixelsTFE)
    
#some tests
#open up a PNG and write it to a RLE document
rb = RLEBitmap()
#rb.open_png('input\golfcourse.png')
rb.open_png('more1_button.png')
#fs = open('output\more1_button.rle','w')
#fs = open('output\golfcourse.rle','w')
rb.write_rle_tostream()
#fs.close()
