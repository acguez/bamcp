#include "utils.h"
namespace fs = boost::filesystem;

RNG utils::rng;


void utils::printA(double*v , size_t s){
	std::cout << "[";
	for(size_t i=0;i<s;++i){
		std::cout << v[i] << " ";
	}
	std::cout << "]" << std::endl;
}
void utils::normalize(double* v, size_t s){
	double sum=0;
	size_t i = 0;
	for(;i<s;++i)
		sum += v[i];
	for(i=0;i<s;++i)
		v[i]=v[i]/sum;
}
double** utils::new2DArray(size_t s1, size_t s2){
	double** a = new double*[s1];
	for(size_t i=0;i<s1;++i)
		a[i] = new double[s2];
	return a;
}
void utils::free2DArray(double** a, size_t s1){
	for(size_t i=0;i<s1;++i)
			delete[] a[i];
	delete[] a;
}
double*** utils::new3DArray(size_t s1, size_t s2, size_t s3){
	double*** a = new double**[s1];
	for(size_t i=0;i<s1;++i){
		a[i] = new double*[s2];
		for(size_t j=0;j<s2;++j)
			a[i][j] = new double[s3];
	}
	return a;
}
void utils::free3DArray(double*** a, size_t s1, size_t s2){
	for(size_t i=0;i<s1;++i){
		for(size_t j=0;j<s2;++j)
			delete[] a[i][j];
		delete[] a[i];
	}
	delete[] a;
}

double utils::inner_prod(double* v1, double* v2, size_t s){
	double sum = 0;
	for(size_t i=0;i<s;++i){
		sum += (v1[i]*v2[i]);
	}
	return sum;
}
double utils::inner_prod(vector<double>& v1,double* v2){
	double sum = 0;
	for(size_t i=0;i<v1.size();++i){
		sum += (v1[i]*v2[i]);
	}
	return sum;
}

double utils::sqnorm_2(double* v1, double* v2, size_t s){
	double sum = 0;
	for(size_t i=0;i<s;++i){
		sum += (v1[i]-v2[i])*(v1[i]-v2[i]);
	}
	return sum;
}
double utils::TV(double* v1, double* v2, size_t s){
	double sum = 0;
	for(size_t i=0;i<s;++i){
		if(v1[i] > 0)
			sum += fabs(v1[i]-v2[i]);	
	}
	return sum;
}
double utils::TV(const vector<double>& p, const vector<double>& q){
	double sum = 0;
	for(size_t i=0;i<p.size();++i){
		if(p(i) > 0)
			sum += fabs(p(i)-q(i));	
	}
	return sum;
}

double utils::KLdiv(const vector<double>& p, const vector<double>& q){
	double sum = 0;
	for(size_t i=0;i<p.size();++i){
		if(p(i) > 0)
			sum += p(i)*log(p(i)/(q(i)+0.0000001));	
	}
	return sum;
}


void utils::setSeed(ulong _x){std::srand(_x);rng = RNG(_x);}

//Hacky way of getting the svn revision
int utils::getSvnRevision(){
	fs::path p(".svn/entries");
	fs::ifstream ifs(p,std::ios::in);
	if(ifs.is_open()){
		std::string line;
		//Want the fourth line
		getline(ifs,line);getline(ifs,line);getline(ifs,line);
		getline(ifs,line);
		return StringToInt(line);
	}
	else{
		std::cout << "Error: Could not read svn revision" << std::endl;
		return -1;
	}
}
/* 
 * Generate a sample from the dirichlet distribution given
 * the input parameters.
 *
 * References:
 *      [1]  L. Devroye, "Non-Uniform Random Variate Generation", 
 *      Springer-Verlag, 1986
 */
void utils::sampleDirichlet(double* probs, const uint* params, uint nump, double prior){
	double gammaSum = 0;
	double randGamma;
	bool allZeros = true;
  for (unsigned i = 0; i < nump; i++){
    randGamma = rng.gamma((double)params[i]+prior, 1);
		if(randGamma > 0)
			allZeros = false;
    probs[i] = randGamma;
    gammaSum += randGamma;
  }
	if(allZeros)
	{
		//Handle special case, occurs when params are really small
		//because of numerical approximation around 0
		probs[rand() % nump] = 1;
	}
	else if(gammaSum < 1e-300){
		for (unsigned i = 0; i < nump; i++)
			probs[i] = 0;
		probs[rand() % nump] = 1;
  }
	else{
		//Normalize
		for(size_t i=0;i<nump;++i)
			probs[i] = probs[i] / gammaSum;
	}
	
}
/* 
 * Generate a sample from the dirichlet distribution given
 * the input parameters.
 *
 * References:
 *      [1]  L. Devroye, "Non-Uniform Random Variate Generation", 
 *      Springer-Verlag, 1986
 */
void utils::sampleDirichlet(double* probs, double* params, uint nump){
	double gammaSum = 0;
	double randGamma;
	bool allZeros = true;

  for (unsigned i = 0; i < nump; i++){
		if(params[i] > 0){
			randGamma = rng.gamma(params[i], 1);
			if(randGamma > 0)
				allZeros = false;
			probs[i] = randGamma;
			gammaSum += randGamma;
		}
		else
			probs[i] = 0;
  }
	if(allZeros)
	{
		//Handle special case, occurs when params are really small
		//because of numerical approximation around 0
		probs[rand() % nump] = 1;
	}
	else if(gammaSum < 1e-300){
		for (unsigned i = 0; i < nump; i++)
			probs[i] = 0;
		probs[rand() % nump] = 1;
  }
	else{
		//Normalize
		for(size_t i=0;i<nump;++i){
				probs[i] = probs[i] / gammaSum;
		}
	}
	
}
/* 
 * Generate a sample from the dirichlet distribution given
 * the input parameters.
 *
 * References:
 *      [1]  L. Devroye, "Non-Uniform Random Variate Generation", 
 *      Springer-Verlag, 1986
 */
void utils::sampleDirichlet(double* probs, const vector<double> &params){
	double gammaSum = 0;
	double randGamma;
	bool allZeros = true;
  for (unsigned i = 0; i < params.size(); i++){
    randGamma = rng.gamma(params(i), 1);
		if(randGamma > 0)
			allZeros = false;
    probs[i] = randGamma;
    gammaSum += randGamma;
  }
	if(allZeros)
	{
		//Handle special case, occurs when params are really small
		//because of numerical approximation around 0
		probs[rand() % params.size()] = 1;
	}
	else if(gammaSum < 1e-300){
		for (unsigned i = 0; i < params.size(); i++)
			probs[i] = 0;
		probs[rand() % params.size()] = 1;
  }
	else{
		//Normalize
		for(size_t i=0;i<params.size();++i)
			probs[i] = probs[i] / gammaSum;
	}
	
}
vector<double> utils::sampleDirichlet(const vector<double> &params){
	double gammaSum = 0;
	double randGamma;
	vector<double> probs(params.size());
	bool allZeros = true;
  for (unsigned i = 0; i < params.size(); i++){
    randGamma = rng.gamma(params(i), 1);
		if(randGamma > 0)
			allZeros = false;
    probs(i) = randGamma;
    gammaSum += randGamma;
  }
	if(allZeros)
	{
		//Handle special case, occurs when params are really small
		//because of numerical approximation around 0
		probs(rand() % params.size()) = 1;
	}
	else{
		assert(gammaSum > 1e-300);
		//Normalize
		probs = probs / gammaSum; 
	}		
	return probs;
}

void utils::dump(const vector<double> &v, std::string& filename){
	fs::path p(filename);
	fs::ofstream ofs(p, std::ios::out);
	if(ofs.is_open())
	{
		BOOST_FOREACH(double d, v){
			ofs << d << std::endl;
		}
		ofs.close();
	}
	else{
		std::cout << "Error: couldn't open file for writing: " << filename << std::endl;
	}
}

//Same as above but print each std::vector on a *column
void utils::dumpc(const vector<std::vector<double> > &vv, std::string& filename){
  
	fs::path p(filename);
	fs::ofstream ofs(p, std::ios::out);
	if(ofs.is_open())
	{
		for(size_t row = 0; row < vv(0).size(); ++row){
			for(size_t col = 0; col < vv.size(); ++col){
				ofs << vv(col)[row] << " ";
			}
			ofs << std::endl;
		}
		ofs.close();
	}
	else{
		std::cout << "Error: couldn't open file for writing: " << filename << std::endl;
	}
}

//Same as above but print each std::vector on a *row*
void utils::dump(const std::vector<std::vector<double> > &vv, std::string& filename){
  
	fs::path p(filename);
	fs::ofstream ofs(p, std::ios::out);
	if(ofs.is_open())
	{
		for(size_t row = 0; row < vv.size(); ++row){
			for(size_t col = 0; col < vv[0].size(); ++col){
				ofs << vv[row][col] << " ";
			}
			ofs << std::endl;
		}
		ofs.close();
	}
	else{
		std::cout << "Error: couldn't open file for writing: " << filename << std::endl;
	}
}
//Same as above but with vector<vector<double> >   *row*
void utils::dump(const vector<vector<double> > &vv, std::string& filename){
	fs::path p(filename);
	fs::ofstream ofs(p, std::ios::out);
	if(ofs.is_open())
	{
		for(size_t row = 0; row < vv.size(); ++row){
			for(size_t col = 0; col < vv(0).size(); ++col){ 
				ofs << vv(row)(col) << " ";
			}
			ofs << std::endl;
		}
		ofs.close();
	}
	else{
		std::cout << "Error: couldn't open file for writing: " << filename << std::endl;
	}
}

// Function to extract a float value from a string
float utils::StringToFloat(const std::string String)
{
	// Create a stringstream object
	std::stringstream StringStream;

	// Set the internal string buffer
	StringStream.str(String);

	// Declare a variable for the value
	float Value = 0.0f;

	// Extract the value
	StringStream >> Value;

	// Return the value
	return Value;
}

// Function to extract an int value from a string
int utils::StringToInt(const std::string String)
{
	// Create a stringstream object
	std::stringstream StringStream;

	// Set the internal string buffer
	StringStream.str(String);

	// Declare a variable for the value
	int Value = 0;

	// Extract the value
	StringStream >> Value;

	// Return the value
	return Value;
}
