#include "main.h"
#include "replay.h"

#define DO_AUTON 1 // this is automatically commented by comp_upload.sh when uploading for a competition
//#define REC_AUTON 1 // this is automatically enabled by comp_upload.sh when uploading for a competition

// define gear ratios and stuff
#define PRONG_GEAR_RATIO 4.8
#define PRONG_PORT 10
#define PRONG_SPEED 22

#define AUTON_DRIVE_SPEED 55
#define AUTON_SEEK_TIME 2160

#define RIGHT_SENSITIVITY 0.65

#define REC_LENGTH 60 * 500

// for controller axis
struct Joystick {
	int x;
	int y;
};

// order: [left, right, top, bottom]
uint8_t wheels[4] = {1, 19, 2, 11};


/**
 * Runs initialization code. This occurs as soon as the program is started.
 *
 * All other competition modes are blocked by initialize; it is recommended
 * to keep execution time for this mode under a few seconds.
 */
void initialize() {
	// 36:1 gear ratio (red gearbox) and reverse
	motor_set_reversed(PRONG_PORT, true);
	motor_set_gearing(PRONG_PORT, E_MOTOR_GEARSET_36);
	
	// set prong units to degrees and brake mode to hold
	motor_set_brake_mode(PRONG_PORT, E_MOTOR_BRAKE_HOLD);
	motor_set_encoder_units(PRONG_PORT, E_MOTOR_ENCODER_DEGREES);
	
	motor_tare_position(PRONG_PORT);
	
	for (int i = 0; i < 4; i++) {
		motor_set_gearing(wheels[i], E_MOTOR_GEARSET_18); // all wheels are 18:1
		motor_set_brake_mode(wheels[i], E_MOTOR_BRAKE_BRAKE);
		// right and top motors are reversed
		if (i == 1 || i == 2) {
			motor_set_reversed(wheels[i], false);
		}
		else {
			motor_set_reversed(wheels[i], true);
		}
	}
	controller_print(E_CONTROLLER_MASTER, 0, 0, "Good morning!");
}

void stop_all_motors() {
	for (int i = 0; i < 4; i++) {
		motor_move(wheels[i], 0);
	}
	motor_move_velocity(PRONG_PORT, 0);
}

/**
	* this function hangs until the motor has spun to the correct position
	@param port the port to spin
	@param position the postition to rotate the port to - needs a direction
	@param gear_ratio the gear ratio of the the motor. do 1 if there's no gear
	*/
int spin_to(uint8_t port, double position, int32_t velocity, double gear_ratio) {
	int time_taken = 0;
	position *= gear_ratio;
	while (!(motor_get_position(port) < position + 5 && motor_get_position(port) > position - 5)) {
		motor_move_velocity(port, velocity);
		delay(2); // delay until within 10 units of position
		time_taken += 2;

		if (time_taken % 256 == 0) {
			printf("moving motor %d to position %lf. current thingy is at %lf\r\n", port, position / gear_ratio, motor_get_position(port) / gear_ratio);
		}
	}
	motor_move_velocity(port, 0);
	printf("\r\n\r\n\tMove done!\r\n\r\n");
	return time_taken;
}

void spin_all_wheels(int speed) {
	for (int i = 0; i < 4; i++) {
		motor_move(wheels[i], speed);
	}
}

int is_pressing(int button) {
	return controller_get_digital(E_CONTROLLER_MASTER, button);
}

void play_recording(ReplayStep* replay) {
	for (int i = 0; replay[i-1].last != 1; i++) {
		for (int ii = 0; ii < 4; ii++) {
			if (replay[i].wheels[ii] == replay[i - 1].wheels[ii]) {
				continue;
			}
			motor_move_velocity(wheels[ii], replay[i].wheels[ii]);
		}
		if (replay[i].prong != replay[i - 1].prong) {
			motor_move_velocity(PRONG_PORT, replay[i].prong);
		}
		delay(2);
	}
}

void play_auton_recording() {
	ReplayStep* replay = read_replay("/usd/rec");
	play_recording(replay);
}

/**
 * Runs while the robot is in the disabled state of Field Management System or
 * the VEX Competition Switch, following either autonomous or opcontrol. When
 * the robot is enabled, this task will exit.
 */
void disabled() {}

/**
 * Runs after initialize(), and before autonomous when connected to the Field
 * Management System or the VEX Competition Switch. This is intended for
 * competition-specific initialization routines, such as an autonomous selector
 * on the LCD.
 *
 * This task will exit when the robot is enabled and autonomous or opcontrol
 * starts.
 */
void competition_initialize() {}

/**
 * Runs the user autonomous code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the autonomous
 * mode. Alternatively, this function may be called in initialize or opcontrol
 * for non-competition testing purposes.
 *
 * If the robot is disabled or communications is lost, the autonomous task
 * will be stopped. Re-enabling the robot will restart the task, not re-start it
 * from where it left off.
 */
void autonomous() {
	#ifndef DO_AUTON
		return;
	#endif
	
	#ifdef REC_AUTON
		play_auton_recording();
		return;
	#endif
	
	typedef enum {seeking = 0, lifting, returning, dropping} State; // thank god for c
	
	State state = seeking;
	
	int total_time = 0;
	int return_time = 0;
	
	motor_move_absolute(PRONG_PORT, 55.0 * PRONG_GEAR_RATIO, 22); // start prong movement
	
	controller_clear_line(E_CONTROLLER_MASTER, 0);
	controller_print(E_CONTROLLER_MASTER, 0, 0, "We are autonomizing!!!!");
	
	while (total_time < 15000) {
		switch(state) {
			case seeking:
				if (total_time >= AUTON_SEEK_TIME) {
					state = lifting;
				}
				else {
					spin_all_wheels(AUTON_DRIVE_SPEED);
				}
				break;
			
			case lifting:
				stop_all_motors();
				total_time += spin_to(PRONG_PORT, 28.0, -22, PRONG_GEAR_RATIO); // speed needs to be backwards so that it goes up
				state = returning;
				break;
				
			case returning:
				if (return_time >= AUTON_SEEK_TIME) {
					state = dropping;
					stop_all_motors();
				}
				else {
					spin_all_wheels(AUTON_DRIVE_SPEED * -1);
					return_time += 2;
				}
				break;
				
			case dropping:
				spin_to(PRONG_PORT, 55.0, 22, PRONG_GEAR_RATIO);
				goto auton_done;
				break;
			
			default:
				printf("why is this here?");
				break;
		}
		
		total_time += 2;
		delay(2);
	}
	auton_done: ;
	stop_all_motors();
	printf("\r\n\r\n\tAuton done!\r\n\r\n");
}

/**
 * Runs the operator control code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the operator
 * control mode.
 *
 * If no competition control is connected, this function will run immediately
 * following initialize().
 *
 * If the robot is disabled or communications is lost, the
 * operator control task will be stopped. Re-enabling the robot will restart the
 * task, not resume it from where it left off.
 */
void opcontrol() {
	struct Joystick left_stick = {0, 0};
	struct Joystick right_stick = {0, 0};
	
	
	int wheel_power[4] = {0, 0, 0, 0};
	int frames = 0;
	
	controller_clear_line(E_CONTROLLER_MASTER, 0);
	controller_print(E_CONTROLLER_MASTER, 0, 0, "vroom vroom!");
	
	bool record_armed = false;
	bool recording = false;
	ReplayStep replay[REC_LENGTH - 1];
	int replay_step = 0;
	
	while (true) {
		left_stick.x = controller_get_analog(E_CONTROLLER_MASTER, E_CONTROLLER_ANALOG_LEFT_X);
		left_stick.y = controller_get_analog(E_CONTROLLER_MASTER, E_CONTROLLER_ANALOG_LEFT_Y);
		
		right_stick.x = controller_get_analog(E_CONTROLLER_MASTER, E_CONTROLLER_ANALOG_RIGHT_X);
		right_stick.y = controller_get_analog(E_CONTROLLER_MASTER, E_CONTROLLER_ANALOG_RIGHT_Y);
		
		// do some math stuff idk it works
		wheel_power[0] = left_stick.y + left_stick.x;
		wheel_power[1] = left_stick.y + left_stick.x;
		
		wheel_power[2] = left_stick.y - left_stick.x;
		wheel_power[3] = left_stick.y - left_stick.x;
		
		// robot go spiiiinnnnnn
		wheel_power[0] -= right_stick.x * RIGHT_SENSITIVITY;
		wheel_power[1] += right_stick.x * RIGHT_SENSITIVITY;
		
		wheel_power[2] += right_stick.x * RIGHT_SENSITIVITY;
		wheel_power[3] -= right_stick.x * RIGHT_SENSITIVITY;
		
		// da prongs
		if (is_pressing(E_CONTROLLER_DIGITAL_R1) && (motor_get_position(PRONG_PORT) / PRONG_GEAR_RATIO > 0 || is_pressing(E_CONTROLLER_DIGITAL_A))) {
			motor_move_velocity(PRONG_PORT, -PRONG_SPEED);
			if (is_pressing(E_CONTROLLER_DIGITAL_A)) {
				motor_tare_position(PRONG_PORT);
			}
		}
		else if (is_pressing(E_CONTROLLER_DIGITAL_R2) && (motor_get_position(PRONG_PORT) / PRONG_GEAR_RATIO < 70 || is_pressing(E_CONTROLLER_DIGITAL_A))) {
			motor_move_velocity(PRONG_PORT, PRONG_SPEED);
			if (is_pressing(E_CONTROLLER_DIGITAL_A)) {
				motor_tare_position(PRONG_PORT);
			}
		}
		else {
			motor_move_velocity(PRONG_PORT, 0);
		}
		
		// actually spin motors
		for (int i = 0; i < 4; i++) {
			motor_move(wheels[i], wheel_power[i]);
		}
		
		// prints and stuff
		if (frames == 250) {
			frames = 0;
			printf("prong position: %lf\r\n", motor_get_position(PRONG_PORT) / PRONG_GEAR_RATIO);
			printf("in %d units\r\n", motor_get_encoder_units(PRONG_PORT)); // should be 0
		}
		

		// arm recording
		if (record_armed && !recording) {
			bool wheel_moving = false;
			for (int i = 0; i < 4; i++) {
				if (motor_get_actual_velocity(wheels[i]) > 0) {
					wheel_moving = true;
					break;
				}
			}
			if (motor_get_actual_velocity(PRONG_PORT) > 0 || wheel_moving) {
				recording = true;
				printf("Began recording!\r\n");
			}
		}
		
		if (recording) {
			for (int i = 0; i < 4; i++) {
				replay[replay_step].wheels[i] = motor_get_actual_velocity(wheels[i]);
			}
			replay[replay_step].prong = motor_get_actual_velocity(PRONG_PORT);
			replay_step++;
			if (replay_step == REC_LENGTH + 1 || is_pressing(E_CONTROLLER_DIGITAL_LEFT)) {
				replay[replay_step].last = 1; // indicate end of recording
				
				srand(frames);
				int randomid = rand() % 100;
				char filename[12];
				sprintf(filename, "/usd/%d.rec", randomid);
				
				write_replay(replay, filename);
				
				printf("\r\n\r\n\tRecording written to %s\r\n\r\n", filename);
				
				replay_step = 0;
				recording = false;
				record_armed = false;
			}
		}
		
		if (controller_get_digital_new_press(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_X)) {
			record_armed = !record_armed;
			printf("Record toggle\r\n");
		}
		
		if (is_pressing(E_CONTROLLER_DIGITAL_LEFT) && is_pressing(E_CONTROLLER_DIGITAL_A)) {
			play_auton_recording();
		}
		
		delay(2);
		frames++;
	}
}
