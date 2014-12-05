/******************************************************************************
Class:Matrix4
Implements:
Author:Rich Davison
Description:VERY simple 4 by 4 matrix class. Students are encouraged to modify 
this as necessary! Overloading the [] operator to allow acces to the values
array in a neater way might be a good start, as the floats that make the matrix 
up are currently public.

-_-_-_-_-_-_-_,------,   
_-_-_-_-_-_-_-|   /\_/\   NYANYANYAN
-_-_-_-_-_-_-~|__( ^ .^) /
_-_-_-_-_-_-_-""  ""   

*//////////////////////////////////////////////////////////////////////////////
#pragma once

#include <iostream>
#include "common.h"
#include "Vector3.h"
#include "Vector4.h"

class Vector3;

//TODO: Might be worth implementing move constructors
class Matrix3	{
public:
	Matrix3(void);
	Matrix3(float elements[16]);
	~Matrix3(void);

	float	values[9];

	//Set all matrix values to zero
	void	ToZero();
	//Sets matrix to identity matrix (1.0 down the diagonal)
	void	ToIdentity();

	static Matrix3 GetIdentitiy();

	float& operator[](const int location){ return values[location]; };

	//Multiplies 'this' matrix by matrix 'a'. Performs the multiplication in 'OpenGL' order (ie, backwards)
	inline Matrix3 operator*(const Matrix3 &a) const{	
		Matrix3 out;
		//Students! You should be able to think up a really easy way of speeding this up...
		for(unsigned int r = 0; r < 3; ++r) {
			for(unsigned int c = 0; c < 3; ++c) {
				out.values[c + (r*3)] = 0.0f;
				for(unsigned int i = 0; i < 3; ++i) {
					out.values[c + (r*3)] += this->values[c+(i*3)] * a.values[(r*3)+i];
				}
			}
		}
		return out;
	}

	inline Vector3 operator*(const Vector3 &v) const {
		Vector3 vec;

		float temp;

		vec.x = v.x*values[0] + v.y*values[4] + v.z*values[8]  + values[12];
		vec.y = v.x*values[1] + v.y*values[5] + v.z*values[9]  + values[13];
		vec.z = v.x*values[2] + v.y*values[6] + v.z*values[10] + values[14];

		temp =  v.x*values[3] + v.y*values[7] + v.z*values[11] + values[15];

		vec.x = vec.x/temp;
		vec.y = vec.y/temp;
		vec.z = vec.z/temp;

		return vec;
	};

	inline Vector4 operator*(const Vector4 &v) const {
		return Vector4(
			v.x*values[0] + v.y*values[4] + v.z*values[8]  +v.w * values[12],
			v.x*values[1] + v.y*values[5] + v.z*values[9]  +v.w * values[13],
			v.x*values[2] + v.y*values[6] + v.z*values[10] +v.w * values[14],
			v.x*values[3] + v.y*values[7] + v.z*values[11] +v.w * values[15]
		);
	};



	//Handy string output for the matrix. Can get a bit messy, but better than nothing!
	inline friend std::ostream& operator<<(std::ostream& o, const Matrix3& m){
		o << "Mat3(";
		o << "\t"	<< m.values[0] << "," << m.values[1] << "," << m.values[2] << std::endl;
		o << "\t\t" << m.values[3] << "," << m.values[4] << "," << m.values[5] << std::endl;
		o << "\t\t" << m.values[6] << "," << m.values[7] << "," << m.values[8] << std::endl;
		return o;
	}
};

