// StudyMosek.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

/* This function prints log output from MOSEK to the terminal. */
 static void MSKAPI printstr(void *handle,	 MSKCONST char str[])
	 {
	 printf("%s", str);
	 } /* printstr */

int main()
{
	MSKenv_t env = NULL;
	MSKtask_t task = NULL;
	MSKrescodee r;
	r = MSK_makeenv(&env, NULL);
	r = MSK_maketask(env, numcon, numvar, &task);


	return 0;
}

