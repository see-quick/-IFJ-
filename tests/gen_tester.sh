#!/bin/bash

#
 # Predmet:  IFJ
 # Projekt:  Implementace prekladace imperativniho jazyka IFJ18
 # Soubor:   gen_tester.sh
 #
 # Popis:  Skript pre vypis testovanie vygenerovaneho kodu v IFJcode18 interpretom
 #
 # Datum: 1.11.2018 19:06
 #
 # Autori:   Maros Orsak       vedouci
 #           Polishchuk Kateryna     <xpolis03@fit.vutbr.cz>
 #           Igor Ignac
 #           Marek Rohel
#

usage() {
  echo  "Run program simply without any arguments: bash gen_tester.sh"
  echo  "Options:"
  echo  "        -h --help display this information."
  echo  "        -s for silent run -> interpret errors will be displayed"
  echo  "        -n no Output files will be generated"
  echo  "        -e to run only test for generation code from prece.c"
  echo  "        -g to run test with input and output in code gen(inptuf,inputi...)"
  exit 1
}

#setting colors for later output
red=`tput setaf 1`
green=`tput setaf 2`
reset=`tput sgr0`

#IMPORTANT  ---> DESTROYS MALLOC AND MEMORY ERRORS
#           ---> ONLY FOR NICE FORMAT OF OUTPUT
#           ---> RUN TESTS TO SEE MEMORY ERRORS!!
export MALLOC_CHECK_=0

if [[ "$1" == "-h" || "$1" == "--help" && -z $2 ]]; then
  usage
fi

#silent run -> ic18int error outputs supressed
if [[ "$1" == "-s" ]];then
  silent="-s"
fi

#disable filegeneration
if [[ "$1" == "-n" ]];then
  filegen="-n"
fi

echo "Testing return values from interpret..."
echo "Running all test dirs from ../tests with expected return value of 0..."
echo ""

touch test.code #temporary file for generated output of ./ifj

testsucc=0  #counter for succ tests
testfail=0  #counter for failed tests

##
# @brief  Run all tests from test directory which doesnt have standard .out and .in files
#
# @param  $1  test directory with tests
##
function testRunner() {

  testpath=$1

  lines=$(find $testpath -type d | wc -l) #number of files in dir
  ((lines--))

  for ((n=0; n<$lines;n++)); do #loop over test dir
    line=$((n+1))
    currentdir=$(find $testpath -maxdepth 1 -mindepth 1 -type d -printf '%f\n' | awk NR==$line) #name of dir with tests
    filescounter=$(find $testpath/$currentdir -type f | wc -l) #number of tests in dir

    for ((m=0; m<$filescounter;m++)); do #loop over tests in dir
      linef=$((m+1))
      testfile=$(find $testpath/$currentdir -type f -printf "%f\n" | awk NR==$linef)  #name of test dir
      testtype=${testfile:0:5} #extrakt nazvu testu

      if [ "$testtype" != "error" ]; then  #all tests without name error should return value of 0
        timeout 1 ../src/ifj < $testpath/$currentdir/$testfile 2>/dev/null > test.code
        retvalIFJ=$(echo $?)

        testname="$testfile.out"  #creating output file by testname
        if [[ "$filegen" == "-n" ]]; then
          :
        else
          touch codeGenData/$testname
        fi

        if [[ "$silent" == "-s" ]]; then
          timeout 1 ./ic18int test.code > /dev/null 2>&1
        else
          timeout 1 ./ic18int test.code 1>/dev/null
        fi
        retval=$(echo $?)
        if [ "$retval" -eq "124" ]; then
            echo "${red}[TEST FAILED]${reset} Program got stucked killed process after 3 sec"
        fi

        if [ "$retvalIFJ" -eq "124" ]; then
          echo "${red}[TEST FAILED]${reset} Program got stucked killed process after 3 sec"
        fi

        if [ "$retval" -eq "0" ]; then
          ((testsucc++))
          echo "${green}[TEST PASSED]${reset}"
        else
          ((testfail++))
          echo "${red}[TEST FAILED]${reset} : $currentdir/$testfile"
          echo "  expected return value = 0. Returned value = $retval"
        fi
      fi
    done
  done
}

##
# @brief  Run all tests from test directory with sample .in and .out files to comparision
#
# @param  $1  test directory with tests
##
function genTester {
testpath=$1

tmpout="temp.out"
touch $tmpout

lines=$(find $testpath -type d | wc -l) #number of files in dir
((lines--))

for ((n=0; n<$lines;n++)); do #loop over test dir
  line=$((n+1))
  currentdir=$(find $testpath -maxdepth 1 -mindepth 1 -type d -printf '%f\n' | awk NR==$line) #name of dir with tests
  filescounter=$(find $testpath/$currentdir -type f | wc -l) #number of tests in dir

  for ((m=0; m<$filescounter;m++)); do #loop over tests in dir
    linef=$((m+1))
    testfile=$(find $testpath/$currentdir -type f -printf "%f\n" | awk NR==$linef)  #name of test dir
    testtype=${testfile:0:5} #extrakt nazvu testu

    filetypeIN=$(echo $testfile | rev) #input file type
    filetypeIN=${filetypeIN:0:2}
    filetypeIN=$(echo $filetypeIN | rev)

    filetypeOUT=$(echo $testfile | rev) #output file type
    filetypeOUT=${filetypeOUT:0:3}
    filetypeOUT=$(echo $filetypeOUT | rev)


    if [ "$filetypeIN" == "18" ] && [ "$filetypeOUT" == "j18" ]; then  #all tests without name error should return value of 0 + exclude .in and .out files
      testCheck=$(echo $testfile | rev)
      testCheck=${testCheck:0:2}
      if [ "$testCheck" != "ni" ] || [ "$testCheck" != "tu" ]; then

        timeout 1 ../src/ifj < $testpath/$currentdir/$testfile 2>/dev/null > test.code
        retvalIFJ=$(echo $?)

        infile="$testfile.in"
        outfile="$testfile.out"

        timeout 1 ./ic18int test.code < $testpath/$currentdir/$infile > $tmpout
        retval=$(echo $?)
        if [ "$retval" -eq "124" ]; then
          echo "${red}[TEST FAILED]${reset} Program got stucked killed process after 3 sec"
        fi

        if [ "$retvalIFJ" -eq "124" ]; then
          echo "${red}[TEST FAILED]${reset} Program got stucked killed process after 3 sec"
        fi

        if [ "$retval" -eq "0" ]; then
          ((testsucc++))
          echo "${green}[TEST PASSED]${reset}"
        else
          ((testfail++))
          echo "${red}[TEST FAILED]${reset} : $currentdir/$testfile"
          echo "  expected return value = 0. Returned value = $retval"
        fi
        echo "" >> $tmpout
        storeError=$((cmp $tmpout $testpath/$currentdir/$outfile) 2>&1)
        ret=$(echo $?)

        if [[ $storeError == *"cmp: EOF"* ]]; then
         ret="0" #end of file error not error od tests
        else
          cmp $tmpout $testpath/$currentdir/$outfile
        fi

        if [[ "$ret" -eq "1" ]]; then
          if [[ "$retval" -eq "0" ]]; then
            ((testfail++));
            ((testsucc--));
           fi
          echo "${red}[TEST FAILED]${reset} Outputs aren't the same!";
        fi
      fi
    fi
  done
done
rm $tmpout
}

#TESTS FOR code_gen only in prece.c for @Maros
if [[ "$1" == "-e" ]]; then
  testDIR="preceGen"
  filegen="-n"
  testRunner "$testDIR"
elif [[ "$1" == "-g" ]]; then
  testDIR="genData"
  filegen="-n"
  genTester "$testDIR"
else
  TESTS FOR testData
  testDIR="testData"
  testRunner "$testDIR"

  #TESTS FOR syntaxData
  testDIR="syntaxData"
  testRunner "$testDIR"

  #TESTS FOR semanticData
  testDIR="semanticData"
  testRunner "$testDIR"

  #TESTS FOR expressionsData
  testDIR="expressionsData"
  testRunner "$testDIR"

  #TESTS FOR prece
  testDIR="preceGen"
  filegen="-n"
  testRunner "$testDIR"

  #inbuild
  testDIR="genData"
  filegen="-n"
   genTester "$testDIR"
fi



#tests total succession
tests=$((testfail+testsucc))
total=$(echo " scale=2;
var1 = $testsucc * 100;
var1 = var1 / $tests;
var1 " \
| bc)

echo "TESTED $tests tests, PASSED: $testsucc, FAILED: $testfail Totally: ${green}$total%${reset}"
