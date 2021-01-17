#include <ros/ros.h> 
#include <serial/serial.h>  //ROS已经内置了的串口�? 
#include <std_msgs/String.h> 
#include <std_msgs/Empty.h> 
#include <math.h>
#include <sensor_msgs/Joy.h>

serial::Serial ser; //声明串口对象
sensor_msgs::Joy jy;

/* serial */
std::string param_port_path_;
int param_baudrate_;
int param_loop_rate_;
serial::parity_t param_patity_;

int j = 0;
uint8_t sum = 0;
ros::Publisher pub;


int main (int argc, char** argv) 
{ 
    //初�?�化节点 
    ros::init(argc, argv, "nunchunk"); 
    //声明节点句柄 
    ros::NodeHandle nh; 

    uint8_t rec[2000] = {'\0'}; 

    //发布主�?? 
    pub = nh.advertise<sensor_msgs::Joy>("nunchunk", 10);

	nh.param<std::string>("nunchunk/port", param_port_path_, "/dev/ttyUSB1");
	nh.param<int>("nunchunk/baudrate", param_baudrate_, 9600);
	nh.param<int>("nunchunk/loop_rate", param_loop_rate_, 20);

    try 
    { 
    //设置串口属性，并打开串口 
        ser.setPort(param_port_path_); 
        ser.setBaudrate(param_baudrate_); 
        serial::Timeout to = serial::Timeout::simpleTimeout(1000); 
        ser.setTimeout(to); 
        ser.open(); 
    } 
    catch (serial::IOException& e) 
    { 
        ROS_ERROR_STREAM("Unable to open port "); 
        return -1; 
    } 

    //检测串口是否已经打开，并给出提示信息 
    if(ser.isOpen()) 
    { 
        ROS_INFO_STREAM("Serial Port"<<param_port_path_<<" initialized"); 
    } 
    else 
    { 
        return -1; 
    } 

    //指定率 
    ros::Rate loop_rate(param_loop_rate_); 

    uint8_t axisX = 0;
    uint8_t axisY = 0;
    uint8_t button1 = 0;
    uint8_t button2 = 0;
    uint16_t accX = 0;
    uint16_t accY = 0;
    uint16_t accZ = 0;
    double roll = 0;
    double pitch = 0;
    while(ros::ok()) 
    { 
        // while(ser.available() < 10);

            //ROS_INFO("num:%d \n",ser.available());
            
        ser.read(rec,1);
        if (rec[0] == 0xaa)
        {
            ser.read(rec+1,11);
            
            for(j = 0; j<11 ; ++j){
                sum += rec[j];
            }
            //ROS_INFO("1:%d 2:%d 3:%d 4:%d \n",rec_right[0],rec_right[1],rec_right[2],rec_right[3]);
            //ROS_INFO("sum=%x;rec[9]=%x\n",sum,rec[9]);
            if (rec[11] == sum ){

                jy.header.stamp = ros::Time().now();
                jy.axes.clear();
                jy.buttons.clear();

                axisX = rec[1];
                axisY = rec[2];

                button1 = rec[3];
                button2 = rec[4];

                accX = rec[5] << 8 | rec[6];
                accY = rec[7] << 8 | rec[8];
                accZ = rec[9] << 8 | rec[10];

                roll = atan2(double(accX - 511), double(accZ - 511)) * 180 / M_PI;
                pitch = atan2(double(accY - 511), double(accZ - 511)) * 180 / M_PI;

                jy.axes.push_back(double(axisX));
                jy.axes.push_back(double(axisY));

                jy.axes.push_back(double(roll));
                jy.axes.push_back(double(pitch));

                jy.buttons.push_back(button1);
                jy.buttons.push_back(button2);

                pub.publish(jy);

                ROS_INFO("%d,%d,%d,%d,%d,%d,%d,%f,%f\n", axisX, axisY, button1, button2, accX, accY, accZ, roll, pitch);
           
            }
            sum = 0;
            ser.flushInput(); 
        }
        else{
            continue;
        }        
        //处理ROS的信�?，比如�?�阅消息,并调用回调函�? 
        ros::spinOnce(); 
        loop_rate.sleep(); 
    } //
    ser.close();
}
