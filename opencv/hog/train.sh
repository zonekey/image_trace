opencv_traincascade -vec faces_16_16.vec -bg bg.txt -info info.dat -numPos 1500 -numNeg 2000 -data cascade_16_16/ -precalcValBufSize 2048 -precalcIdxBufSize 2048 -w 16 -h 16 -featureType hog -numStages 25

