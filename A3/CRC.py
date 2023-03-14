import os
import sys
import random


def crc8(data, crc_code):           # data is a string of 128 bits and crc_code is string of 9 bits returns crc encoded string
    data_init = data
    data = data + "00000000"        # add 8 0s to the end of data
    data = list(data)         
    crc_code = list(crc_code)
    rem = data[:len(crc_code)]      # list for easy access and rem is the dividing value in division and ends as remainder
    
    for i in range(len(data) - 8):
        if rem[0] == "0":           # remove first 0 and add next bit 
            rem.pop(0)
            if(len(data) > len(crc_code) + i):
                rem.append(data[len(crc_code) + i])
        else:                       # xor with crc code and remove first bit for len of crc code(9) and add next bit
            for j in range(len(crc_code)):
                if rem[j] == crc_code[j]:
                    rem[j] = "0"
                else:
                    rem[j] = "1"
            rem.pop(0)
            if(len(data) > len(crc_code) + i):
                rem.append(data[len(crc_code) + i])
    # The last iteration doesn't add a bit so we have rem of 8 bits
    rem = "".join(rem)
    encode = data_init + rem        # add the remainder to the original data
    return encode

def crc_error(encode, crc_code):    # returns true if no error detected and false if error detected 
    data = encode
    data = list(data)
    crc_code = list(crc_code)
    rem = data[:len(crc_code)]
    # Till next comment same as crc8 function
    for i in range(len(data) - 8):
        if rem[0] == "0":     
            rem.pop(0)
            if(len(data) > len(crc_code) + i):
                rem.append(data[len(crc_code) + i])
        else:
            for j in range(len(crc_code)):
                if rem[j] == crc_code[j]:
                    rem[j] = "0"
                else:
                    rem[j] = "1"
            rem.pop(0)
            if(len(data) > len(crc_code) + i):
                rem.append(data[len(crc_code) + i])
    rem = "".join(rem)
    # Till here same as crc8 function
    if rem == "00000000":           # if remainder is 0 then no error detected
        return True
    else:
        return False
    
def corrupt_odd(encode):            # randomly corrupt odd number of bits (3 or above) and return corrupted string and num of errors introduced
    encode = list(encode)
    num = random.randint(3, len(encode))
    if num % 2 == 0:                # pick num(odd) - number of errors to introduce
        num -= 1    
    loc = []
    for i in range(num):            # pick num unique locations to introduce errors
        loc.append(random.randint(0, len(encode) - 1))
        while loc[i] in loc[:i]:
            loc[i] = random.randint(0, len(encode) - 1)
    for i in range(num):
        if encode[loc[i]] == "0":
            encode[loc[i]] = "1"
        else:
            encode[loc[i]] = "0"
    encode = "".join(encode)
    return encode, num              # return corrupted string and num of errors introduced


def corrupt_burst(encode, loc):     # Given a starting location introduce a burst error of 6 bits
    # corrupt 6 bits starting at loc
    encode = list(encode)
    for i in range(6):              # corrupt 6 bits starting at loc
        if encode[loc + i] == "0":
            encode[loc + i] = "1"
        else:
            encode[loc + i] = "0"
    encode = "".join(encode)
    return encode
    

def main():
    inpfile = sys.argv[1]           # input file
    outfile = sys.argv[2]           # output file
    f = open(inpfile, "r")
    g = open(outfile, "w")
    crc_code = "100000111"
    lines = f.readlines()           # lines now stores all original strings
    g.write("*" * 50 + "\n")
    for line in lines:              # for each original string
        line = line.strip()
        g.write("Original String: " + line + "\n")
        encode = crc8(line, crc_code)
        g.write("Original String with CRC: " + encode + "\n")
        g.write("\nCorrupted strings with odd number of errors: \n")
        g.write("\n" + "-" * 50 + "\n")
        for i in range(10):         # introduce 10 corrupted strings with odd number of errors
            c_encode, num = corrupt_odd(encode)
            g.write("Corrupted String: " + c_encode + "\n")
            g.write("Number of Errors Introduced: " + str(num) + "\n")  # print number of errors introduced
            g.write("CRC Check: ")                                      # CRC error check
            if str(crc_error(c_encode, crc_code)) == "True":
                g.write("No Error Detected\n")
            else:
                g.write("Error Detected\n")
            g.write("-" * 50 + "\n")
        g.write("\nCorrupted strings with burst errors: \n")
        g.write("\n" + "-" * 50 + "\n")
        loc = []                    # Choose 5 unique starting locations for burst errors from 100 to 110
        for i in range(5):
            loc.append(random.randint(100,110))
            while loc[i] in loc[:i]:
                loc[i] = random.randint(100,110)
        for i in range(5):          # introduce 5 corrupted strings with burst errors
            c_encode = corrupt_burst(encode, loc[i])
            g.write("Corrupted String: " + c_encode + "\n")
            g.write("Location of Burst Error: " + str(loc[i]) + "\n")
            g.write("CRC Check: ")  # CRC error check
            if str(crc_error(c_encode, crc_code)) == "True":
                g.write("No Error Detected\n")
            else:
                g.write("Error Detected\n")
            g.write("-" * 50 + "\n")
        g.write("End of File: " + inpfile + "\n")

            
            

        g.write("\n" + "*" * 50 + "\n")

if __name__ == "__main__":          # call main
    main()

