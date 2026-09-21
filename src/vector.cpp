#include<string.h>
#include<stdio.h>
#include<limits>
#include<math.h>
#include<stdlib.h>
//#include <printf.h>
#include <stddef.h>
#include "vector.h"

Vector::Vector(double a, double b, double c) : x(a), y(b), z(c) {
}
void Vector::operator -= (const Vector rhs) {
   x-=rhs.x; y-=rhs.y; z-=rhs.z;
}
void Vector::operator += (const Vector rhs) {
   x+=rhs.x; y+=rhs.y; z+=rhs.z;
}
void Vector::operator *= (const double rhs) {
   x*=rhs; y*=rhs; z*=rhs;
}
void Vector::operator *= (const float rhs) {
   x*=rhs; y*=rhs; z*=rhs;
}
void Vector::operator *= (const int rhs) {
   x*=rhs; y*=rhs; z*=rhs;
}
void Vector::operator /= (const double rhs) {
   x/=rhs; y/=rhs; z/=rhs;
}
void Vector::operator /= (const float rhs) {
   x/=rhs; y/=rhs; z/=rhs;
}
void Vector::operator /= (const int rhs) {
   x/=rhs; y/=rhs; z/=rhs;
}


Vector Vector::operator - (const Vector rhs) {
   return Vector(x-rhs.x, y-rhs.y, z-rhs.z);
}
Vector Vector::operator + (const Vector rhs) {
   return Vector(x+rhs.x, y+rhs.y, z+rhs.z);
}
/*
Vector Vector::operator * (const Vector a) {
   return Vector(y*a.z-z*a.y, z*a.x-x*a.z, x*a.y-y*a.x);
}*/
Vector Vector::operator * (const double rhs) {
   return Vector(x*rhs, y*rhs, z*rhs);
}
Vector Vector::operator * (const float rhs) {
   return Vector(x*rhs, y*rhs, z*rhs);
}
Vector Vector::operator * (const int rhs) {
   return Vector(x*rhs, y*rhs, z*rhs);
}
Vector Vector::operator / (const double rhs) {
   return Vector(x/rhs, y/rhs, z/rhs);
}
Vector Vector::operator / (const float rhs) {
   return Vector(x/rhs, y/rhs, z/rhs);
}
Vector Vector::operator / (const int rhs) {
   return Vector(x/rhs, y/rhs, z/rhs);
}
Vector Vector::cross(const Vector a) {
   return Vector(y*a.z-z*a.y, z*a.x-x*a.z, x*a.y-y*a.x);
}
double Vector::mag2(){
   return x*x+y*y+z*z; 
}
double Vector::mag(){
   return sqrt(x*x+y*y+z*z); 
}
double Vector::dot(const Vector a){
   return x*a.x+y*a.y+z*a.z;
}
Vector Vector::normalize(){
   double m = mag();
   return Vector(x/m, y/m, z/m); 
}

// every caller passes in right, up, and vect, so full 3x3 solve isnt necessary
// we can just use their dot products (projections onto each axis)
Vector solveScalers(Vector v1, Vector v2, Vector v3, Vector C){
   return Vector(C.dot(v1), C.dot(v2), C.dot(v3));
}

Ray::Ray(const Vector& po, const Vector& ve): point(po), vector(ve){}
