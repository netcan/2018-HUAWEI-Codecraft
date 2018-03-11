/****************************************************************************
 > File Name: ecs_test.cpp
 > Author: Netcan
 > Blog: http://www.netcan666.com/
 > Mail: netcan1996@gmail.com
 > Created Time: 2018-03-11 -- 16:44
 ****************************************************************************/

#include "gtest/gtest.h"
#include "../datetime.h"

TEST(Datetime, test_loading) {
	datetime dt("2015-01-06 17:35:07");
	EXPECT_EQ(dt.date.year, 2015);
	EXPECT_EQ(dt.date.month, 1);
	EXPECT_EQ(dt.date.day, 6);

	EXPECT_EQ(dt.time.hour, 17);
	EXPECT_EQ(dt.time.minute, 35);
	EXPECT_EQ(dt.time.second, 7);

}

TEST(Datetime, test_lt) {
	EXPECT_LT(datetime("2015-01-06 17:35:07"),
	          datetime("2015-01-06 17:35:08"));

	EXPECT_LT(datetime("2015-01-06 17:35:07"),
	          datetime("2015-01-06 17:36:07"));

	EXPECT_LT(datetime("2015-01-06 17:35:07"),
	          datetime("2015-01-06 18:36:07"));
}

TEST(Datetime, test_eq) {
	EXPECT_EQ(datetime("2015-01-06 17:35:07"),
	          datetime("2015-01-06 17:35:07"));
}

TEST(Datetime, test_julia_date) {
	EXPECT_EQ(Date(2018, 5, 21), Date::gdate(
			Date::jd(Date(2018, 5, 21))
	));
}

TEST(Datetime, test_substract) {
	EXPECT_EQ(Date(2018, 3, 11) - Date(1996, 7, 8), 7916);
}

TEST(Datetime, test_add) {
	EXPECT_EQ(Date(1996, 7, 8) + 7916, Date(2018, 3, 11));
}
