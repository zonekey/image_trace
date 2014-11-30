all: m_prepare m_hog_train m_hog_test m_cas_test m_neg_maker test_rotate

CFLAGS=`pkg-config --cflags opencv`
LDFLAGS=`pkg-config --libs opencv`

util.o: util.cpp util.h
	$(CXX) -o $@ -c -O2 util.cpp $(CFLAGS)

m_prepare: m_prepare.cpp util.o
	$(CXX) -o $@ -O2 $(CFLAGS) $^ $(LDFLAGS)

m_hog_train: m_hog_train.cpp
	$(CXX) -o $@ -O2 $(CFLAGS) $^ $(LDFLAGS)

m_hog_test: m_hog_test.cpp
	$(CXX) -o $@ -O2 $(CFLAGS) $^ $(LDFLAGS)

m_cas_test: m_haar_cascade_test.cpp
	$(CXX) -o $@ -O2 $(CFLAGS) $^ $(LDFLAGS)

m_neg_maker: m_neg_maker.cpp
	$(CXX) -o $@ -O2 $(CFLAGS) $^ $(LDFLAGS)

test_rotate: test_rotate.cpp
	$(CXX) -o $@ -O2 $(CFLAGS) $^ $(LDFLAGS)

clean:
	rm -f m_prepare m_hog_train m_hog_test m_neg_maker
	rm -f *.o
