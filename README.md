# AutoGradingProgramC
Line Argument - recieves config file , which includes 3 lines -
1.Folder to check location.
2.Input file focation.
3.Expected output file location.

The program searches for a c file in the sub folders of the folder given, compiles the file, 
runs the a.out file with the input file and compares the output to the expected output.
gives grades according to the following rules:

No c file - 0
Compile Error = 10
Wrong output = 50
Similiar output = 75
Correct output = 100
