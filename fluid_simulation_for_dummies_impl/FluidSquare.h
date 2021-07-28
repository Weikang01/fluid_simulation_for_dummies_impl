#pragma once
struct FluidSquare
{
	int size;
	float dt;
	float diff;
	float visc;

	float* s;
	float* density;

	float* Vx;
	float* Vy;

	float* Vx0;
	float* Vy0;

	FluidSquare() = default;
};

FluidSquare* FluidSquareCreate(int size, int diffusion, int viscosity, float dt);

void FluidSquareFree(FluidSquare* square);

void FluidSquareAddDensity(FluidSquare* square, int x, int y, float amount);

void FluidSquareAddVelocity(FluidSquare* square, int x, int y, float amountX, float amountY);

void FluidSquareStep(FluidSquare* square);

static void set_bnd_2D(int b, float* x, int N);

static void lin_solve_2D(int b, float* x, float* x0, float a, float c, int iter, int N);

static void diffuse_2D(int b, float* x, float* x0, float diff, float dt, int iter, int N);

static void project_2D(float* velocX, float* velocY, float* p, float* div, int iter, int N);

static void advect_2D(int b, float* d, float* d0, float* velocX, float* velocY, float dt, int N);