package com.qualcomm.ftcrobotcontroller.opmodes;
import com.qualcomm.ftcrobotcontroller.R;
import com.qualcomm.robotcore.eventloop.opmode.OpMode;
import com.qualcomm.robotcore.eventloop.opmode.OpMode;
import com.qualcomm.robotcore.hardware.DcMotor;
import com.qualcomm.robotcore.hardware.Servo;
import com.qualcomm.robotcore.util.Range;

import android.widget.TextView;
import android.widget.Toast;



//Rember to register opmode in FtcOpModeRegister.java !
public class EDebug extends OpMode {

    Servo servo1;

    String teleConvert;
    int teleInt = 3;

    public void init() {





    }

    public void dt(String text)
    {
        //make a new line
        teleInt++;
        //convert to string
        teleConvert = Integer.toString(teleInt);
        //print to console new line
        telemetry.addData(teleConvert, text);
    }

    @Override
    public void loop() {



    }






}

