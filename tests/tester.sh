#!/bin/bash

#
 # Predmet:  IFJ
 # Projekt:  Implementace prekladace imperativniho jazyka IFJ18
 # Soubor:   tester.bash
 #
 # Popis:  Skript pre automaticke testovanie lexikalnej analyzy pomocou vstupnych dat
 #
 # Datum: 23.9.2018 20:20
 #
 # Autori:   Maros Orsak       vedouci
 #           Polishchuk Kateryna     <xpolis03@fit.vutbr.cz>
 #           Igor Ignac
 #           Marek Rohel
#

usage() {
  echo  "Automaticke testy pre jazyk ifj18. Spustenie testov pomocou:
        bash tester.bash [program] [test directory]
        program - nazov prelozeneho programu(ifj)
        test directory - nazov zlozky s testami"
  exit 1
}

#nastavenie farieb pre farebny vystup testov
red=`tput setaf 1`
green=`tput setaf 2`
reset=`tput sgr0`

#uplna cesta k testom
testpath="$(realpath .)/$2"

if [[ "$1" == "--help" && -z $2 ]]; then
  usage

elif [[ ! -z "$1" && ! -z "$2" ]]; then
  prog="$(realpath .| rev | cut -c 6- | rev)src/$1" #cesta k prelozenemu projektu
  lines=$(find $testpath -type d | wc -l) #pocet suborov s testami
  ((lines--))
  testsucc=0  #counter pre uspesne testy
  testfail=0  #counter pre neuspesne testy

  for ((n=0; n<$lines;n++)); do #loop cez testovacie adresare
    line=$((n+1))
    currentdir=$(find $testpath -maxdepth 1 -mindepth 1 -type d -printf '%f\n' | awk NR==$line) #nazov adresaru s testami
    filescounter=$(find $testpath/$currentdir -type f | wc -l) #pocet testov v adresari

    for ((m=0; m<$filescounter;m++)); do #loop cez konkretne testy v adresaroch
      linef=$((m+1))
      testfile=$(find $testpath/$currentdir -type f -printf "%f\n" | awk NR==$linef)  #nazov testovacieho suboru
      testtype=${testfile:0:6} #extrakt nazvu testu

      testCheck=$(echo $testfile | rev)
      testCheck=${testCheck:0:2}
      if [ "$testCheck" == "ni" ] || [ "$testCheck" == "tu" ]; then #skip vsetky subory ktore maju priponu .in alebo .out
        continue
      fi

      if [ "$testtype" == "error1" ]; then  #vsetky testy obsahujuce chybu maju na zaciatku error. error1->chyba v programu v rámci lexikální analýzy (chybná struktura aktuálního lexému).
        timeout 1 $prog < $testpath/$currentdir/$testfile > /dev/null 2>&1  #presmerovanie vystupu lexeru
        retval=$(echo $?)
        if [ "$retval" -eq "124" ]; then
            echo "${red}[TEST FAILED]${reset} Program got stucked killed process after 3 sec"
        fi
        if [ "$retval" -eq "1" ]; then
          ((testsucc++))
          echo "${green}[TEST PASSED]${reset}"
        else
          ((testfail++))
          echo "${red}[TEST FAILED]${reset} : $currentdir/$testfile"
          echo "  expected return value = 1. Returned value = $retval"
        fi
      ##### Syntax tests  #####
      elif [ "$testtype" == "error2" ]; then #testy oznacene error2 --> chyba v programu v rámci syntaktické analýzy (chybná syntaxe programu).
        timeout 1 $prog < $testpath/$currentdir/$testfile > /dev/null 2>&1
        retval=`echo $?`

        if [ "$retval" -eq "124" ]; then
            echo "${red}[TEST FAILED]${reset} Program got stucked killed process after 3 sec"
        fi
        if [ "$retval" -eq "2" ]; then
          ((testsucc++))
          echo "${green}[TEST PASSED]${reset}"
        else
          ((testfail++))
          echo "${red}[TEST FAILED]${reset} : $currentdir/$testfile"
          echo "  expected return value = 2. Returned value = $retval"
        fi
      ##### Semantic tests  #####
      elif [ "$testtype" == "error3" ]; then #testy oznacene error3 --> sémantická chyba v programu – nedefinovaná funkce/proměnná, pokus o redefinici funkce/proměnné, atp
        timeout 1 $prog < $testpath/$currentdir/$testfile > /dev/null 2>&1
        retval=$(echo $?)
        if [ "$retval" -eq "124" ]; then
            echo "${red}[TEST FAILED]${reset} Program got stucked killed process after 3 sec"
        fi
        if [ "$retval" -eq "3" ]; then
          ((testsucc++))
          echo "${green}[TEST PASSED]${reset}"
        else
          ((testfail++))
          echo "${red}[TEST FAILED]${reset} : $currentdir/$testfile"
          echo "  expected return value = 3. Returned value = $retval"
        fi
      ##### Semantic tests of type compability  #####
    elif [ "$testtype" == "error4" ]; then #testy oznacene error4 --> sémantická/běhová chyba typové kompatibility v aritmetických, řetězcových a relačních výrazech.
      timeout 1 $prog < $testpath/$currentdir/$testfile > /dev/null 2>&1
      retval=$(echo $?)
      if [ "$retval" -eq "124" ]; then
          echo "${red}[TEST FAILED]${reset} Program got stucked killed process after 3 sec"
      fi
      if [ "$retval" -eq "4" ]; then
        ((testsucc++))
        echo "${green}[TEST PASSED]${reset}"
      else
        ((testfail++))
        echo "${red}[TEST FAILED]${reset} : $currentdir/$testfile"
        echo "  expected return value = 4. Returned value = $retval"
      fi
      elif [ "$testtype" == "error5" ]; then #error5 -> sémantická chyba v programu – špatný počet parametrů u volání funkce.
        timeout 1 $prog < $testpath/$currentdir/$testfile > /dev/null 2>&1
        retval=$(echo $?)
        if [ "$retval" -eq "124" ]; then
            echo "${red}[TEST FAILED]${reset} Program got stucked killed process after 3 sec"
        fi
        if [ "$retval" -eq "5" ]; then
          ((testsucc++))
          echo "${green}[TEST PASSED]${reset}"
        else
          ((testfail++))
          echo "${red}[TEST FAILED]${reset} : $currentdir/$testfile"
          echo "  expected return value = 5. Returned value = $retval"
        fi
      elif [ "$testtype" == "error6" ]; then #error6 -> ostatní sémantické chyby
        timeout 1 $prog < $testpath/$currentdir/$testfile > /dev/null 2>&1
        retval=$(echo $?)
        if [ "$retval" -eq "124" ]; then
            echo "${red}[TEST FAILED]${reset} Program got stucked killed process after 3 sec"
        fi
        if [ "$retval" -eq "6" ]; then
          ((testsucc++))
          echo "${green}[TEST PASSED]${reset}"
        else
          ((testfail++))
          echo "${red}[TEST FAILED]${reset} : $currentdir/$testfile"
          echo "  expected return value = 6. Returned value = $retval"
        fi
      ##### Zero division #####
      elif [ "$testtype" == "error9" ]; then #error9 -> běhová chyba dělení nulou.
        timeout 1 $prog < $testpath/$currentdir/$testfile > /dev/null 2>&1
        retval=$(echo $?)
        if [ "$retval" -eq "124" ]; then
            echo "${red}[TEST FAILED]${reset} Program got stucked killed process after 3 sec"
        fi
        if [ "$retval" -eq "9" ]; then
          ((testsucc++))
          echo "${green}[TEST PASSED]${reset}"
        else
          ((testfail++))
          echo "${red}[TEST FAILED]${reset} : $currentdir/$testfile"
          echo "  expected return value = 9. Returned value = $retval"
        fi
      ##### internal error  #####
    elif [ "$testtype" == "error99" ]; then #error99 -> interní chyba překladače tj. neovlivněná vstupním programem (např. chyba alokace paměti, atd.).
      timeout 1 $prog < $testpath/$currentdir/$testfile > /dev/null 2>&1
      retval=$(echo $?)
      if [ "$retval" -eq "124" ]; then
          echo "${red}[TEST FAILED]${reset} Program got stucked killed process after 3 sec"
      fi
      if [ "$retval" -eq "99" ]; then
        ((testsucc++))
        echo "${green}[TEST PASSED]${reset}"
      else
        ((testfail++))
        echo "${red}[TEST FAILED]${reset} : $currentdir/$testfile"
        echo "  expected return value = 99. Returned value = $retval"
      fi
      ##### Success #####
      else
        timeout 1  $prog < $testpath/$currentdir/$testfile > /dev/null 2>&1
        retval=$(echo $?)
        if [ "$retval" -eq "124" ]; then
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

  #Totalna uspesnost testov v zlozke
  tests=$((testfail+testsucc))
  total=$(echo " scale=2;
  var1 = $testsucc * 100;
  var1 = var1 / $tests;
  var1 " \
  | bc)

  echo "TESTED $tests tests, PASSED: $testsucc, FAILED: $testfail Totally: ${green}$total%${reset}"
  echo "..."
  echo "Trying to run valgrind to check for correct allocations"

  #Navratova hodnota valgrindu nastavena na 66 aby sa dal detekovat problem pri alokacii
  valgrind --leak-check=full --show-reachable=yes --error-exitcode=66 $prog < testData/01basic_programs/example_prog01.ifj18 > /dev/null 2>&1

  if [ $? -eq "66" ]; then
    echo "${red}[TEST FAILED]${reset} : Memory wasn't deallocated successfully"
    echo "  Run valgrind for further details"
  else
    echo "${green}[TEST PASSED]${reset} : All allocated memory was deallocated"
  fi
fi
