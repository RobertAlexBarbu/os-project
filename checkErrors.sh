cFile=$1;
gcc -Wall $1 2> log.txt
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

if [ $warnings -eq -1 ]
then
  echo "We have $errors error/s"
fi
if [ $errors -eq -1 ]
then
  echo "We have $warnings warning/s"
fi
exit 0