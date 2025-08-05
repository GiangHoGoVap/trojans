#ifndef COURSE_H
#define COURSE_H

#include <iostream>
#include <string>
using namespace std;

class Course {
    friend istream& operator>>(istream& is, Course& c) {
        return is >> c.name >> c.teacher >> c.lectures >> c.min_working_days >> c.students;
    }
    private:
        string name, teacher;
        unsigned lectures, students, min_working_days;
    public:
        const string& Name() const { return name; }
        const string& Teacher() const { return teacher; }
        unsigned Lectures() const { return lectures; }
        unsigned Students() const { return students; }
        unsigned MinWorkingDays() const { return min_working_days; }
};

#endif