项目名称：logger

功能特性: I'm pretty lazy, so I'll leave the project exploration to you

如何运行：cmake

使用示例:

###
#include <logger.h>

int main() {
  LOG_INIT_CONSOLE();  // console/color

  LOG(Debug) << "Debug";
  LOG(Info) << "Info";
  LOG(Warning) << "Warning";
  LOG(Error) << "Error";
  
  LOG_SHUTDOWN();
}
###
