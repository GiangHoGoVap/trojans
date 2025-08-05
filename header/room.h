#ifndef ROOM_H
#define ROOM_H

#include <iostream>
#include <string>

using namespace std;

class Room {
	friend istream& operator>>(istream& is, Room& r) { 
		return is >> r.name >> r.capacity; 
	}
    private:
        string name;
        unsigned capacity;
    public:
        const string& Name() const { return name; }
        unsigned Capacity() const { return capacity; }
};

#endif