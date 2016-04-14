#include "stdafx.h"
#define NUMCON 1   /* Number of constraints.             */
#define NUMVAR 3   /* Number of variables.               */
#define NUMANZ 3   /* Number of non-zeros in A.           */
#define NUMQNZ 2   /* Number of non-zeros in Q.           */

static void MSKAPI printstr(void *handle, MSKCONST char str[])
{
	printf("%s", str);
} /* printstr */

int main(int argc, char *argv[])
{

	double        c[] = { -1.0,0.1,1.0 };

	MSKboundkeye  bkc[] = { MSK_BK_UP };
	double        buc[] = { -6.99 };
	double        blc[] = { -MSK_INFINITY };

	MSKboundkeye  bkx[] = { MSK_BK_FR,
		MSK_BK_FR,
		MSK_BK_FR };
	double        blx[] = { -MSK_INFINITY,
		-MSK_INFINITY,
		-MSK_INFINITY };
	double        bux[] = { +MSK_INFINITY,
		+MSK_INFINITY,
		+MSK_INFINITY };

	MSKint32t     aptrb[] = { 0,   1,   2 },
		aptre[] = { 1,   2,   3 },
		asub[] = { 0,   0,   0 };
	double        aval[] = { -2.0, -4.2, -1.0 };

	MSKint32t     qsubi[NUMQNZ];
	MSKint32t     qsubj[NUMQNZ];
	double        qval[NUMQNZ];

	MSKint32t     i, j;
	double        xx[NUMVAR];

	MSKenv_t      env = NULL;
	MSKtask_t     task = NULL;
	MSKrescodee   r;

	/* Create the mosek environment. */
	r = MSK_makeenv(&env, NULL);

	if (r == MSK_RES_OK)
	{
		/* Create the optimization task. */
		r = MSK_maketask(env, NUMCON, NUMVAR, &task);

		if (r == MSK_RES_OK)
		{
			r = MSK_linkfunctotaskstream(task, MSK_STREAM_LOG, NULL, printstr);

			/* Append 'NUMCON' empty constraints.
			The constraints will initially have no bounds. */
			if (r == MSK_RES_OK)
				r = MSK_appendcons(task, NUMCON);

			/* Append 'NUMVAR' variables.
			The variables will initially be fixed at zero (x=0). */
			if (r == MSK_RES_OK)
				r = MSK_appendvars(task, NUMVAR);

			/* Optionally add a constant term to the objective. */
			if (r == MSK_RES_OK)
				r = MSK_putcfix(task, 0.5*1.01);
			for (j = 0; j<NUMVAR && r == MSK_RES_OK; ++j)
			{
				/* Set the linear term c_j in the objective.*/
				if (r == MSK_RES_OK)
					r = MSK_putcj(task, j, c[j]);

				/* Set the bounds on variable j.
				blx[j] <= x_j <= bux[j] */
				if (r == MSK_RES_OK)
					r = MSK_putvarbound(task,
						j,           /* Index of variable.*/
						bkx[j],      /* Bound key.*/
						blx[j],      /* Numerical value of lower bound.*/
						bux[j]);     /* Numerical value of upper bound.*/

									 /* Input column j of A */
				

			}

			if (r == MSK_RES_OK)
				r = MSK_putarow(task,
					0,                 
					3, 
					aptrb,     
					aval);    
			/* Set the bounds on constraints.
			for i=1, ...,NUMCON : blc[i] <= constraint i <= buc[i] */
			for (i = 0; i<NUMCON && r == MSK_RES_OK; ++i)
				r = MSK_putconbound(task,
					i,           /* Index of constraint.*/
					bkc[i],      /* Bound key.*/
					blc[i],      /* Numerical value of lower bound.*/
					buc[i]);     /* Numerical value of upper bound.*/

			if (r == MSK_RES_OK)
			{
				/*
				* The lower triangular part of the Q
				* matrix in the objective is specified.
				*/

				qsubi[0] = 0;   qsubj[0] = 0;  qval[0] = 1.0;
				qsubi[1] = 1;   qsubj[1] = 1;  qval[1] = 1.0;
				/*qsubi[2] = 2;   qsubj[2] = 0;  qval[2] = -1.0;
				qsubi[3] = 2;   qsubj[3] = 2;  qval[3] = 2.0;*/

				/* Input the Q for the objective. */

				r = MSK_putqobj(task, NUMQNZ, qsubi, qsubj, qval);
			}

			if (r == MSK_RES_OK)
			{
				MSKrescodee trmcode;

				/* Run optimizer */
				r = MSK_optimizetrm(task, &trmcode);

				/* Print a summary containing information
				about the solution for debugging purposes*/
				MSK_solutionsummary(task, MSK_STREAM_MSG);

				if (r == MSK_RES_OK)
				{
					MSKsolstae solsta;
					int j;

					MSK_getsolsta(task, MSK_SOL_ITR, &solsta);

					switch (solsta)
					{
					case MSK_SOL_STA_OPTIMAL:
					case MSK_SOL_STA_NEAR_OPTIMAL:
						MSK_getxx(task,
							MSK_SOL_ITR,    /* Request the interior solution. */
							xx);

						printf("Optimal primal solution\n");
						for (j = 0; j<NUMVAR; ++j)
							printf("x[%d]: %e\n", j, xx[j]);

						break;
					case MSK_SOL_STA_DUAL_INFEAS_CER:
					case MSK_SOL_STA_PRIM_INFEAS_CER:
					case MSK_SOL_STA_NEAR_DUAL_INFEAS_CER:
					case MSK_SOL_STA_NEAR_PRIM_INFEAS_CER:
						printf("Primal or dual infeasibility certificate found.\n");
						break;

					case MSK_SOL_STA_UNKNOWN:
						printf("The status of the solution could not be determined.\n");
						break;
					default:
						printf("Other solution status.");
						break;
					}
				}
				else
				{
					printf("Error while optimizing.\n");
				}
			}

			if (r != MSK_RES_OK)
			{
				/* In case of an error print error code and description. */
				char symname[MSK_MAX_STR_LEN];
				char desc[MSK_MAX_STR_LEN];

				printf("An error occurred while optimizing.\n");
				MSK_getcodedesc(r,
					symname,
					desc);
				printf("Error %s - '%s'\n", symname, desc);
			}
		}
		MSK_deletetask(&task);
	}
	MSK_deleteenv(&env);

	return (r);
} /* main */
