import os
import sys


def make_grammar(filename):
    dictfile = open(filename, "r")
    gramfile = open("out.voca", "w")
    for line in dictfile:
        words = line.split(" ")
        outline = words[0]
        if len(outline) < 4 :
            num_tabs = 3
        else:
            num_tabs = 3 - (len(outline) / 4)
        for i in range(0,num_tabs):
            outline += "\t"
        for word in words[2:]:
            if word is not '' :
                if '[' in word:
                    pass
                elif word == "sp\n":
                    break
                else:
                    outline += " " + word
        outline += "\n"
        gramfile.write(outline)

    

if __name__ == "__main__" :
    make_grammar(sys.argv[1])
