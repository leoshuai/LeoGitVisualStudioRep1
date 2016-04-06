// StudyMosek.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


int main()
{
	MSKenv_t env = NULL;
	MSKtask_t task = NULL;
	MSKrescodee res;
	res = MSK_makeenv(&env, NULL);
	catch {res == MSK_RES_OK; }


	return 0;
}

