#ifndef TIMETABLE_H
#define TIMETABLE_H

#include <string>
#include <vector>
#include "faculty.h"

using namespace std;

class Timetable {
    private:
        const Faculty & in;  
        unsigned warnings;
        vector<vector<unsigned> > tt;  // (courses X periods) timetable matrix
        // redundant data
        vector<vector<unsigned> > room_lectures; // number of lectures per room in the same period (should be 0 or 1)
        vector<vector<unsigned> > curriculum_period_lectures; // number of lectures per curriculum in the same period (should be 0 or 1)
        vector<vector<unsigned> > course_daily_lectures; // number of lectures per course per day
        vector<unsigned> working_days; // number of days of lecture per course
        vector<vector<unsigned> > used_rooms; // rooms used for each lecture on the course
    public:
        Timetable(const Faculty & f, const string file_name) : in(f), tt(in.Courses(), vector<unsigned>(in.Periods(), 0)),
            room_lectures(in.Rooms() + 1, vector<unsigned>(in.Periods())), 
            curriculum_period_lectures(in.Curricula(), vector<unsigned>(in.Periods())),
            course_daily_lectures(in.Courses(), vector<unsigned>(in.Days())), 
            working_days(in.Courses()), used_rooms(in.Courses()) {
            unsigned day, period, p;
            int c, r;
            string course_name, room_name;
            ifstream is(file_name.c_str());

            warnings = 0;

            if (is.fail()) {
                cerr << "Output file does not exist!" << endl;
                exit(1);
            }

            for (unsigned i = 0; i < tt.size(); i++) {
                for (unsigned j = 0; j < tt[i].size(); j++) {
                    tt[i][j] = 0;
                }
            }

            while (is >> course_name >> room_name >> day >> period) {
                c = in.CourseIndex(course_name);
                if (c == -1) {
                    cerr << "WARNING: Nonexisting course " << course_name << " (entry skipped)" << endl;
                    warnings++;
                    continue;
                }
                r = in.RoomIndex(room_name);
                if (r == -1) {
                    cerr << "WARNING: Nonexisting room " << room_name << " (entry skipped)" << endl;
                    warnings++;
                    continue;
                }

                if (day >= in.Days()) {
                    cerr << "WARNING: Nonexisting day " << day << " (entry skipped)" << endl;
                    warnings++;
                    continue;
                }
                if (period >= in.PeriodsPerDay()) {
                    cerr << "WARNING: Nonexisting period " << period << " (entry skipped)" << endl;
                    warnings++;
                    continue;
                }
                p = day * in.PeriodsPerDay() + period;
                if (tt[c][p] != 0) {
                    cerr << "WARNING: Course " << course_name << " already scheduled at period "
                         << p << " (entry skipped)" << endl;
                    warnings++;
                    continue;
                }
                tt[c][p] = r;
            }
            UpdateRedundantData();
        }

        // Inspect timetable
        unsigned operator()(unsigned i, unsigned j) const { return tt[i][j]; }
        unsigned& operator()(unsigned i, unsigned j) { return tt[i][j]; }
        // Inspect redundant data
        unsigned RoomLectures(unsigned i, unsigned j) const { return room_lectures[i][j]; }
        unsigned CurriculumPeriodLectures(unsigned i, unsigned j) const { return curriculum_period_lectures[i][j]; }
        unsigned CourseDailyLectures(unsigned i, unsigned j) const { return course_daily_lectures[i][j]; }
        unsigned WorkingDays(unsigned i) const { return working_days[i]; }
        unsigned UsedRoomsNo(unsigned i) const { return used_rooms[i].size(); }
        unsigned UsedRooms(unsigned i, unsigned j) const { return used_rooms[i][j]; }
        void InsertUsedRoom(unsigned i, unsigned j) { used_rooms[i].push_back(j); }
        unsigned Warnings() const { return warnings; }
        void UpdateRedundantData() {
            unsigned p, c, r, d, g, gn, i;
            for (r = 1; r < in.Rooms() + 1; r++)
                for (p = 0; p < in.Periods(); p++)
                    room_lectures[r][p] = 0;
            gn = in.Curricula();
            for (g = 0; g < gn; g++)
                for (p = 0; p < in.Periods(); p++)
                    curriculum_period_lectures[g][p] = 0;
            for (c = 0; c < in.Courses(); c++) {
                for (p = 0; p < in.Periods(); p++) {
                    r = tt[c][p];
                    if (r != 0)
                        room_lectures[r][p]++;
                }
            }
            for (c = 0; c < in.Courses(); c++) {
                for (g = 0; g < in.Curricula(); g++) {
                    if (in.CurriculumMember(c, g)) {
                        for (p = 0; p < in.Periods(); p++) {
                            if (tt[c][p] != 0) {
                                curriculum_period_lectures[g][p]++;
                            }
                        }
                    }
                }
            }
            for (c = 0; c < in.Courses(); c++) {
                working_days[c] = 0;
                for (d = 0; d < in.Days(); d++) {
                    course_daily_lectures[c][d] = 0;
                    for (p = d * in.PeriodsPerDay(); p < (d + 1) * in.PeriodsPerDay(); p++) {
                        if (tt[c][p] != 0)
                            course_daily_lectures[c][d]++;
                    }
                    if (course_daily_lectures[c][d] >= 1)
                        working_days[c]++;
                }
            }
            for (c = 0; c < in.Courses(); c++) {
                for (p = 0; p < in.Periods(); p++) {
                    r = tt[c][p];
                    if (r != 0) {
                        for (i = 0; i < used_rooms[c].size(); i++)
                            if (used_rooms[c][i] == r)
                                break;
                        if (i == used_rooms[c].size()) {
                            used_rooms[c].push_back(r);
                        }
                    }
                }
            }
        }
};

#endif