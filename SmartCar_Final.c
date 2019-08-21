#include <Servo.h>
#include "Pitches.h"

// debug ���� ����
//#define DEBUG               // DEBUG��� �����ÿ��� �� ���� �ּ�ó��, �ø��� ����ʹ� "�� ��" ����
unsigned long debug_time;
boolean debug_driving_status = false;

// Pin map
#define LIGHT    13  // Light ���� ��
#define LED      13  // LED ���� ��
#define M_PWM    5   // DC���� PWM ��
#define M_DIR1   7   // DC���� DIR1 ��
#define M_DIR2   8   // DC���� DIR2 ��

#define M2_PWM    6   // DC����2 PWM ��
#define M2_DIR1   11   // DC����2 DIR1 ��
#define M2_DIR2   12   // DC����2 DIR2 ��

#define SERVO    9   // �������� ��
#define BUZZER   3   // ���� ��
#define BATTERY  A0  // ���͸� üũ ��

#define FC_TRIG 13   // ���� ������ ���� TRIG ��
#define FC_ECHO 10  // ���� ������ ���� ECHO ��
#define FL_TRIG A4  // �������� ������ ���� TRIG ��
#define FL_ECHO A3  // �������� ������ ���� ECHO ��
#define FR_TRIG 3   // ������� ������ ���� TRIG ��
#define FR_ECHO 4   // ������� ������ ���� ECHO ��
#define L_TRIG  A2  // ���� ������ ���� TRIG ��
#define L_ECHO  A1  // ���� ������ ���� ECHO ��
#define R_TRIG  2   // ���� ������ ���� TRIG ��
#define R_ECHO  A5  // ���� ������ ���� ECHO ��

#define MAX_DISTANCE  2000 // ������ ������ �ִ� �����Ÿ�

// �ڵ��� Ʃ�� �Ķ����
int servo_dir = 1; // ���� ȸ�� ����(����: 1, �ݴ�:-1)
int motor_dir = 1; // ���� ȸ�� ����(����:1, �ݴ�:-1)
int angle_limit = 50; // ���� ���� ȸ�� ���� �� (����: ��)
int angle_offset = 0; // ���� ���� �߾Ӱ� ������ (����: ��)
int max_rc_pwm = 255; // RC���� ���� �ִ� ��� (0 ~ 255)
int min_rc_pwm = 110; // RC���� ���� �ּ� ��� (0 ~ 255)
int punch_pwm = 200; // ���� ������ �غ� ��� (0 ~ 255)
int punch_time = 50; // ���� ������ �غ� �ð� (���� msec)
int stop_time = 30; // �������� ��ȯ �ð� (���� msec)
int melody_tempo = 3500; // ��ε� ���� �ӵ�
int melody_num = 41; // ��ε� �� ����
int battery_cell = 2; // ���͸� �� ����
float voltage_error = 1.08; // ���� ���� (1�� ���� ����)
// �������� Ʃ�� �Ķ����
//int max_ai_pwm = 200; // �������� ���� �ִ� ��� (0 ~ 255)
int max_ai_pwm = 200; // �������� ���� �ִ� ��� (0 ~ 255)
int min_ai_pwm = 80; // �������� ���� �ּ� ��� (0 ~ 255)
int center_detect = 200; // ���� ���� �Ÿ� (����: mm)
int center_start = 160; // ���� ��� �Ÿ� (����: mm)
//int center_stop = 50; // ���� ���� �Ÿ� (����: mm)
int center_stop = 70; // ���� ���� �Ÿ� (����: mm)
int diagonal_detect = 80; // �밢 ���� �Ÿ� (����: mm)
int diagonal_start = 120; // �밢 ��� �Ÿ� (����: mm)
int diagonal_stop = 65; // �밢 ���� �Ÿ� (����: mm)
int side_detect = 250; // �¿� ���� �Ÿ� (����: mm)
int side_start = 160; // �¿� ���� ���� �Ÿ� (����: mm)
int side_stop = 50; // �¿� ���� �� �Ÿ� (����: mm)
float steering_gain = 1.5; // �¿� ���� �������


// ��ε� ����
int melody[] = {
    NOTE_E5, NOTE_DS5, NOTE_E5, NOTE_DS5, NOTE_E5, NOTE_B4, NOTE_D5, NOTE_C5, NOTE_A4, 0,
    NOTE_C4, NOTE_E4, NOTE_A4, NOTE_B4, 0, NOTE_E4, NOTE_GS4, NOTE_B4, NOTE_C5, 0,
    NOTE_E4, NOTE_E5, NOTE_DS5, NOTE_E5, NOTE_DS5, NOTE_E5, NOTE_B4, NOTE_D5, NOTE_C5, NOTE_A4, 0,
    NOTE_C4, NOTE_E4, NOTE_A4, NOTE_B4, 0, NOTE_E4, NOTE_C5, NOTE_B4, NOTE_A4, 0
};
// ��ε� �� ������
int duration[] = {
    16, 16, 16, 16, 16, 16, 16, 16, 8, 16,
    16, 16, 16, 8, 16, 16, 16, 16, 8, 16,
    16, 16, 16, 16, 16, 16, 16, 16, 16, 8, 16,
    16, 16, 16, 8, 16, 16, 16, 16, 4, 16
};

Servo servo;
float cur_steering;
float cur_speed;
float max_pwm;
float min_pwm;
bool sound = false;
bool autoDriving = false;
int melody_index = 0;
unsigned long melody_time;
unsigned long battery_time;
unsigned long rc_time;
float f_center;
float f_left;
float f_right;
float left;
float right;
float b_center;

// ���� ������ �Ÿ� ��, ���� ó���� ���
//float prev_f_center = 0;
//float prev_f_left = 0;
//float prev_f_right = 0;
//float prev_left = 0;
//float prev_right = 0;


// �չ��� ����
void SetSteering(float steering)
{
    cur_steering = constrain(steering, -1, 1);
    
    float angle = cur_steering * angle_limit;
    if(servo_dir < 0)
        angle = -angle;
    
    int servoAngle = angle + 90;
    if(servo_dir < 0)
        servoAngle -= angle_offset;
    else
        servoAngle += angle_offset;
    servoAngle = constrain(servoAngle, 0, 180);
    
    servo.write(servoAngle);
}

// �޹��� ����ȸ��
void SetSpeed(float speed)
{
    
    speed = constrain(speed, -1, 1);

    if((cur_speed * speed < 0) // �����̴� �� �ݴ� ���� ����̰ų�
        || (cur_speed != 0 && speed == 0)) // �����̴ٰ� �������
    {
        cur_speed = 0;
        digitalWrite(M_PWM, HIGH);
        digitalWrite(M_DIR1, LOW);
        digitalWrite(M_DIR2, LOW);

        digitalWrite(M2_PWM, HIGH);
        digitalWrite(M2_DIR1, LOW);
        digitalWrite(M2_DIR2, LOW);

        
        if(stop_time > 0) 
          delay(stop_time);
    }

    if(cur_speed == 0 && speed != 0) // �������¿��� ����̶��
    {
        if(punch_time > 0)
        {
            if(speed * motor_dir > 0)
            {
                analogWrite(M_PWM, punch_pwm);
                digitalWrite(M_DIR1, HIGH);
                digitalWrite(M_DIR2, LOW);

                analogWrite(M2_PWM, punch_pwm);
                digitalWrite(M2_DIR1, HIGH);
                digitalWrite(M2_DIR2, LOW);
            }
            else if(speed * motor_dir < 0)
            {
                analogWrite(M_PWM, punch_pwm);
                digitalWrite(M_DIR1, LOW);
                digitalWrite(M_DIR2, HIGH);

                analogWrite(M2_PWM, punch_pwm);
                digitalWrite(M2_DIR1, LOW);
                digitalWrite(M2_DIR2, HIGH);
            }
            
            delay(punch_time);
        }
    }

    if(speed != 0) // ����� ������ �ƴ϶��
    {
        int pwm = abs(speed) * (max_pwm - min_pwm) + min_pwm;           // 0 ~ 255�� ��ȯ
        if(speed * motor_dir > 0)
        {
            analogWrite(M_PWM, pwm);
            digitalWrite(M_DIR1, HIGH);
            digitalWrite(M_DIR2, LOW);

            analogWrite(M2_PWM, pwm);
            digitalWrite(M2_DIR1, HIGH);
            digitalWrite(M2_DIR2, LOW);
        }
        else if(speed * motor_dir < 0)
        {
            analogWrite(M_PWM, pwm);
            digitalWrite(M_DIR1, LOW);
            digitalWrite(M_DIR2, HIGH);

            analogWrite(M2_PWM, pwm);
            digitalWrite(M2_DIR1, LOW);
            digitalWrite(M2_DIR2, HIGH);
        }
    }

    cur_speed = speed;
}

// ����Ʈ �ѱ�
void LightON()
{
    digitalWrite(LIGHT, HIGH);
}

// ����Ʈ ����
void LightOFF()
{
    digitalWrite(LIGHT, LOW);
}

// ��ε� ����
void StartMelody()
{
    sound = true;
    melody_index = 0;
    melody_time = millis();
    tone(BUZZER, melody[melody_index]);
}

// ��ε� ����
void StopMelody()
{
    sound = false;
    noTone(BUZZER);
}

// ��ε�
void PlayMelody()
{
    if(sound == false)
        return;
    
    unsigned long t = millis();
    if(t - melody_time >= (melody_tempo / duration[melody_index]))
    {
        melody_index++;
        if(melody_index >= melody_num)
            melody_index = 0;
        
        if(melody[melody_index] == 0)
            noTone(BUZZER);
        else
            tone(BUZZER, melody[melody_index]);
        melody_time = t;
    }
}

// ���͸� üũ, �����н� �����
void CheckBattery()
{
    
    // 1�ʿ� �ѹ��� üũ
    if(millis() - battery_time < 1000)
        return;
  
    float voltage = (float)analogRead(BATTERY) / 1023;      
    voltage *= 5;                                           // �Ƶ��̳� 5V
    voltage *= 3;                                           // ȸ�λ� ���кй� ���� (VIN�� 10K, 20K �������� �й�)
    voltage *= voltage_error;

    if(voltage < (2.8 * battery_cell))
    {
//        while(true)
//        {
//            tone(BUZZER, NOTE_C5);
//            digitalWrite(LED, HIGH);
//            delay(1000);
//            
//            noTone(BUZZER);
//            digitalWrite(LED, LOW);
//            delay(1000);      
//        }
    }
    else
    {
        #ifndef DEBUG
        Serial.print("B");
        Serial.print(battery_cell);
        Serial.print(":");
        Serial.println(voltage);
        #endif
    }
    
    battery_time = millis();
    rc_time = millis();
        
}

// ������ �Ÿ�����
float GetDistance(int trig, int echo)
{ 
    digitalWrite(trig, LOW);
    delayMicroseconds(4);
    digitalWrite(trig, HIGH);
    delayMicroseconds(20);
    digitalWrite(trig, LOW);
    
    unsigned long duration = pulseIn(echo, HIGH, 5000);
    if(duration == 0)
        return MAX_DISTANCE;
    else
        return duration * 0.17;     // ���� 340m/s
}

// �������� ����
void StartAutoDriving()
{
    autoDriving = true;
    max_pwm = max_ai_pwm;
    min_pwm = min_ai_pwm;
    LightOFF();
    StopMelody();
    
    SetSteering(0);
    SetSpeed(0);
}

// �������� ����
void StopAutoDriving()
{
    autoDriving = false;
    max_pwm = max_rc_pwm;
    min_pwm = min_rc_pwm;
    SetSteering(0);
    SetSpeed(0);
}

// ��������
void AutoDriving()
{
    if(!autoDriving)
        return;

    f_center = 0;
    f_left = 0;
    f_right = 0;
    left = 0;
    right = 0;
    
    // �Ǵ�
    // �Ǵ� �ٰŰ� ���ٸ� ���� ���¿� �����ϰ� ����
    float compute_speed = cur_speed;
    float compute_steering = cur_steering;
    
    if(cur_speed == 0) // ���� ���� ���� �����̸�
    {
        f_center = GetDistance(FC_TRIG, FC_ECHO);
        
        if(f_center > center_start) // ���濡 �����Ǵ� ���� ����� �ִٸ�
        {
            // ����Ѵ�
            compute_speed = 0.1;
            compute_steering = 0;
        }
    }
    else if(cur_speed > 0) // ���� ���� ���� �����̸�
    {
        f_center = GetDistance(FC_TRIG, FC_ECHO);
        f_left = GetDistance(FL_TRIG, FL_ECHO);
        f_right = GetDistance(FR_TRIG, FR_ECHO);
        if(f_center <= center_stop || f_left <= diagonal_stop || f_right <= diagonal_stop)  // ���濡 �����Ǹ�
        {

          #ifdef DEBUG
            // �ѹ��� üũ        
//            if(millis() - debug_time < 500)
//                return;

            //debug_driving_status = false;       // �Ͻ� ���� ���
            
            Serial.println("R");

            Serial.print("f_center:");Serial.println(f_center);
            Serial.print("center_stop:");Serial.println(center_stop);
            Serial.print("f_left:");Serial.println(f_left);
            Serial.print("diagonal_stop:");Serial.println(diagonal_stop);
            Serial.print("f_right:");Serial.println(f_right);
            Serial.print("diagonal_stop:");Serial.println(diagonal_stop);

            
            Serial.print(f_center);Serial.print("\t");
            Serial.print(f_left);Serial.print("\t");
            Serial.print(f_right);Serial.print("\t");
            Serial.print(left);Serial.print("\t");
            Serial.print(right);Serial.print("\t");
            Serial.print(compute_steering);Serial.print("\t");
            Serial.print(compute_speed);Serial.println();
            //Serial.println("-----------------------------");   
            debug_time = millis(); 
        #endif
        
            // �����Ѵ�
            compute_speed = -0.1;
            // �Ʒ� ���� �ҽ� �ּ�ó��
//            if(cur_steering > 0)
//                compute_steering = -1;
//            else
//                compute_steering = 1;

            if (f_left > f_right) 
            {
                compute_steering = 1;
            }
            else 
            {
                compute_steering = -1;
            }
            
        }
        else if(f_left <= diagonal_detect || f_right <= diagonal_detect) // �¿����� ��� ���̶� �����ȴٸ�
        {
//            if(f_center > center_detect) // ������ �������� �ʴ´ٸ�
//            {
//                #ifdef DEBUG
//                    Serial.println("FRONT NO DETECT");
//                #endif
//                // �¿����� �� �� ���� �����Ǿ���
//                if(f_left < f_right) // �������� �����Ǿ��ٸ�
//                {
//                    // �������� �ִ� ����
//                    compute_steering = 1;
//                }
//                else if(f_right < f_left) // �������� �����Ǿ��ٸ�
//                {
//                    // �������� �ִ� ����
//                    compute_steering = -1;
//                }
//            }
        }
        else
        {
            // ����� �¿� ���������� �����Ѵ�
            // �ӵ� ����
            if(f_center <= center_detect) // ���濡 �����ȴٸ�
            {
                // �Ÿ��� ���� �ӵ� ����
                compute_speed = (float)(f_center - center_stop) / (float)(center_detect - center_stop);
            }
            else // ���濡 ���ٸ�
            {
                // �ְ� �ӵ�
                compute_speed = 1;
            }
        
            // ���� ����
            left = GetDistance(L_TRIG, L_ECHO);
            right = GetDistance(R_TRIG, R_ECHO);
            if(left <= side_start && right <= side_start) // �¿� ��� �����Ǹ�
            {
                // �Ÿ����� ���� �����Ѵ�
                float diff = (float)(right - side_stop) - (float)(left - side_stop);
                diff /= (float)(side_start - side_stop);
                // ����� Gain�� �����Ͽ� �������� ���� �� �ִ�
                compute_steering = diff * steering_gain;
            }
            else if(f_center <= center_detect && (left > side_detect || right > side_detect))
            // ������ �����Ǵµ� ��� �����̶� �������� �ʴ´ٸ�
            {
                if(left <= side_detect) // ������ �����ȴٸ�
                {
                    // �������� �ִ� ����
                    compute_steering = 1;
                }
                else if(right <= side_detect) // ������ �����ȴٸ�
                {
                    // �������� �ִ� ����
                    compute_steering = -1;
                }
            }
        }
    }
    else
    {
        // ���� ���� ���� �����̸�
        f_center = GetDistance(FC_TRIG, FC_ECHO);
        f_right = GetDistance(FR_TRIG, FR_ECHO);
        f_left = GetDistance(FL_TRIG, FL_ECHO);
        if(f_center > center_start && f_left > diagonal_start && f_right > diagonal_start) // ���濡 �������� ������
        {
            // ������ �ݴ�� ���� ȸ�� ����
            compute_speed = 0.1;
            if(cur_steering > 0) // �� ���� ���̸�
                servo.write(90);
            else // �� ���� ���̸�
                servo.write(90);
        }
    }


    #ifdef DEBUG
        if (debug_driving_status) {     // ���� ���
            // ����
            SetSteering(compute_steering);
            SetSpeed(compute_speed);
        }
        else {                          // �Ͻ����� ���, ���� ������ �ø��� ����ͷ� ������.
            SetSteering(0);
            SetSpeed(0);
    
            if(millis() - debug_time < 3000)
                return;            
            Serial.println("PAUSE");
            // ��� ������ ������ ���� �����´�.
            f_center = GetDistance(FC_TRIG, FC_ECHO);
            f_right = GetDistance(FR_TRIG, FR_ECHO);
            f_left = GetDistance(FL_TRIG, FL_ECHO);
            left = GetDistance(L_TRIG, L_ECHO);
            right = GetDistance(R_TRIG, R_ECHO);
        
            Serial.print(f_center);Serial.print("\t");
            Serial.print(f_left);Serial.print("\t");
            Serial.print(f_right);Serial.print("\t");
            Serial.print(left);Serial.print("\t");
            Serial.print(right);Serial.print("\t");
            Serial.print(compute_steering);Serial.print("\t");
            Serial.print(compute_speed);Serial.println();
            debug_time = millis(); 
        }
    #else
        // ����
        SetSteering(compute_steering);
        SetSpeed(compute_speed);
    #endif
    

    

//    #ifdef DEBUG
//        // �ѹ��� üũ        
//        if(millis() - debug_time < 500)
//            return;
//            
//        Serial.print(f_center);Serial.print("\t");
//        Serial.print(f_left);Serial.print("\t");
//        Serial.print(f_right);Serial.print("\t");
//        Serial.print(left);Serial.print("\t");
//        Serial.print(right);Serial.print("\t");
//        Serial.print(compute_steering);Serial.print("\t");
//        Serial.print(compute_speed);Serial.println();
//        //Serial.println("-----------------------------");   
//        debug_time = millis(); 
//    #endif
}

void setup()
{
    max_pwm = max_rc_pwm;
    min_pwm = min_rc_pwm;
    
    servo.attach(SERVO);    
    pinMode(M_PWM, OUTPUT);
    pinMode(M_DIR1, OUTPUT);
    pinMode(M_DIR2, OUTPUT);

    pinMode(M2_PWM, OUTPUT);
    pinMode(M2_DIR1, OUTPUT);
    pinMode(M2_DIR2, OUTPUT);
    
    pinMode(LIGHT, OUTPUT);
    pinMode(LED, OUTPUT);
    pinMode(FC_TRIG, OUTPUT);
    pinMode(FC_ECHO, INPUT);
    pinMode(FL_TRIG, OUTPUT);
    pinMode(FL_ECHO, INPUT);
    pinMode(FR_TRIG, OUTPUT);
    pinMode(FR_ECHO, INPUT);
    //pinMode(BC_TRIG, OUTPUT);
    //pinMode(BC_ECHO, INPUT);
    pinMode(L_TRIG, OUTPUT);
    pinMode(L_ECHO, INPUT);
    pinMode(R_TRIG, OUTPUT);
    pinMode(R_ECHO, INPUT);
    
    digitalWrite(LIGHT, LOW);
    SetSteering(0);
    SetSpeed(0);
    
    //Serial.begin(9600);
    Serial.begin(9600);
    
    battery_time = millis();
    debug_time = millis(); 

    #ifdef DEBUG        
        Serial.print("FC");Serial.print("\t");
        Serial.print("FL");Serial.print("\t");
        Serial.print("FR");Serial.print("\t");
        Serial.print("L");Serial.print("\t");
        Serial.print("R");Serial.print("\t");
        Serial.print("STEERING");Serial.print("\t");
        Serial.print("SPEED");Serial.println();
        //Serial.println("-----------------------------");
    #endif
}

void loop()
{    
    CheckBattery();

    /*
        ���ŵ����� ����
        T:1
        P:1
        L:1
        S:1
        A:1
     */
    if(Serial.available() > 0)
    {
        String packet = Serial.readStringUntil('\n');
        if(packet != 0)
        {
            int index = packet.indexOf(':');
            if(index >= 0)
            {
                String cmd = packet.substring(0, index);
                String param = packet.substring(index + 1);
                if(cmd.equals("T") && !autoDriving)         // ������
                {
                    SetSteering(param.toFloat());
                    rc_time = millis();
                }
                else if(cmd.equals("P") && !autoDriving)    // �ӵ����
                {
                    SetSpeed(param.toFloat());
                    rc_time = millis();
                }
                else if(cmd.equals("L") && !autoDriving)    // ����Ʈ���
                {
                    if(param.toInt() == 1)
                        LightON();
                    else
                        LightOFF();
                }
                else if(cmd.equals("S") && !autoDriving)    // ������
                {
                    if(param.toInt() == 1)
                        StartMelody();
                    else
                        StopMelody();
                }
                else if(cmd.equals("A"))                    // ����������
                {
                    if(param.toInt() == 1)
                        StartAutoDriving();
                    else
                        StopAutoDriving();

                    #ifdef DEBUG
                        if(param.toInt() == 1)
                            debug_driving_status = true;
                        else
                            debug_driving_status = false;
                    #endif
                }
            }

            #ifdef DEBUG
                String cmd = packet;
                if(cmd.equals(" "))         // ����� ���� ����, ���� ��� ���
                {
                    if (debug_driving_status) 
                        debug_driving_status = false;
                    else
                        debug_driving_status = true;
                }
            #endif
            

        }
    }

    PlayMelody();
    AutoDriving();

    // ���� ��Ʈ�ѽ� 3�� ���� ������� ��ȣ ������ ����
    if(!autoDriving)
    {
        if(millis() - rc_time > 3000)
            SetSpeed(0);
    }
}
