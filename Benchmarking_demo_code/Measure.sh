#!/bin/bash
LOCALMEM=$("/usr/local/bin/memory.sh" | tail -n 1 | awk '{ print $2}');
# Get Benchmarking parameters as the outputs from MB1, they have to be in the right order
if [ $? -eq 0 ]
then
	# Get the outputs from MB1
	MB1_RESPONSE[0]=$(egrep "MB1 Error" --binary-files=text response.txt | grep '00B1'  | head -n 1 | awk '{ print $7}')
	MB1_RESPONSE[1]=$(egrep "MB1 Error" --binary-files=text response.txt | grep '00B2'  | head -n 1 | awk '{ print $7}')
	MB1_RESPONSE[2]=$(egrep "MB1 Error" --binary-files=text response.txt | grep '00B3'  | head -n 1 | awk '{ print $7}')
	MB1_RESPONSE[3]=$(egrep "MB1 Error" --binary-files=text response.txt | grep '00B4'  | head -n 1 | awk '{ print $7}')
  	MB1_RESPONSE[4]=$(egrep "MB1 Error" --binary-files=text response.txt | grep '00B5'  | head -n 1 | awk '{ print $7}')
  
	# Get the outputs from MB1
	MB2_RESPONSE[0]=$(egrep "MB2 Error" --binary-files=text response.txt | grep '00B2'  | head -n 1 | awk '{ print $7}')
	MB2_RESPONSE[1]=$(egrep "MB2 Error" --binary-files=text response.txt | grep '00B3'  | head -n 1 | awk '{ print $7}')
	MB2_RESPONSE[2]=$(egrep "MB2 Error" --binary-files=text response.txt | grep '00B4'  | head -n 1 | awk '{ print $7}')
	
	# Get the outputs from MB3
	MB3_RESPONSE[0]=$(egrep "MB3 Error" --binary-files=text response.txt | grep '00B2'  | head -n 1 | awk '{ print $7}')
	MB3_RESPONSE[1]=$(egrep "MB3 Error" --binary-files=text response.txt | grep '00B3'  | head -n 1 | awk '{ print $7}')
	MB3_RESPONSE[2]=$(egrep "MB3 Error" --binary-files=text response.txt | grep '00B4'  | head -n 1 | awk '{ print $7}')

	DDR[0]=$(egrep "Have to" --binary-files=text response.txt | grep 'to'  | head -n 1 | awk '{ print $4}')
	DDR[1]=$(egrep "binary size" --binary-files=text response.txt | grep 'size'  | head -n 1 | awk '{ print $2}')
	
	#00B1
	MB1_EXEC=$(echo "ibase=16; ${MB1_RESPONSE[0]%?}"|bc)
	MB1_EXEC_F=$(echo "scale=10;$MB1_EXEC/1000000" | bc)
	#00B2
	DMA1=$(echo "ibase=16; ${MB1_RESPONSE[1]%?}"|bc) 
	DMA2=$(echo "ibase=16; ${MB2_RESPONSE[0]%?}"|bc) 
	DMA3=$(echo "ibase=16; ${MB3_RESPONSE[0]%?}"|bc) 
	DMA_COUNT=$(($DMA1+$DMA2+$DMA3))
	#00B3
	COMM1=$(echo "ibase=16; ${MB1_RESPONSE[2]%?}"|bc) 
	COMM2=$(echo "ibase=16; ${MB2_RESPONSE[1]%?}"|bc) 
	COMM3=$(echo "ibase=16; ${MB3_RESPONSE[1]%?}"|bc) 
	COMM_COUNT=$(($COMM1+$COMM2+$COMM3))
	#00B4
	DYN1=$(echo "ibase=16; ${MB1_RESPONSE[3]%?}"|bc) 
	DYN2=$(echo "ibase=16; ${MB2_RESPONSE[2]%?}"|bc) 
	DYN3=$(echo "ibase=16; ${MB3_RESPONSE[2]%?}"|bc) 
	DYN_COUNT=$(($DYN1+$DYN2+$DYN3))
	#DDR
	DDR1=$(echo "${DDR[0]%?}")
	DDR2=$(echo "${DDR[1]%?}" | cut -d '"' -f2)
	DDR3=$(echo "ibase=16; ${MB1_RESPONSE[4]%?}"|bc)
	DDR_COUNT=$(($DDR1+$DDR2+$DDR3))
	
	echo "Benchmarking parameters for code base in this directory          : $(pwd)" 	 >> Log.txt
	echo "Date and time of run                                             : $(date)"	 >> Log.txt
	echo "-------------------------------------------------------------------------"	 >> Log.txt
	echo "Total execution time taken by the decoder in seconds             : $MB1_EXEC_F"    >> Log.txt
	echo "Total number of DMAs used by all the core		         : $DMA_COUNT" 	 	 >> Log.txt
	echo "Total data memory and instruction memory (local memory) in bytes : $LOCALMEM"      >> Log.txt
	echo "Total amount of Communication memory usage by all cores in bytes : $COMM_COUNT"    >> Log.txt
	echo "Total amount of Dynamic memory usage by all cores in bytes 	 : $DYN_COUNT"	 >> Log.txt
	echo "Total amount of DDR usage by all cores in bytes 	 	 : $DDR_COUNT"	   	 >> Log.txt
	echo "-------------------------------------------------------------------------"	 >> Log.txt
else
	echo "Error :  Please Debug";	 >> Log.txt
fi

