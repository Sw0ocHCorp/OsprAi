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

using namespace std;

#define MHz 	1000000
#define NONE 	0
#define ARM 	10
#define DISARM 	15

template<typename T, unsigned int MS>
class CircularBuffer {
	private:
		T Data[MS];
		unsigned int MaxSize= MS;
		unsigned int Head=0;
		unsigned int Tail= MS;
	public:
		CircularBuffer() {

		}

		void enqueue(T data) {
			if (Tail != Head) {
				Data[Head]= data;
				Head++;
				if (Head >= MaxSize)
					Head= 0;
			} else {
				throw out_of_range("Head");
			}
		}

		T dequeue() {
			if ((Tail +1) != Head) {
				Tail++;
				return Data[Tail];
			} else 
				throw out_of_range("Tail");
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

		}

		StaticVector(std::initializer_list<T> init) {
			if ((unsigned int)init.size() <= MaxSize) {
				copy(init.begin(), init.end(), Data);
				Size= init.size();
			}
		}
		void Add(T data) {
			if (Size < MaxSize) {
				Data[Size]= data;
				Size++;
			} else {
				throw std::out_of_range("Size");
			}
		}

		void Add(const T *data, unsigned int n) {
			if (Size + (n-1) < MaxSize) {
				for (int i= 0; i < (int)n; i++) {
					Data[Size]= data[i];
					Size++;
				}
			} else {
				throw std::out_of_range("Size");
			}
		}

		void Remove(T data) {
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

        void RemoveAt(int index) {
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

		void Clear() {
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

		vector<T> SubVec(int start, int end) {
			vector<T> d(end-start);
			for (int i= 0; i < end-start; i++) {
				d[i]= Data[i+start];
			}
			return d;
		}

		vector<T> SubVec(int index, bool toEnd= true) {
			if (index < 0 || index > Size) {
				throw out_of_range("Size");
			} else {
				vector<T> d;
				if (toEnd) {
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
		int GetMaxSize() {
			return MaxSize;
		}
};

struct SetPoint {
	uint8_t ArmingCmd;
	float ThetaSetpoint;
	StaticVector<float, 3> LinsSpeedSetpoint;
	StaticVector<float, 3> RotSpeedSetpoint;
	StaticVector<float, 10> ServosAngleSetpoint;
};

struct WorldMap {
	float LinSpeed[3];
	float RotSpeed[3];
	float Theta;
	float Altitude;
	uint8_t ArmingState;
	StaticVector<float, 10> MotorsSpeed;
	StaticVector<float, 10> ServosAngle;

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

int findPattern(const char *data, int dataSize, const char *pattern, int patternSize) {
    int index= -1;
    if (dataSize >= patternSize) {
        for(int i= 0; i < dataSize; i++) {
            if (data[i] == pattern[0] && index == -1) {
                index= i;
            } else if (i - index < index + patternSize && data[i] != pattern[i - index] && index >= 0) {
                index= -1;
                break;
            } else if (i - index >= index + patternSize && index >= 0)
                break;
        }
    }
    return index;
}

int findPattern(const uint8_t *data, int dataSize, const uint8_t *pattern, int patternSize) {
    int index= -1;
	//IF there is enough elements in data
    if (dataSize >= patternSize) {
        for(int i= 0; i < dataSize; i++) {
			//IF first element of the pattern found
            if (data[i] == pattern[0] && index == -1) {
                index= i;
				//Check if all elements of the pattern match the nexts data elements
				for (int j= 0; j < patternSize; j++) {
					//IF not stop search, pattern not match
					if (data[i+j] != pattern[j]) {
						index = -1;
						break;
					}
				}
				// IF all elements matches FIRST pattern found
				if (index >= 0)
					break;
            } 
			//IF pattern found, STOP search
			if (index >= 0) 
				break;
        }
    }
    return index;
}

#endif