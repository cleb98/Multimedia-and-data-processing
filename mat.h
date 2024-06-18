
//Exercise 1
//Write a program that generates a gray level image of 256x256 pixels, in which the first row is made of 256 zeros,
//  the second one of 256 ones, the third one of 256 values 2 and so on.Save the image in the binary PGM and plain PGM formats,
//  described above.Verify that the image is viewable in XnView.
// The images should appear as a gradient from black to white from top to bottom.

//we need to model an image as a matrix.
//useful a template to manage both the types of images, grayscale and color.
//the template es a type T to represent the type of the pixel.
#include <vector>
#include <cstdint>

template <typename T>
class mat
{
	int rows_, cols_; //use of int instead of size_t to avoid problems with unsigned types (ex. negative values of rows and cols)
	std::vector<T> data_; //vector of pixels
public:
	mat(int rows = 0, int cols = 0)
		: rows_(rows), cols_(cols), data_(rows* cols) {}
/*
	define the operator () to access the element of a matrix 
	Restituendo un riferimento (T&), 
	 permetti la modifica diretta degli elementi della matrice. 
	 cosi dalla classe mat si può 
	 recuperare il valore di un elemento,
	 e anche modificarlo direttamente.
	ex: se m è un oggetto di tipo mat<int>, 
	 facendo m(1, 2) = 5; 
	 imposterà l'elemento alla prima riga 
	 e seconda colonna al valore 5.
		 */
//per accedere un elemento della matrice salvato in array lineare (es: m(0,2)->data_[0*3+2]= data_[2])
//if object is const, we can't modify the value of the element
	T& operator()(int r, int c) {	return data_[r * cols_ + c]; }
	const T& operator()(int r, int c) const {	return data_[r * cols_ + c]; }

//acces data as vector of pixels (m[1])
	T& operator[](int i) { return data_[i]; }
	const T& operator[](int i) { return data_[i]; }

//getters: rows, columns, n° pixels, rawsize(number of bytes our vector takes up in memory)
	int rows() const { return rows_; }
	int cols() const { return cols_; }
	int size() const { return rows_ * cols_; }
	int rawsize const{ return rows_ * cols_ * sizeof(T); }

//resize image (for ex when is fullfilled with 0)
	void resize(int rows , int cols ) {
		rows_ = rows;
		cols_ = cols;
		data_.resize(rows * cols);
	}
//threat data as bunch of bytes ready to be saved in a file of to put in memory
//need to cast the vector as pointer char:
//can be used:	&data_[0] (place in memory where 1st pixel is)
//or data method of vector-> data_.data()
	char* rawdata() {
		//return reinterpret_cast<char*>(&data_[0]); 
		return reinterpret_cast<char*>(data_.data());
	}
	const char* rawdata() { 
		return reinterpret_cast<char*>(data_.data());
	}


//2 methods for data, 1st only access data, 2nd also modifies it
	const auto& data() const { return data_; } //auto&:voglio accedere alla matrice,non una copia (metterebbe troppa roba in memoria)
	auto& data()  { return data_; }

// way to go throught elements in vetcot: iterator
	auto begin() { return data_.begin(); }
	auto begin() const { return data_.begin(); }
	auto end() { return data_.end(); }
	auto end() const { return data_.end(); }






};



int main() {
	return 0;
}
