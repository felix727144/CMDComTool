#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <string>
#include <list>
#include <chrono>
#include <thread>
#include <signal.h>
#include "serial/serial.h"

#include "ftxui/component/captured_mouse.hpp"      // for ftxui
#include "ftxui/component/component.hpp"           // for Input, Renderer, Vertical
#include "ftxui/component/component_base.hpp"      // for ComponentBase
#include "ftxui/component/component_options.hpp"   // for ButtonOption
#include "ftxui/component/screen_interactive.hpp"  // for Component, ScreenInteractive
#include "ftxui/component/loop.hpp"                // for Loop
#include "ftxui/component/event.hpp"               // for Event, Event::Custom
#include "ftxui/dom/elements.hpp"                  // for operator|, Element, size, border, frame, vscroll_indicator, HEIGHT, LESS_THAN
#include "ftxui/screen/screen.hpp"
#include "ftxui/screen/string.hpp"
#include "scroller.hpp"

using namespace ftxui;

class CSerialSetting {
public:
    std::vector<std::string> baudrates() {
        return {
            "4800   bps",
            "9600   bps",
            "19200  bps",
            "38400  bps",
            "43000  bps",
            "56000  bps",
            "57600  bps",
            "115200 bps"
        };
    }
    int baudrate_str2int(std::string b) { std::string t; for(auto& item : b) { if (' ' == item) { break; } t.push_back(item); } return std::stoi(t); }
};

class CSerialOper {
private:
    int                           m_device_select;
    int                           m_baudrate_select;    
    std::vector<serial::PortInfo> m_devices;
public:
    CSerialOper() { m_devices = serial::list_ports(); }
    std::vector<std::string> summary_devices() {        
        std::vector<std::string> ret;
        for (int index = 0; index < m_devices.size(); ++index) {
            auto& p = m_devices[index];
            char sz[128] = {0};
            snprintf(sz, sizeof(sz), "[%d] Disp:[%s] hid:[%s] port:[%s]", index, p.description.c_str(), p.hardware_id.c_str(), p.port.c_str());
            ret.push_back(sz);
        }
        return ret;
    }
    serial::PortInfo device_info(int device_index) { return m_devices[device_index]; }
};

class CSerialInstance {
private:
    std::atomic<bool>                m_exit;
    ScreenInteractive *              m_pscreen;
public:
    std::shared_ptr<serial::Serial>  m_instance;
    std::shared_ptr<std::thread>     m_work_thread;
    std::vector<std::string>         m_receive_datas;
    std::vector<std::string>         m_wait_send_datas;
public:    
    CSerialInstance() : m_exit(false) {}
    bool open_serial_device(serial::PortInfo port_info, int baudrate, ScreenInteractive * ps) {
        m_pscreen = ps;
        m_instance.reset(new serial::Serial(port_info.port, baudrate, serial::Timeout::simpleTimeout(50)));
        if (false == m_instance->isOpen()) { return false; }
        m_work_thread.reset(new std::thread(&CSerialInstance::thread_impl_receive, this));
        return true;
    }
    void stop_all() { m_exit = true; m_work_thread->join(); }
    void thread_impl_receive() {
        while (true) {
            if (m_exit) { break; }

            {
                auto& wait_send = m_wait_send_datas;
                if (wait_send.size() > 0) { m_instance->write(wait_send.back()); wait_send.clear(); }
            }

            int avail_size = m_instance->available();
            if (0 == avail_size) { std::this_thread::sleep_for(std::chrono::milliseconds(50)); continue; }

            {
                m_receive_datas.push_back(m_instance->read(avail_size));
                m_pscreen->PostEvent(Event::Custom);
            }            
        }
    }
};

int main(int argc, char* argv[]) {
    CSerialSetting  setting;
    CSerialOper     oper;
    CSerialInstance serial;

    auto devices = oper.summary_devices();
    if (devices.empty()) { printf("Current no serial device exist"); return 0; }

    serial::PortInfo device_info;
    {
        int select = 0;
        auto screen = ScreenInteractive::FitComponent();
        MenuOption option;
        option.on_enter = [&](){
            device_info = oper.device_info(select);
            screen.ExitLoopClosure()();
        };
        auto menu = Menu(&devices, &select, &option);   
        screen.Loop(menu);
    }
    
    int device_baudrate = -1;
    {
        auto bs     = setting.baudrates();
        int  select = bs.size() - 1;
        auto screen = ScreenInteractive::FitComponent();
        auto clicked = [&](){
            device_baudrate = setting.baudrate_str2int(bs[select]); 
            screen.ExitLoopClosure()();
        };
        auto layout = Container::Horizontal({
            Button("Enter", clicked, ButtonOption::Animated(Color::Green)),
            Dropdown(&bs, &select)
        });
        screen.Loop(layout);
    }    

    {
        auto screen = ScreenInteractive::FitComponent();
        serial.open_serial_device(device_info, device_baudrate, &screen);

        std::string input_text;
        InputOption option;
        option.on_enter = [&] {
            serial.m_wait_send_datas.push_back(input_text);
            input_text = "";
        };
        auto cp_input  = Input(&input_text, "command", option);
        auto cp_output = Renderer([&](){
            Elements elems;
            for (auto& item : serial.m_receive_datas) { elems.push_back(text(item)); }
            return vbox(window(text("output"), vbox(elems)));
        });
        auto cp_scroll = Scroller(cp_output);
        auto cp_button = Button("Exit", screen.ExitLoopClosure());
        auto cp = Renderer([&]() {
            return vbox(
                cp_scroll->Render(),
                hbox(
                    window(text("input"), cp_input->Render()),
                    cp_button->Render()
                )
            );
        });        
        cp->Add(cp_scroll);
        cp->Add(cp_input);
        cp->Add(cp_button);
        
        screen.Loop(cp);
    }

    serial.stop_all();
    return 0;
}