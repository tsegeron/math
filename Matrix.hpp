//
// Created by Ural Shagiziganov on 20.10.2022.
//

# pragma once

# include <memory>
# include <string>
# include <vector>
# include <ostream>
# include <iomanip>
# include <algorithm>

# include <iostream>

template<typename VT>
class Matrix {

	typedef std::allocator<VT>						allocator_type;
	typedef VT										value_type;
	typedef value_type&								reference;
	typedef value_type*								pointer;
	typedef std::vector<std::vector<value_type>>	matrix_type;

private:
	size_t			_w = 10; // param for std::setw in operator<<


private:
	size_t			_rows;
	size_t			_columns;
	pointer 		_ptr;
//	matrix_type		_matrix;
	allocator_type	_alloc;

	Matrix() : _rows(0), _columns(0), _ptr(nullptr) {};
	void	allocateMemory(const value_type& val);
	void	deallocateMemory();

	struct	InvalidParams : std::exception {
		InvalidParams() : _err_msg("Invalid number of rows or columns") {};
		const char* what() const noexcept override { return _err_msg.c_str(); };
	private:
		std::string	_err_msg;
	};

	struct	OutOfRangeCall : std::exception {
		OutOfRangeCall() : _err_msg("Calling index is out of range") {};
		const char* what() const noexcept override { return _err_msg.c_str(); };
	private:
		std::string	_err_msg;
	};

public:
	explicit Matrix(size_t rows, size_t columns, const value_type &value = value_type(),
					typename std::enable_if<std::is_integral<VT>::value || std::is_floating_point<VT>::value, VT>::type * = nullptr);
	explicit Matrix(const std::vector<VT> &vec,
					typename std::enable_if<std::is_integral<VT>::value || std::is_floating_point<VT>::value, VT>::type * = nullptr);
	explicit Matrix(std::vector<VT> &&vec,
					typename std::enable_if<std::is_integral<VT>::value || std::is_floating_point<VT>::value, VT>::type * = nullptr);
	explicit Matrix(const matrix_type &vec,
					typename std::enable_if<std::is_integral<VT>::value || std::is_floating_point<VT>::value, VT>::type * = nullptr); // std::vector<std::vector<T>>
	explicit Matrix(matrix_type &&vec,
					typename std::enable_if<std::is_integral<VT>::value || std::is_floating_point<VT>::value, VT>::type * = nullptr); // std::vector<std::vector<T>>
	Matrix(const Matrix &other);
	Matrix(Matrix &&other) noexcept ;
	Matrix& operator = (const Matrix &other);
	Matrix& operator = (Matrix &&other) noexcept ;
	virtual ~Matrix() { deallocateMemory(); };

	/* Element access */
	reference			operator()(size_t row, size_t column); // starts from 1
	reference			operator[](int index); // starts from 0
	value_type			at(int index) const; // starts from 0, not changeable
	value_type			at(size_t row, size_t column) const; // starts from 1, not changeable
	pointer				data() { return _ptr; }; // raw data

	/* matrix mods */
	Matrix&				transpose();


	/*  */
	size_t				rows()		const { return _rows; };
	size_t				columns()	const { return _columns; };
	std::pair<VT, VT>	shape()		const { return std::pair<VT, VT>(_rows, _columns); };
	size_t				size()		const { return _rows * _columns; };

	void				setw(const size_t w) { _w = w; };

	template<class U>
	friend std::ostream& operator << (std::ostream &o, const Matrix<U>&);
};


template<typename VT>
Matrix<VT>::Matrix(size_t rows, size_t columns, const value_type &value,
				   typename std::enable_if<std::is_integral<VT>::value || std::is_floating_point<VT>::value, VT>::type *)
		: _rows(rows), _columns(columns), _ptr(nullptr), _alloc()
{
	if (_rows < 1 || _columns < 1)
		throw InvalidParams();

	_ptr = _alloc.allocate(_rows * _columns);

	for (size_t i = 0; i < _rows * _columns; ++i)
		_alloc.template construct(_ptr + i, value);
}

template<typename VT>
Matrix<VT>::Matrix(const std::vector<VT> &vec,
				   typename std::enable_if<std::is_integral<VT>::value || std::is_floating_point<VT>::value, VT>::type *)
		: _rows(1), _columns(vec.size()), _ptr(nullptr), _alloc()
{
	if (_columns < 1)
		throw InvalidParams();

	_ptr = _alloc.allocate(_columns);

	for (size_t i = 0; i < _columns; ++i)
		_alloc.template construct(_ptr + i, vec[i]);
}

template<typename VT>
Matrix<VT>::Matrix(std::vector<VT> &&vec,
				   typename std::enable_if<std::is_integral<VT>::value || std::is_floating_point<VT>::value, VT>::type *)
		: _rows(1), _columns(vec.size()), _ptr(nullptr), _alloc()
{
	if (_columns < 1)
		throw InvalidParams();

	_ptr = _alloc.allocate(_columns);

	for (size_t i = 0; i < _columns; ++i)
		*(_ptr + i) = *(vec.data() + i);
}

template<typename VT>
Matrix<VT>::Matrix(const matrix_type &vec,
				   typename std::enable_if<std::is_integral<VT>::value || std::is_floating_point<VT>::value, VT>::type *)
		: _rows(0), _columns(0), _ptr(nullptr), _alloc()
{
	if (vec.empty() || vec[0].size() < 1)
		throw InvalidParams();

	size_t	cols = vec[0].size();
	if (vec.size() < 1 || !std::all_of(vec.begin(), vec.end(),
									   [&cols](const std::vector<VT> &row) { return row.size() == cols;}))
		throw InvalidParams();

	_rows		= vec.size();
	_columns	= cols;
	_ptr		= _alloc.allocate(_rows * _columns);

	for (size_t i = 0; i < _rows; ++i)
		for (size_t j = 0; j < _columns; ++j)
			_alloc.template construct(_ptr + i * _columns + j, vec[i][j]);
}

template<typename VT>
Matrix<VT>::Matrix(matrix_type &&vec,
				   typename std::enable_if<std::is_integral<VT>::value || std::is_floating_point<VT>::value, VT>::type *)
		: _rows(0), _columns(0), _ptr(nullptr), _alloc()
{
	if (vec.empty() || vec[0].size() < 1)
		throw InvalidParams();

	size_t	cols = vec[0].size();
	if (vec.size() < 1 || !std::all_of(vec.begin(), vec.end(),
									   [&cols](const std::vector<VT> &row) { return row.size() == cols; }))
		throw InvalidParams();

	_rows		= vec.size();
	_columns	= cols;
	_ptr		= _alloc.allocate(_rows * _columns);

	for (size_t i = 0; i < _rows; ++i)
		for (size_t j = 0; j < _columns; ++j)
			_alloc.template construct(_ptr + i * _columns + j, vec[i][j]);
}

template<typename VT>
Matrix<VT>::Matrix(const Matrix &other)
		: _rows(other._rows), _columns(other._columns), _alloc(other._alloc)
{
	*this = other;
}

template<typename VT>
Matrix<VT>::Matrix(Matrix &&other) noexcept
		: _rows(other._rows), _columns(other._columns), _alloc(other._alloc)
{
	*this = std::move(other);
}

template<typename VT>
Matrix<VT>&	Matrix<VT>::operator=(const Matrix<VT> &other) {
	if (this != &other) {
		if (_rows != other._rows && _columns != other._columns) {
			deallocateMemory();
			_ptr = _alloc.allocate(_rows * _columns);
		} else {
			for (size_t i = 0; i < _rows * _columns; ++i)
				_alloc.destroy(_ptr + i);
		}

		_rows		= other._rows;
		_columns	= other._columns;

		for (size_t i = 0, offset; i < _rows; ++i)
			for (size_t j = 0; j < _columns; ++j) {
				offset = i * _columns + j;
				_alloc.template construct(_ptr + offset, other.at(offset));
			}
	}
	return *this;
}

template<typename VT>
Matrix<VT>&	Matrix<VT>::operator=(Matrix<VT> &&other) noexcept {
	if (this != &other) {
		if (_rows != other._rows && _columns != other._columns) {
			deallocateMemory();
			_ptr = _alloc.allocate(_rows * _columns);
		} else {
			for (size_t i = 0; i < _rows * _columns; ++i)
				_alloc.destroy(_ptr + i);
		}

		_rows		= other._rows;
		_columns	= other._columns;
		_ptr		= other._ptr;
	}
	return *this;
}

template<typename VT>
VT&	Matrix<VT>::operator()(const size_t row, const size_t column) {
	if (row > _rows || column > _columns || row < 1 || column < 1)
		throw OutOfRangeCall();

	return *(_ptr + _columns * (row - 1) + column - 1);
}

template<typename VT>
VT&	Matrix<VT>::operator[](const int index) {
	if (index >= _rows * _columns)
		throw OutOfRangeCall();

	if (index >= 0)
		return *(_ptr + index);
	else
		return *(_ptr + _rows * _columns + index);
}

template<typename VT>
VT	Matrix<VT>::at(const int index) const {
	if (index >= _rows * _columns)
		throw OutOfRangeCall();

	if (index >= 0)
		return *(_ptr + index);
	else
		return *(_ptr + _rows * _columns + index);
}

template<typename VT>
VT	Matrix<VT>::at(const size_t row, const size_t column) const {
	if (row > _rows || column > _columns || row < 1 || column < 1)
		throw OutOfRangeCall();

	return *(_ptr + _columns * (row - 1) + column - 1);
}

template<class VT>
Matrix<VT>&	Matrix<VT>::transpose() {
	if (_rows == 1 || _columns == 1) {
		size_t	tmp = _columns;
		_columns	= _rows;
		_rows		= tmp;
		return *this;
	}

	Matrix<VT>	tmp(_columns, _rows);
	for(size_t i = 1; i <= _rows; ++i)
		for(size_t j = 1; j <= _columns; ++j)
			tmp(j, i) = this->at(i, j);
	*this = tmp;

	return *this;
}




/* --------- Private Functions Implementation --------- */

template<typename VT>
void	Matrix<VT>::allocateMemory(const value_type& val) {
	_ptr = _alloc.allocate(_rows * _columns);

	for (size_t i = 0; i < _rows * _columns; ++i)
		_alloc.template construct(_ptr + i, val);
}

template<typename VT>
void	Matrix<VT>::deallocateMemory() {
	for (size_t i = 0; i < _rows * _columns; ++i)
		_alloc.destroy(_ptr + i);
	_alloc.deallocate(_ptr, _rows * _columns);
}



/* ------------- Non-member functions ------------- */

template<typename VT>
std::ostream& operator << (std::ostream &o, const Matrix<VT>& m) {
	for(size_t i = 1; i <= m._rows; ++i) {
		for(size_t j = 1; j <= m._columns; ++j)
			o << std::setw(m._w) << m.at(i, j);
		if (i != m._rows)
			o << "\r\n";
	}
	return o;
}
