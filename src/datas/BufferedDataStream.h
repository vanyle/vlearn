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

#include <mutex>
#include <cstdint>

typedef uint32_t uint;

namespace vio{

struct BufferedDataStream {
	char* data;
	uint capacity;
	std::mutex dataMutex;

	uint start = 0;
	uint length = 0;

	unsigned int writeRequestSize = 0;

	bool isClosed = false;

	BufferedDataStream(uint capacity = 16384); // capacity in bytes
	~BufferedDataStream();

	uint spaceAvailable();
	uint dataStoredSize(); // dataStored + spaceAvailable = bufferSize

	// locks if there is not enough space available ie spaceAvailable() < size
	void write(void* input, uint size); // might lock if not enough room is available

	// never locks
	uint read(void* output, uint size); // returns number of bytes written to output
	void close();
};

template<typename T>
struct GeneralBufferedDataStream {
	BufferedDataStream bds = BufferedDataStream(sizeof(T) * 20);
	void write(T* element) { // can lock
		bds.write(element, sizeof(T));
	}
	uint spaceAvailable() {
		return bds.spaceAvailable() / sizeof(T);
	}
	uint dataStoredSize() {
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

}