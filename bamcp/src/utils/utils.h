#pragma once

#include "rng.h"
#include <boost/numeric/ublas/vector.hpp>
using namespace boost::numeric::ublas;
#include <vector>
#include <string>
#include <boost/foreach.hpp>

#include <sstream>
#include <iostream>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>
namespace fs = boost::filesystem;

typedef unsigned int uint;
namespace utils{

extern RNG rng;

void printA(double*v , size_t s);

void normalize(double* v, size_t s);

double** new2DArray(size_t s1, size_t s2);
void free2DArray(double **a, size_t s1);
void free3DArray(double ***a, size_t s1, size_t s2);
double*** new3DArray(size_t s1, size_t s2, size_t s3);

double inner_prod(double* v1, double* v2, size_t s);
double inner_prod(vector<double>& v1,double* v2);

double sqnorm_2(double* v1, double* v2, size_t s);

double TV(double* v1, double* v2, size_t s);
double TV(const vector<double>& p, const vector<double>& q);
double KLdiv(const vector<double>& p, const vector<double>& q);

void setSeed(ulong _x);

void sampleDirichlet(double* probs, double* params, uint nump);
void sampleDirichlet(double* probs, const uint* params, uint nump, double prior);

void sampleDirichlet(double* probs, const vector<double> &params);
vector<double> sampleDirichlet(const vector<double> &params);

template <class T>
void append(T x, std::string& filename){
	fs::path p(filename);
	fs::ofstream ofs(p, std::ios::app);
	ofs << x << std::endl;
	ofs.close();
}

//Overwrite file with elements of std::vector (print as column)
template <class T>
void dump(const std::vector<T> &v, std::string& filename){
	fs::path p(filename);
	fs::ofstream ofs(p, std::ios::out);
	if(ofs.is_open()){
		BOOST_FOREACH(T d, v){
			ofs << d << std::endl;
		}
		ofs.close();
	}
	else
		std::cout << "Error: couldn't open file for writing: " << filename << std::endl;
}

//print each std::vector on a *column*
template <class T>
void dumpc(const std::vector<std::vector<T> > &vv, std::string& filename, uint N){
  
	fs::path p(filename);
	fs::ofstream ofs(p, std::ios::out);
	if(ofs.is_open())
	{
		for(size_t row = 0; row < N; ++row){
			for(size_t col = 0; col < vv.size(); ++col){
				ofs << vv[col][row] << " ";
			}
			ofs << std::endl;
		}
		ofs.close();
	}
	else{
		std::cout << "Error: couldn't open file for writing: " << filename << std::endl;
	}
};

void dump(const vector<double> &v, std::string& filename);
void dumpc(const vector<std::vector<double> > &vv, std::string& filename);
void dump(const std::vector<std::vector<double> > &vv, std::string& filename);
void dump(const vector<vector<double> > &vv, std::string& filename);


float StringToFloat(const std::string String);
int StringToInt(const std::string String);

int getSvnRevision();

}
