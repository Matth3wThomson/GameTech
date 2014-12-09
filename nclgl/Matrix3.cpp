#include "Matrix3.h"

Matrix3::Matrix3(void){
	ToIdentity();
}

Matrix3::Matrix3( float elements[9] ){
	memcpy(this->values, elements, 9*sizeof(float));
}

Matrix3::~Matrix3(){
	ToIdentity();
}

void Matrix3::ToIdentity(void){
	ToZero();

	values[0] = 1.0f;
	values[4] = 1.0f;
	values[8] = 1.0f;
}

Matrix3 Matrix3::GetIdentitiy(){
	return Matrix3();
}

void Matrix3::ToZero(){
	for (int i=0; i<9; ++i){
		values[i] = 0.0f;
	}
}


