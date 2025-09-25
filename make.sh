#!/bin/bash

# Separate PSG-inclusive music tracker files into PSG + DMA
for file in gbt/*; do 
    if [ -f "$file" ]; then 
        basename=$( echo "$file" | sed -r "s/.+\/(.+)\..+/\1/" )
        utils/s3msplit/s3msplit.py \
            --input $file \
            --psg psg/${basename}_psg.s3m \
            --dma music/${basename}_dma.s3m
        utils/s3m2gbt/s3m2gbt.py \
            --input psg/${basename}_psg.s3m \
            --name ${basename}_psg \
            --output psg/${basename}_psg.c \
            --instruments
        rm psg/${basename}_psg.s3m
    fi 
done
cp maxmod/* music/

make