############################################################################
#Benchmarking Script Single Image                                          #
#Initial Configuration of the Makefile should be as given below            #
#IMAGE_OUTPUT_HEIGHT?=Height                                               #                                                      
#IMAGE_OUTPUT_WIDTH?=Width                                                 #
#DATA_FILES=../input/Image.jpg                                                #
############################################################################
#Author : Benchmarking Team ESL - 2013/14                                 #
############################################################################

#Removes old files
rm -R output 
rm Log.txt

#Creates a new directory for output files
mkdir output


if [ $# -eq 0 ] 
then 
	echo "Please send arguments as : ./SingleImage.sh FileName (without file extension) Width Height"
else
#Image 1
	echo Start of testing jpeg file : "$1" >> Log.txt 
	sed -i "s/Image/$1/g" Makefile
	sed -i "s/Height/$3/g" Makefile
	sed -i "s/Width/$2/g" Makefile
	rm binary.pnm
	make clean
	make run
	bmptoppm "../reference_output/$1.bmp" > "../reference_output/$1.ppm"
	echo "---------------------Image Comparison------------"  >> Log.txt 
	diff ../reference_output/"$1".ppm binary.pnm >> Log.txt
	cp binary.pnm output/"$1".pnm
	echo ""  >> Log.txt
	echo "---------------------Measurement-----------------"  >> Log.txt 
	bash Measure.sh
	echo End of testing jpeg file : "$1" >> Log.txt


	#Finishing Sequence
	sed -i "s/$1/Image/g" Makefile
	sed -i "s/$2/Width/g" Makefile
	sed -i "s/$3/Height/g" Makefile
	#End
	cat Log.txt

	echo "---------------------End of script-------------------------" >> Log.txt
fi
