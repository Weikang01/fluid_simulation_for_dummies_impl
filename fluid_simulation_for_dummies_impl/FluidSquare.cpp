#include "FluidSquare.h"
#include <malloc.h>
#include <iostream> 
#define IX_2D(x,y) ((x) + (y) * N)

FluidSquare* FluidSquareCreate(int size, int diffusion, int viscosity, float dt)
{
	FluidSquare* square = new FluidSquare;
	int N = size;

	square->size = size;
	square->dt = dt;
	square->diff = diffusion;
	square->visc = viscosity;

	square->s = new float[N * N];
	square->density = new float[N * N];

	square->Vx = new float[N * N];
	square->Vy = new float[N * N];

	square->Vx0 = new float[N * N];
	square->Vy0 = new float[N * N];

	return square;
}

void FluidSquareFree(FluidSquare* square)
{
	delete[] square->s;
	delete[] square->density;

	delete[] square->Vx;
	delete[] square->Vy;

	delete[] square->Vx0;
	delete[] square->Vy0;

	free(square);
}

void FluidSquareAddDensity(FluidSquare* square, int x, int y, float amount)
{
	int N = square->size;
	square->density[IX_2D(x, y)] += amount;
}

void FluidSquareAddVelocity(FluidSquare* square, int x, int y, float amountX, float amountY)
{
	int N = square->size;
	int index = IX_2D(x, y);

	square->Vx[index] += amountX;
	square->Vy[index] += amountY;
}

void FluidSquareStep(FluidSquare* square)
{
	int N = square->size;
	float visc = square->visc;
	float diff = square->diff;
	float dt = square->dt;
	float* Vx = square->Vx;
	float* Vy = square->Vy;
	float* Vx0 = square->Vx0;
	float* Vy0 = square->Vy0;
	float* s = square->s;
	float* density = square->density;

	diffuse_2D(1, Vx0, Vx, visc, dt, 4, N);
	diffuse_2D(2, Vy0, Vy, visc, dt, 4, N);

	project_2D(Vx0, Vy0, Vx, Vy, 4, N);

	advect_2D(1, Vx, Vx0, Vx0, Vy0, dt, N);
	advect_2D(2, Vy, Vy0, Vx0, Vy0, dt, N);

	project_2D(Vx, Vy, Vx0, Vy0, 4, N);

	diffuse_2D(0, s, density, diff, dt, 4, N);
	advect_2D(0, density, s, Vx, Vy, dt, N);
}

void set_bnd_2D(int b, float* x, int N)
{
	for (int i = 1; i < N - 1; i++)
	{
		x[IX_2D(i, 0)] = b == 2 ? -x[IX_2D(i, 1)] : x[IX_2D(i, 1)];
		x[IX_2D(i, N - 1)] = b == 2 ? -x[IX_2D(i, N - 2)] : x[IX_2D(i, N - 2)];
	}
	for (int j = 1; j < N - 1; j++)
	{
		x[IX_2D(0, j)] = b == 1 ? -x[IX_2D(1, j)] : x[IX_2D(1, j)];
		x[IX_2D(N - 1, j)] = b == 1 ? -x[IX_2D(N - 2, j)] : x[IX_2D(N - 2, j)];
	}

	x[IX_2D(0, 0)] = .5f * (x[IX_2D(1, 0)] + x[IX_2D(0, 1)]);
	x[IX_2D(0, N - 1)] = .5f * (x[IX_2D(1, N - 1)] + x[IX_2D(0, N - 2)]);
	x[IX_2D(N - 1, 0)] = .5f * (x[IX_2D(N - 2, 0)] + x[IX_2D(N - 1, 1)]);
	x[IX_2D(N - 1, N - 1)] = .5f * (x[IX_2D(N - 2, N - 1)] + x[IX_2D(N - 1, N - 2)]);
}

void lin_solve_2D(int b, float* x, float* x0, float a, float c, int iter, int N)
{
}

void diffuse_2D(int b, float* x, float* x0, float diff, float dt, int iter, int N)
{
}

void project_2D(float* velocX, float* velocY, float* p, float* div, int iter, int N)
{
}

void advect_2D(int b, float* d, float* d0, float* velocX, float* velocY, float dt, int N)
{
}
