import sys
import os

infile = sys.argv[1]
outfile = sys.argv[2]



with open(sys.argv[1]) as f:
    with open(outfile+".h","w") as hout:
        hout.write("// Token list - generated by gentoks.py from %s\n\n" % infile)
        hout.write("#include \"tokeniser.h\"\n");
        hout.write("extern TokenRegistry %s[];\n" % outfile)
        with open(outfile+".cpp","w") as cppout:
            cppout.write("// Token table - generated by gentoks.py from %s\n\n" % infile)
            cppout.write("#include \"%s.h\"\n" % outfile);
            cppout.write("TokenRegistry %s[] = {\n" % outfile)
            content = f.readlines()

            ct=0
            for x in content:
                if ':' in x:
                    val,name = x.split(':')
                    val = val.strip()
                    name = name.strip()
                    cppout.write("\t{\"%s\", %s},\n" % (val,name))
                    hout.write("#define\t%s\t%d\n" % (name,ct))
                    ct+=1
            cppout.write("{NULL,-10}\n");
            cppout.write("};\n")
        
