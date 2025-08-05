#ifndef CURRICULUM_H
#define CURRICULUM_H

#include <string>
#include <vector>

using namespace std;

class Curriculum {
    private:
        string name;
        vector<unsigned> members;
    public:
        const string& Name() const { return name; }
        void SetName(const string& n) { name = n; }
        unsigned Size() const { return members.size(); }
        void AddMember(unsigned e) { members.push_back(e); }
        unsigned operator[](unsigned i) const { return members[i]; }
};

#endif