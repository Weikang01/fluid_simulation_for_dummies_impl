#include "FluidCube.h"
#include <malloc.h>
#include <iostream> 
#define IX(x,y,z) ((x) + (y) * N + (z) * N * N)

FluidCube* FluidCubeCreate(int size, int diffusion, int viscosity, float dt)
{
	FluidCube* cube = new FluidCube;
	int N = size;

	cube->size = size;
	cube->dt = dt;
	cube->diff = diffusion;
	cube->visc = viscosity;

	cube->s = new float[N * N * N];
	cube->density = new float[N * N * N];

	cube->Vx = new float[N * N * N];
	cube->Vy = new float[N * N * N];
	cube->Vz = new float[N * N * N];

	cube->Vx0 = new float[N * N * N];
	cube->Vy0 = new float[N * N * N];
	cube->Vz0 = new float[N * N * N];

	return cube;
}

void FluidCubeFree(FluidCube* cube)
{
	delete[] cube->s;
	delete[] cube->density;

	delete[] cube->Vx;
	delete[] cube->Vy;
	delete[] cube->Vz;

	delete[] cube->Vx0;
	delete[] cube->Vy0;
	delete[] cube->Vz0;

	free(cube);
}

void FluidCubeAddDensity(FluidCube* cube, int x, int y, int z, float amount)
{
	int N = cube->size;
	cube->density[IX(x, y, z)] += amount;
}

void FluidCubeAddVelocity(FluidCube* cube, int x, int y, int z, float amountX, float amountY, float amountZ)
{
	int N = cube->size;
	int index = IX(x, y, z);

	cube->Vx[index] += amountX;
	cube->Vy[index] += amountY;
	cube->Vz[index] += amountZ;
}

void FluidCubeStep(FluidCube* cube)
{
	int N = cube->size;
	float visc = cube->visc;
	float diff = cube->diff;
	float dt = cube->dt;
	float* Vx = cube->Vx;
	float* Vy = cube->Vy;
	float* Vz = cube->Vz;
	float* Vx0 = cube->Vx0;
	float* Vy0 = cube->Vy0;
	float* Vz0 = cube->Vz0;
	float* s = cube->s;
	float* density = cube->density;

	/*
	diffuse - Put a drop of soy sauce in some water, and you'll notice that it doesn't stay still, but it spreads out. This happens even if the water and sauce are both perfectly still. This is called diffusion. We use diffusion both in the obvious case of making the dye spread out, and also in the less obvious case of making the velocities of the fluid spread out.
	*/
	diffuse(1, Vx0, Vx, visc, dt, 4, N);
	diffuse(2, Vy0, Vy, visc, dt, 4, N);
	diffuse(3, Vz0, Vz, visc, dt, 4, N);

	/*
	project - Remember when I said that we're only simulating incompressible fluids? This means that the amount of fluid in each box has to stay constant. That means that the amount of fluid going in has to be exactly equal to the amount of fluid going out. The other operations tend to screw things up so that you get some boxes with a net outflow, and some with a net inflow. This operation runs through all the cells and fixes them up so everything is in equilibrium.
	*/
	project(Vx0, Vy0, Vz0, Vx, Vy, 4, N);

	/*
	advect - Every cell has a set of velocities, and these velocities make things move. This is called advection. As with diffusion, advection applies both to the dye and to the velocities themselves.
	*/
	advect(1, Vx, Vx0, Vx0, Vy0, Vz0, dt, N);
	advect(2, Vy, Vy0, Vx0, Vy0, Vz0, dt, N);
	advect(3, Vz, Vz0, Vx0, Vy0, Vz0, dt, N);

	project(Vx, Vy, Vz, Vx0, Vy0, 4, N);

	diffuse(0, s, density, diff, dt, 4, N);
	advect(0, density, s, Vx, Vy, Vz, dt, N);
}

static void set_bnd(int b, float* x, int N)
{
	for (int j = 1; j < N - 1; j++)
	{
		for (int i = 1; i < N - 1; i++)
		{
			x[IX(i, j, 0)] = b == 3 ? -x[IX(i, j, 1)] : x[IX(i, j, 1)];
			x[IX(i, j, N - 1)] = b == 3 ? -x[IX(i, j, N - 2)] : x[IX(i, j, N - 2)];
		}
	}
	for (int k = 1; k < N - 1; k++)
	{
		for (int i = 1; i < N - 1; i++)
		{
			x[IX(i, 0, k)] = b == 2 ? -x[IX(i, 1, k)] : x[IX(i, 1, k)];
			x[IX(i, N - 1, k)] = b == 2 ? -x[IX(i, N - 2, k)] : x[IX(i, N - 2, k)];
		}
	}
	for (int k = 1; k < N - 1; k++)
	{
		for (int j = 1; j < N - 1; j++)
		{
			x[IX(0, j, k)] = b == 1 ? -x[IX(1, j, k)] : x[IX(1, j, k)];
			x[IX(N - 1, j, k)] = b == 1 ? -x[IX(N - 2, j, k)] : x[IX(N - 2, j, k)];
		}
	}

	x[IX(0, 0, 0)] = .33f * (x[IX(1, 0, 0)]
		+ x[IX(0, 1, 0)]
		+ x[IX(0, 0, 1)]);
	x[IX(0, N - 1, 0)] = .33f * (x[IX(1, N - 1, 0)]
		+ x[IX(0, N - 2, 0)]
		+ x[IX(0, N - 1, 1)]);
	x[IX(0, 0, N - 1)] = .33f * (x[IX(1, 0, N - 1)]
		+ x[IX(0, 1, N - 1)]
		+ x[IX(0, 0, N - 2)]);
	x[IX(0, N - 1, N - 1)] = .33f * (x[IX(1, N - 1, N - 1)]
		+ x[IX(0, N - 2, N - 1)]
		+ x[IX(0, N - 1, N - 2)]);
	x[IX(N - 1, 0, 0)] = .33f * (x[IX(N - 2, 0, 0)]
		+ x[IX(N - 1, 1, 0)]
		+ x[IX(N - 1, 0, 1)]);
	x[IX(N - 1, N - 1, 0)] = .33f * (x[IX(N - 2, N - 1, 0)]
		+ x[IX(N - 1, N - 2, 0)]
		+ x[IX(N - 1, N - 1, 1)]);
	x[IX(N - 1, 0, N - 1)] = .33f * (x[IX(N - 2, 0, N - 1)]
		+ x[IX(N - 1, 1, N - 1)]
		+ x[IX(N - 1, 0, N - 2)]);
	x[IX(N - 1, N - 1, N - 1)] = .33f * (x[IX(N - 2, N - 1, N - 1)]
		+ x[IX(N - 1, N - 2, N - 1)]
		+ x[IX(N - 1, N - 1, N - 2)]);
}

static void lin_solve(int b, float* x, float* x0, float a, float c, int iter, int N)
{
	float cRecip = 1.0f / c;
	for (int k = 0; k < iter; k++)
	{
		for (int m = 1; m < N - 2; m++)
		{
			for (int j = 1; j < N - 2; j++)
			{
				for (int i = 1; i < N - 2; i++)
				{
					x[IX(i, j, m)] = (x0[IX(i, j, m)]
						+ a * (
							x[IX(i + 1, j, m)]
							+ x[IX(i - 1, j, m)]
							+ x[IX(i, j + 1, m)]
							+ x[IX(i, j - 1, m)]
							+ x[IX(i, j, m + 1)]
							+ x[IX(i, j, m - 1)]
							)) * cRecip;
				}
			}
		}
	}
	set_bnd(b, x, N);
}

static void diffuse(int b, float* x, float* x0, float diff, float dt, int iter, int N)
{
	float a = dt * diff * (N - 2) * (N - 2);
	lin_solve(b, x, x0, a, 1 + 6 * a, iter, N);
}

static void project(float* velocX, float* velocY, float* velocZ, float* p, float* div, int iter, int N)
{
	for (int k = 1; k < N - 1; k++)
	{
		for (int j = 1; j < N - 1; j++)
		{
			for (int i = 1; i < N - 1; i++)
			{
				div[IX(i, j, k)] = -.5f * (
					velocX[IX(i + 1, j, k)]
					- velocX[IX(i - 1, j, k)]
					+ velocY[IX(i, j + 1, k)]
					- velocY[IX(i, j - 1, k)]
					+ velocX[IX(i, j, k + 1)]
					- velocX[IX(i, j, k - 1)]
					) / N;
				p[IX(i, j, k)] = 0;
			}
		}
	}
	set_bnd(0, div, N);
	set_bnd(0, p, N);
	lin_solve(0, p, div, 1, 6, iter, N);

	for (int k = 1; k < N - 1; k++)
	{
		for (int j = 1; j < N - 1; j++)
		{
			for (int i = 1; i < N - 1; i++)
			{
				velocX[IX(i, j, k)] -= .5f * (p[IX(i + 1, j, k)] - p[IX(i - 1, j, k)]) * N;
				velocY[IX(i, j, k)] -= .5f * (p[IX(i, j + 1, k)] - p[IX(i, j - 1, k)]) * N;
				velocZ[IX(i, j, k)] -= .5f * (p[IX(i, j, k + 1)] - p[IX(i, j, k - 1)]) * N;
			}
		}
	}
	set_bnd(1, velocX, N);
	set_bnd(2, velocY, N);
	set_bnd(3, velocZ, N);
}

static void advect(int b, float* d, float* d0, float* velocX, float* velocY, float* velocZ, float dt, int N)
{
	float i0, i1, j0, j1, k0, k1;

	float dtx = dt * (N - 2);
	float dty = dt * (N - 2);
	float dtz = dt * (N - 2);

	float s0, s1, t0, t1, u0, u1;
	float tmp1, tmp2, tmp3, x, y, z;

	float Nfloat = N;
	float ifloat = 0.f, jfloat = 0.f, kfloat = 0.f;
	int i, j, k;

	for (k = 1; k < N - 1; k++, kfloat++)
	{
		for (j = 1; j < N - 1; j++, jfloat++)
		{
			for (i = 1; i < N - 1; i++, ifloat)
			{
				tmp1 = dtx * velocX[IX(i, j, k)];
				tmp2 = dty * velocY[IX(i, j, k)];
				tmp3 = dtz * velocZ[IX(i, j, k)];

				x = ifloat - tmp1;
				y = jfloat - tmp2;
				z = kfloat - tmp3;

				if (x < .5f) x = .5f;
				if (x > Nfloat + .5f) x = Nfloat + .5f;
				i0 = floorf(x);
				i1 = i0 + 1.0f;

				if (y < .5f) y = .5f;
				if (y > Nfloat + .5f) y = Nfloat + .5f;
				j0 = floorf(y);
				j1 = j0 + 1.0f;

				if (z < .5f) z = .5f;
				if (z > Nfloat + .5f) z = Nfloat + .5f;
				k0 = floorf(z);
				k1 = k0 + 1.0f;

				s1 = x - i0;
				s0 = 1.0f - s1;
				t1 = y - j0;
				t0 = 1.0f - t1;
				u1 = z - k0;
				u0 = 1.0f - u1;

				int i0i = i0;
				int i1i = i1;
				int j0i = j0;
				int j1i = j1;
				int k0i = k0;
				int k1i = k1;

				d[IX(i, j, k)] =
					s0 * (t0 * (u0 * d0[IX(i0i, j0i, k0i)]
						+ u1 * d0[IX(i0i, j0i, k1i)])
						+ (t1 * (u0 * d0[IX(i0i, j1i, k0i)]
							+ u1 * d0[IX(i0i, j1i, k1i)])))
					+ s1 * (t0 * (u0 * d0[IX(i1i, j0i, k0i)]
						+ u1 * d0[IX(i1i, j0i, k1i)])
						+ (t1 * (u0 * d0[IX(i1i, j1i, k0i)]
							+ u1 * d0[IX(i1i, j1i, k1i)])));
			}
		}
	}
	set_bnd(b, d, N);
}
