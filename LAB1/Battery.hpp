#include <Windows.h>
#include <iostream>
#include <powrprof.h>
#include <Poclass.h>
#include <Setupapi.h>
#include <devguid.h>
#include <iomanip>

class Battery
{
private:
	std::string power_supply;
	std::string saving_mode;
	std::string state_charge;
	std::string battery_type;
	int percent;
	unsigned long life_time;

public:
	Battery();
	static std::string get_power_supply(SYSTEM_POWER_STATUS status);
	static std::string get_state_charge(SYSTEM_POWER_STATUS status);
	static std::string get_saving_mode(SYSTEM_POWER_STATUS status);
	static void scan_battery_info();
	friend std::ostream& operator<<(std::ostream& out, const Battery& battery);
};
