#include "stdafx.h"
static void MSKAPI printstr(void *handle, MSKCONST char str[])
{
	printf("%s", str);
} /* printstr */

int main(int argc, char *argv[])
{
	using namespace Eigen;
	using namespace std;
	const int n = 2;
	VectorXd xk(n),s(n),y(n),One(n);
	xk << 1.0, -0.1;
	s << -2, -4.2;
	y = xk;
	VectorXi rs(n);
	rs.setLinSpaced(n, 0, n - 1);
	One.setOnes(n);
	MSKboundkeye  bkx[] = { MSK_BK_FR,
		MSK_BK_FR,
		MSK_BK_FR };
	double        blx[] = { -MSK_INFINITY,
		-MSK_INFINITY,
		-MSK_INFINITY };
	double        bux[] = { +MSK_INFINITY,
		+MSK_INFINITY,
		+MSK_INFINITY };
	MSKint32t     i, j;
	MSKint32t ind_con=0;
	double        xx[n+1];

	MSKenv_t      env = NULL;
	MSKtask_t     task = NULL;
	MSKrescodee   r;

	/* Create the mosek environment. */
	r = MSK_makeenv(&env, NULL);

	if (r == MSK_RES_OK)
	{
		/* Create the optimization task. */
		r = MSK_maketask(env, 1, n+1, &task);

		if (r == MSK_RES_OK)
		{
			r = MSK_linkfunctotaskstream(task, MSK_STREAM_LOG, NULL, printstr);

			/* Append 'NUMCON' empty constraints.
			The constraints will initially have no bounds. */
			if (r == MSK_RES_OK)
				r = MSK_appendcons(task, 1);

			/* Append 'n+1' variables.
			The variables will initially be fixed at zero (x=0). */
			if (r == MSK_RES_OK)
				r = MSK_appendvars(task, n+1);

			/* Optionally add a constant term to the objective. */
			if (r == MSK_RES_OK)
				r = MSK_putcfix(task, 0.5*xk.squaredNorm());
			/* Set the linear term c_j in the objective.*/
			// Leo comments c = [-xk;1 ] 
			for (j = 0; j < n && r == MSK_RES_OK; ++j)
			{
				if (r == MSK_RES_OK)
					r = MSK_putcj(task, j, -xk(j));
			}
			/*if (r == MSK_RES_OK)
				r = MSK_putcslice(task, 0, n - 1, ((-1.0)*xk).data());*///I was going to use putcslice and replace all elements at once
			// but I found that this is not going to be easy.
			if (r == MSK_RES_OK)
				r = MSK_putcj(task, n, 1.0);

			for (j = 0; j < n+1 && r == MSK_RES_OK; ++j)
			{
				/* Set the bounds on variable j.
				blx[j] <= x_j <= bux[j] */
				if (r == MSK_RES_OK)
					r = MSK_putvarbound(task,
						j,           /* Index of variable.*/
						bkx[j],      /* Bound key.*/
						blx[j],      /* Numerical value of lower bound.*/
						bux[j]);     /* Numerical value of upper bound.*/
			}

//==================================================
			/* Set the bounds on constraints.
			for i=1, ...,numcon : blc[i] <= constraint i <= buc[i] */
			if (r == MSK_RES_OK)
			r = MSK_putconbound(task,
				ind_con,           /* Index of constraint.*/
				MSK_BK_UP,      /* Bound key.*/
				-MSK_INFINITY,      /* Numerical value of lower bound.*/
				-6.99);     /* Numerical value of upper bound.*/

			if (r == MSK_RES_OK)
				r = MSK_putarow(task,
					ind_con,                 /* Row index.*/
					n,//aptre[i] - aptrb[i], /* Number of non-zeros in row i.*/
					rs.data(),//asub + aptrb[i],     /* Pointer to column indexes of row i.*/
					s.data());    /* Pointer to values of row i.*/

			if (r == MSK_RES_OK)
				r = MSK_putaij(task,
					ind_con,                 /* Row index.*/
					n,//aptre[i] - aptrb[i], /* Number of non-zeros in row i.*/
					-1.0);    /* Pointer to values of row i.*/
		
			if (r == MSK_RES_OK)
			{
				/* Input the Q for the objective. */

				r = MSK_putqobj(task, n, rs.data(), rs.data(), One.data());
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
						for (j = 0; j<n+1; ++j)
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
	int cc;
	std::cin >> cc;
	return (r);
} /* main */
