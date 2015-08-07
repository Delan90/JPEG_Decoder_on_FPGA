############################################################################
#Benchmarking Script                                                       #
#Initial Configuration of Makefile should be as given below                #
#IMAGE_OUTPUT_HEIGHT?=Height                                               #                                                      
#IMAGE_OUTPUT_WIDTH?=Width                                                 #
#DATA_FILES=../input/Image.jpg                                                #
############################################################################
#Author : Benchmarking Team ESL - 2013/14                                  #
############################################################################


#Remove old files and folders
rm -R output 
rm Log.txt

#Create a new folder for output files
mkdir output

#Image 1
echo start Reference.jpg >> Log.txt
sed -i "s/Image/Reference/g" ../Makefile
sed -i "s/Height/768/g" ../Makefile
sed -i "s/Width/1024/g" ../Makefile
rm binary.pnm
make clean
make run
bmptoppm "../reference_output/Reference.bmp" > "../reference_output/Reference.ppm"
echo "---------------------Image Comparison----------------------"  >> Log.txt 
diff ../reference_output/Reference.ppm binary.pnm >> Log.txt
cp binary.pnm output/Reference.pnm
echo ""  >> Log.txt
echo "---------------------Measurement---------------------------"  >> Log.txt  
bash Measure.sh
echo end Reference.jpg >> Log.txt 



#Image 2
echo start Restart_intervals.jpg >> Log.txt
sed -i "s/Reference/Restart_intervals/g" Makefile
rm binary.pnm
make clean
make run
bmptoppm "../reference_output/Restart_intervals.bmp" > "../reference_output/Restart_intervals.ppm"
echo "---------------------Image Comparison----------------------"  >> Log.txt 
diff ../reference_output/Restart_intervals.ppm binary.pnm >> Log.txt
cp binary.pnm output/Restart_intervals.pnm
echo ""  >> Log.txt
echo "---------------------Measurement---------------------------"  >> Log.txt 
bash Measure.sh
echo end Restart_intervals.jpg >> Log.txt 



#Image 3
echo start Subsampling_122.jpg >> Log.txt
sed -i "s/Restart_intervals/Subsampling_122/g" Makefile
rm binary.pnm
make clean
make run
bmptoppm "../reference_output/Subsampling_122.bmp" > "../reference_output/Subsampling_122.ppm"
echo "---------------------Image Comparison----------------------"  >> Log.txt 
diff ../reference_output/Subsampling_122.ppm binary.pnm >> Log.txt
cp binary.pnm output/Subsampling_122.pnm
echo ""  >> Log.txt
echo "---------------------Measurement---------------------------"  >> Log.txt 
bash Measure.sh
echo end Subsampling_122.jpg >> Log.txt 



#Image 4
echo start Black_white.jpg >> Log.txt
sed -i "s/Subsampling_122/Black_white/g" Makefile
rm binary.pnm
#make clean
#make run
bmptoppm "../reference_output/Black_white.bmp" > "../reference_output/Black_white.ppm"
echo "---------------------Image Comparison----------------------"  >> Log.txt 
diff ../reference_output/Black_white.ppm binary.pnm >> Log.txt
#cp binary.pnm output/Black_white.pnm
echo ""  >> Log.txt
echo "---------------------Measurement---------------------------"  >> Log.txt 
bash Measure.sh
echo end Black_white.jpg >> Log.txt 



#Image 5
echo start Landscape.jpg >> Log.txt
sed -i "s/Black_white/Landscape/g" Makefile
rm binary.pnm
make clean
make run
bmptoppm "../reference_output/Landscape.bmp" > "../reference_output/Landscape.ppm"
echo "---------------------Image Comparison----------------------"  >> Log.txt 
diff ../reference_output/Landscape.ppm binary.pnm >> Log.txt
cp binary.pnm output/Landscape.pnm
echo ""  >> Log.txt
echo "---------------------Measurement---------------------------"  >> Log.txt 
bash Measure.sh
echo end Landscape.jpg >> Log.txt 



#Image 6
echo start Low_frequency.jpg >> Log.txt
sed -i "s/Landscape/Low_frequency/g" Makefile
rm binary.pnm
make clean
make run
bmptoppm "../reference_output/Low_frequency.bmp" > "../reference_output/Low_frequency.ppm"
echo "---------------------Image Comparison----------------------"  >> Log.txt 
diff ../reference_output/Low_frequency.ppm binary.pnm >> Log.txt
cp binary.pnm output/Low_frequency.pnm
echo ""  >> Log.txt
echo "---------------------Measurement---------------------------"  >> Log.txt 
bash Measure.sh
echo end Low_frequency.jpg >> Log.txt



#Image 7
echo start Uround_MCU_blocks.jpg >> Log.txt
sed -i "s/Low_frequency/Uround_MCU_blocks/g" Makefile
sed -i "s/1024/369/g" Makefile
sed -i "s/768/353/g" Makefile
rm binary.pnm
make clean
make run
bmptoppm "../reference_output/Uround_MCU_blocks.bmp" > "../reference_output/Uround_MCU_blocks.ppm"
echo "---------------------Image Comparison----------------------"  >> Log.txt 
diff ../reference_output/Uround_MCU_blocks.ppm binary.pnm >> Log.txt
cp binary.pnm output/Uround_MCU_blocks.pnm
echo ""  >> Log.txt
echo "---------------------Measurement---------------------------"  >> Log.txt  
bash Measure.sh
echo end Uround_MCU_blocks.jpg >> Log.txt



#Image 8
echo start Critical_size.jpg >> Log.txt
sed -i "s/Uround_MCU_blocks/Critical_size/g" Makefile
sed -i "s/369/17/g" Makefile
sed -i "s/353/1/g" Makefile
rm binary.pnm
make clean
make run
bmptoppm "../reference_output/Critical_size.bmp" > "../reference_output/Critical_size.ppm"
echo "---------------------Image Comparison----------------------"  >> Log.txt 
diff ../reference_output/Critical_size.ppm binary.pnm >> Log.txt
cp binary.pnm output/Critical_size.pnm
echo ""  >> Log.txt
echo "---------------------Measurement---------------------------"  >> Log.txt 
bash Measure.sh
echo end Critical_size.jpg >> Log.txt



#Image 9
echo start Random_image.jpg >> Log.txt
sed -i "s/Critical_size/Random_image/g" Makefile
sed -i "s/17/600/g" Makefile
sed -i "s/IMAGE_OUTPUT_HEIGHT?=1/IMAGE_OUTPUT_HEIGHT?=600/g" Makefile
rm binary.pnm
#make clean
#make run
bmptoppm "../reference_output/Random_image.bmp" > "../reference_output/Random_image.ppm"
echo "---------------------Image Comparison----------------------"  >> Log.txt 
diff ../reference_output/Random_image.ppm binary.pnm >> Log.txt
#cp binary.pnm output/Random_image.pnm
echo ""  >> Log.txt 
echo "---------------------Measurement---------------------------"  >> Log.txt 
bash Measure.sh
echo end Random_image.jpg >> Log.txt




#Finishing Sequence
sed -i "s/Random_image/Image/g" Makefile
sed -i "s/IMAGE_OUTPUT_WIDTH?=600/IMAGE_OUTPUT_WIDTH?=Width/g" Makefile
sed -i "s/IMAGE_OUTPUT_HEIGHT?=600/IMAGE_OUTPUT_HEIGHT?=Height/g" Makefile
#End



echo "---------------------End of script-------------------------" >> Log.txt
