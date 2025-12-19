//
// light.h
//
//   (see light.cpp for more information)
//

#ifndef LIGHT_H
#define LIGHT_H


class Light {

public:
  Light(int,int);
  void loop(void);
  void control(int);
  int status;		// go ahead an look at status when needed

     
private:
  void relayControl(int,int);

  int myPin;
  int myAddress;
};

#endif
