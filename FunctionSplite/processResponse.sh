
echo "Important Message: Ensure you have set fifo=1 in mb1/main and mb2/main"

max=20
for ((i=1, j=2;  i<=$max; ++i,++j )) ; 
do
    #Update the FIFO size in code
    find mb1/ -name '*.c' | xargs sed -i "s/fifo_size=$i/fifo_size=$j/g"
    find mb2/ -name '*.c' | xargs sed -i "s/fifo_size=$i/fifo_size=$j/g"

    #make clean;
    #make run;

    echo "MB1 FIFO Size" >> response.txt
    grep fifo_size mb1/main.c | head -n1  >> response.txt
    echo "MB2 FIFO Size" >> response.txt
    grep fifo_size mb2/main.c | head -n1  >> response.txt

    echo "ImageSize" >> response.txt
    grep ^IMAGE_OUTPUT_HEIGHT Makefile >> response.txt
    grep ^IMAGE_OUTPUT_WIDTH  Makefile >> response.txt

    echo "ImageName" >> response.txt
    grep ^DATA_FILES  Makefile  >> response.txt

    echo "Compiliation Flags" >> response.txt
    grep ^OPTFLAGS  Makefile  >> response.txt

    echo "MB1 Timing" >> response.txt
    grep --binary-files=text ^MB1.*.Error:.*.0DDE response.txt >> response.txt
    echo "MB2 Timing" >> response.txt
    grep --binary-files=text ^MB2.*.Error:.*.0DDE response.txt >> response.txt
    echo "MB3 Timing" >> response.txt
    grep --binary-files=text ^MB3.*.Error:.*.0DDE response.txt >> response.txt

    mv response.txt $j."$(date +%Y%m%d_%H%M%S)".txt 
    mv $j."$(date +%Y%m%d_%H%M%S)".txt /home/emb14/emb1405/DataSet/
done

    find mb1/ -name '*.c' | xargs sed -i "s/fifo_size=21/fifo_size=1/g"
    find mb2/ -name '*.c' | xargs sed -i "s/fifo_size=21/fifo_size=1/g"


