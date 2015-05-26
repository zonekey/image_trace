opencv_traincascade -vec faces_24_24.vec \
					-bg bg.txt \
					-info info.dat \
					-numPos 300 \
					-numNeg 400 \
					-data cascade_24_24/ \
					-w 24 -h 24

