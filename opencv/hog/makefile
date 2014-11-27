all: m_hog_prepare m_hog_train m_hog_test

CFLAGS=`pkg-config --cflags opencv`
LDFLAGS=`pkg-config --libs opencv`

util.o: util.cpp util.h
	$(CXX) -o $@ -c -O2 util.cpp $(CFLAGS)

m_hog_prepare: m_hog_prepare.cpp util.o
	$(CXX) -o $@ -O2 $(CFLAGS) $^ $(LDFLAGS)

m_hog_train: m_hog_train.cpp
	$(CXX) -o $@ -O2 $(CFLAGS) $^ $(LDFLAGS)

m_hog_test: m_hog_test.cpp
	$(CXX) -o $@ -O2 $(CFLAGS) $^ $(LDFLAGS)

clean:
	rm -f m_hog_prepare m_hog_train m_hog_test
	rm -f *.o

