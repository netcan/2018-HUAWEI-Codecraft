/****************************************************************************
 > File Name: datetime.cpp
 > Author: Netcan
 > Blog: http://www.netcan666.com/
 > Mail: netcan1996@gmail.com
 > Created Time: 2018-03-11 -- 16:05
 ****************************************************************************/

#include "datetime.h"

int Date::jd(const Date& date) {
	return date.day-32075+
	       1461*(date.year+4800+(date.month-14)/12)/4+
	       367*(date.month-2-(date.month-14)/12*12)/
	       12-3*((date.year+4900+(date.month-14)/12)/100)/4;
}

Date Date::gdate(int jd) {
	Date ret;
	int L = jd+68569;
	int N = 4*L/146097;
	L = L-(146097*N+3)/4;
	ret.year = 4000*(L+1)/1461001;
	L = L-1461*ret.year/4+31;
	ret.month = 80*L/2447;
	ret.day = L-2447*ret.month/80;
	L = ret.month/11;
	ret.month = ret.month+2-12*L;
	ret.year = 100*(N-49)+ret.year+L;
	return ret;
}
