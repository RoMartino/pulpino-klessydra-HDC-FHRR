#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "functions.h"
//#define igrid 257
#define igrid 33
#define jgrid 17

//#ifdef _WIN32
//#include <intrin.h>
//#else
//#include <x86intrin.h>
//#endif

double u[igrid][jgrid+1],uo[igrid][jgrid+1];
double v[igrid+1][jgrid],vo[igrid+1][jgrid];
double p[igrid+1][jgrid+1],po[igrid+1][jgrid+1];
double m[igrid][jgrid];
const double Re = 100.0;
const double delta = 0.5;
const double dx = 16.0/(igrid-1), dy=1.0/(jgrid-1), dt=0.001;
double error = 1.0;

int main()
{
    int i,j,step = 1;

    __asm__("csrw 0x300, 0x8;" );// each thread enables it's own interrupt
    sync_barrier_reset();
    sync_barrier_thread_registration();
    if (Klessydra_get_coreID() == 0) {
        // initialization of u
        for(i=1;i<=igrid-1;i++)
        {
            for(j=1;j<=jgrid-1;j++)
                u[i][j]=0.0;
        }
        for(j=1;j<=jgrid-1;j++)
            u[0][j] = 0.2;
        for(i=0;i<=igrid-1;i++)
        {
            u[i][0] = -u[i][1];
            u[i][jgrid] = -u[i][jgrid-1];
        }

        // initialization of v
        for(i=0;i<=igrid;i++)
        {
            for(j=0;j<=jgrid-1;j++)
                v[i][j]=0.0;
        }

        //initialization of p
        for(i=0;i<=igrid;i++)
        {
            for(j=0;j<=jgrid;j++)
                p[i][j]=1.0;
        }

        //unsigned long int count = __rdtsc();
        //while(error > 0.000001)
        while(step < 2)
        {
///*
            for(i=0;i<=igrid-1;i++)
            {
                for(j=0;j<=jgrid;j++)
                    uo[i][j] = u[i][j];
            }
            for(i=0;i<=igrid;i++)
            {
                for(j=0;j<=jgrid-1;j++)
                    vo[i][j] = v[i][j];
            }
            for(i=0;i<=igrid;i++)
            {
                for(j=0;j<=jgrid;j++)
                    po[i][j] = p[i][j];
            }
//*/

            //solving x-momentum equation
            for(i=1;i<=igrid-2;i++)
            {
                for(j=1;j<=jgrid-1;j++)
                {
                    u[i][j] = u[i][j]-(dt/dx/dy)*( (((u[i+1][j]*u[i+1][j])-(u[i-1][j]*u[i-1][j]))*dy/(2.0))
                                           + (((u[i][j]+u[i][j+1])*(v[i][j]+v[i+1][j]) - (u[i][j]+u[i][j-1])*(v[i][j-1]+v[i+1][j-1]))*dx/(4.0))
                                           + ((p[i+1][j]-p[i][j])*dy)
                                           - (1.0/Re)*(((u[i+1][j]-2.0*u[i][j]+u[i-1][j])*dy/(dx))+((u[i][j+1]-2.0*u[i][j]+u[i][j-1])*dx/(dy))) );
                }
            }

            //x-momentum boundary conditions
            for(j=1;j<=jgrid-1;j++)
            {
                u[0][j] = 0.2;
                u[igrid-1][j] = u[igrid-1][j]-(dt/dx/dy)*( (((u[igrid-1][j]*u[igrid-1][j])-(u[igrid-2][j]*u[igrid-2][j]))*dy)
                                           + (((u[igrid-1][j]+u[igrid-1][j+1])*(v[igrid-1][j]+v[igrid][j]) - (u[igrid-1][j]+u[igrid-1][j-1])*(v[igrid-1][j-1]+v[igrid][j-1]))*dx/(4.0))
                                           + ((p[igrid][j]-p[igrid-1][j])*dy)
                                           - (1.0/Re)*(((u[igrid-1][j]-2.0*u[igrid-2][j]+u[igrid-3][j])*dy/(dx))+((u[igrid-1][j+1]-2.0*u[igrid-1][j]+u[igrid-1][j-1])*dx/(dy))) );
            }

            for(i=0;i<=igrid-1;i++)
            {
                u[i][0] = -u[i][1];
                u[i][jgrid] = -u[i][jgrid-1];
            }


           //solving y-momentum equation
            for(i=1;i<=igrid-1;i++)
            {
                for(j=1;j<=jgrid-2;j++)
                {
                    v[i][j] = v[i][j]-dt*( (((v[i][j+1]*v[i][j+1])-(v[i][j-1]*v[i][j-1]))*dx/(2.0))
                                           + (((v[i][j]+v[i+1][j])*(u[i][j]+u[i][j+1]) - (v[i][j]+v[i-1][j])*(u[i-1][j]+u[i-1][j+1]))*dy/(4.0))
                                           + ((p[i][j+1]-p[i][j])*dx)
                                           - (1.0/Re)*(((v[i][j+1]-2.0*v[i][j]+v[i][j-1])*dx/(dy))+((v[i+1][j]-2.0*v[i][j]+v[i-1][j])*dy/(dx))) );
                }
            }

            //y-momentum boundary conditions
            for(i=0;i<=igrid;i++)
            {
                v[i][0] = 0.0;
                v[i][jgrid-1] = 0.0;
            }


            //continuity equation - solve for p
            for(i=1;i<=igrid-1;i++)
            {
                for(j=1;j<=jgrid-1;j++)
                {
                    p[i][j] = p[i][j] - delta*(dt/dx/dy)*( ((u[i][j] - u[i-1][j])*dy)  + ((v[i][j] - v[i][j-1])*dx) );
                }
            }

            //p boundary conditions
            for(i=0;i<=igrid;i++)
            {
                p[i][0] = p[i][1];
                p[i][jgrid] = p[i][jgrid-1];
            }

            //error
            error = 0.0;
            for(i=1;i<=igrid-1;i++)
            {
                for(j=1;j<=jgrid-1;j++)
                {
                    m[i][j] = ( ((u[i][j]-uo[i][j])/dt) + ((v[i][j]-vo[i][j])/dt) );
                    error = error + fabs(m[i][j]);
                }
            }

            if(step%1000 == 1)
                printf("Error is %f for step %d\n",error,step);

            step++;

        }
        //count = __rdtsc() - count;
        //printf("\ncount = %lu\n", count);

    } // Klessydra_get_coreID() == 0
    sync_barrier();
    /*
    double uc[igrid][jgrid], vc[igrid][jgrid], pc[igrid][jgrid];

    for(i=0;i<=igrid-1;i++)
    {
        for(j=0;j<=jgrid-1;j++)
        {
            uc[i][j] = 0.5*(u[i][j]+u[i][j+1]);
            vc[i][j] = 0.5*(v[i][j]+v[i+1][j]);
            pc[i][j] = 0.25*(p[i][j]+p[i][j+1]+p[i+1][j]+p[i+1][j+1]);
        }
    }

    // OUTPUT DATA
	FILE *fout2, *fout3;
	fout2 = fopen("UVP1.plt","w+t");
	fout3 = fopen("Central_U1.plt","w+t");

	if ( fout2 == NULL )
	{
        printf("\nERROR when opening file\n");
        fclose( fout2 );
	}

    else
	{
        fprintf( fout2, "VARIABLES=\"X\",\"Y\",\"U\",\"V\",\"P\"\n");
        fprintf( fout2, "ZONE  F=POINT\n");
        fprintf( fout2, "I=%d, J=%d\n", igrid, jgrid );

        for ( j = 0 ; j < (jgrid) ; j++ )
        {
            for ( i = 0 ; i < (igrid) ; i++ )
            {
                double xpos, ypos;
                xpos = i*dx;
                ypos = j*dy;
                fprintf( fout2, "%5.8lf\t%5.8lf\t%5.8lf\t%5.8lf\t%5.8lf\n", xpos, ypos, uc[i][j], vc[i][j], pc[i][j] );
            }
        }
	}

	fclose( fout2 );

	// CENTRAL --U
    fprintf(fout3, "VARIABLES=\"U\",\"Y\"\n");
    fprintf(fout3, "ZONE F=POINT\n");
    fprintf(fout3, "I=%d\n", igrid );

    for ( j = 0 ; j < jgrid ; j++ )
    {
        double ypos;
        ypos = (double) j*dy;
        fprintf( fout3, "%5.8lf\t%5.8lf\n", (uc[igrid/2][j] + uc[(igrid/2)+1][j])/(2.), ypos );
    }
    */
    return 0;
}
