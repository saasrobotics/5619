#pragma config(Sensor, S1,     ,               sensorI2CCustom)
#pragma config(Sensor, S2,     ,               sensorI2CCustom)
#pragma config(Sensor, S3,     IRsensor,       sensorI2CCustom)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

#include "JoystickDriver.c"
#include "5619Drive.h"

//******************************Global Constants**********************************
/*The threshold variable is the "dead head" for inconsistencies in the joystick*/
const int Threshold = 15;
//The spin of the robot in Omni mode will always be at the same speed since it uses
//the shoulder buttons and not the analog
//spin during tank drive is handled differently by the y values of the analog.
const int SpinSpeed = 65;

const float scissorMultiplier = (99.0/128.0)*(70.0/100.0);

//This value can be changed by the "start" button to switch between
//tank and omni drive. Not sure why we want it, but there it is
bool OMNI=false;

//Value for whether or not the robot is moving, helpful for continuous motion function
bool SMoving = false;

// Determines whether the ball arbiter is allowing balls to be dispensed or not.
bool arbiterDispensing = false;
bool arbiterToggleReleased = true;

// global values to catch the values of the analog joysticks
//renamed so that the code is more readible
int rightJoystickOneX;
int rightJoystickOneY;
int leftJoystickOneX;
int leftJoystickOneY;

// Keeps track of the number of loop ticks.
int tick = 0;
int lastDataTick = 0;

//maxSpeed can be set to slow down the robot so that we don't have to
//send a 100 speed to the motors  This value corresponds to the highest
//value sent to the motors.  SHOULD NEVER BE SET ABOVE 100
//This value will not effect the center wheel since we need it to go faster
int maxSpeed = 100;
//******************************End of Global Constants*****************************

//******************************Joystick setup Functions****************************
//This function will reset the input value to zero if it is within the deadhead threshold;
//Threshold is a global value
int setDeadHead(int coordinateValue){
	if (abs(coordinateValue)<Threshold){
		return 0;}
	else {
			return coordinateValue;
		}
}
/** // TimB: Possible shortening using ternary operator:
int setDeadHead(int coordinateValue)
{
	return ( abs(coordinateValue) >= Threshold ) ? coordinateValue : 0;
}
*/

//Takes the values from the joystick and make appropriate mappings
void updateJoystick() {

	if (DEBUG){
		writeDebugStreamLine("In updateJoystick");
		writeDebugStreamLine("Joystick (x,y): (%i, %i)",joystick.joy1_x1,joystick.joy1_y1);
		Sleep(1000);
	}
	//All of these are divided by 128 and then multiplied by 100
	//This will create a proportional value between -100 and 100 which can then
	// be used as power to the motors

	// X value of right joystick from -128 to 127
	rightJoystickOneX = (maxSpeed*joystick.joy1_x2)/128;
	// Y value of right joystick from -128 to 127
	rightJoystickOneY = (maxSpeed*joystick.joy1_y2)/128;
	// X and Y joystick value from 128 to 127
	leftJoystickOneY = (maxSpeed*joystick.joy1_y1)/128;
	leftJoystickOneX = (maxSpeed*joystick.joy1_x1)/128;


	//We are going to handle the dead head functionality in order to clean up the later
	// code. So we will set joystick values to zero if they are within the deadhead zone
	rightJoystickOneX=setDeadHead(rightJoystickOneX);
	rightJoystickOneY=setDeadHead(rightJoystickOneY);
	leftJoystickOneX=setDeadHead(leftJoystickOneX);
	leftJoystickOneY=setDeadHead(leftJoystickOneY);
}
//********************************End joystick setup functions*****************

//*****************Wrapper functions for drive functionality******************
void spinLeft(){
	if (DEBUG){
		writeDebugStreamLine("In spinLeft");
	}
	Drive_spinLeft(SpinSpeed);
}

void spinRight() {
	if (DEBUG){
		writeDebugStreamLine("In spinRight");
	}
	Drive_spinRight(SpinSpeed);
}

void allStop(){
	if (DEBUG){
		writeDebugStreamLine("In allStop");
	}
	Drive_allStop();
}

void turn180() {
//	nMotorEncoder[mtr_S1_C1_1] = 0;
//	nMotorEncoder[mtr_S1_C1_2] = 0;
//	while(nMotorEncoder[mtr_S1_C1_1 < 1440 * number_rotations]) {
//			Drive_turn(80, -80);
//}
	 int number_rotations = 2.5;
	 long CurrentLeft = GetEncValue(1);
	 long CurrentRight = GetEncValue(2);
	 SetEncValue(1, CurrentLeft + 1440*number_rotations, (byte)95);
	 SetEncValue(2, CurrentRight + 1440*number_rotations, (byte)-95);
}
//********************End Wrapper functions***************************

//Left analog control will drive the Omni Drive (with the center wheel)
void leftOMNIAnalogControl() {
	int centerPowerLevel=0;  //Good programming, we initialize it to a value
	float controllerAngleRad=0;  // Need initialization here
	//This value catches an issue later on in the code where the cos function
	//only returns a positive value.  This will adjust the direction.
	int centerDirection = 1;  //1 is right, -1 is left
	if (leftJoystickOneX<0)
	{
		centerDirection=-1;
	}

	//This if-else will catch the divide by zero problem
	if (leftJoystickOneX==0){
		centerPowerLevel=0;
	}
	else {
		//No need to convert to convert to degrees since the cos below
		//takes a radian value I THINK    TEST TEST
		controllerAngleRad=atan(leftJoystickOneY/leftJoystickOneX);  //In radians
		if (DEBUG){
			writeDebugStreamLine("Controller Angle: %f", controllerAngleRad);
		}
		//This calculation needs more documentation.  More to come.
		/*
		I believe that atan gives values from -PI to PI
		1) We want the centerPowerLevel (CPL) at 100 at angle=0 radians [right]
		2) We want the CPL at 0 when angle=pi/2 (90 degrees) [straight]
		3) We want the CPL at -100 when angle=pi (180 degree) [left]
		4) We want the CPL at 0 when angle=-pi/2 (270 degrees) [backward]
		This coincides with a cos function with an amplitude of 100

		QUESTION: Is the positive value for the center wheel left or right?
		ASSUMPTION: Positive = right CHECKED - This is correct
		However, interesting issue.  The cos function only returns a positive value
		so the "centerDirection" value makes the adjustment
		*/
		centerPowerLevel = (int)( centerDirection*100*cos(controllerAngleRad));
	}
	if (DEBUG){
		writeDebugStreamLine("In leftAnalogControl");
		writeDebugStreamLine("centerPowerIs: %i",centerPowerLevel ); //TEST!!!
		writeDebugStreamLine("controllerAngleRad: %i", controllerAngleRad);
	}
	Drive_driveOmni(leftJoystickOneY,centerPowerLevel);
}

//Only used when OMNI=false set by the "start" button
// Tank drive ignores the x value of the analog controller (Should this be the case?)
void tankAnalogControl() {
	if (DEBUG){
		writeDebugStreamLine("In tankAnalogControl");
	}
	//Sets threshold for moving forward and backward,
	//makes it so that when moving both joysticks one direction it goes that direction
	//The value 75 may be too high
	if(leftJoystickOneY >= 75 && rightJoystickOneY >= 75) {
		int avg = (leftJoystickOneY + rightJoystickOneY) / 2;
		Drive_turn(avg,avg);
	}
	else if(leftJoystickOneY <= -75 && rightJoystickOneY <= -75) {
		int avg = (leftJoystickOneY + rightJoystickOneY) / 2;
		Drive_turn(avg,avg);
	}
	else
		Drive_turn(leftJoystickOneY,rightJoystickOneY);
}
//********************************************************************************
//********************************************************************************
/*
These are the drive controls
1) driver controls
2) operator controls
*/
void driveJoyStickControl(){
	if (DEBUG){
		writeDebugStreamLine("In Drive Joystick Control");
	}
	//This set of if statements handle the different button functions.
	// Question to test.  Should the statements be mutually exclusive
	// Should we only handle one input per cycle?  Results could be strange TEST
	// I will write it to give priority to certain functions
	// buttons come before analog joysticks

	// If any of the letter buttons are pressed, register this as an all stop
	if (joy1Btn(1) || joy1Btn(2) || joy1Btn(3)){
		allStop();
	}
	//180 degree turn
	if (joy1Btn(4))
	{
		turn180();
	}

	// Right bumper puts the grabber up,
	//   left bumper puts the grabber down.
	if (joy1Btn(6))
	{
		Drive_grabberUp();
	}
	else if (joy1Btn(5))
	{
		Drive_grabberDown();
	}

	//back button do nothing placeholder
	if (joy1Btn(9))
	{
	}
	//start button switch between tank and OmniDrive
	//Note: OMNI=false, this will remove any use of the center wheel
	if (joy1Btn(10))
	{
		//if we are switching OFF Omni play Beep Beep
		if (OMNI)
		{
			PlaySound(soundBeepBeep);
		}
		//If we are switching back to Omni Beep Beep Beep Beep
		else {
			PlaySound(soundUpwardTones);
		}
		OMNI=!OMNI;
	}

	if (OMNI)
	{
		//Left analog button will manage the omnidrive and
		//move the robot at the angle given on the analog pad
		leftOMNIAnalogControl();
	} else {
		//Right analog will run the robot like a tank drive not using the center wheel
		tankAnalogControl();
	}

}

void operatorJoystickControl()
{
	//This set of if statements handle the different button functions.
	// Question to test.  Should the statements be mutually exclusive
	// Should we only handle one input per cycle?  Results could be strange TEST

	// Button A triggers whether the ball arbiter is allowing balls to be
	//   dispensed or not.
	// arbiterToggleReleased ensures that the loop will not process the toggle
	//   button until it has been released.
	if (joy2Btn(2) && arbiterToggleReleased)
	{
		arbiterDispensing = !arbiterDispensing;
		arbiterToggleReleased = false;
	}
	else if (!joy2Btn(2))
	{
		arbiterToggleReleased = true;
	}

	if (arbiterDispensing)
	{
		Drive_arbiterDispense();
	} else {
		Drive_arbiterQueue();
	}

	// Right trigger runs the sweeper in,
	//   left trigger runs the sweeper out,
	//   otherwise turn the sweeper off
	if (joy2Btn(8)){
		Drive_sweeperIn();
	}
	else if (joy2Btn(7))
	{
		Drive_sweeperOut();
	}
	else
	{
		Drive_sweeperStop();
	}

	// do nothing placeholder
	// Maybe reset encoders????
	if (joy2Btn(10)){
		//reset encoders
	}

	if (abs(joystick.joy2_y1) > Threshold) // TODO: make these use nice variable names
	{
		Drive_scissorLift(joystick.joy2_y1*scissorMultiplier);

	} else {
		Drive_scissorLift(0);
	}
}
//************************************End Driver/Operator Functions*******************************
//************************************************************************************************

task main()
{

	waitForStart();

	while(true)
	{
		getJoystickSettings(joystick);
		updateJoystick();
		driveJoyStickControl();
		operatorJoystickControl();

		tick++;
	}

}
