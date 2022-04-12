// ConsoleApplication1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <future>
#include "signalrclient/hub_connection.h"
#include "signalrclient/hub_connection_builder.h"
#include "signalrclient/signalr_value.h"
#include <list>
#include <stdio.h>
#include <stdlib.h>
#include <ctime>
#include <string> 
#include <windows.h>
#include <nlohmann/json.hpp>
#include <chrono>

using json = nlohmann::json;
using std::cout; using std::endl;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::system_clock;

namespace ns {
	// a simple struct to model a person
	struct speedometer {
		std::string raceId;
		int speed;
		long raceTime;
	};

	void to_json(json& j, const speedometer& p) {
		j = json{ {"raceId", p.raceId}, {"speed", p.speed}, {"raceTime", p.raceTime} };
	}

	void from_json(const json& j, speedometer& p) {
		j.at("raceId").get_to(p.raceId);
		j.at("speed").get_to(p.speed);
		j.at("raceTime").get_to(p.raceTime);
	}
}

void callSignalRConnection(json speed) {
	std::promise<void> start_task;
	signalr::hub_connection connection = signalr::hub_connection_builder::create("https://csun-fsae.azurewebsites.net/racehub").build();

	connection.on("Speedometer", [](const std::vector<signalr::value>& m)
		{
			std::cout << m[0].as_string() << std::endl;
		});

	connection.start([&start_task](std::exception_ptr exception) {
		start_task.set_value();
		});

	start_task.get_future().get();

	std::promise<void> send_task;

	std::vector<signalr::value> args{ speed.dump() };
	connection.invoke("Speedometer", args, [&send_task](const signalr::value& value, std::exception_ptr exception) {
		send_task.set_value();
		});

	send_task.get_future().get();

	std::promise<void> stop_task;
	connection.stop([&stop_task](std::exception_ptr exception) {
		stop_task.set_value();
		});

	stop_task.get_future().get();
}

int main()
{
	GUID gidReference;
	HRESULT hCreateGuid = CoCreateGuid(&gidReference);
	std::string RaceId = std::to_string(hCreateGuid);
	
	time_t now = time(0);
	long RaceTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

	srand((unsigned)time(0));

	for (int i = 0; i < 100; i++) {
		int Speed = 1 + (rand() % 50);

		ns::speedometer s{ RaceId, Speed, RaceTime };

		json j = s;

		callSignalRConnection(j);
	}
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
