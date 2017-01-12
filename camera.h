# pragma once

#include "linear_algebra.h"

#define M_PI 3.14156265
#define PI_OVER_TWO 1.5707963267948966192313216916397514420985

// Camera struct, used to store interactive camera data, copied to the GPU and used by OpenCL for each frame
struct Camera {
	Vector3Df position;		// 16 bytes
	Vector3Df view;			// 16
	Vector3Df up;			// 16
	Vector2Df resolution;	// 8
	Vector2Df fov;			// 8
	float apertureRadius;	// 4
	float focalDistance;	// 4
	//float dummy1;		// 4
	//float dummy2;		// 4
};

// class for interactive camera object, updated on the CPU for each frame and copied into Camera struct
class InteractiveCamera
{
private:

	Vector3Df centerPosition;
	Vector3Df viewDirection;
	float yaw;
	float pitch;
	float radius;
	float apertureRadius;
	float focalDistance;

	void fixYaw();
	void fixPitch();
	void fixRadius();
	void fixApertureRadius();
	void fixFocalDistance();

public:
	InteractiveCamera();
	virtual ~InteractiveCamera();
	void changeYaw(float m);
	void changePitch(float m);
	void changeRadius(float m);
	void changeAltitude(float m);
	void changeFocalDistance(float m);
	void strafe(float m);
	void goForward(float m);
	void rotateRight(float m);
	void changeApertureDiameter(float m);
	void setResolution(float x, float y);
	void setFOVX(float fovx);

	void buildRenderCamera(Camera* renderCamera);

	Vector2Df resolution;
	Vector2Df fov;
};
