# Joshua Hesseltine
# CS344 mypython.py
# script creates 3 files and writes random characters to each file, prints to screen
# creates 2 random ints and prints to screen then multiples the two random values and
# prints the product to the screen
# Resources Used:
# http://stackoverflow.com/questions/2030053/random-strings-in-python
# http://stackoverflow.com/questions/8084260/python-printing-a-file-to-stdoutS
# http://stackoverflow.com/questions/2823316/generate-a-random-letter-in-python

import random
import string

# declare files and open for writing
file1 = open("a", 'w')
file2 = open("b", 'w')
file3 = open("c", 'w')

# set files to array of files
files = [file1, file2, file3]
# for each file in array of files
letters =  'abcdefghijklmnopqrstuvwxyz'

# for each file in array of files
for file in files:
    rand = ''.join(random.choice(letters) for n in xrange(10))
    file.write(rand) #write random characters
    file.write('\n') #write new line
    file.close()

# open files
file1 = open("a", 'r')
file2 = open("b", 'r')
file3 = open("c", 'r')

files = [file1, file2, file3]
# loop through open files
for file in files:
    line = file.read(10) # read first line
    print "Random string:  ", line
    file.close()

# create random numbers
from random import randint
num1 = randint(1,42) # random is selected between arguments 1 and 42
num2 = randint(1,42)
print "Random int:  ", num1
print "Random int:  ", num2
product = num1 * num2 # calculate product
print "Product: ", product # print product