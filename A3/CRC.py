import os
import sys
import random


def crc8(data, crc_code):       # data is a string of bytes 
    data_init = data
    data = data + "00000000"    # add 8 0s to the end of data
    data = list(data)         
    crc_code = list(crc_code)
    rem = data[:len(crc_code)]
    
    for i in range(len(data) - 8):
        #print(rem)
        if rem[0] == "0":      # remove first 0 and add next bit
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
    #print(rem)
    encode = data_init + rem
    return encode

def crc_error(encode, crc_code):
    data = encode
    data = list(data)
    crc_code = list(crc_code)
    rem = data[:len(crc_code)]
    
    for i in range(len(data) - 8):
        #print(rem)
        if rem[0] == "0":      # remove first 0 and add next bit
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
    #print(rem)
    if rem == "00000000":
        return True
    else:
        return False
    
def corrupt_odd(encode):    # randomly corrupt odd number of bits (3 or above)
    encode = list(encode)
    num = random.randint(3, len(encode))
    if num % 2 == 0:
        num -= 1
    # pick num unique locations to corrupt 
    loc = []
    for i in range(num):
        loc.append(random.randint(0, len(encode) - 1))
        while loc[i] in loc[:i]:
            loc[i] = random.randint(0, len(encode) - 1)
    for i in range(num):
        if encode[loc[i]] == "0":
            encode[loc[i]] = "1"
        else:
            encode[loc[i]] = "0"
    encode = "".join(encode)
    return encode, num


def corrupt_burst(encode, loc):
    # corrupt 6 bits starting at loc
    encode = list(encode)
    for i in range(6):
        if encode[loc + i] == "0":
            encode[loc + i] = "1"
        else:
            encode[loc + i] = "0"
    encode = "".join(encode)
    return encode
    

def main():
    inpfile = sys.argv[1]
    outfile = sys.argv[2]
    f = open(inpfile, "r")
    g = open(outfile, "w")
    crc_code = "100000111"
    lines = f.readlines()
    g.write("*" * 50 + "\n")
    for line in lines:
        line = line.strip()
        g.write("Original String: " + line + "\n")
        encode = crc8(line, crc_code)
        g.write("Original String with CRC: " + encode + "\n")
        g.write("\nCorrupted strings with odd number of errors: \n")
        g.write("\n" + "-" * 50 + "\n")
        for i in range(10):
            c_encode, num = corrupt_odd(encode)
            g.write("Corrupted String: " + c_encode + "\n")
            g.write("Number of Errors Introduced: " + str(num) + "\n")
            g.write("CRC Check: ")
            if str(crc_error(c_encode, crc_code)) == "True":
                g.write("No Error Detected\n")
            else:
                g.write("Error Detected\n")
            g.write("-" * 50 + "\n")
        g.write("\nCorrupted strings with burst errors: \n")
        g.write("\n" + "-" * 50 + "\n")
        loc = []
        for i in range(5):
            loc.append(random.randint(100,110))
            while loc[i] in loc[:i]:
                loc[i] = random.randint(100,110)
        for i in range(5):
            c_encode = corrupt_burst(encode, loc[i])
            g.write("Corrupted String: " + c_encode + "\n")
            g.write("Location of Burst Error: " + str(loc[i]) + "\n")
            g.write("CRC Check: ")
            if str(crc_error(c_encode, crc_code)) == "True":
                g.write("No Error Detected\n")
            else:
                g.write("Error Detected\n")
            g.write("-" * 50 + "\n")
        g.write("End of File: " + inpfile + "\n")

            
            

        g.write("\n" + "*" * 50 + "\n")

if __name__ == "__main__":
    main()

