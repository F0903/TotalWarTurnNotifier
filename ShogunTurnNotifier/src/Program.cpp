#include <thread>
#include <iostream>
#include <fstream>
#include <exception>
#include <string>

#include <Windows.h>

import ShogunProcessReader;
import PSAudioEngine;

int main()
{
	auto audio = PSAudioEngine();
	const auto shogun = ShogunProcessReader();
	while (true)
	{
		//TODO: Move this to custom console class.
		system("cls");

		try 
		{ 
			if (shogun.IsMyTurn())
			{
				std::cout << "It is your turn." << '\n';
				audio.PlayFile("./media/sound.wav");
			}			 
		}
		catch (const std::exception& ex) { std::cerr << ex.what() << '\n'; break; }

		using namespace std::chrono_literals;
		std::this_thread::sleep_for(2000ms);
	}
}