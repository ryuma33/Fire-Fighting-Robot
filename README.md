# Fire-Fighting Robot

An ESP32-based robot that detects fire with three flame sensors, drives toward it, and puts it out with a water pump and a sweeping servo nozzle.

## How it works

1. Three flame sensors (left, middle, right) are read continuously.
2. When any sensor sees fire, the robot turns or drives toward it:
   - **Middle only:** moves forward
   - **Left / right only:** backs up a little, then turns toward that side
   - **Middle + left / middle + right:** backs up, then makes a slight turn toward that side
3. The pump turns on and the servo sweeps the nozzle from 45° to 135° and back.
   The sensors are checked at every servo step, so spraying stops as soon as the fire is out.
4. The robot backs up a little, and the cycle repeats while fire is still visible.

## Hardware

| Part | Notes |
|---|---|
| ESP32 dev board | |
| L298N (or similar) motor driver + 2 DC motors | |
| 3 × flame sensor modules (analog output) | |
| Relay module + small water pump | |
| Servo motor (SG90 or similar) | Points the water nozzle |
| Battery pack | Power the motors/pump separately from the ESP32's USB supply |

## Wiring

| ESP32 pin | Connects to |
|---|---|
| GPIO 27 | Motor driver IN1 |
| GPIO 26 | Motor driver IN2 |
| GPIO 25 | Motor driver IN3 |
| GPIO 33 | Motor driver IN4 |
| GPIO 14 | Motor driver ENA (PWM) |
| GPIO 32 | Motor driver ENB (PWM) |
| GPIO 4  | Pump relay signal |
| GPIO 13 | Servo signal |
| GPIO 34 | Left flame sensor (AO) |
| GPIO 39 | Middle flame sensor (AO) |
| GPIO 35 | Right flame sensor (AO) |
| GPIO 2  | On-board LED (fire indicator) |

Connect all grounds together (ESP32, motor driver, relay, servo, sensors).

> The pump relay uses GPIO 4, not GPIO 12. GPIO 12 is a boot strapping pin, and a relay that pulls it high can stop the ESP32 from booting.

## Software setup

1. Install the [Arduino IDE](https://www.arduino.cc/en/software).
2. Add ESP32 board support: **Boards Manager → search "esp32" → install "esp32 by Espressif Systems"**.
3. Install the **ESP32Servo** library: **Library Manager → search "ESP32Servo"**.
4. Select your board (for example **ESP32 Dev Module**) and port.

## Sketches

| Folder | Purpose |
|---|---|
| `Fire_Fighter_Robot/` | Main robot program |
| `Motor_Test_Code/` | Runs both motors forward for 3 s, stops for 2 s, repeats. Use it to check motor wiring and direction. |

Upload `Motor_Test_Code` first to make sure both wheels spin forward. If a wheel spins the wrong way, swap that motor's two wires on the driver. Then upload `Fire_Fighter_Robot`.

## Tuning

Change these values at the top of `Fire_Fighter_Robot.ino`:

| Setting | Default | Meaning |
|---|---|---|
| `fireThreshold` | `500` | A sensor reading below this counts as fire (the ESP32 ADC reads 0–4095). Check the real values in the Serial Monitor and adjust. |
| `motorSpeed` | `200` | Motor PWM speed, 0–255 |
| `move_forward`, `turning_time`, `move_backward` | `300`, `300`, `150` ms | How long each movement lasts |
| `servo_step_delay` | `150` ms | Time between servo steps while spraying |
