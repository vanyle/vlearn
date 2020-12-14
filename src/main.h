// What is this ?
/*
This is a queue like data structure made to pass messages around different threads.
You can write as much data as you like to the queue by writing might lock the thread
as the structure waits for the other end to read some data.

Reading data never locks the read but might return garbage is nothing is available.
Call dataStoredSize() to know if stuff is available.

When reading / writing, a (shallow) copy of the data given is made.
Because of the way the copying is done, this data structure is not made
for fancy styles with complex contructors / destructors, just for simple structs.
*/

struct BufferedDataStream {
	char* data;
	unsigned int capacity;
	std::mutex dataMutex;

	unsigned int start = 0;
	unsigned int length = 0;

	unsigned int writeRequestSize = 0;

	bool isClosed = false;

	BufferedDataStream(unsigned int capacity = 16384); // capacity in bytes
	~BufferedDataStream();

	unsigned int spaceAvailable();
	unsigned int dataStoredSize(); // dataStored + spaceAvailable = bufferSize

	// locks if there is not enough space available ie spaceAvailable() < size
	void write(void* input, unsigned int size); // might lock if not enough room is available

	// never locks
	unsigned int read(void* output, unsigned int size); // returns number of bytes written to output
	void close();
};

template<typename T>
struct GeneralBufferedDataStream {
	BufferedDataStream bds = BufferedDataStream(sizeof(T) * 20);
	void write(T* element) { // can lock
		bds.write(element, sizeof(T));
	}
	unsigned int spaceAvailable() {
		return bds.spaceAvailable() / sizeof(T);
	}
	unsigned int dataStoredSize() {
		return bds.dataStoredSize() / sizeof(T);
	}
	// returns garbage if no element is available.
	// will never lock thread
	T read() {
		T element;
		//T * buffer = new T;
		bds.read(&element, sizeof(T));
		//element = *buffer;
		//delete buffer;
		return element;
	}
	// prevents writing and reading from the buffer and forces the mutex to unlock.
	void close() {
		bds.close();
	}
};