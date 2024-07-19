//It's a port from linux __secs_to_tm.c
//https://git.musl-libc.org/cgit/musl/tree/src/time/__secs_to_tm.c?h=v0.9.15

#include "h/unixTimeConverter.hpp"

/* 2000-03-01 (mod 400 year, immediately after feb29 */
#define LEAPOCH (946684800LL + 86400*(31+29))

#define DAYS_PER_400Y (365*400 + 97)
#define DAYS_PER_100Y (365*100 + 24)
#define DAYS_PER_4Y   (365*4   + 1)

bool TryConvertUnixTimeToDateTime(uint32_t unixTime, DateTime* result)
{
    uint64_t days, secs;
	uint32_t remdays, remsecs, remyears;
	uint32_t qc_cycles, c_cycles, q_cycles;
	uint32_t years, months;
	uint32_t wday, yday, leap;
	static const char days_in_month[] = {31,30,31,30,31,31,30,31,30,31,31,29};

	/* Reject time_t values whose year would overflow int */
	if (unixTime < INT_MIN * 31622400LL || unixTime > INT_MAX * 31622400LL)
	{
		ESP_LOGW("UnixTimeToDateTime", "unixTime not in correct range");
		return false;
	}

	secs = unixTime - LEAPOCH;
	days = secs / 86400;
	remsecs = secs % 86400;
	if (remsecs < 0) {
		remsecs += 86400;
		days--;
	}

	wday = (3+days)%7;
	if (wday < 0) wday += 7;

	qc_cycles = days / DAYS_PER_400Y;
	remdays = days % DAYS_PER_400Y;
	if (remdays < 0) {
		remdays += DAYS_PER_400Y;
		qc_cycles--;
	}

	c_cycles = remdays / DAYS_PER_100Y;
	if (c_cycles == 4) c_cycles--;
	remdays -= c_cycles * DAYS_PER_100Y;

	q_cycles = remdays / DAYS_PER_4Y;
	if (q_cycles == 25) q_cycles--;
	remdays -= q_cycles * DAYS_PER_4Y;

	remyears = remdays / 365;
	if (remyears == 4) remyears--;
	remdays -= remyears * 365;

	//leap = !remyears && (q_cycles || !c_cycles);
	//yday = remdays + 31 + 28 + leap;
	//if (yday >= 365+leap) yday -= 365+leap;

	years = remyears + 4*q_cycles + 100*c_cycles + 400*qc_cycles;

	for (months=0; days_in_month[months] <= remdays; months++)
		remdays -= days_in_month[months];

	//if (years+100 > INT_MAX || years+100 < INT_MIN)
	//{
	//	ESP_LOGW("UnixTimeToDateTime", "unixTime year can overflow int: %d", years);
	//	return result;
	//}

	result->year = years % 100; //+ 100;
	result->month = months + 2;
	if (result->month >= 12) {
		result->month -=12;
		result->year++;
	}
	result->date = remdays + 1;
	result->dayOfWeek = wday;
	//tm->tm_yday = yday;

	result->hour = remsecs / 3600;
	result->minute = remsecs / 60 % 60;
	result->second = remsecs % 60;

	return true;
}