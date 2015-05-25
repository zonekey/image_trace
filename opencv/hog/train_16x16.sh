opencv_traincascade -vec faces_16_16.vec -bg bg.txt -info info.dat -numPos 800 \
						-numNeg 4000 -data cascade_16_16/ -precalcValBufSize 2048 \
						-precalcIdxBufSize 2048 -w 16 -h 16

