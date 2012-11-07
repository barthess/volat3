#ifndef PROFILER_H
#define PROFILER_H

U32 DiffTime(U32 uiOldTime);
const U32 SIZE = 100;

class C_Profiler 
{  
private:
  U32 uiStart;
  U32 uiInterruptList;
  U32 puiList[SIZE];
  U16 usCount;
  C_Profiler(const C_Profiler&);
  C_Profiler& operator=(const C_Profiler&);
public:
  C_Profiler(void) : uiStart(0), uiInterruptList(0), usCount(0) {};
  void Init(U32 uiInterrupt){uiInterruptList = uiInterrupt; uiStart = DiffTime(0);}
  void SaveTime(U16 ucID);
};
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#endif //#ifndef PROFILER_H

