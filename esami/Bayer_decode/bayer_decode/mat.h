#pragma once

#include <vector>
#include <iostream>
#include <fstream>
#include <string>

template <typename T>
class mat 
{
	int cols_, rows_;
	std::vector<T> data_;

public:

	mat(int rows = 0, int cols = 0) : rows_(rows), cols_(cols), data_(rows* cols) {}

	T& operator()(int row, int col) 
	{ 
		return data_[row*cols_ + col];
	}
	const T& operator()(int row , int col) const {
		return data_[row * cols_ + col];
	}

	T& operator[](int i) 
	{
		return data_[i];
	}


	int cols() const 
	{
		return cols_;
	}

	int rows() const 
	{
		return rows_;
	}

	auto& data() {
		return data_;
	}

	const auto& data() const
	{
		return data_;
	}

	int size() const 
	{
		return cols_ * rows_;
	}

	void resize(int rows, int cols) {
		cols_ = cols;
		rows_ = rows;
		data_.resize(rows * cols);
	}

	char* rawdata() 
	{
		return reinterpret_cast<char*>(data_.data());;
	}

	int rawsize() {
		return cols_ * rows_* sizeof(T); 
	}

	auto begin()
	{
		return data_.begin();
	}

	auto end() 
	{
		return data_.end();
	}
};