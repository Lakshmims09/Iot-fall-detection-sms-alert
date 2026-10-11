# IoT-Based Fall Detection and SMS Alert System

## Project Overview
An IoT-based fall detection prototype developed using NodeMCU ESP8266 and MPU6050. The system monitors acceleration and angular velocity to identify possible falls and automatically triggers an SMS notification through the Twilio API.

## Technologies Used
- **Hardware:** NodeMCU ESP8266, MPU6050
- **Programming:** Arduino C/C++
- **Communication:** Wi-Fi, I2C, HTTPS
- **Cloud API:** Twilio SMS API
- **Development Environment:** Arduino IDE

## Key Features
- Real-time acceleration and gyroscope monitoring
- Free-fall detection
- Impact and sudden rotation detection
- Post-fall inactivity check
- Automatic SMS triggering through Twilio
- SMS cooldown to reduce repeated notifications

## Hardware Connections

| MPU6050 | NodeMCU ESP8266 |
|---|---|
| VCC | 3V3 |
| GND | GND |
| SDA | D2 (GPIO4) |
| SCL | D1 (GPIO5) |

## Working Principle
1. The MPU6050 continuously measures acceleration and angular velocity.
2. The algorithm identifies a possible free-fall event.
3. It checks for impact and sudden rotation.
4. After three seconds, it checks whether the sensor is stationary.
5. If the conditions are satisfied, a fall event is confirmed.
6. The ESP8266 connects to Twilio through Wi-Fi and requests an SMS notification.

## Demonstration Results
The prototype successfully demonstrated sensor-based fall detection and automatic SMS triggering.

**Note:** The Twilio free-trial account uses a predefined SMS template rather than a customized emergency alert.

## Limitations
- Requires an active Wi-Fi connection.
- Fall detection thresholds require further testing and calibration.
- Inactivity is checked using a single sensor reading.
- The prototype is intended for academic demonstration and is not a certified emergency-response device.

## Future Enhancements
- Custom emergency SMS alerts
- GPS location tracking
- Improved fall detection accuracy
- Battery-powered wearable design
- Caregiver notification dashboard

## Author
Developed as an IoT-based academic project.


---

## Project Demonstration and Results

The following screenshots demonstrate the working
of the IoT-Based Fall Detection and SMS Alert System.

### 1. Hardware Setup

NodeMCU ESP8266 connected to the MPU6050
accelerometer and gyroscope sensor.

![Hardware Setup](image/Hardware%20connection.jpeg)

### 2. Code Compilation

Successful compilation of the Arduino program
for the NodeMCU ESP8266.

![Code Compilation](image/Compilation%20of%20code.png)

### 3. Normal Condition

The MPU6050 continuously monitors acceleration
and angular velocity during normal conditions.

![Normal Condition](image/Normal%20condition.png)

### 4. Free Fall and Impact Detection

The system detects a possible free-fall event
followed by a sudden impact.

![Impact Detection](image/Impact%20condition.png)

### 5. Rotation and Inactivity Detection

The system identifies sudden rotation and
checks whether the sensor becomes stationary.

![Inactivity Detection](image/Inactivity%20condition.png)

### 6. Confirmed Fall

After the fall detection conditions are satisfied,
the system displays CONFIRMED FALL and initiates
the SMS notification process.

![Fall Detection](image/Fall%20detected.png)

### 7. Twilio SMS Integration

The ESP8266 sends an HTTPS request to Twilio.
HTTP 201 indicates that Twilio accepted the SMS request.

![Twilio SMS Success](image/connected%20to%20twilio%20server.png)

### 8. SMS Received on Mobile Phone

The demonstration shows an SMS notification
received on the mobile phone.

![SMS Received](image/Message%20sent%20to%20phone.jpeg)

---

## Project Outcome

The prototype successfully demonstrated:

- Real-time motion monitoring using MPU6050
- Free-fall and impact detection
- Rotation and inactivity checking
- Automatic SMS triggering through the Twilio API
- Successful SMS delivery to a mobile phone

**Note:** This is an academic prototype.
Fall detection accuracy has not been clinically
validated, and reliable emergency use would require
additional testing and safeguards.


