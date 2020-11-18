#include <thread>
#include <iostream>
#include <exception>

import ShogunProcessReader;

int main()
{
	auto shogun = ShogunProcessReader();
	while (true)
	{
		std::cout.clear();

		try { std::cout << "Turn state: " << (shogun.IsMyTurn() ? "your turn" : "not your turn") << '\n'; }
		catch (const std::exception& ex) { std::cerr << ex.what(); }

		using namespace std::chrono_literals;
		std::this_thread::sleep_for(1000ms);
	}
}