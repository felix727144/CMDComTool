#include <iostream>
#include <serial/serial.h>
#include <unistd.h>
#include <cstdlib>
#include <string>
#include <chrono>
#include <thread>
#include <termio.h>
using namespace std::literals;
using duration_sec = std::chrono::duration<double>;
using sys_time_point = std::chrono::system_clock::time_point;
std::chrono::system_clock::time_point(*system_now)() = std::chrono::system_clock::now;
int main(int argc, char* argv[]) {
    std::string line;
    auto ports=serial::list_ports();
    int portid=-1;
    if(argc!=2 || (argc>2 && 0==strcmp(argv[1],"-h"))){
        portid=0;
	std::cout<<"Usage:CMDComTool <portid>"<<std::endl;
        for(auto p: ports){
            std::cout<<"["<<portid<<"] Disp:["<<p.description<<"] hid:["<<p.hardware_id<<"] port:["<<p.port<<"]"<<std::endl;
            portid++;
        }
        return -1;
    }
    serial::Serial input("/proc/self/fd/0");
    portid=atoi(argv[1]);
    if(portid<0 || portid>=ports.size()){
        std::cerr<<"port id out of range"<<std::endl;
        return -1;
    }
    try{
        serial::Serial s(ports[portid].port,115200,serial::Timeout::simpleTimeout(50));
        sys_time_point last_got_data;
        size_t last_data_size=0;
        std::string line;
        std::string input_line;
        bool loop_flag=1;
        
        while(loop_flag){
            if(last_data_size && (system_now()-last_got_data>100ms)){
                line=s.read(s.available());
                last_data_size=0;
            }else if((s.available()>last_data_size && (system_now()-last_got_data<100ms)) || (s.available()==0)){
                last_got_data=system_now();
                last_data_size=s.available();
            }
            if(input.available()){
                std::string str=input.read(input.available());
                if(str[0]==3){
                    loop_flag=0;
                }
                
                std::cout<<str;
                if(str.find('\r')!=-1){
                    size_t writed=s.write(input_line);
                    //std::cout<<"your command:["<<input_line<<"] writed "<<writed<<" bytes..\r\n>";
		    std::cout<<"\r\n>";
                    input_line.clear();
                }else{
                    input_line+=str;
                }
                std::flush(std::cout);
            }
            if(line.size()){
                std::cout<<"\r                                                  \r";
                std::cout<<"["<<clock()<<"]"<<line<<"\r\n";
                line.clear();
                std::cout<<">";
                if(input_line.size()){
                    std::cout<<input_line;
                }
                std::flush(std::cout);
            }
            std::this_thread::sleep_for(10ms);
        }
    }catch(std::exception &e){
        std::cerr<<"got error "<<e.what()<<std::endl;
        //return -1;
    }
    input.close();
    struct termios options; // The options for the file descriptor
    
    if (tcgetattr(fileno(stdin), &options) == 0) {
        
        options.c_cflag &= (tcflag_t)  ~(CLOCAL | CREAD);
        options.c_lflag |= (tcflag_t) (ICANON | ECHO | ECHOE | ECHOK | ECHONL |
                                            ISIG | IEXTEN); 
        options.c_iflag |= (tcflag_t) (INLCR|ICRNL | IGNBRK);
        options.c_oflag |= (tcflag_t) (OPOST);

        tcsetattr(fileno(stdin),TCSANOW,&options);
    }

   
    std::cout<<"exit!"<<std::endl;
    return 0;
}
