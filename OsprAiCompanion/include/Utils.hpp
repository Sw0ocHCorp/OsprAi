#ifndef UTILS_HPP
#define UTILS_HPP

#include <vector>
#include <string>
#include <string.h>
#include <map>
#include <sstream>  
#include <iomanip>
#include <string>
#include <sys/time.h>
#include <fstream>

using namespace std;

#define MHz 								1000000
#define NONE 								0
#define ARM 								10
#define DISARM 								15

#define LL_PWM_SETPOINT						20
#define HL_SPEED_VEC_SETPOINT				21
#define HL_ANGLE_SETPOINT					22
#define HL_ANGLE_SPEED_VEC_SETPOINT			23
#define SERVO_ANGLE_SETPOINT				30

template<typename T, unsigned int MS>
class CircularBuffer {
	private:
		T Data[MS];
		unsigned int MaxSize= MS;
		unsigned int Head=0;
		unsigned int Tail= MS;
		unsigned int Size= 0;
	public:
		CircularBuffer() {

		}

		T tail() {
			int index= Tail + 1;
			if (index >= MaxSize)
				index= 0;
			return Data[index];
		}

		void enqueue(T data) {
			if (Head + 1 == Tail)  {
				dequeue();
			}
			Data[Head]= data;
			Head++;
			if (Head >= MaxSize)
				Head= 0;
			Size++;
			if (Size > MaxSize)
				Size= MaxSize;
		}

		T dequeue() {
			if (Tail != Head) {
				Tail++;
				if (Tail >= MaxSize)
					Tail= 0;
				Size--;
				if (Size < 0)
					Size= 0;
				return Data[Tail];
			} else {
				throw std::out_of_range("Tail == Head");
			}
		}

		int size() const {
			return Size;
		}
		int GetMaxSize() const {
			return MaxSize;
		}

};

template<typename T, unsigned int MS>
class StaticVector  {
	private:
		T Data[MS];
		unsigned int MaxSize= MS;
		unsigned int Size= 0;
	public:
		StaticVector() {
			memset(Data, 0, MaxSize);
		}

		StaticVector(std::initializer_list<T> init) {
			if ((unsigned int)init.size() <= MaxSize) {
				memset(Data, 0, MaxSize);
				copy(init.begin(), init.end(), Data);
				Size= init.size();
			} else {
				throw std::out_of_range("Size");
			}
		}

		StaticVector(const T * d, unsigned int s) {
			memset(Data, 0, MaxSize);
			add(d, s);
		}

		void add(T data) {
			if (Size < MaxSize) {
				Data[Size]= data;
				Size++;
			} else {
				throw std::out_of_range("Size");
			}
		}

		void add(const T *data, unsigned int n) {
			if (Size + (n-1) < MaxSize) {
				for (int i= 0; i < (int)n; i++) {
					Data[Size]= data[i];
					Size++;
				}
			} else {
				throw std::out_of_range("Size");
			}
		}

		void insert(int index, T data) {
			if (Size < MaxSize && index >= 0 && index <= Size) {
				for(int i= index; i < Size; i++) {
					Data[i+1]= Data[i];
				}
				Size++;
				Data[index]= data;
				
			} else {
				throw std::out_of_range("Size");
			}
		}

		void remove(T data) {
			int index= -1;
			for (int i= 0; i < Size; i++) {
				if (Data[i] == data) {
					index = i;
					break;
				}
			}
			if (index >= 0) {
				if (index < MaxSize -1) {
                    for (int i= index ; i < Size; i++) {
                        Data[i]= Data[i+1];
                    }
                }
                Size--;
			}
		}

        void removeAt(int index) {
			if (index >= 0 && index < MaxSize) {
				if (index < MaxSize -1) {
                    for (int i= index ; i < Size; i++) {
                        Data[i]= Data[i+1];
                    }
                }
                Size--;
			} else {
				throw std::out_of_range("Size");
			}
		}

		void clear() {
			Size= 0;
		}

		T& operator[](int index) {
			if (index < Size)
				return Data[index];
			else 
				throw std::out_of_range("Size");
		}

		const T *data() {
			return Data;
		}

		T *mutData() {
			return Data;
		}

		vector<T> subVec(int start, int end) {
			vector<T> d(end-start);
			for (int i= 0; i < end-start; i++) {
				d[i]= Data[i+start];
			}
			return d;
		}

		vector<T> subVec(int index, bool fromEnd= true) {
			if (index < 0 || index > Size) {
				throw out_of_range("Size");
			} else {
				vector<T> d;
				if (fromEnd) {
					for (int i= index; i < Size; i++) {
						d.push_back(Data[i]);
					}
				}
				else {
					for (int i= 0; i < index; i++) {
						d.push_back(Data[i]);
					}
				}
				return d;
			}
		}

		int size() const {
			return Size;
		}
		int maxSize() {
			return MaxSize;
		}
};

struct SetPoint {
	uint8_t SetpointType;
	uint8_t ArmingCmd;
	float Theta;
	StaticVector<float, 3> LinsSpeed;
	StaticVector<float, 3> RotSpeed;
	StaticVector<int, 10> ServosAngle;
};

struct WorldMap {
	float GPSLocation[2];
	float LinSpeed[3];
	float RotSpeed[3];
	float Theta;
	float Altitude;
	uint8_t ArmingState;
	StaticVector<float, 10> MotorsSpeed;
	StaticVector<float, 10> ServosAngle;

};

struct GPSData {
	float Lat;
	float Lon;
	float Speed;
};

struct IMUData {
	float LinAccelVec[3];
	float RotAccelVec[3];
};

float hexStringToFloat(string hexString) {
    unsigned int hexValue;
    stringstream ss;
    ss << hex << hexString;
    ss >> hexValue;
    float floatValue;
    memcpy(&floatValue, &hexValue, sizeof(floatValue));
    return floatValue;
}

string floatToHexString(float value) {
    unsigned int hexValue;
    memcpy(&hexValue, &value, sizeof(value));
    stringstream ss;
    ss << hex << hexValue;
    return ss.str();
}

string uCharToHexString(unsigned char value) {
    stringstream ss;
    ss << std::hex << setw(2) << setfill('0') << static_cast<int>(value);
    return ss.str();
}

char intToAsciiChar(int value) {
    if (value < 0 || value > 255) {
        throw std::out_of_range("Value must be between 0 and 255");
    }
    return static_cast<char>(value);
}

int findPattern(const char *data, int dataSize, const char *pattern, int patternSize, bool fromEnd= false) {
    int index= -1;
	int sameElements= 0;
	//IF there is enough elements in data
    if (dataSize >= patternSize) {
		if (fromEnd) {
			for(int i= dataSize-patternSize; i >= 0; i--) { 
				if (data[i] == pattern[0] && index == -1) {
					index= i;
					//Check if all elements of the pattern match the nexts data elements
					for (int j= 0; j < patternSize; j++) {
						if (data[i+j] == pattern[j]) {
							sameElements++;
						}
						//IF not stop search, pattern not match
						else {
							index = -1;
							sameElements= 0;
							break;
						}
					}
				} 
				//IF pattern found, STOP search
				if (sameElements == patternSize) 
					break;
			}
		}
		else {
			for(int i= 0; i <= dataSize-patternSize; i++) {
				//IF first element of the pattern found
				if (data[i] == pattern[0] && index == -1) {
					index= i;
					//Check if all elements of the pattern match the nexts data elements
					for (int j= 0; j < patternSize; j++) {
						if (data[i+j] == pattern[j]) {
							sameElements++;
						}
						//IF not stop search, pattern not match
						else {
							index = -1;
							sameElements= 0;
							break;
						}
					}
				} 
				//IF pattern found, STOP search
				if (sameElements == patternSize) 
					break;
			}
		}
    }
    return index;
}

int findPattern(const uint8_t *data, int dataSize, const uint8_t *pattern, int patternSize, bool fromEnd= false) {
    int index= -1;
	int sameElements= 0;
	//IF there is enough elements in data
    if (dataSize >= patternSize) {
		if (fromEnd) {
			for(int i= dataSize-patternSize; i >= 0; i--) { 
				if (data[i] == pattern[0] && index == -1) {
					index= i;
					//Check if all elements of the pattern match the nexts data elements
					for (int j= 0; j < patternSize; j++) {
						if (data[i+j] == pattern[j]) {
							sameElements++;
						}
						//IF not stop search, pattern not match
						else {
							index = -1;
							sameElements= 0;
							break;
						}
					}
				} 
				//IF pattern found, STOP search
				if (sameElements == patternSize) 
					break;
			}
		}
		else {
			for(int i= 0; i <= dataSize-patternSize; i++) {
				//IF first element of the pattern found
				if (data[i] == pattern[0] && index == -1) {
					index= i;
					//Check if all elements of the pattern match the nexts data elements
					for (int j= 0; j < patternSize; j++) {
						if (data[i+j] == pattern[j]) {
							sameElements++;
						}
						//IF not stop search, pattern not match
						else {
							index = -1;
							sameElements= 0;
							break;
						}
					}
				} 
				//IF pattern found, STOP search
				if (sameElements == patternSize) 
					break;
			}
		}
    }
    return index;
}

bool writeInLinuxFile(StaticVector<char, 50> filePath, StaticVector<char, 250> dataToWrite) {
	ofstream file(filePath.data());
	if (!file) {
		cerr << "Error opening " << filePath.data() << endl;
		return false;
	}
	file << dataToWrite.data();
	if (!file) {
		cerr << "Error writing data in " << filePath.data() << endl;
		return false;
 	}
	return true;
}

#endif