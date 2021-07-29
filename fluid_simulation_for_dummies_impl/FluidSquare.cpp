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
	float cRecip = 1.f / c;

	for (int k = 0; k < iter; k++)
	{
		for (int j = 1; j < N - 2; j++)
		{
			for (int i = 1; i < N - 2; i++)
			{
				x[IX_2D(i, j)] = (x0[IX_2D(i, j)]
					+ a * (
						x[IX_2D(i + 1, j)]
						+ x[IX_2D(i - 1, j)]
						+ x[IX_2D(i, j + 1)]
						+ x[IX_2D(i, j - 1)]
						)) * cRecip;
			}
		}
	}

	set_bnd_2D(b, x, N);
}

void diffuse_2D(int b, float* x, float* x0, float diff, float dt, int iter, int N)
{
	float a = dt * diff * (N - 2) * (N - 2);
	lin_solve_2D(b, x, x0, a, 1 + 6 * a, iter, N);
}

void project_2D(float* velocX, float* velocY, float* p, float* div, int iter, int N)
{
	for (int j = 1; j < N - 1; j++)
	{
		for (int i = 1; i < N - 1; i++)
		{
			div[IX_2D(i, j)] = -.5f * (
				velocX[IX_2D(i + 1, j)]
				- velocX[IX_2D(i - 1, j)]
				+ velocY[IX_2D(i, j + 1)]
				- velocY[IX_2D(i, j - 1)]
				) / N;
			p[IX_2D(i, j)] = 0;
		}
	}
	set_bnd_2D(0, div, N);
	set_bnd_2D(0, p, N);
	lin_solve_2D(0, p, div, 1, 6, iter, N);

	for (int j = 1; j < N - 1; j++)
	{
		for (int i = 1; i < N - 1; i++)
		{
			velocX[IX_2D(i, j)] -= .5f * (p[IX_2D(i + 1, j)] - p[IX_2D(i - 1, j)]) * N;
			velocY[IX_2D(i, j)] -= .5f * (p[IX_2D(i, j + 1)] - p[IX_2D(i, j - 1)]) * N;
		}
	}
	set_bnd_2D(1, velocX, N);
	set_bnd_2D(2, velocY, N);
}

void advect_2D(int b, float* d, float* d0, float* velocX, float* velocY, float dt, int N)
{
	float i0, i1, j0, j1;

	float dtx = dt * (N - 2);
	float dty = dt * (N - 2);

	float s0, s1, t0, t1;
	float tmp1, tmp2, x, y;

	float Nfloat = N;
	float ifloat = 0.f, jfloat = 0.f;
	int i, j;

	for (j = 1; j < N - 1; j++, jfloat++)
	{
		for (i = 1; i < N - 1; i++, ifloat)
		{
			tmp1 = dtx * velocX[IX_2D(i, j)];
			tmp2 = dty * velocY[IX_2D(i, j)];

			x = ifloat - tmp1;
			y = jfloat - tmp2;

			if (x < .5f) x = .5f;
			if (x > Nfloat + .5f) x = Nfloat + .5f;
			i0 = floorf(x);
			i1 = i0 + 1.0f;

			if (y < .5f) y = .5f;
			if (y > Nfloat + .5f) y = Nfloat + .5f;
			j0 = floorf(y);
			j1 = j0 + 1.0f;

			s1 = x - i0;
			s0 = 1.0f - s1;
			t1 = y - j0;
			t0 = 1.0f - t1;

			int i0i = i0;
			int i1i = i1;
			int j0i = j0;
			int j1i = j1;

			d[IX_2D(i, j)] =
				s0 * (t0 * d0[IX_2D(i0i, j0i)]
					+ t1 * d0[IX_2D(i0i, j1i)]) + 
				s1 * (t0 * d0[IX_2D(i1i, j0i)]
					+ t1 * d0[IX_2D(i1i, j1i)]);
		}
	}
	set_bnd_2D(b, d, N);
}
