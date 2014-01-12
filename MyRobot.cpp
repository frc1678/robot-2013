#include "WPILib.h"
#include <math.h>
#include "GyroReader.h"
#include "Drivetrain.h"
#include "Toggler.h"
#include "Buttons.h"
#include "AvgCounter.h"

#include "NetworkTables/NetworkTable.h"

class Robot : public IterativeRobot
{
	//Testing for digital IO
	DigitalInput *limitswitch; //TODO make it work! (or delete if electrical team can make it work without help)

	RobotDrive *drivetrain; // robot drive system

	Joystick *driverL;
	Joystick *driverR;
	Joystick *joystick3;

	// Togglers
	Toggler *intakeToggler;
	Toggler *conveyorToggler;
	// Toggler for the shooter
	Toggler *shooterToggler;
	Toggler *shooterDeployToggler;
	Toggler *climberDeployToggler;
	//Toggler *medToggler;

	// Driver Station
	DriverStation *driverStation;
	DriverStationLCD *driverStationLCD;

	// Compressor
	Compressor *compressor;

	// Driver Station digital I/Os
	bool digitalIOs[8];

	// Gear shifting
	Solenoid *gearUp;
	Solenoid *gearDown;

	// Shooter motors
	Victor *shooterOrange;
	Victor *shooterBlue;

	// Trigger Motor
	Victor *trigger;

	// Intake Motor
	Victor *intakeRoller;

	// Conveyor Motor; 
	Victor *conveyor;

	// Intake Deploy Solenoid
	Solenoid *intakeDeploy;
	Solenoid *intakeUndeploy;

	// Shooter Deploy Solenoid
	Solenoid *shooterDeploy;
	Solenoid *shooterUndeploy;

	//Climber Deploy Solenoid
	Solenoid *climberDeploy;
	Solenoid *climberUndeploy;

	// Encoders on drivetrain
	Encoder *right_encoder;
	Encoder *left_encoder;

	// Timer
	Timer *autoTimer;

	Victor *testVictor;

	int testLoops;

	float sendTo; //for ramping in autonomous

	float bangbangValue; //for use with bangbang();

	float rpm; //for running the trigger in runtriggerbangbang;
	float oldrpm; //for seeing when it's changed.
	float triggerRpm; //for the limit above which we can shoot.
	float frisbeeCountRpm; //for the limit below which we know we've shot one.
	bool canCount; //for whether or not we're above the trigger; for counting

	//use_counter: 1 is using counter (the newer bangbang), 0 is not.
#define USE_COUNTER 1

#if USE_COUNTER
	AvgCounter *testShooterCounter;
#else
	// Encoders on shooter
	Encoder *blue_shooter_encoder;
#endif	

	NetworkTable *table;
	
public:
	Robot()
	{
		drivetrain = new RobotDrive(3, 2); // these must be initialized in the same 
		// order as they are declared above.
		driverStation = DriverStation::GetInstance();
		driverStationLCD = DriverStationLCD::GetInstance();

		// Init compressor
		compressor = new Compressor(1, 1); 

		// Initialize joysticks
		driverL = new Joystick(1);
		driverR = new Joystick(2);
		joystick3 = new Joystick(3);

		// Initialize solenoids TODO
		gearUp = new Solenoid(1);
		gearDown = new Solenoid(2);

		// Initialize intake roller
		intakeRoller = new Victor (1); //intake roller is reversed

		// Initialize conveyor
		conveyor = new Victor(4);

		// Initialize trigger
		trigger = new Victor(5); //trigger wheel goes to the back.

		// Initialize shooters
		shooterOrange = new Victor(6);
		shooterBlue = new Victor(7);

		// Initialize intake deployer
		intakeDeploy = new Solenoid(5);
		intakeUndeploy = new Solenoid(6);

		// Initialize shooter deployer
		shooterDeploy = new Solenoid(8);
		shooterUndeploy = new Solenoid(7);

		// Initialize climber deployer
		climberDeploy = new Solenoid(3);
		climberUndeploy = new Solenoid(4);

		// Initialize encoders TODO directions
		right_encoder = new Encoder(5, 6, true);
		left_encoder = new Encoder(3, 4, true);

		// Intialize intake toggler
		intakeToggler = new Toggler();

		// Initialize conveyor toggler
		conveyorToggler = new Toggler();

		// Initialize shooter toggler
		shooterToggler = new Toggler();
		//medToggler = new Toggler();

		// Initialize shooter deploy toggler
		shooterDeployToggler = new Toggler();

		// Initialize climbing toggler
		climberDeployToggler = new Toggler();

		//Autonomous timer
		autoTimer = new Timer();

		//for something in test mode
		testLoops = 0;

		//for ramping in autonomous.
		sendTo = .5;

#if USE_COUNTER
		testShooterCounter = new AvgCounter(2); //channel A of the encoder ONLY. 
		testShooterCounter->SetUpSourceEdge(true, false); //read UP not DOWN.
		testShooterCounter->SetSamplesToAverage(125); //magic number from CD
		// TODO calgamesbeginning bangbangValue = 7200.0f;
		bangbangValue = 7200.0f;
#else
		blue_shooter_encoder = new Encoder(13, 14, false);
		bangbangValue = 375;
#endif

		rpm = 0; //for running the trigger in runtriggerbangbang;
		oldrpm = 0; //for seeing when it's changed. Both this and 
		// TODO calgamesbeginning triggerRpm = 6900.0f; //for the limit.
		// triggerRpm = 7118.0f; 11/7/2013
		triggerRpm = 7180.0f;
		frisbeeCountRpm = 6000.0f; //make sure this works! TODO TODO
		canCount = false;
		
		table = NetworkTable::GetTable("datatable");
	}

	void AutonomousInit()
	{
		compressor->Start();
		drivetrain->SetSafetyEnabled(false);

		// start & reset encoders
		right_encoder->Start();
		left_encoder->Start();

		right_encoder->Reset();
		left_encoder->Reset();

		//check digital IOs
		for (int i = 0; i < 8; i++)
		{
			digitalIOs[i] = driverStation->GetDigitalIn(i+1); // because they are labeled 1-8
			// but we don't want messy arrays and do want all numbers to match up
			//TODO fix this, there's issues with using a different computer as the driverstation
		}
		//Check how many frisbees to start. Use digitalIO # 5
		int numStartingFrisbees = 3;
		if(digitalIOs[5]) //IF dIO 5 is true, then we're in MTTD mode
			//and can start with 4 frisbees.
		{
			//printf("4 start\n");
			numStartingFrisbees = 4; 
		}
		//printf("numStart: %d\n", numStartingFrisbees);
		
#if USE_COUNTER
		testShooterCounter->Start();
#else
		blue_shooter_encoder->Start();
#endif	

#if USE_COUNTER
		testShooterCounter->Reset();
#else
		blue_shooter_encoder->Reset();
#endif
		left_encoder->Start();
		left_encoder->Reset();
		right_encoder->Start();
		right_encoder->Reset();

		drivetrain->SetSafetyEnabled(false);

		autoTimer->Start();
		autoTimer->Reset();

		//all modes: gear down
		gearUp->Set(false);
		gearDown->Set(true);

		if (digitalIOs[0])
		{
			//shoot 3 frisbees
			//is exactly the same code as autoShoot
			DeployIntake();
			DeployShooter();
			Wait(1.0);
			ShootAuto(numStartingFrisbees, bangbangValue);
			Wait(3.0);
			printf("%d\n", numStartingFrisbees);
			TurnShooterOff();
		}
		//center of pyramid
		else if (digitalIOs[1])
		{
			DeployShooter();
			DeployIntake();
			Wait(1.0);
			//with intake down
			ShootAuto(numStartingFrisbees, bangbangValue);
			UndeployIntake(); //maybe have to move earlier?
			TurnShooterOff();
			SpinAutoClock(1440);//1480
			DeployIntake();
			Wait(1.0);
			TurnIntakeHopperOn();
			DriveBackwardAuto(1150);//(550);
			//pick up
			DriveForwardAuto(1150);//(550);
			SpinAutoAnti(1560);//1590
			ShootAuto(5, bangbangValue); //should be only 2, but just in case it doesn't make it all the way up.
			TurnIntakeHopperOff();
			TurnShooterOff();
			//SpinAutoClock(1150);
		}
		//start at left corner
		else if (digitalIOs[2])
		{
			/*DeployShooter();
			Wait(1.0);
			DeployIntake();
			ShootAuto(numStartingFrisbees, bangbangValue);
			TurnIntakeHopperOn();
			//back up
			DriveBackwardAuto(3700);
			//spin, ~120 degrees
			SpinAutoAnti(820);
			DriveBackwardAuto(2800);
			SpinAutoClock(710);
			ShootAuto(5, bangbangValue);*/
			//This is LCorner, all middle. Not recently tested. Replacing with MTTD
			//blue side.
			DeployIntake();
			if(numStartingFrisbees < 4) //Don't put things down if they're already down.
			{
				DeployShooter();
				DeployIntake(); //Just in case there are 4 to start.
				Wait(1.0);
			}
			ShootAuto(numStartingFrisbees, bangbangValue);
			UndeployIntake(); 
			TurnShooterOff(); 
			SpinAutoClock(1325);
			DeployIntake();
			Wait(1.0);
			//Wait(0.5);
			TurnIntakeHopperOn();
			DriveBackwardRampDown(4100);
			//pick up
			DriveForwardAuto(3850);
			SpinAutoAnti(1812); 
			ShootAuto(5, bangbangValue); //should be only 2, but just in case it doesn't make it all the way up.
			TurnIntakeHopperOff();
			TurnShooterOff();
		}
		else if (digitalIOs[3])
		{
			DeployShooter();
			DeployIntake();
			ShootAuto(numStartingFrisbees, bangbangValue);
			printf("three shots\n");
			TurnIntakeHopperOn();
			//small backing up.
			DriveBackwardAuto(750);
			printf("clear of pyramid\n");
			//spin
			SpinAutoClock(475);//(525);

			printf("spin\n");
			DriveBackwardRampDown(3800);
			printf("catch\n");
			DriveForwardAuto(3800);
			SpinAutoAnti(495);
			DriveForwardAuto(750);
			ShootAuto(4, bangbangValue);
		}
		else if (digitalIOs[4])
		{
			//If you update this one, also update blue side [2]
			DeployIntake();
			if(numStartingFrisbees < 4) //Don't put things down if they're already down.
			{
				DeployShooter();
				DeployIntake(); //Just in case there are 4 to start.
				Wait(1.0);
			}
			ShootAuto(numStartingFrisbees, bangbangValue);
			UndeployIntake(); 
			TurnShooterOff(); 
			SpinAutoClock(1325);//1480
			DeployIntake();
			Wait(1.0);
			//Wait(0.5);
			TurnIntakeHopperOn();
			DriveBackwardRampDown(4100);//(550);
			//pick up
			DriveForwardAuto(3850);//(4000);//(550);
			SpinAutoAnti(1510);//1590 
			ShootAuto(5, bangbangValue); //should be only 2, but just in case it doesn't make it all the way up.
			TurnIntakeHopperOff();
			TurnShooterOff();
		}
	}

	void AutonomousPeriodic()
	{
		//compressor->Start();
	}

	void TeleopInit()
	{
		compressor->Start();
		drivetrain->SetSafetyEnabled(false);

		// TODO: Comment / uncomment to test avg count encoder
		// start & reset encoders
		right_encoder->Start();
		left_encoder->Start();
		right_encoder->Reset();
		left_encoder->Reset();

#if USE_COUNTER
		testShooterCounter->Start();
#else
		blue_shooter_encoder->Start();
#endif	

#if USE_COUNTER
		testShooterCounter->Reset();
#else
		blue_shooter_encoder->Reset();
#endif

		//turn all motors off
		TurnShooterOff();
		trigger->Set(0.0);
		TurnIntakeHopperOff();

		//switch to high gear for teleop driving
		gearUp->Set(true);
		gearDown->Set(false);

		//undeploy intake
		//UndeployIntake();
		
		table->PutNumber("end", 0.0);
	}

	void TeleopPeriodic()
	{
		compressor->Start();

		//set how often this loops.
		//this->SetPeriod(1.0/500.0);

		//check digital IOs
		for (int i = 0; i < 8; i++)
		{
			digitalIOs[i] = driverStation->GetDigitalIn(i+1); // because they are labeled 1-8
			// but we don't want messy arrays and do want all numbers to match up
		}

		//Reset drivetrain encoders. Not really needed ever.
		if (driverL->GetRawButton(8))
		{
			left_encoder->Reset();
			right_encoder->Reset();
		}

		//shifting and drivetrain. contained in other files
		bool shiftUp = driverL->GetRawButton(kButtonShiftUp);
		bool shiftDown = driverL->GetRawButton(kButtonShiftDown);
		runShifting(shiftUp, shiftDown, gearUp, gearDown);
		//runDrivetrain(driverR->GetY(), driverR->GetTwist(), drivetrain);
		runDrivetrain(driverL->GetY(), driverR->GetY(), drivetrain);//, driverStation->GetAnalogIn(2));

		// Toggler for shooter deploy (up and down)

		if (digitalIOs[7])
		{
			shooterDeployToggler->toggle(driverR->GetRawButton(kButtonShooterDeployToggle));
		}
		else
		{
			shooterDeployToggler->toggle(joystick3->GetRawButton(kButtonShooterDeployToggle));
		}
		if (shooterDeployToggler->isRecentlyChanged())
		{
			if (shooterDeployToggler->status())
			{
				DeployShooter();
				printf("shooterdeploy! ");
			}
			else
			{
				UndeployShooter();
				printf("shooterUNdeploy... ");
			}
		}

		if (driverL->GetRawButton(8))
		{
			TurnShooterOn();
		}

		//Put in climber deploy on a toggle
		if (digitalIOs[7])
		{
			// avoid button conflict
			climberDeployToggler->toggle(driverR->GetRawButton(kButtonClimberDeployToggleSingleMode));
		}
		else
		{
			climberDeployToggler->toggle(driverR->GetRawButton(kButtonClimberDeployToggle));
		}
		if (climberDeployToggler->isRecentlyChanged())
		{
			if (climberDeployToggler->status())
			{
				DeployClimber();
				printf("deploy");
			}
			else
			{
				UndeployClimber();
				printf("undeploy");
			}
		}

		//Run the conveyor and the intake on a toggle.
		if (digitalIOs[7])
		{
			conveyorToggler->toggle(driverR->GetRawButton(kButtonConveyor));
		}
		else
		{
			conveyorToggler->toggle(joystick3->GetRawButton(kButtonConveyor));
		}

		if (conveyorToggler->status())
		{
			TurnIntakeHopperOn();
		}
		else
		{
			TurnIntakeHopperOff();
		}

		// Trigger wheel
		if (digitalIOs[6]) // SPLIT for driver only. TODO override
		{
			if (joystick3->GetRawButton(kButtonTrigger)
					&&shooterToggler->status())
			{
				RunTriggerBangbang();
			}
			else
			{
				trigger->Set(0.0);
			}
		}
		else
		{
			if (driverR->GetRawButton(kButtonTrigger)
					&&shooterToggler->status())//, when you have time test adding that in
			{
				RunTriggerBangbang();
			}
			else
			{
				trigger->Set(0.0);
			}
		}

		// Toggle the shooter
		if (digitalIOs[7]) //for testing w/joystick3 only, set the analogin>2.5
		{
			shooterToggler->toggle(joystick3->GetRawButton(kButtonShooterToggle));
		}
		else if (digitalIOs[6])
		{
			shooterToggler->toggleOn(joystick3->GetRawButton(kButtonShooterToggle));
			shooterToggler->toggleOff(joystick3->GetRawButton(6));
		}
		else
		{
			shooterToggler->toggleOn(driverR->GetRawButton(kButtonShooterToggle));
			shooterToggler->toggleOff(joystick3->GetRawButton(kButtonShooterToggle));
		}
		if (shooterToggler->status())
		//if(joystick3->GetRawButton(kButtonShooterToggle))
		{
			//TurnShooterOn();
			//bangbang(400,400);

#if USE_COUNTER
			bangbangCounter(bangbangValue);
#else
			bangbang(bangbangValue);
#endif
		}
		else
		{
			TurnShooterOff();
		}

		// Put in toggle for the intake deploy (flip in and out.)
		if (digitalIOs[7])
		{
			intakeToggler->toggle(driverR->GetRawButton(kButtonIntakeDeployToggle));
		}
		else
		{
			intakeToggler->toggle(joystick3->GetRawButton(kButtonIntakeDeployToggle));
		}

		if (intakeToggler->isRecentlyChanged())
		{
			if (intakeToggler->status())
			{
				DeployIntake();
				intakeRoller->Set(0.0);
				printf("intakedeploy! ");
			}
			else
			{
				UndeployIntake();
				printf("intakeUNdeploy... ");
			}
		}

		//Clear the system.
		if ((digitalIOs[7] && driverR->GetRawButton(kButtonClearSystem))
				|| joystick3->GetRawButton(kButtonClearSystem))
		{
			ClearSystem();
		}
		else if ((digitalIOs[7] && driverR->GetRawButton(kButtonClearIntake))
				|| joystick3->GetRawButton(kButtonClearIntake))
		{
			ClearIntake();
		}

		//all printfs.
		//printf("8: %i, 9: %i\n", triggerProximitySensor8->Get(), triggerProximitySensor9->Get());
		printf("leftencoder: %i, rightencoder: %i\n", left_encoder->Get(),
				right_encoder->Get());
		//printf("blue: %f\n", blue_shooter_encoder->GetRate());
		printf("encoder: %f\n", (60.0f / (100.0f
				* testShooterCounter->GetPeriod())) * (75.0f / 7.0f));

		//Update the printfs on the driver station.
		driverStationLCD->UpdateLCD();
		//TimerUpdate();
		
		//NetworkTables testing
		table->PutNumber("left", right_encoder->Get());
	}

	/**
	 * Runs during test mode
	 */
	void TestInit()
	{
		left_encoder->Start();
		right_encoder->Start();
		left_encoder->Reset();
		right_encoder->Reset();

#if USE_COUNTER
		testShooterCounter->Start();
#else
		blue_shooter_encoder->Start();
#endif 

		testLoops = 0;
	}
	void TestPeriodic()
	{
#if USE_COUNTER
		printf("left: %i, right: %i, shooter: %f", left_encoder->Get(),
				right_encoder->Get(), (60.0f / (100.0f
						* testShooterCounter->GetPeriod())) * (75.0f / 7.0f));
#else
		printf("left: %i, right: %i, shooter: %f", left_encoder->Get(), right_encoder->Get(), blue_shooter_encoder->GetRate());
#endif

		/*testLoops++;
		 if(testLoops < 50)
		 {
		 drivetrain->TankDrive(1.0, -1.0);
		 }
		 else
		 {
		 drivetrain->TankDrive(0.0, 0.0);
		 }*/
		//float a = driverStation->GetAnalogIn(1)/5;
		//drivetrain->TankDrive(a, a);
		/*float a = joystick3->GetY();
		 printf("%f\n", a);
		 shooterOrange->Set(-a);
		 shooterBlue->Set(-a);
		 *///printf("left: %i, right: %i\n", left_encoder->Get(), right_encoder->Get());
		//printf("blue: %f, orange: %f\n", blue_shooter_encoder->GetRate(), orange_shooter_encoder->GetRate());
	}

	//disabled
	void DisabledInit()
	{
		table->PutNumber("end", 44.0);
		intakeToggler->disable();
		conveyorToggler->disable();
		shooterToggler->disable();
		shooterDeployToggler->disable();
		climberDeployToggler->disable();
		drivetrain->TankDrive(0.0, 0.0);
	}

	void DisabledPeriodic()
	{

	}

	//Teleop voids. Also used occasionally in autonomous.
	//Intake.
	void DeployIntake()
	{
		printf("Deploy intake!\n");

		intakeDeploy->Set(true);
		intakeUndeploy->Set(false);

		driverStationLCD->PrintfLine((DriverStationLCD::Line) 1,
				"intake deploy  ");

		TimerSetTimeout(2.0);
		TimerStart();
	}
	void UndeployIntake()
	{
		printf("Stopped intake motor!\n");
		intakeRoller->Set(0.0);

		printf("Undeploy intake!\n");
		intakeDeploy->Set(false);
		intakeUndeploy->Set(true);

		driverStationLCD->PrintfLine((DriverStationLCD::Line) 1,
				"intake undeploy");
	}
	void StartIntakeMotor(void)
	{
		printf("Start intake motor!\n");
		intakeRoller->Set(-1.0); //backwards
	}

	//Hopper.
	void DeployShooter()
	{
		shooterDeploy->Set(true);
		shooterUndeploy->Set(false);

		driverStationLCD->PrintfLine((DriverStationLCD::Line) 2,
				"hopper deploy  ");
	}
	void UndeployShooter()
	{
		shooterDeploy->Set(false);
		shooterUndeploy->Set(true);
		driverStationLCD->PrintfLine((DriverStationLCD::Line) 2,
				"hopper undeploy");
	}

	//Climber
	void DeployClimber()
	{
		climberDeploy->Set(true);
		climberUndeploy->Set(false);
		driverStationLCD->PrintfLine((DriverStationLCD::Line) 5,
				"climber deploy  ");
	}
	void UndeployClimber()
	{
		climberDeploy->Set(false);
		climberUndeploy->Set(true);
		driverStationLCD->PrintfLine((DriverStationLCD::Line) 5,
				"climber undeploy");
	}

	//Shooter wheels.
	//NOTE: TurnShooterOn is usually not used.
	void TurnShooterOn()
	{
		//shooterOrange->Set(0.7);
		//shooterOrange->Set(0.85);
		//shooterBlue->Set(0.85);
		shooterOrange->Set(1.0);
		shooterBlue->Set(1.0);
		driverStationLCD->PrintfLine((DriverStationLCD::Line) 3, "shooter on ");
	}
	void TurnShooterOff()
	{
		shooterOrange->Set(0.0);
		shooterBlue->Set(0.0);

		driverStationLCD->PrintfLine((DriverStationLCD::Line) 3, "shooter off");
	}
	// 6000? rpm
#if USE_COUNTER
	void bangbangCounter(float topRpm)
	{
		//set motors at 1.0 if under target, 0.0 if over target
		float rpm = (60.0f / (100.0f * testShooterCounter->GetPeriod()))
				* (75.0f / 7.0f);

		printf("RPM: %f\n", rpm);

		if (rpm <= topRpm)
		{
			//			printf("k");
			shooterBlue->Set(1.0);
			shooterOrange->Set(1.0);
		}
		else
		{
			//			printf("n");
			shooterBlue->Set(0.0);
			shooterOrange->Set(0.0);
		}
		driverStationLCD->PrintfLine((DriverStationLCD::Line) 3, "shooter on ");
	}

#else
	// Mutliply rates by 25/8  ?
	void bangbang(float topBlueValue)
	{
		//set motors at 1.0 if under target, 0.0 if over target
		float blueShooterEncoderValue = blue_shooter_encoder->GetRate();

		printf("Encode value: %f\n", blueShooterEncoderValue);

		//if(blueShooterEncoderValue <= topBlueValue)
		/*if(blueShooterEncoderValue >= 0-topBlueValue)
		 {
		 printf("k");
		 shooterBlue->Set(1.0);
		 shooterOrange->Set(1.0);
		 }
		 else
		 {
		 printf("n");
		 shooterBlue->Set(0.0);
		 shooterOrange->Set(0.0);
		 }*/
		driverStationLCD->PrintfLine((DriverStationLCD::Line) 3, "shooter on ");
	}

#endif

	void TurnIntakeHopperOn()
	{
		conveyor->Set(-1.0);
		//don't run intake if the tail's up. TODO
		intakeRoller->Set(-1.0);
		//intakeRoller->Set(-1.0);
		driverStationLCD->PrintfLine((DriverStationLCD::Line) 0, "conveyor on ");
	}

	void TurnIntakeHopperOff()
	{
		intakeRoller->Set(0.0);
		conveyor->Set(-0.0);
		driverStationLCD->PrintfLine((DriverStationLCD::Line) 0, "conveyor off");
	}

	//TODO is this bad practice?
	bool RunTriggerBangbang() //If we are using bangbang, run the trigger as prescribed.
	{
		//The boolean is to return when it has changed from on to off
		//as in, when a shot has completed.
		//So things have been made global.
		rpm = (60.0f / (100.0f * testShooterCounter->GetPeriod())) * (75.0f
				/ 7.0f);
		//float triggerRpm = 6750; //This is the lower limit for shooting

		bool returnMe = false;

		//Override. Hit trigger on manipulator and right joystick.
		if (joystick3->GetRawButton(kButtonTrigger)
				&& driverR->GetRawButton(kButtonTrigger) && !IsAutonomous())
		{
			printf("joystickoverdrive");
			trigger->Set(1.0);
		}
		//Delay for trigger: If rpm is too low, prevent shooting
		else if (rpm <= triggerRpm)
		{
			//printf("low");
			if (oldrpm > triggerRpm)
			{
				returnMe = true;
			} //Comment out this one for MTTDCHANGE
			trigger->Set(0.0);
		}
		else
		{
			trigger->Set(1.0);
			canCount = true;
		}
		//printf("trigger!\n");
		
		/*if(rpm < frisbeeCountRpm)
		{
			if(canCount)
			{
				returnMe = true;
			}
			canCount = false;
		} // This one is MTTDCHANGE*/

		oldrpm = rpm; //for next time.
		return returnMe;
	}

	void ClearSystem()
	{
		//DeployIntake();
		intakeRoller->Set(1.0);
		conveyor->Set(1.0);
		trigger->Set(-1.0);
		//shooterOrange->Set(-0.05);
	}

	void ClearIntake()
	{
		//DeployIntake();
		intakeRoller->Set(1.0);
	}

	//Drivetrain shifting
	void runShifting(bool shiftUpValue, bool shiftDownValue,
			Solenoid *gearUpSolenoid, Solenoid *gearDownSolenoid)
	{
		// Only shift if a shift button is pressed
		if (shiftUpValue || shiftDownValue)
		{
			if (shiftUpValue)
			{
				printf("gear up\n");
				driverStationLCD->PrintfLine((DriverStationLCD::Line) 6,
						"high gear");
				gearUpSolenoid->Set(true);
				gearDownSolenoid->Set(false);
			}
			if (shiftDownValue)
			{
				driverStationLCD->PrintfLine((DriverStationLCD::Line) 6,
						"low gear ");
				printf("gear down\n");
				gearUpSolenoid->Set(false);
				gearDownSolenoid->Set(true);
			}
		}
		driverStationLCD->UpdateLCD();
	}

	//Autonomous routines, or the building blocks of them.
	//Shoot the given number of shots at the given speed.
	void ShootAuto(int numShots, float bangbangvalue) //donot change to camelcase
	{
		printf("Shooter deployed");
		int numShot = 0; //number of frisbees that we have shot within this round.
		autoTimer->Start();
		autoTimer->Reset();
		while (numShot < numShots && IsAutonomous())
		{
			//bangbang(21739, 14285);
			//TurnShooterOn(); //375
			printf("numshot: %d, %d\n", numShot, numShots);

			TurnIntakeHopperOn();

			bool changed = RunTriggerBangbang(); //This *should* run the motors
			//as they are supposed to, and return true if it switched from off to on.

			if (changed)
			{
				numShot++;
			}
#if USE_COUNTER
			bangbangCounter(bangbangvalue);
#else
			bangbang(bangbangvalue);
#endif

			//trigger->Set(0.0); //TODO testing
		}
		trigger->Set(0.0);
		TurnIntakeHopperOff();
		TurnShooterOff();
		//Wait(0.5);
	}
	//Drive forward for (approximately) so many encoder clicks.
	void DriveForwardAuto(int numEncoderClicks)
	{
		left_encoder->Reset();
		right_encoder->Reset();
		printf("Drive Forward:\n");
		while (IsAutonomous() && (left_encoder->Get()>(0-numEncoderClicks))
				|| right_encoder->Get()<numEncoderClicks)
		{
			drivetrain->TankDrive(-1.0, -1.0);
			printf("left:%i, right:%i\n", left_encoder->Get(),
					right_encoder->Get());
		}
		drivetrain->TankDrive(0.0, 0.0);
		Wait(0.2);
	}
	//Drive backward for (approximately) so many encoder clicks.
	void DriveBackwardAuto(int numEncoderClicks)
	{
		left_encoder->Reset();
		right_encoder->Reset();
		printf("Drive Backward:\n");
		while (IsAutonomous() && (left_encoder->Get()<numEncoderClicks
				|| right_encoder->Get() > (0-numEncoderClicks)))
		{
			drivetrain->TankDrive(1.0, 1.0);
			printf("left: %i, right:%i\n", left_encoder->Get(),
					right_encoder->Get());
		}
		drivetrain->TankDrive(0.0, 0.0);
		Wait(0.2);
	}
	//Spin clockwise for (approximately) so many encoder clicks.
	void SpinAutoClock(int numEncoderClicks)
	{
		left_encoder->Reset();
		right_encoder->Reset();
		while (IsAutonomous() && (left_encoder->Get()>(0-numEncoderClicks)
				&& right_encoder->Get()>(0-numEncoderClicks)))
		{
			drivetrain->TankDrive(-1.0, 1.0);
			printf("left:%i, right:%i\n", left_encoder->Get(),
					right_encoder->Get());
		}
		drivetrain->TankDrive(0.0, 0.0);
		printf("before if: left: %i, right:%i\n", left_encoder->Get(),
				right_encoder->Get());
		if (left_encoder->Get()<= (0-numEncoderClicks) || right_encoder->Get()
				<= (0-numEncoderClicks))
		{
			drivetrain->TankDrive(0.0, 0.0);
			printf("left:%i, right:%i\n", left_encoder->Get(),
					right_encoder->Get());
		}
		Wait(0.2);
	}
	//Spin counterclockwise for (approximately) so many encoder clicks. Anti is shorter than counter.
	void SpinAutoAnti(int numEncoderClicks)
	{
		left_encoder->Reset();
		right_encoder->Reset();
		while (IsAutonomous() && (left_encoder->Get()<numEncoderClicks
				&& right_encoder->Get()<numEncoderClicks))
		{
			drivetrain->TankDrive(1.0, -1.0);
			printf("left:%i, right:%i\n", left_encoder->Get(),
					right_encoder->Get());
		}
		drivetrain->TankDrive(0.0, 0.0);
		printf("before if: left: %i, right:%i\n", left_encoder->Get(),
				right_encoder->Get());
		if (left_encoder->Get()>= numEncoderClicks || right_encoder->Get()
				>= numEncoderClicks)
		{
			drivetrain->TankDrive(0.0, 0.0);
			printf("left:%i, right:%i\n", left_encoder->Get(),
					right_encoder->Get());
		}
		Wait(0.2);
	}
	//Drive forward starting slowly for (approximately) so many encoder clicks.
	void DriveForwardRamping(int numEncoderClicks)
	{
		sendTo = 0.5;
		while (IsAutonomous() && left_encoder->Get() > (0-numEncoderClicks))
		{
			drivetrain->TankDrive(-sendTo, -sendTo);
			if (left_encoder->Get() < (0-numEncoderClicks - 100))
			{
				sendTo += 0.1;
				if (sendTo >= 1.0)
				{
					sendTo = 1.0;
				}
			}
			else
			{
				sendTo -= 0.1;
				if (sendTo <= 0.5)
				{
					sendTo = 0.5;
				}
			}
		}
		drivetrain->TankDrive(0.0, 0.0);
		Wait(0.2);
	}
	//Drive backward, slowing down at the end for (approximately) so many encoder clicks.
	void DriveBackwardRampDown(int numEncoderClicks)
	{
		left_encoder->Reset();
		right_encoder->Reset();
		printf("Drive Backward:\n");
		sendTo = 1.0;
		while (IsAutonomous() && (left_encoder->Get()<numEncoderClicks
				|| right_encoder->Get() > (0-numEncoderClicks)))
		{
			drivetrain->TankDrive(sendTo, sendTo);
			printf("left: %i, right:%i\n", left_encoder->Get(),
					right_encoder->Get());

			if (left_encoder->Get() > (numEncoderClicks-500)
					|| right_encoder->Get() < (0 - numEncoderClicks-1000))
			{
				sendTo = 0.25;
			}
			else if (left_encoder->Get() > (numEncoderClicks-1000)
					|| right_encoder->Get() < (0-(numEncoderClicks-1000)))
			{
				sendTo = 0.5;
			}
		}
		drivetrain->TankDrive(0.0, 0.0);
		Wait(0.2);
	}

	///////////////////////
	// Timer stuff
	///////////////////////


	int count;
	int timeout;
	bool isStarted;

	void TimerSetTimeout(float t)
	{
		timeout = (int)(t * 50);
		count = 0;
	}

	void TimerUpdate()
	{
		if (isStarted)
		{
			count++;
			if (count >= timeout)
			{
				StartIntakeMotor(); // stupid function pointers
				count = 0;
				isStarted = false;
			}
		}
	}

	void TimerStart()
	{
		count = 0;
		isStarted = true;
	}

};

START_ROBOT_CLASS(Robot)
;
