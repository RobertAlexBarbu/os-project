cFile=$1;
gcc -Wall -o test $1 2> log.txt
errors=-1
warnings=-1
function checkError {
  if echo $1 | grep "error"
  then
    return 1
  else
    return 0
  fi
}
function checkWarning {
  if echo $1 | grep "warning"
  then
    return 1
  else
    return 0
  fi
}
while read line
do
  checkError "$line"
  errors=`expr $errors + $?`
  checkWarning "$line"
  warnings=`expr $warnings + $?`
done < log.txt

if test "$warnings" -eq -1
then
  warnings=`expr $warnings + 1`
fi
if test "$errors" -eq -1
then
  errors=`expr $errors + 1`
fi
echo "((start)) $errors errors $warnings warnings ((stop))"
exit 0
