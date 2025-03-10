#ifndef FUNCTIONS_H
#define FUNCTIONS_H
#include <Arduino.h>

class Functions {
public:
    Functions(bool ready);
    bool Functions_Request(String rxString);
    bool Functions_Run();
    void a0();
    void a1(int repeticiones, int tiempo);
    void a2();
    void a3();
    void a4();
    void a5();
    void a6();
    void a7();
    void a8();
    void a9();
    void b0();
    void b1();
    void b2();
    void b3();
    void b4();
    void b5();
    void b6();
    void b7();
    void b8();
    void b9();
    void m0();
    void m1();
    void m2();
    void m3();
    void m4();
    void m5();
    void m6();
    void m7();
    void m8();
    void m9();
    void n0();
    void n1();
    void n2();
    void n3();
    void n4();
    void n5();
    void n6();
    void n7();
    void n8();
    void n9();
    void c0();
    void c1();
    void c2();
    void c3();
    void c4();
    void c5();
    void c6();
    void c7();
    void c8();
    void c9();
    void s0();
    void s1();
    void s2();
    void s3();
    void s4();
    void s5();
    void s6();
    void s7();
    void s8();
    void s9();
private:
    bool    firstScan;
    char    _function_Mode;
    char    _function_Number;
    String  _function_Parameter1;
    String  _function_Parameter2;
    String  _function_Parameter3;
    String  _function_Parameter4;
    int     _x1;
    int     _x2;
    int     _x3;
    int     _x4;
    byte    _LED_Azul=35;  

};

#endif // FUNCTIONS_H