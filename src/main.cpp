#include <Arduino.h>
#include <shiftreg.h>
#include <gyroscope.h>
#include <storage.h>
#include <temperature.h>
#include <SerialBT.h>

void setup()
{
    Serial.begin();
    delay(5000);
    Serial.println("Starting bluetooth");

    SerialBT.setName("AF-23");
    SerialBT.setTimeout(50);
    SerialBT.begin();

    Serial.println("Bluetooth started, waiting for connection. Starting modules.");

    // Initialize modules
    shiftreg::init();
    //gyroscope::init();
    //storage::init();
    temperature::init();

    pinMode(16, INPUT);

    //pinMode(10, OUTPUT);
    //analogWrite(10, 255);

    Serial.println("Module setup finished");
}

void loop()
{
    if (SerialBT.availableForWrite())
    {
        while (SerialBT.available()) {
            String commandPart1 = SerialBT.readStringUntil(';');
            if (commandPart1 == "MC")
            {
                Serial.println("Command MC..");

                String commandPart2 = SerialBT.readStringUntil(';');

                // SET_PWR command
                if (commandPart2 == "SET_PWR")
                {
                    Serial.println("Command SET_PWR..");
                    String commandPart3 = SerialBT.readString();

                    if (commandPart3 == "0\r\n" || commandPart3 == "1\r\n")
                    {
                        shiftreg::set_motor_pwr((bool) commandPart3.toInt());
                        Serial.print("MOTOR_PWR set to ");
                        Serial.print(commandPart3);
                        SerialBT.print("MOTOR_PWR set to ");
                        SerialBT.print(commandPart3);
                    }
                    else
                    {
                        Serial.println("Invalid commandPart3!");
                        SerialBT.println("Invalid commandPart3!");
                    }
                }
                // SET_FOOT_SW command
                else if (commandPart2 == "SET_FOOT_SW")
                {
                    Serial.println("Command SET_FOOT_SW..");
                    String commandPart3 = SerialBT.readString();
                    
                    if (commandPart3 == "0\r\n" || commandPart3 == "1\r\n")
                    {
                        shiftreg::set_motor_foot_sw((bool) commandPart3.toInt());
                        Serial.print("FOOT_SW set to ");
                        Serial.print(commandPart3);
                        SerialBT.print("FOOT_SW set to ");
                        SerialBT.print(commandPart3);
                    }
                    else
                    {
                        Serial.println("Invalid commandPart3!");
                        SerialBT.println("Invalid commandPart3!");
                    }
                }
                // SET_BRAKE_SW command
                else if (commandPart2 == "SET_BRAKE_SW")
                {
                    Serial.println("Command SET_BRAKE_SW..");
                    String commandPart3 = SerialBT.readString();
                    
                    if (commandPart3 == "0\r\n" || commandPart3 == "1\r\n")
                    {
                        shiftreg::set_motor_brake_sw((bool) commandPart3.toInt());
                        Serial.print("BRAKE_SW set to ");
                        Serial.print(commandPart3);
                        SerialBT.print("BRAKE_SW set to ");
                        SerialBT.print(commandPart3);
                    }
                    else
                    {
                        Serial.println("Invalid commandPart3!");
                        SerialBT.println("Invalid commandPart3!");
                    }
                }
                // SET_REVERSE command
                else if (commandPart2 == "SET_REVERSE")
                {
                    Serial.println("Command SET_REVERSE..");
                    String commandPart3 = SerialBT.readString();
                    
                    if (commandPart3 == "0\r\n" || commandPart3 == "1\r\n")
                    {
                        shiftreg::set_motor_reverse((bool) commandPart3.toInt());
                        Serial.print("REVERSE set to ");
                        Serial.print(commandPart3);
                        SerialBT.print("REVERSE set to ");
                        SerialBT.print(commandPart3);
                    }
                    else
                    {
                        Serial.println("Invalid commandPart3!");
                        SerialBT.println("Invalid commandPart3!");
                    }
                }
                // SET_LO_SPEED command
                else if (commandPart2 == "SET_LO_SPEED")
                {
                    Serial.println("Command SET_LO_SPEED..");
                    String commandPart3 = SerialBT.readString();
                    
                    if (commandPart3 == "0\r\n" || commandPart3 == "1\r\n")
                    {
                        shiftreg::set_motor_lo_speed((bool) commandPart3.toInt());
                        Serial.print("LO_SPEED set to ");
                        Serial.print(commandPart3);
                        SerialBT.print("LO_SPEED set to ");
                        SerialBT.print(commandPart3);
                    }
                    else
                    {
                        Serial.println("Invalid commandPart3!");
                        SerialBT.println("Invalid commandPart3!");
                    }
                }
                // SET_HI_SPEED command
                else if (commandPart2 == "SET_HI_SPEED")
                {
                    Serial.println("Command SET_HI_SPEED..");
                    String commandPart3 = SerialBT.readString();
                    
                    if (commandPart3 == "0\r\n" || commandPart3 == "1\r\n")
                    {
                        shiftreg::set_motor_hi_speed((bool) commandPart3.toInt());
                        Serial.print("HI_SPEED set to ");
                        Serial.print(commandPart3);
                        SerialBT.print("HI_SPEED set to ");
                        SerialBT.print(commandPart3);
                    }
                    else
                    {
                        Serial.println("Invalid commandPart3!");
                        SerialBT.println("Invalid commandPart3!");
                    }
                }
                // SET_NC0 command
                else if (commandPart2 == "SET_NC0")
                {
                    Serial.println("Command SET_NC0..");
                    String commandPart3 = SerialBT.readString();
                    
                    if (commandPart3 == "0\r\n" || commandPart3 == "1\r\n")
                    {
                        shiftreg::set_nc0((bool) commandPart3.toInt());
                        Serial.print("NC0 set to ");
                        Serial.print(commandPart3);
                        SerialBT.print("NC0 set to ");
                        SerialBT.print(commandPart3);
                    }
                    else
                    {
                        Serial.println("Invalid commandPart3!");
                        SerialBT.println("Invalid commandPart3!");
                    }
                }
                // SET_NC1 command
                else if (commandPart2 == "SET_NC1")
                {
                    Serial.println("Command SET_NC1..");
                    String commandPart3 = SerialBT.readString();
                    
                    if (commandPart3 == "0\r\n" || commandPart3 == "1\r\n")
                    {
                        shiftreg::set_nc1((bool) commandPart3.toInt());
                        Serial.print("NC1 set to ");
                        Serial.print(commandPart3);
                        SerialBT.print("NC1 set to ");
                        SerialBT.print(commandPart3);
                    }
                    else
                    {
                        Serial.println("Invalid commandPart3!");
                        SerialBT.println("Invalid commandPart3!");
                    }
                }
                // SET_ANALOG_BRAKE command
                else if (commandPart2 == "SET_ANALOG_BRAKE")
                {
                    Serial.println("Command SET_ANALOG_BRAKE..");
                    String commandPart3 = SerialBT.readString();
                    
                    int newBrakeVal = commandPart3.toInt();

                    if (newBrakeVal < 0 || newBrakeVal > 255)
                    {
                        Serial.println("Invalid commandPart3!");
                        SerialBT.println("Invalid commandPart3!");
                    }
                    else
                    {
                        // TODO: Set analog brake correctly
                        Serial.print("ANALOG_BRAKE set to ");
                        Serial.print(commandPart3);
                        SerialBT.print("ANALOG_BRAKE set to ");
                        SerialBT.print(commandPart3);
                    }
                }
                // GET_HALL command
                else if (commandPart2 == "GET_HALL")
                {
                    Serial.println("Command GET_HALL..");

                    // Read out the line ending of the command
                    SerialBT.readString();

                    // TODO: Read the hall sensor correctly
                    bool hallValue = digitalRead(3);

                    Serial.print("Returning value: ");
                    Serial.println(hallValue);
                    SerialBT.println(hallValue);
                }
                // GET_ANALOG_THROTTLE command
                else if (commandPart2 == "GET_ANALOG_THROTTLE")
                {
                    Serial.println("Command GET_ANALOG_THROTTLE..");

                    // Read out the line ending of the command
                    SerialBT.readString();

                    // TODO: Read the hall sensor correctly
                    int throttleValue = analogRead(27);

                    Serial.print("Returning value: ");
                    Serial.println(throttleValue);
                    SerialBT.println(throttleValue);
                }
                else
                {
                    Serial.println("Invalid commandPart2!");
                    SerialBT.println("Invalid commandPart2!");
                }
            }
            else if (commandPart1 == "TEMP")
            {
                Serial.println("Command TEMP..");

                String commandPart2 = SerialBT.readString();

                if (commandPart2 == "GET_TEMP\r\n")
                {
                    Serial.println("Command GET_TEMP..");

                    float temp = temperature::getTemperature();

                    Serial.print("Returning temperature: ");
                    Serial.print(temp);
                    Serial.println(" °C");

                    SerialBT.println(temp);
                }
                else
                {
                    Serial.println("Invalid commandPart2!");
                    SerialBT.println("Invalid commandPart2!");
                }

            }
            else
            {
                Serial.println("Invalid commandPart1!");
                SerialBT.println("Invalid commandPart1!");
            }
        }
    }
    else
    {
        Serial.println("Not connected!");
    }

    //temperature::loop();

    //Serial.println(digitalRead(16));

    //gyroscope::loop();

    //shiftreg::set_motor_pwr(true);
    //shiftreg::set_motor_foot_sw(true);
    //shiftreg::set_motor_brake_sw(true);
    //shiftreg::set_motor_reverse(true);
    //shiftreg::set_motor_lo_speed(true);
    //shiftreg::set_motor_hi_speed(true);
    //shiftreg::set_nc0(true);
    //shiftreg::set_nc1(true);

    delay(20);
}