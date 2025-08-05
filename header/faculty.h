#ifndef FACULTY_H
#define FACULTY_H

#include <fstream>
#include <string>
#include <vector>
#include "course.h"
#include "room.h"
#include "curriculum.h"

using namespace std;

class Faculty {
    private:
        string name;
        unsigned courses, rooms, curricula, periods, periods_per_day;

        vector<Course> course_vect;
        vector<Room> room_vect;
        vector<Curriculum> curricula_vect;

        vector<vector<bool> > availability;
        vector<vector<bool> > conflict;
    public:
        unsigned Courses() const { return courses; }
        unsigned Rooms() const { return rooms; }
        unsigned Curricula() const { return curricula; }
        unsigned Periods() const { return periods; }
        unsigned PeriodsPerDay() const { return periods_per_day; }
        unsigned Days() const { return periods / periods_per_day; }

        bool Available(unsigned c, unsigned p) const { return availability[c][p]; }
        bool Conflict(unsigned c1, unsigned c2) const { return conflict[c1][c2]; }
        const Course& CourseVector(int i) const { return course_vect[i]; }
        const Room& RoomVector(int i) const { return room_vect[i]; }    
        const Curriculum& CurriculaVector(int i) const { return curricula_vect[i]; }

        int PeriodIndex(const string&) const;
        const string& Name() const { return name; }

        const unsigned MIN_WORKING_DAYS_COST;
        const unsigned CURRICULUM_COMPACTNESS_COST;
        const unsigned ROOM_STABILITY_COST;

    public:
        Faculty(const string& file_name) : MIN_WORKING_DAYS_COST(5), CURRICULUM_COMPACTNESS_COST(2), ROOM_STABILITY_COST(1) {
            string curriculum, course_name, room_name, period_name, teacher_name, priority, buffer;
            string course_name1, course_name2;
            unsigned curriculum_size, days, constraints, i;
            ifstream is(file_name.c_str());

            if (is.fail()) {
                cerr << "Input file does not exist!" << endl;
                exit(1);
            }

            is >> buffer >> name;
            is >> buffer >> courses;
            is >> buffer >> rooms;
            is >> buffer >> days;
            is >> buffer >> periods_per_day;
            is >> buffer >> curricula;
            is >> buffer >> constraints;

            periods = days * periods_per_day;

            course_vect.resize(courses);
            room_vect.resize(rooms + 1); // location 0 of room_vect is not used (teaching in room 0 means NOT TEACHING)
            curricula_vect.resize(curricula);
            availability.resize(courses, vector<bool> (periods, true));
            conflict.resize(courses, vector<bool> (courses));

            is >> buffer;
            for (i = 0; i < courses; i++) { is >> course_vect[i]; }

            is >> buffer;
            for (i = 1; i <= rooms; i++) { is >> room_vect[i]; }

            is >> buffer;
            for (i = 0; i < curricula; i++) {
                is >> buffer >> curriculum_size;
                curricula_vect[i].SetName(buffer);
                unsigned i1, i2;
                for (i1 = 0; i1 < curriculum_size; i1++) {
                    int c1; unsigned c2;
                    is >> course_name;
                    c1 = CourseIndex(course_name);
                    curricula_vect[i].AddMember(c1);
                    for (i2 = 0; i2 < i1; i2++) {
                        c2 = curricula_vect[i][i2];
                        conflict[c1][c2] = true;
                        conflict[c2][c1] = true;
                    }
                }
            }

            is >> buffer;
            int c, p, period_index, day_index;

            for (i = 0; i < constraints; i++) {
                is >> course_name >> day_index >> period_index;
                p = day_index * periods_per_day + period_index;
                c = CourseIndex(course_name);
                availability[c][p] = false;
            }

            for (unsigned c1 = 0; c1 < courses - 1; c1++) {
                for (unsigned c2 = c1 + 1; c2 < courses; c2++) {
                    if (course_vect[c1].Teacher() == course_vect[c2].Teacher()) {
                        conflict[c1][c2] = true;
                        conflict[c2][c1] = true;
                    }
                }
            }
        }

        int CourseIndex(const string& name) const {
            for (unsigned i = 0; i < course_vect.size(); i++) {
                if (course_vect[i].Name() == name) { return i; }
            }
            return -1;
        }

        int CurriculumIndex(const string& name) const {
            for (unsigned i = 0; i < curricula_vect.size(); i++) {
                if (curricula_vect[i].Name() == name) { return i; }
            }
            return -1;
        }

        int RoomIndex(const string& name) const {
            for (unsigned i = 0; i < room_vect.size(); i++) {
                if (room_vect[i].Name() == name) { return i; }
            }
            return -1;
        }

        bool CurriculumMember(unsigned c, unsigned g) const {
            for (unsigned i = 0; i < curricula_vect[g].Size(); i++) {
                if (curricula_vect[g][i] == c) { return true; }
            }
            return false;
        }
};

#endif