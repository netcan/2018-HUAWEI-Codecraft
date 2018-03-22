/****************************************************************************
 > File Name: datetime.h
 > Author: Netcan
 > Blog: http://www.netcan666.com/
 > Mail: netcan1996@gmail.com
 > Created Time: 2018-03-11 -- 16:05
 ****************************************************************************/

#pragma once

#include <cstdio>
#include <string>
using std::string;

struct Date {
	int year, month, day;
	explicit Date(int year = 0, int month = 0, int day = 0):
			year(year), month(month), day(day) {}
	bool operator<(const Date &b) const {
		return jd(*this) < jd(b);
	}
	bool operator==(const Date &b) const {
		return this->year == b.year &&
		       this->month == b.month &&
		       this->day == b.day;
	}
	bool operator<=(const Date &b) const {
		return jd(*this) <= jd(b);
	}
	int operator-(const Date &b) const {
		return jd(*this) - jd(b);
	}

	Date &operator+=(int day) {
		*this = gdate(jd(*this) + day);
		return *this;
	}
	friend Date operator+(Date lhs, int day) {
		lhs += day;
		return lhs;
	}

	static int jd(const Date& date);
	static Date gdate(int jd);

};

struct Time {
	int hour, minute, second;
	Time(uint8_t hour, uint8_t minute, uint8_t second):
			hour(hour), minute(minute), second(second) {}
	bool operator<(const Time &b) const {
		return this->hour < b.hour ||
		       this->minute < b.minute ||
		       this->second < b.second;
	}
	bool operator==(const Time &b) const {
		return this->hour == b.hour &&
		       this->minute == b.minute &&
		       this->second == b.second;
	}
};

struct datetime {
	Date date;
	Time time;

	datetime(): date(0, 0, 0),
	            time(0, 0, 0) {}
	explicit  datetime(const Date& date, const Time time = Time(0, 0, 0)): date(date), time(time) {}

    explicit datetime(const char *time_str): date(0, 0, 0),
                                             time(0, 0, 0) {
        sscanf(time_str,
               "%d-%d-%d %d:%d:%d",
               &date.year, &date.month, &date.day, &time.hour, &time.minute, &time.second);
    }
    bool operator<(const datetime &b) const {
	    return this->date < b.date || this->time < b.time;
    }
	bool operator==(const datetime &b) const {
		return this->date == b.date && this->time == b.time;
	}

};



