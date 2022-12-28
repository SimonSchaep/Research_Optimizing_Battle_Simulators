#pragma once
// Here we determine which application is currently active
// Create the define here using the "ActiveApp_..." convention and set up the correct include and typedef in the #ifdef below.

#define ActiveApp_Battle_Simulator_NoOptimizations


//---------- Registered Applications -----------
#ifdef ActiveApp_Battle_Simulator_NoOptimizations
#include "projects/exam/BattleSimulator/App_Battle_Simulator.h"
typedef App_Battle_Simulator CurrentApp;
#endif


class App_Selector {
public: 
	static IApp* CreateApp() {
		IApp* myApp = new CurrentApp();
		return myApp;
	}
};