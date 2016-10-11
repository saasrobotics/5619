package org.firstinspires.ftc.teamcode.TankArcadeDrive;

import com.qualcomm.robotcore.eventloop.opmode.Disabled;
import com.qualcomm.robotcore.eventloop.opmode.TeleOp;

import org.firstinspires.ftc.teamcode.TankArcadeDrive.DriveOPBase;
import org.redshiftrobotics.config.Config;
import org.redshiftrobotics.config.ConfigBase;

/**
 * Created by Eric Golde on 9/24/2016.
 */

//drive config
@TeleOp(name="Drive Config", group="Drive")
@Disabled
public class DriveConfigFile extends ConfigBase {

    public static Config config = new Config();

    @Override
    public void init() {
        // We must initialize each of the op modes that uses variables that this
        // mode configures. Calling this will add the appropriate variables to "config"
        // declared above.
        DriveOPBase.initMe(); //call initMe cause static { } doesn't get called so this is a workaround

        super.config = config;
        super.init(); //call the init in super
    }

    @Override
    public void loop() {
        super.loop(); //pass in loop
    }
}

