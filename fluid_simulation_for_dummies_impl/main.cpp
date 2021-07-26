#include <malloc.h>
#include <iostream> 
#define IX(x,y,z) ((x) + (y) * N + (z) * N * N)
struct FluidCube
{
	int size;
	float dt;
	float diff;
	float visc;

	float* s;
	float* density;

	float* Vx;
	float* Vy;
	float* Vz;

	float* Vx0;
	float* Vy0;
	float* Vz0;

	FluidCube() = default;
};

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
	* TODO:
	diffuse - Put a drop of soy sauce in some water, and you'll notice that it doesn't stay still, but it spreads out. This happens even if the water and sauce are both perfectly still. This is called diffusion. We use diffusion both in the obvious case of making the dye spread out, and also in the less obvious case of making the velocities of the fluid spread out.
	*/
	diffuse(1, Vx0, Vx, visc, dt, 4, N);
	diffuse(2, Vy0, Vy, visc, dt, 4, N);
	diffuse(3, Vz0, Vz, visc, dt, 4, N);

	/*
	* TODO:
	project - Remember when I said that we're only simulating incompressible fluids? This means that the amount of fluid in each box has to stay constant. That means that the amount of fluid going in has to be exactly equal to the amount of fluid going out. The other operations tend to screw things up so that you get some boxes with a net outflow, and some with a net inflow. This operation runs through all the cells and fixes them up so everything is in equilibrium.
	*/
	project(Vx0, Vy0, Vz0, Vx, Vy, 4, N);

	/*
	* TODO:
	advect - Every cell has a set of velocities, and these velocities make things move. This is called advection. As with diffusion, advection applies both to the dye and to the velocities themselves.
	*/
	advect(1, Vx, Vx0, Vx0, Vy0, Vz0, dt, N);
	advect(2, Vy, Vy0, Vx0, Vy0, Vz0, dt, N);
	advect(3, Vz, Vz0, Vx0, Vy0, Vz0, dt, N);

	project(Vx, Vy, Vz, Vx0, Vy0, 4, N);

	diffuse(0, s, density, diff, dt, 4, N);
	advect(0, density, s, Vx, Vy, Vz, dt, N);
}

void main()
{

}
