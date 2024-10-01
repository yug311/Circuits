#!/usr/bin/env

import os, sys, glob, time, subprocess, signal
import time

subdirectories = ['first', 'second', 'third']
formats = {'first':'file', 'second':'file', 'third':'file'}# if a program has single liner input and output, we put all test cases in single file. Otherwise, we have a file for test and associated file with results
weight = {'first':1, 'second':1, 'third':1}
scores = {'first':0, 'second':0, 'third':0}
total = {'first':40, 'second':20, 'third':40}
runtime = {'first':2050, 'second':2050}
test_cases_directory = ""

def compare_files(file1, file2):
    f1 = open(file1);
    f2 = open(file2);

    return_value = 1;
    f1_data = f1.readlines();
    f2_data = f2.readlines();
    i = 0;
#    print(f1_data)
#    print(f2_data)

    for (line1, line2) in zip(f1_data, f2_data):
        i = i+ 1;
        if line1.rstrip() == line2.rstrip():
            return_value = 0
            continue;
        else:
            print("expected result:", line1.rstrip(), ", line number is:", i);
            print("program produced:", line2.rstrip(), ", line number is:", i);
            return_value = 1;
            break
        if return_value == 1:
            break;
    f1.close();
    f2.close();
    return return_value




def make_executable(dirname):
    if os.path.isfile('Makefile') or os.path.isfile('makefile'):
        os.system("make clean")
#        print "here"
        os.system("make")
    else:
        print ("No Makefile found in", dirname)
        print ("Please submit a Makefile to receive full grade.")
        os.system("gcc -o %s *.c *.h"%(dirname), verbose=False)


def grade_first(dirname):
    print ("Grading file", dirname)
    prevdir = os.getcwd()
    os.chdir(dirname)
    myscore = 0 ;

    make_executable(dirname)
    
    if not os.path.isfile(dirname):
        print ("Executable %s missing. Please check the compilation output."%(dirname))
        os.chdir(prevdir)
        print ("my score is ", myscore)
        return

    test_dir = test_cases_directory + dirname + "/"

    score = 0
    for testfile in sorted(os.listdir(test_dir)):
        if os.path.isdir(testfile) or not testfile.startswith("circ"):
            continue
        print ("Found the test files, %s"%(testfile))
        resultfile = "out"+testfile[4:len(testfile)]
        if not os.path.isfile(test_dir + resultfile):
            print ("Found test files, %s, but no output file."%(testfile))
            continue
        print ("Found test files: %s. The output will be compared to the file %s"%(testfile,resultfile))

        print ("Now testing " + dirname + " on " + testfile)
        my_run_command = "./%s %s >> %s"%(dirname, test_dir + testfile, resultfile);
        print("Executing:", my_run_command);
        os.system(my_run_command);
        os.system("rm -f test_r")
        command = "diff -iwbB " + test_dir + resultfile + " " + resultfile + " >> test_r"
        os.system(command)
        if os.stat("test_r").st_size==0:
            print ("Your output is correct\n")
            score = score + weight[dirname]
        else:
            print ("Your output is not correct\n")

    score = (score * (total[dirname])/5) 
    print ("Score is ", score)
    scores[dirname] = score
    os.chdir(prevdir)



def global_grade(dirname):

    target = len(subdirectories)

    for subdir in subdirectories:
	
        if not os.path.isdir(os.path.join(subdir)):
            continue
        print (subdir, " found!")
        if subdir in formats  and subdir == 'first':
            grade_first(subdir)        
        elif subdir in formats and subdir == 'second':
            grade_first(subdir)
        elif subdir in formats and subdir == 'third':
            grade_first(subdir)


    
if __name__ == '__main__':
    basepath = "pa6"
    tarmode = False #by default check the directory

    test_cases_directory = os.getcwd() + "/testcases/"
    if len(sys.argv)>1:
        if sys.argv[1].strip().endswith("tar"):
            tarmode=True

    if tarmode==False:
        if not os.path.isdir(basepath):
            print ("pa6 is not present in this directory.")
            sys.exit(1)
        else:
            prevdir = os.getcwd()
            if os.path.exists("obj_temp"):
                os.system("rm -rf obj_temp")
            os.system("mkdir obj_temp")
            os.chdir("obj_temp")
            os.system("cp -r ../pa6 .")
            if os.path.isdir("pa6"):
                os.chdir("pa6")
                print ("Grading the content of pa6.")

            global_grade("pa6")

    else:
        prevdir = os.getcwd()
        print ("dir is ", prevdir)
        if not os.path.exists("pa6.tar"):
            print ("Expecting pa6.tar in current directory. Current directory is %s"%(prevdir))
            print ("Please make sure you created pa6.tar in the right directory")
            sys.exit(1)
        if os.path.exists("obj_temp"):
            print ("Deleting the directory obj_temp.")
            os.system("rm -rf obj_temp")
        os.system("mkdir obj_temp")
        os.chdir("obj_temp")
        os.system("tar -xvf ../pa6.tar")
        if os.path.isdir("pa6"):
            os.chdir("pa6")
            global_grade("pa6")
        else:
            print ("There is not directory named pa6 in pa6.tar.")
            print ("Please check your tar file.")
        os.chdir(prevdir)

    final_score = scores['first'] + scores['second'] + scores['third']    
    print ("Your final score for PA6 is " , final_score , "/ 100.0")

