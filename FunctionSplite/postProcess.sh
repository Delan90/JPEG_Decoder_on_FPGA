echo "$(grep --binary-files=text  ^DATA_FILES response.txt | awk '{ print $2 }')"";" 
echo "$(grep --binary-files=text  HEIGHT response.txt | awk '{ print $2 }')"";" 
echo "$(grep --binary-files=text  WIDTH response.txt | awk '{ print $2 }')"";" 
echo "$(grep --binary-files=text  ^OPTFLAGS response.txt | awk '{ print $2 }')"";" 
#fifo already has semi-colon at the end of line.
echo "$(grep --binary-files=text  fifo_size response.txt |head -n1| awk '{ print $3 }')" 

echo "$(grep --binary-files=text  ^MB1.*.Error:.*.0DDE response.txt | awk '{ print $7}')"";" 
echo "$(grep --binary-files=text  ^MB2.*.Error:.*.0DDE response.txt | awk '{ print $7 }')"";"
echo "$(grep --binary-files=text  ^MB3.*.Error:.*.0DDE response.txt | awk '{ print $7 }')"";"
