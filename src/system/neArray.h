
#pragma once

template<typename T>
struct neArray{
	neArray(){
		size = 0;
		data = 0;
	}
	neArray(int n){
		size = n;
		data = new T[n];
	}
	~neArray(){
		if(data&&size)
			delete[] data;
	}
	T& operator[] (int n){
		return data[n];
	}
	void Resize(int n){
		if(data)
			delete[] data;
		data = new T[n];
		size = n;
	}
	void Clear(){
		if(data&&size){
			delete[] data;
			data = 0;
			size = 0;
		}
	}

	int size;
	T *data;

private:
	neArray<T>(const neArray<T> &old);
	neArray<T>& operator= (const neArray<T> &na);
};
