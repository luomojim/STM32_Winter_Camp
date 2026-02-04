// #include <webots/robot.h>
#include <webots/motor.h>
#include <webots/distance_sensor.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define TIME_STEP 64
#define MAX_SPEED 6.28
#define IR_SENSOR_COUNT 6
#define IR_THRESHOLD 800.0
#define ULTRASONIC_STOP_TRIGGER 0.01
#define ULTRASONIC_DELAY_TRIGGER 0.05
#define ULTRASONIC_INVALID_LIMIT 2
#define ULTRASONIC_MAX_VALID_DISTANCE 1.0
#define PID_KP 0.1
#define PID_KI 0.01
#define PID_KD 0.05
#define MOVE_MS_PER_CM 20.0
#define TURN_90DURATION_MS 800.0
#define WAIT_200MS 0.2

typedef struct
{
	double kp;
	double ki;
	double kd;
	double target;
	double actual;
	double error;
	double last_error;
	double sum_error;
	double output;
} PID;

typedef struct
{
	WbDeviceTag left_motor;
	WbDeviceTag right_motor;
	WbDeviceTag ir_sensors[IR_SENSOR_COUNT];
	WbDeviceTag ultrasonic_sensor;

	int is_moving;
	double move_start_time;
	double move_duration;
	double move_left_vel;
	double move_right_vel;

	int ultrasonic_invalid_count;

	PID pid_left;
	PID pid_right;
} SmartCar;

void pid_init(PID *pid, double kp, double ki, double kd)
{
	pid->kp = kp;
	pid->ki = ki;
	pid->kd = kd;
	pid->target = 0.0;
	pid->actual = 0.0;
	pid->error = 0.0;
	pid->last_error = 0.0;
	pid->sum_error = 0.0;
	pid->output = 0.0;
}

double pid_calculate(PID *pid, double target, double actual)
{
	pid->target = target;
	pid->actual = actual;
	pid->error = target - actual;
	pid->sum_error += pid->error;
	double delta_error = pid->error - pid->last_error;

	pid->output = (pid->kp * pid->error) +
				  (pid->ki * pid->sum_error) +
				  (pid->kd * delta_error);

	pid->last_error = pid->error;
	return pid->output;
}

void _start_move(SmartCar *car, double left_vel, double right_vel, double duration_ms)
{
	car->is_moving = 1;
	car->move_start_time = wb_robot_get_time();
	car->move_duration = duration_ms / 1000.0;
	car->move_left_vel = left_vel;
	car->move_right_vel = right_vel;

	wb_motor_set_velocity(car->left_motor, left_vel);
	wb_motor_set_velocity(car->right_motor, right_vel);
}

int _check_move_finish(SmartCar *car)
{
	if (!car->is_moving)
		return 1;

	double elapsed_time = wb_robot_get_time() - car->move_start_time;
	if (elapsed_time >= car->move_duration)
	{
		wb_motor_set_velocity(car->left_motor, 0.0);
		wb_motor_set_velocity(car->right_motor, 0.0);
		car->is_moving = 0;
		return 1;
	}

	wb_motor_set_velocity(car->left_motor, car->move_left_vel);
	wb_motor_set_velocity(car->right_motor, car->move_right_vel);
	return 0;
}

void _wait_ms(SmartCar *car, double ms)
{
	double start_time = wb_robot_get_time();
	double wait_seconds = ms / 1000.0;

	while (wb_robot_step(TIME_STEP) != -1)
	{
		if (wb_robot_get_time() - start_time >= wait_seconds)
			break;
	}
}

void motor_forward(SmartCar *car)
{
	if (!car->is_moving)
	{
		wb_motor_set_velocity(car->left_motor, 0.6 * MAX_SPEED);
		wb_motor_set_velocity(car->right_motor, 0.6 * MAX_SPEED);
	}
}

void motor_stop(SmartCar *car)
{
	car->is_moving = 0;
	wb_motor_set_velocity(car->left_motor, 0.0);
	wb_motor_set_velocity(car->right_motor, 0.0);
}

void move_back_cm(SmartCar *car, int cm)
{
	double duration_ms = cm * MOVE_MS_PER_CM;
	_start_move(car, -0.6 * MAX_SPEED, -0.6 * MAX_SPEED, duration_ms);
}

void move_forward_cm(SmartCar *car, int cm)
{
	double duration_ms = cm * MOVE_MS_PER_CM;
	_start_move(car, 0.6 * MAX_SPEED, 0.6 * MAX_SPEED, duration_ms);
}

void turn_right(SmartCar *car)
{
	_start_move(car, 0.6 * MAX_SPEED, -0.6 * MAX_SPEED, TURN_90DURATION_MS);
}

void turn_left(SmartCar *car)
{
	_start_move(car, -0.6 * MAX_SPEED, 0.6 * MAX_SPEED, TURN_90DURATION_MS);
}

void get_ir_values(SmartCar *car, int ir_values[IR_SENSOR_COUNT])
{
	for (int i = 0; i < IR_SENSOR_COUNT; i++)
	{
		double raw_value = wb_distance_sensor_get_value(car->ir_sensors[i]);
		ir_values[i] = (raw_value > IR_THRESHOLD) ? 1 : 0;
	}
}

double get_ultrasonic_distance(SmartCar *car)
{
	double raw_distance = wb_distance_sensor_get_value(car->ultrasonic_sensor);

	if (raw_distance <= 0.0 || raw_distance > ULTRASONIC_MAX_VALID_DISTANCE)
	{
		return 0.0;
	}
	return raw_distance;
}

void smart_car_init(SmartCar *car)
{
	car->is_moving = 0;
	car->move_start_time = 0.0;
	car->move_duration = 0.0;
	car->move_left_vel = 0.0;
	car->move_right_vel = 0.0;

	car->ultrasonic_invalid_count = 0;

	car->left_motor = wb_robot_get_device("left wheel motor");
	car->right_motor = wb_robot_get_device("right wheel motor");
	if (car->left_motor == 0 || car->right_motor == 0)
	{
		printf("【致命错误】电机设备获取失败，请检查Webots场景中设备名称是否为'left wheel motor'/'right wheel motor'\n");
		exit(1);
	}
	wb_motor_set_position(car->left_motor, INFINITY);
	wb_motor_set_position(car->right_motor, INFINITY);
	motor_stop(car);

	char ir_sensor_name[30];
	for (int i = 0; i < IR_SENSOR_COUNT; i++)
	{
		sprintf(ir_sensor_name, "ir_sensor_%d", i + 1);
		car->ir_sensors[i] = wb_robot_get_device(ir_sensor_name);
		if (car->ir_sensors[i] == 0)
		{
			printf("【致命错误】红外传感器%d获取失败，请检查Webots场景中设备名称是否为'%s'\n", i + 1, ir_sensor_name);
			exit(1);
		}
		wb_distance_sensor_enable(car->ir_sensors[i], TIME_STEP);
	}

	car->ultrasonic_sensor = wb_robot_get_device("ultrasonic_sensor");
	if (car->ultrasonic_sensor == 0)
	{
		printf("【致命错误】超声波传感器获取失败，请检查Webots场景中设备名称是否为'ultrasonic_sensor'\n");
		exit(1);
	}
	wb_distance_sensor_enable(car->ultrasonic_sensor, TIME_STEP);

	pid_init(&car->pid_left, PID_KP, PID_KI, PID_KD);
	pid_init(&car->pid_right, PID_KP, PID_KI, PID_KD);
}

int main(int argc, char **argv)
{
	wb_robot_init();

	SmartCar car;
	smart_car_init(&car);

	int ir_values[IR_SENSOR_COUNT];
	double ultrasonic_distance;
	int ultrasonic_trigger, ultrasonic_stop;

	while (wb_robot_step(TIME_STEP) != -1)
	{
		_check_move_finish(&car);
		if (car.is_moving)
			continue;

		get_ir_values(&car, ir_values);
		int red1 = ir_values[0], red2 = ir_values[1], red3 = ir_values[2];
		int red4 = ir_values[3], red5 = ir_values[4], red6 = ir_values[5];

		ultrasonic_distance = get_ultrasonic_distance(&car);
		ultrasonic_trigger = 0;
		ultrasonic_stop = 0;

		if (ultrasonic_distance > 0.0)
		{
			car.ultrasonic_invalid_count = 0;
			if (ultrasonic_distance <= ULTRASONIC_STOP_TRIGGER)
			{
				ultrasonic_stop = 1;
			}
			else if (ultrasonic_distance > ULTRASONIC_STOP_TRIGGER && ultrasonic_distance <= ULTRASONIC_DELAY_TRIGGER)
			{
				ultrasonic_trigger = 1;
			}
		}
		else
		{
			car.ultrasonic_invalid_count++;
			if (car.ultrasonic_invalid_count >= ULTRASONIC_INVALID_LIMIT)
			{
				ultrasonic_stop = 1;
			}
		}

		if (ultrasonic_stop)
		{
			motor_stop(&car);
		}

		else if (ultrasonic_trigger && red1 && red2 && !red3 && !red4 && !red5 && !red6)
		{
			motor_stop(&car);
			_wait_ms(&car, 200);
			move_back_cm(&car, 3);
			while (!_check_move_finish(&car))
				wb_robot_step(TIME_STEP);
			turn_right(&car);
			while (!_check_move_finish(&car))
				wb_robot_step(TIME_STEP);
			move_forward_cm(&car, 10);
			while (!_check_move_finish(&car))
				wb_robot_step(TIME_STEP);
			turn_right(&car);
			while (!_check_move_finish(&car))
				wb_robot_step(TIME_STEP);
			motor_forward(&car);
		}

		else if (ultrasonic_trigger && red1 && !red2 && !red3 && !red4 && !red5 && !red6)
		{
			move_back_cm(&car, 3);
			while (!_check_move_finish(&car))
				wb_robot_step(TIME_STEP);
			turn_right(&car);
			while (!_check_move_finish(&car))
				wb_robot_step(TIME_STEP);
			move_forward_cm(&car, 14);
			while (!_check_move_finish(&car))
				wb_robot_step(TIME_STEP);
			turn_left(&car);
			while (!_check_move_finish(&car))
				wb_robot_step(TIME_STEP);
			move_forward_cm(&car, 14);
			while (!_check_move_finish(&car))
				wb_robot_step(TIME_STEP);
			turn_left(&car);
			while (!_check_move_finish(&car))
				wb_robot_step(TIME_STEP);
			move_forward_cm(&car, 14);
			while (!_check_move_finish(&car))
				wb_robot_step(TIME_STEP);
			turn_right(&car);
			while (!_check_move_finish(&car))
				wb_robot_step(TIME_STEP);
			motor_forward(&car);
		}

		else if (ultrasonic_trigger && !red1 && red2 && !red3 && !red4 && !red5 && !red6)
		{
			motor_stop(&car);
			_wait_ms(&car, 200);
			move_back_cm(&car, 3);
			while (!_check_move_finish(&car))
				wb_robot_step(TIME_STEP);
			turn_left(&car);
			while (!_check_move_finish(&car))
				wb_robot_step(TIME_STEP);
			move_forward_cm(&car, 14);
			while (!_check_move_finish(&car))
				wb_robot_step(TIME_STEP);
			turn_right(&car);
			while (!_check_move_finish(&car))
				wb_robot_step(TIME_STEP);
			move_forward_cm(&car, 14);
			while (!_check_move_finish(&car))
				wb_robot_step(TIME_STEP);
			turn_right(&car);
			while (!_check_move_finish(&car))
				wb_robot_step(TIME_STEP);
			move_forward_cm(&car, 14);
			while (!_check_move_finish(&car))
				wb_robot_step(TIME_STEP);
			turn_left(&car);
			while (!_check_move_finish(&car))
				wb_robot_step(TIME_STEP);
			motor_forward(&car);
		}

		else if (ultrasonic_trigger && !red1 && !red2 && !red3 && !red4 && !red5 && !red6)
		{
			wb_motor_set_velocity(car.left_motor, 0.3 * MAX_SPEED);
			wb_motor_set_velocity(car.right_motor, 0.3 * MAX_SPEED);
		}

		else if (!red1 && !red2 && (red3 || red5))
		{
			if (red3 && red5)
			{
				wb_motor_set_velocity(car.left_motor, 0.7 * MAX_SPEED);
				wb_motor_set_velocity(car.right_motor, 0.6 * MAX_SPEED);
			}
			else if (red3)
			{
				wb_motor_set_velocity(car.left_motor, 0.6 * MAX_SPEED);
				wb_motor_set_velocity(car.right_motor, 0.7 * MAX_SPEED);
			}
			else if (red5)
			{
				wb_motor_set_velocity(car.left_motor, 0.7 * MAX_SPEED);
				wb_motor_set_velocity(car.right_motor, 0.6 * MAX_SPEED);
			}
		}

		else if (!red1 && !red2 && (red4 || red6))
		{
			if (red4 && red6)
			{
				wb_motor_set_velocity(car.left_motor, 0.6 * MAX_SPEED);
				wb_motor_set_velocity(car.right_motor, 0.7 * MAX_SPEED);
			}
			else if (red4)
			{
				wb_motor_set_velocity(car.left_motor, 0.7 * MAX_SPEED);
				wb_motor_set_velocity(car.right_motor, 0.6 * MAX_SPEED);
			}
			else if (red6)
			{
				wb_motor_set_velocity(car.left_motor, 0.6 * MAX_SPEED);
				wb_motor_set_velocity(car.right_motor, 0.7 * MAX_SPEED);
			}
		}

		else
		{
			motor_forward(&car);
		}
	}

	wb_robot_cleanup();
	return 0;
}
