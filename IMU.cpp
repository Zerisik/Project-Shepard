#include <cstdint>
#include <string>
#include <iostream>
#include <chrono>
#include <thread>

#include <iio.h>
#include <glm.hpp>
#include <gtc/quaternion.hpp>


/// IIO and glm libraries are necessary to run, make sure to add those

const std::string IMU_Name = "icm42688";
const std::string Mag_Name = "mmc5603";

struct VRData
{
	//data from IMU
	double accX, accY, accZ;
	double gyroX, gyroY, gyroZ;

	//data from magnetometer
	double magX, magY, magZ;

};

glm::quat camera_orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f); //output quaternion from data fusion algorithm, sent to camera

class Sensor //responsible for extracting data from sensor
{
public:
	struct iio_device* device; //reference to sensor device
	struct iio_channel *ch_x, *ch_y, *ch_z; // extracted channels from device

	bool initialize(struct iio_context* contx, const std::string& name, const std::string& var_name)
	{
		device = iio_context_find_device(contx, name.c_str()); //try getting access to sensor device

		//check if device variable was initialized
		if (device == NULL) return false;
		else
		{
			//get access to channels of the device containing info on x,y,z variables
			ch_x = iio_device_find_channel(device, (var_name + "_x").c_str(), false);
			ch_y = iio_device_find_channel(device, (var_name + "_y").c_str(), false);
			ch_z = iio_device_find_channel(device, (var_name + "_z").c_str(), false);



			return ch_x != NULL && ch_y != NULL && ch_z != NULL;
		}
	}

	void readData(double& x, double& y, double& z) //reads data from chosen sensor
	{
		long long rx, ry, rz;

		//reading raw values of x,y,z from chosen device
		iio_channel_attr_read_longlong(ch_x, "raw", &rx);
		iio_channel_attr_read_longlong(ch_y, "raw", &ry);
		iio_channel_attr_read_longlong(ch_z, "raw", &rz);


		x = double(rx), y = double(ry), z = double(rz);

	}
};


void update_camera(VRData &currentdata, float dt) //data fusion algorithm
{
	double gX = currentdata.gyroX, gY = currentdata.gyroY, gZ = currentdata.gyroZ;
	double aX = currentdata.accX, aY = currentdata.accY, aZ = currentdata.accZ;
	double mX = currentdata.magX, mY = currentdata.magY, mZ = currentdata.magZ;

	//creating vector of movement
	glm::vec3 gyro_delta = glm::vec3(gX, gY, gZ) * dt;

	//creating quaternion of rotation and predicting the rotation of camera
	glm::quat gyro_quat = glm::quat(gyro_delta);
	glm::quat gyro_quat_prediction = camera_orientation * gyro_quat;

	//create absolute reference point based on accelerometer and magnetometer
	float pitch = std::atan2(-aX, std::sqrt(aY * aY + aZ * aZ));
	float roll = std::atan2(aY, aZ);
	float yaw = std::atan2(mZ * std::sin(roll) - mY * std::cos(roll), aX * std::cos(pitch) + mY * std::sin(pitch) * std::sin(roll) + mZ * std::sin(pitch) * std::cos(roll));

	//turn Euler angles into quaternion of reference point
	glm::quat ref_quat = glm::quat(glm::vec3(pitch, yaw, roll));

	//Fusion algorithm
	camera_orientation = glm::slerp(gyro_quat_prediction, ref_quat, 0.03f);

	//Normalization
	camera_orientation = glm::normalize(camera_orientation);
}

int main()
{
	//Initialization section - only run once
	struct iio_context* contx = iio_create_local_context();

	Sensor IMUGyro;
	Sensor IMUAccel;
	Sensor Magneto;

	//check for initialization fails
	bool ini_ok = true;
	if (!IMUGyro.initialize(contx, IMU_Name, "gyro")) ini_ok = false;
	else if (!IMUAccel.initialize(contx, IMU_Name, "accel")) ini_ok = false;
	else if (!Magneto.initialize(contx, Mag_Name, "magn")) ini_ok = false;
	
	if (!ini_ok)
	{
		std::cerr << "Unable to initialize. Please check connections.";
		iio_context_destroy(contx);
		return -1;
	}

	VRData currentData;
	auto before_timestamp = std::chrono::high_resolution_clock::now(); // initialization of the clock
	auto current_timestamp = std::chrono::high_resolution_clock::now();

	std::cout << "Everything is OK. Starting data extraction loop. Press Ctrl+C to stop.";

	//Loop section - it will run repeatedly unless stopped by the user
	while (true)
	{
		//reading data
		IMUGyro.readData(currentData.gyroX, currentData.gyroY, currentData.gyroZ);
		IMUAccel.readData(currentData.accX, currentData.accY, currentData.accZ);
		Magneto.readData(currentData.magX, currentData.magY, currentData.magZ);

		current_timestamp = std::chrono::high_resolution_clock::now();
		std::chrono::duration<float> duration = current_timestamp - before_timestamp;
		float dt = duration.count();
		before_timestamp = current_timestamp;

		//here is the data fusion function
		update_camera(currentData, dt);

		std::cout << "w: " << camera_orientation.w << " \nx: " << camera_orientation.x << "\ny: " << camera_orientation.y << "\nz: " << camera_orientation.z << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(15)); //wait for 15 milliseconds before reading another portion of data

	}
	iio_context_destroy(contx);
	return 0;
}