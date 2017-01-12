// OpenCL ray tracing tutorial by Sam Lapere, 2016
// http://raytracey.blogspot.com

#include <iostream>
#include <fstream>
#include <vector>
#include "cl_gl_interop.h"
#include <CL\cl.hpp>
#include "linear_algebra.h"
#include "camera.h"
#include "geometry.h"
#include "user_interaction.h"

// TODO
// cleanup()
// check for cl-gl interop

using namespace std;
using namespace cl;

const int sphere_count = 4;


// OpenCL objects
Device device;
CommandQueue queue;
Kernel kernel;
Context context;
Program program;
Buffer cl_output;
Buffer cl_spheres;
Buffer cl_camera;
Buffer cl_accumbuffer;
BufferGL cl_vbo;
vector<Memory> cl_vbos;

// image buffer (not needed with real-time viewport)
// cl_float4* cpu_output;
cl_int err;
unsigned int framenumber = 0;

Camera* hostRendercam = NULL;
Sphere cpu_spheres[sphere_count];

void pickPlatform(Platform& platform, const vector<Platform>& platforms){

	if (platforms.size() == 1) platform = platforms[0];
	else{
		int input = 0;
		cout << "\nChoose an OpenCL platform: ";
		cin >> input;

		// handle incorrect user input
		while (input < 1 || input > platforms.size()){
			cin.clear(); //clear errors/bad flags on cin
			cin.ignore(cin.rdbuf()->in_avail(), '\n'); // ignores exact number of chars in cin buffer
			cout << "No such option. Choose an OpenCL platform: ";
			cin >> input;
		}
		platform = platforms[input - 1];
	}
}

void pickDevice(Device& device, const vector<Device>& devices){

	if (devices.size() == 1) device = devices[0];
	else{
		int input = 0;
		cout << "\nChoose an OpenCL device: ";
		cin >> input;

		// handle incorrect user input
		while (input < 1 || input > devices.size()){
			cin.clear(); //clear errors/bad flags on cin
			cin.ignore(cin.rdbuf()->in_avail(), '\n'); // ignores exact number of chars in cin buffer
			cout << "No such option. Choose an OpenCL device: ";
			cin >> input;
		}
		device = devices[input - 1];
	}
}

void printErrorLog(const Program& program, const Device& device){

	// Get the error log and print to console
	string buildlog = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
	cerr << "Build log:" << std::endl << buildlog << std::endl;

	// Print the error log to a file
	FILE *log = fopen("errorlog.txt", "w");
	fprintf(log, "%s\n", buildlog);
	cout << "Error log saved in 'errorlog.txt'" << endl;
	system("PAUSE");
	exit(1);
}

void initOpenCL()
{
	// Get all available OpenCL platforms (e.g. AMD OpenCL, Nvidia CUDA, Intel OpenCL)
	vector<Platform> platforms;
	Platform::get(&platforms);
	cout << "Available OpenCL platforms : " << endl << endl;
	for (int i = 0; i < platforms.size(); i++)
		cout << "\t" << i + 1 << ": " << platforms[i].getInfo<CL_PLATFORM_NAME>() << endl;

	// Pick one platform
	Platform platform;
	pickPlatform(platform, platforms);
	cout << "\nUsing OpenCL platform: \t" << platform.getInfo<CL_PLATFORM_NAME>() << endl;

	// Get available OpenCL devices on platform
	vector<Device> devices;
	platform.getDevices(CL_DEVICE_TYPE_ALL, &devices);

	cout << "Available OpenCL devices on this platform: " << endl << endl;
	for (int i = 0; i < devices.size(); i++){
		cout << "\t" << i + 1 << ": " << devices[i].getInfo<CL_DEVICE_NAME>() << endl;
		cout << "\t\tMax compute units: " << devices[i].getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>() << endl;
		cout << "\t\tMax work group size: " << devices[i].getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>() << endl << endl;
	}

	// Pick one device
	//Device device;
	pickDevice(device, devices);
	cout << "\nUsing OpenCL device: \t" << device.getInfo<CL_DEVICE_NAME>() << endl;
	cout << "\t\t\tMax compute units: " << device.getInfo<CL_DEVICE_MAX_COMPUTE_UNITS>() << endl;
	cout << "\t\t\tMax work group size: " << device.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>() << endl;

	// Create an OpenCL context on that device.
	// Windows specific OpenCL-OpenGL interop
	cl_context_properties properties[] =
	{
		CL_GL_CONTEXT_KHR, (cl_context_properties)wglGetCurrentContext(),
		CL_WGL_HDC_KHR, (cl_context_properties)wglGetCurrentDC(),
		CL_CONTEXT_PLATFORM, (cl_context_properties)platform(),
		0
	};

	context = Context(device, properties);

	// Create a command queue
	queue = CommandQueue(context, device);


	// Convert the OpenCL source code to a string// Convert the OpenCL source code to a string
	string source;
	ifstream file("opencl_kernel.cl");
	if (!file){
		cout << "\nNo OpenCL file found!" << endl << "Exiting..." << endl;
		system("PAUSE");
		exit(1);
	}
	while (!file.eof()){
		char line[256];
		file.getline(line, 255);
		source += line;
	}

	const char* kernel_source = source.c_str();

	// Create an OpenCL program with source
	program = Program(context, kernel_source);

	// Build the program for the selected device 
	cl_int result = program.build({ device }); // "-cl-fast-relaxed-math"
	if (result) cout << "Error during compilation OpenCL code!!!\n (" << result << ")" << endl;
	if (result == CL_BUILD_PROGRAM_FAILURE) printErrorLog(program, device);
}

// #define float3(x, y, z) {{x, y, z}}  // macro to replace ugly initializer braces

void initScene(Sphere* cpu_spheres){

	// floor
	cpu_spheres[0].radius = 200.0f;
	cpu_spheres[0].position = Vector3Df(0.0f, -200.4f, 0.0f);
	cpu_spheres[0].color = Vector3Df(0.9f, 0.3f, 0.0f);
	cpu_spheres[0].emission = Vector3Df(0.0f, 0.0f, 0.0f);

	// left sphere
	cpu_spheres[1].radius = 0.16f;
	cpu_spheres[1].position = Vector3Df(-0.25f, -0.24f, -0.1f);
	cpu_spheres[1].color = Vector3Df(0.9f, 0.8f, 0.7f);
	cpu_spheres[1].emission = Vector3Df(0.0f, 0.0f, 0.0f);

	// right sphere
	cpu_spheres[2].radius = 0.16f;
	cpu_spheres[2].position = Vector3Df(0.25f, -0.24f, 0.1f);
	cpu_spheres[2].color = Vector3Df(0.9f, 0.8f, 0.7f);
	cpu_spheres[2].emission = Vector3Df(0.0f, 0.0f, 0.0f);

	// lightsource
	cpu_spheres[3].radius = 1.0f;
	cpu_spheres[3].position = Vector3Df(0.0f, 1.36f, 0.0f);
	cpu_spheres[3].color = Vector3Df(0.0f, 0.0f, 0.0f);
	cpu_spheres[3].emission = Vector3Df(9.0f, 8.0f, 6.0f);
}

// hash function to calculate new seed for each frame
// see http://www.reedbeta.com/blog/2013/01/12/quick-and-easy-gpu-random-numbers-in-d3d11/

unsigned int WangHash(unsigned int a) {
	a = (a ^ 61) ^ (a >> 16);
	a = a + (a << 3);
	a = a ^ (a >> 4);
	a = a * 0x27d4eb2d;
	a = a ^ (a >> 15);
	return a;
}


void initCLKernel(){

	// pick a rendermode
	unsigned int rendermode = 1;

	// Create a kernel (entry point in the OpenCL source program)
	kernel = Kernel(program, "render_kernel");

	// specify OpenCL kernel arguments
	//kernel.setArg(0, cl_output);
	kernel.setArg(0, cl_spheres);
	kernel.setArg(1, window_width);
	kernel.setArg(2, window_height);
	kernel.setArg(3, sphere_count);
	kernel.setArg(4, cl_vbo);
	kernel.setArg(5, framenumber);
	kernel.setArg(6, cl_camera);
	kernel.setArg(7, rand());
	kernel.setArg(8, rand());
	kernel.setArg(9, cl_accumbuffer);
	kernel.setArg(10, WangHash(framenumber));
}

void runKernel(){
	// every pixel in the image has its own thread or "work item",
	// so the total amount of work items equals the number of pixels
	std::size_t global_work_size = window_width * window_height;
	std::size_t local_work_size = kernel.getWorkGroupInfo<CL_KERNEL_WORK_GROUP_SIZE>(device);;

	// Ensure the global work size is a multiple of local work size
	if (global_work_size % local_work_size != 0)
		global_work_size = (global_work_size / local_work_size + 1) * local_work_size;

	//Make sure OpenGL is done using the VBOs
	glFinish();

	//this passes in the vector of VBO buffer objects 
	queue.enqueueAcquireGLObjects(&cl_vbos);
	queue.finish();

	// launch the kernel
	queue.enqueueNDRangeKernel(kernel, NULL, global_work_size, local_work_size); // local_work_size
	queue.finish();

	//Release the VBOs so OpenGL can play with them
	queue.enqueueReleaseGLObjects(&cl_vbos);
	queue.finish();
}


void render(){

	//cpu_spheres[1].position.y += 0.01f;
	queue.enqueueWriteBuffer(cl_spheres, CL_TRUE, 0, sphere_count * sizeof(Sphere), cpu_spheres);

	if (buffer_reset){
		float arg = 0;
		queue.enqueueFillBuffer(cl_accumbuffer, arg, 0, window_width * window_height * sizeof(cl_float3));
		framenumber = 0;
	}
	buffer_reset = false;
	framenumber++;

	// build a new camera for each frame on the CPU
	interactiveCamera->buildRenderCamera(hostRendercam);
	// copy the host camera to a OpenCL camera
	queue.enqueueWriteBuffer(cl_camera, CL_TRUE, 0, sizeof(Camera), hostRendercam);
	queue.finish();

	// kernel.setArg(0, cl_spheres);  //  works even when commented out
	kernel.setArg(5, framenumber);
	kernel.setArg(6, cl_camera);
	kernel.setArg(7, rand());
	kernel.setArg(8, rand());
	kernel.setArg(10, WangHash(framenumber));

	runKernel();

	drawGL();
}

void cleanUp(){
	//	delete cpu_output;
}

// initialise camera on the CPU
void initCamera()
{
	delete interactiveCamera;
	interactiveCamera = new InteractiveCamera();

	interactiveCamera->setResolution(window_width, window_height);
	interactiveCamera->setFOVX(45);
}

void main(int argc, char** argv){

	// initialise OpenGL (GLEW and GLUT window + callback functions)
	initGL(argc, argv);
	cout << "OpenGL initialized \n";

	// debug statements
	/*
	cout << "size of Vector3Df: " << sizeof(Vector3Df) << "\n";
	cout << "size of cl_float3: " << sizeof(cl_float3) << "\n";
	cout << "size of cl_float4: " << sizeof(cl_float4) << "\n";
	cout << "size of cl_float: " << sizeof(cl_float) << "\n";
	cout << "size of float: " << sizeof(float) << "\n";
	cout << "size of int: " << sizeof(int) << "\n";
	cout << "size of cl_int: " << sizeof(cl_int) << "\n";
	cout << "size of Sphere: " << sizeof(Sphere) << "\n";
	cout << "size of Camera: " << sizeof(Camera) << "\n";
	*/


	// initialise OpenCL
	initOpenCL();

	// create vertex buffer object
	createVBO(&vbo);

	// call Timer():
	Timer(0);

	//make sure OpenGL is finished before we proceed
	glFinish();

	// initialise scene
	initScene(cpu_spheres);

	cl_spheres = Buffer(context, CL_MEM_READ_ONLY, sphere_count * sizeof(Sphere));
	queue.enqueueWriteBuffer(cl_spheres, CL_TRUE, 0, sphere_count * sizeof(Sphere), cpu_spheres);

	// initialise an interactive camera on the CPU side
	initCamera();
	// create a CPU camera
	hostRendercam = new Camera();
	interactiveCamera->buildRenderCamera(hostRendercam);

	cl_camera = Buffer(context, CL_MEM_READ_ONLY, sizeof(Camera));
	queue.enqueueWriteBuffer(cl_camera, CL_TRUE, 0, sizeof(Camera), hostRendercam);

	// create OpenCL buffer from OpenGL vertex buffer object
	cl_vbo = BufferGL(context, CL_MEM_WRITE_ONLY, vbo);
	cl_vbos.push_back(cl_vbo);

	// reserve memory buffer on OpenCL device to hold image buffer for accumulated samples
	cl_accumbuffer = Buffer(context, CL_MEM_WRITE_ONLY, window_width * window_height * sizeof(cl_float3));

	// intitialise the kernel
	initCLKernel();

	// start rendering continuously
	glutMainLoop();

	// release memory
	cleanUp();

	system("PAUSE");
}
