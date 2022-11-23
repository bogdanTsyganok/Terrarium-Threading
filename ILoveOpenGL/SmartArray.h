#pragma once

template <class T>
class cSmartArray
{
private:
	const unsigned int INIT_SIZE = 25;
public:
	unsigned int currentSize;
	unsigned int currentEndIndex = 0;
	cSmartArray()
	{
		currentSize = INIT_SIZE;
		allocateSpace();
	}

	~cSmartArray() {
		delete[] data;
	};

	T** data;

	T* getAtIndex(unsigned int index)
	{
		return this->data[index];
	}

	void allocateSpace()
	{
		unsigned int prevSize = currentSize;
		currentSize *= 2;
		T** pTempArray = new T*[currentSize];
		for (unsigned int i = 0; i != currentSize; i++)
		{
			pTempArray[i] = nullptr;
		}
		for (unsigned int i = 0; i != this->currentEndIndex; i++)
		{
			pTempArray[i] = data[i];
		}
		data = pTempArray;
		return;
	}

	/*cSmartArray<T>* operator=(const cSmartArray<T> arr)
	{
		this->data = arr.data;
		return this;
	}*/

	void insertAtEnd(T* thePerson)
	{
		data[currentEndIndex] = thePerson;
		currentEndIndex++;
		if (currentEndIndex > currentSize)
		{
			allocateSpace();
		}
		return;
	}

	void copyToEnd(T thePerson)
	{
		data[currentEndIndex] = new T();
		*(data[currentEndIndex]) = thePerson;
		currentEndIndex++;
		if (currentEndIndex > currentSize)
		{
			allocateSpace();
		}
		return;
	}

	unsigned int getSize(void)
	{
		return currentEndIndex;
	}


};