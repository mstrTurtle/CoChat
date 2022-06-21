#include <memory>  // for allocator, __shared_ptr_access
#include <string>  // for char_traits, operator+, string, basic_string
#include <semaphore>
#include <string.h>
 
#include "ftxui/component/captured_mouse.hpp"  // for ftxui
#include "ftxui/component/component.hpp"       // for Input, Renderer, Vertical
#include "ftxui/component/component_base.hpp"  // for ComponentBase
#include "ftxui/component/component_options.hpp"  // for InputOption
#include "ftxui/component/screen_interactive.hpp"  // for Component, ScreenInteractive
#include "ftxui/dom/elements.hpp"  // for text, hbox, separator, Element, operator|, vbox, border
#include "ftxui/util/ref.hpp"  // for Ref
#include "ftxui/component/event.hpp"           // for Event

#include <deque>
#include <semaphore> // 本来是用semaphore，但C++11支持condition_variable。其实也挺好用的
#include <condition_variable>
extern char sendline[4096];
//extern std::counting_semaphore<10086> sem_getline{0};

extern std::condition_variable cv;
extern bool ready; // 这是cv用的，请勿乱改。sendline那里的。
extern std::mutex m; // cv专用的

std::mutex m_msg; // 访问msgToShow用的

std::vector<std::string> msgToShow;

void appendMsg(std::string msg){
  std::lock_guard lg(m_msg);
  msgToShow.push_back(msg);
}

int guimain() {
  using namespace ftxui;

  msgToShow.push_back("==WELCOME==");
 
  std::string lineToSendStr;
 
  
  Component lineToSendBox = Input(&lineToSendStr, "Type msg here to send");
 
  auto component = Container::Vertical({
      lineToSendBox
  });

  std::vector<std::string> msgToShowCopy;
  MenuOption option;
  int selected; // focus last entry
  auto menu = Menu(&msgToShowCopy, &selected, &option);
 
  auto renderer = Renderer(component, [&] {
    {
      std::lock_guard lg(m_msg);
      msgToShowCopy = msgToShow;
    }
    selected = msgToShowCopy.size()-1;
    return vbox({
               menu->Render() | vscroll_indicator | yframe | flex,
               separator(),
               hbox(text("send> "), lineToSendBox->Render()),
           }) |
           border;
  });

  renderer |= CatchEvent([&](Event event) {
    //msgToShow.push_back("inCE "+ lineToSendStr);
    if(event==Event::Return){
      //msgToShow.push_back("inReturn "+ lineToSendStr);
      lineToSendStr.push_back('\n'); // 需要一个结束符，给服务器做辨别。
      strncpy(sendline, lineToSendStr.data(), 4096);
      lineToSendStr.clear();
      //msgToShow.push_back("[YOU SEND] "+ lineToSendStr);
      //lineToSendStr.clear();
      {
        std::lock_guard lg(m);
        ready=true;
      }
      cv.notify_one();
      return true;
    }
    else
      return false;
  });
 
  auto screen = ScreenInteractive::Fullscreen();
  screen.Loop(renderer);
  return 0;
}
 
// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

extern int tmain(int argc, char** argv);

int main(int argc, char* argv[]){
  std::thread th_gui{guimain};
  std::thread th_bg{tmain,argc,argv};
  th_gui.join();
  th_bg.join();
  return 0;
}