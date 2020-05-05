// app.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "..\lib\trace.h"

using namespace trace;

int main(
) {
	tracer::config trcfg;
	trcfg.output = {output::debug_monitor};
	//trcfg.show_flag = {/*show_flag::process__breaf_info,*/ show_flag::process__trace_id};

	int i = -26;
	
	//tracer tracer(trcfg);
	//tracer.trace_n(__FUNCTIONW__, L"i: %i", i);

	TRACE_INIT(trcfg);
	TRACE_N(L"i: %i", i);
	TRACE_W(L"i: %i", i);
	TRACE_E(L"i: %i", i);
    
	std::cout << "Hello World!\n";
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
