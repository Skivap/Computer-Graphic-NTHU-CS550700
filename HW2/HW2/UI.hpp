#ifndef UI_HPP
#define UI_HPP

#include "shared.hpp";

class UI
{
	int lastCheckOption = 1;
public:
	int slider = 400;
	bool option1 = true;
	bool option2 = false;
	bool option3 = false;
	bool option4 = false;
	bool option5 = false;
	bool option6 = false;
	bool option7 = false;
	void Render();
};

#endif